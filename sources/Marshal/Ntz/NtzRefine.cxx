// mmap zero-copy input, OpenMP batched dispatch, async writer, mpmath Newton via rs_batch.py.
#include "weil_mmap.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using weil_mmap::MappedFile;
using weil_mmap::parse_line_view;
using weil_mmap::scan_line_offsets;

struct Cli {
    std::string root = ".";
    std::string input = "odlyzko_zeros2m.txt";
    std::string output = "traces/zeros_refined.txt";
    std::string report = "traces/zeros_refine_report.json";
    std::string python = "python";
    std::string batch_script = "scripts/rs_batch.py";
    size_t max_zeros = 0;
    size_t offset = 0;
    int batch_size = 64;
    int threads = 0;
};

static bool parse_args(int argc, char** argv, Cli& c) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--root" && i + 1 < argc) c.root = argv[++i];
        else if (a == "--input" && i + 1 < argc) c.input = argv[++i];
        else if (a == "--output" && i + 1 < argc) c.output = argv[++i];
        else if (a == "--report" && i + 1 < argc) c.report = argv[++i];
        else if (a == "--python" && i + 1 < argc) c.python = argv[++i];
        else if (a == "--batch-script" && i + 1 < argc) c.batch_script = argv[++i];
        else if (a == "--max-zeros" && i + 1 < argc)
            c.max_zeros = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        else if (a == "--offset" && i + 1 < argc)
            c.offset = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        else if (a == "--batch" && i + 1 < argc) c.batch_size = std::atoi(argv[++i]);
        else if (a == "--threads" && i + 1 < argc) c.threads = std::atoi(argv[++i]);
        else if (a == "-h" || a == "--help") {
            std::cerr << "Usage: weil_zeros_refine [--input PATH] [--output PATH] [--max-zeros N]\n";
            return false;
        }
    }
    return true;
}

struct OutBatch {
    std::vector<double> gammas;
    std::vector<double> z_before;
    std::vector<double> z_after;
};

class AsyncWriter {
public:
    explicit AsyncWriter(const std::string& path) : path_(path) {
        worker_ = std::thread([this] { run(); });
    }
    ~AsyncWriter() {
        {
            std::lock_guard<std::mutex> lk(mu_);
            done_ = true;
        }
        cv_.notify_one();
        if (worker_.joinable()) worker_.join();
    }

    void enqueue(OutBatch&& b) {
        std::lock_guard<std::mutex> lk(mu_);
        q_.push(std::move(b));
        cv_.notify_one();
    }

private:
    void run() {
        std::ofstream out(path_, std::ios::binary);
        for (;;) {
            OutBatch b;
            {
                std::unique_lock<std::mutex> lk(mu_);
                cv_.wait(lk, [&] { return done_ || !q_.empty(); });
                if (q_.empty() && done_) break;
                b = std::move(q_.front());
                q_.pop();
            }
            for (double g : b.gammas) {
                char buf[64];
                const int n = std::snprintf(buf, sizeof(buf), "%.21f\n", g);
                out.write(buf, n);
            }
        }
    }

    std::string path_;
    std::mutex mu_;
    std::condition_variable cv_;
    std::queue<OutBatch> q_;
    bool done_ = false;
    std::thread worker_;
};

static double gamma_at(const MappedFile& mf, const std::vector<size_t>& starts, size_t idx) {
    const size_t b0 = starts[idx];
    const size_t b1 = (idx + 1 < starts.size()) ? starts[idx + 1] : mf.size;
    double v = 0;
    if (!parse_line_view(mf.data + b0, mf.data + b1, v)) return 0;
    return v;
}

static std::string temp_path(int bi, const char* suffix) {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = std::getenv("TMP");
    if (!tmp) tmp = ".";
    return std::string(tmp) + "\\weil_rs_" + std::to_string(bi) + suffix;
}

static bool refine_batch(const Cli& c, int bi, const std::vector<double>& in, OutBatch& out) {
    const std::string tmp_in = temp_path(bi, "_in.txt");
    const std::string tmp_out = temp_path(bi, "_out.txt");
    {
        std::ofstream tin(tmp_in, std::ios::binary);
        for (double g : in) tin << g << "\n";
    }
    std::ostringstream cmd;
    cmd << "cmd /c \"cd /d \"" << c.root << "\" && \"" << c.python << "\" \""
        << c.batch_script << "\" < \"" << tmp_in << "\" > \"" << tmp_out << "\"\"";
    if (std::system(cmd.str().c_str()) != 0) return false;

    std::ifstream tout(tmp_out);
    out.gammas.clear();
    out.z_before.clear();
    out.z_after.clear();
    double g = 0, z0 = 0, z1 = 0;
    while (tout >> g >> z0 >> z1) {
        out.gammas.push_back(g);
        out.z_before.push_back(z0);
        out.z_after.push_back(z1);
    }
    std::remove(tmp_in.c_str());
    std::remove(tmp_out.c_str());
    return out.gammas.size() == in.size();
}

int main(int argc, char** argv) {
    Cli c;
    if (!parse_args(argc, argv, c)) return 1;

#ifdef _OPENMP
    if (c.threads > 0) omp_set_num_threads(c.threads);
#endif

#ifdef _WIN32
    std::string chdir_cmd = "cd /d \"" + c.root + "\"";
    std::system(chdir_cmd.c_str());
#endif

    MappedFile mf;
    if (!mf.map(c.input)) {
        std::cerr << "Cannot mmap: " << c.input << "\n";
        return 1;
    }
    const auto starts = scan_line_offsets(mf);
    size_t n_use = starts.size() > c.offset ? starts.size() - c.offset : 0;
    if (c.max_zeros > 0) n_use = std::min(n_use, c.max_zeros);

    std::cerr << "mmap refine=" << n_use << " batch=" << c.batch_size << "\n";

    const auto t0 = std::chrono::steady_clock::now();
    AsyncWriter writer(c.output);

    std::atomic<double> max_abs_z{0};
    std::atomic<double> max_shift{0};
    std::atomic<size_t> converged{0};

    const int n_batches = static_cast<int>((n_use + static_cast<size_t>(c.batch_size) - 1) /
                                           static_cast<size_t>(c.batch_size));

    #ifdef _OPENMP
    #pragma omp parallel for schedule(dynamic)
    #endif
    for (int bi = 0; bi < n_batches; ++bi) {
        const size_t i0 = static_cast<size_t>(bi) * static_cast<size_t>(c.batch_size);
        const size_t i1 = std::min(i0 + static_cast<size_t>(c.batch_size), n_use);
        std::vector<double> coarse;
        coarse.reserve(i1 - i0);
        for (size_t i = i0; i < i1; ++i)
            coarse.push_back(gamma_at(mf, starts, c.offset + i));

        OutBatch ob;
        if (!refine_batch(c, bi, coarse, ob)) continue;
        for (size_t k = 0; k < ob.gammas.size(); ++k) {
            const double za = k < ob.z_after.size() ? ob.z_after[k] : 0;
            max_abs_z.store(std::max(max_abs_z.load(), std::fabs(za)));
            max_shift.store(std::max(max_shift.load(), std::fabs(ob.gammas[k] - coarse[k])));
            if (std::fabs(za) < 1e-12) converged.fetch_add(1);
        }
        writer.enqueue(std::move(ob));
    }

    const double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    std::ostringstream rep;
    rep << "{\n";
    rep << "  \"engine\": \"cpp_mmap_python_mpmath\",\n";
    rep << "  \"n_refined\": " << n_use << ",\n";
    rep << "  \"batch_size\": " << c.batch_size << ",\n";
    rep << "  \"wall_s\": " << wall << ",\n";
    rep << "  \"max_abs_Z_after\": " << max_abs_z.load() << ",\n";
    rep << "  \"max_gamma_shift\": " << max_shift.load() << ",\n";
    rep << "  \"converged_count\": " << converged.load() << ",\n";
    rep << "  \"throughput_per_s\": " << (n_use / std::max(wall, 1e-9)) << "\n";
    rep << "}\n";

    std::ofstream rf(c.report);
    rf << rep.str();
    std::cerr << "max|Z|=" << max_abs_z.load() << " wall=" << wall << "s\n";
    std::cout << rep.str();
    return max_abs_z.load() < 1e-10 ? 0 : 1;
}

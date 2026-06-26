#include "NtzGenerate.hxx"

#include "IO/MappedView.hxx"
#include "IO/ZeroLoader.hxx"
#include "Ntz/RiemannSiegel.hxx"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Ntz {
namespace {

namespace fs = std::filesystem;
using weil_rs::RSTable;
using weil_rs::kPi;
using weil_rs::refine_zero;

double gamma_asymptotic(size_t n1) {
    const double n = static_cast<double>(n1);
    if (n < 2.0) return 14.134725142;
    const double L = std::log(n);
    const double W = L - std::log(std::max(L, 1.0));
    return 2.0 * kPi * n / std::max(W, 0.5);
}

double coarse_at(const Marshal::IO::MappedView& mv, const std::vector<size_t>& starts, size_t idx) {
    if (idx + 1 >= starts.size()) return 0;
    double v = 0;
    const char* b = mv.data + starts[idx];
    const char* e = mv.data + starts[idx + 1];
    if (!Marshal::IO::ParseLineView(b, e, v)) return 0;
    return v;
}

struct OutBatch {
    int batch_id = 0;
    std::vector<double> gammas;
    std::vector<double> z_after;
    std::vector<double> shifts;
};

class ShardWriter {
public:
    explicit ShardWriter(const std::string& dir) : dir_(dir) {
        fs::create_directories(dir_);
    }

    std::string WriteShard(int batch_id, const std::vector<double>& gammas) {
        const std::string path = dir_ + "/ntz_shard_" + std::to_string(batch_id) + ".bin";
        std::ofstream out(path, std::ios::binary);
        const uint64_t n = gammas.size();
        out.write(reinterpret_cast<const char*>(&n), sizeof(n));
        out.write(reinterpret_cast<const char*>(gammas.data()),
                  static_cast<std::streamsize>(n * sizeof(double)));
        shard_paths_.push_back(path);
        return path;
    }

    const std::vector<std::string>& paths() const { return shard_paths_; }

private:
    std::string dir_;
    std::vector<std::string> shard_paths_;
};

class AsyncTextWriter {
public:
    explicit AsyncTextWriter(const std::string& path) : path_(path) {
        worker_ = std::thread([this] { run(); });
    }
    ~AsyncTextWriter() {
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
                const int n = std::snprintf(buf, sizeof(buf), "%.17g\n", g);
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

bool append_coarse_tail(const Marshal::IO::MappedView& mv, const std::vector<size_t>& starts,
                        size_t from_idx, size_t want_total, std::vector<double>& merged) {
    while (merged.size() < want_total && from_idx + 1 < starts.size()) {
        const double v = coarse_at(mv, starts, from_idx);
        if (v > 0) merged.push_back(v);
        ++from_idx;
    }
    return merged.size() >= want_total || from_idx + 1 >= starts.size();
}

void RepairRefinedSequence(const NtzGenerateOptions& opt, std::vector<double>& refined,
                           const Marshal::IO::MappedView& mv, const std::vector<size_t>& starts,
                           size_t input_lines, size_t offset) {
    if (refined.empty()) return;
    RSTable tab;
    for (size_t i = 0; i < refined.size(); ++i) {
        const size_t file_idx = offset + i;
        double lo = i > 0 ? refined[i - 1] + 1e-9 : refined[i] - 1.0;
        double hi = refined[i] + 2.0;
        if (i + 1 < refined.size()) hi = std::max(hi, refined[i + 1]);
        if (file_idx + 1 < input_lines) hi = std::max(hi, coarse_at(mv, starts, file_idx + 1));
        if (!(lo < hi)) {
            lo = refined[i] - 0.5;
            hi = refined[i] + 0.5;
        }
        auto rr = refine_zero(tab, refined[i], opt.max_iter * 2, opt.tol, lo, hi);
        refined[i] = rr.gamma_out;
        if (std::fabs(rr.z_after) >= opt.tol && i > 0 && i + 1 < refined.size()) {
            const double mid = 0.5 * (refined[i - 1] + refined[i + 1]);
            rr = refine_zero(tab, mid, opt.max_iter * 2, opt.tol, refined[i - 1], refined[i + 1]);
            refined[i] = rr.gamma_out;
        }
        if (std::fabs(hardy_Z(tab, refined[i])) >= opt.tol && i > 0 && i + 1 < refined.size()) {
            const double a = refined[i - 1];
            const double b = refined[i + 1];
            double blo = hardy_Z(tab, a);
            double bhi = hardy_Z(tab, b);
            if (blo * bhi <= 0.0) {
                double l = a;
                double h = b;
                for (int it = 0; it < 96; ++it) {
                    const double m = 0.5 * (l + h);
                    const double zm = hardy_Z(tab, m);
                    if (std::fabs(zm) < opt.tol) {
                        refined[i] = m;
                        break;
                    }
                    if (blo * zm <= 0.0) {
                        h = m;
                        bhi = zm;
                    } else {
                        l = m;
                        blo = zm;
                    }
                }
            }
        }
        if (i > 0 && refined[i] <= refined[i - 1]) {
            const double mid = 0.5 * (refined[i - 1] + (i + 1 < refined.size() ? refined[i + 1]
                                                                               : refined[i] + 1.0));
            rr = refine_zero(tab, mid, opt.max_iter * 2, opt.tol, refined[i - 1],
                            i + 1 < refined.size() ? refined[i + 1] : mid + 1.0);
            refined[i] = rr.gamma_out;
        }
    }
}

}  // namespace

bool RunNtzGenerate(const NtzGenerateOptions& opt, NtzGenerateReport& rep, std::string& err) {
#ifdef _OPENMP
    if (opt.threads > 0) omp_set_num_threads(opt.threads);
#endif

    Marshal::IO::MappedView mv;
    std::vector<size_t> starts;
    const bool have_input = mv.Map(opt.input);
    if (have_input) starts = Marshal::IO::ScanLineOffsets(mv);

    const size_t input_lines = have_input ? (starts.size() > 1 ? starts.size() - 1 : 0) : 0;
    const size_t avail_from_input =
        (opt.offset < input_lines) ? input_lines - opt.offset : 0;

    size_t refine_n = opt.count;
    if (refine_n == 0) refine_n = avail_from_input > 0 ? avail_from_input : 1000;
    const size_t merged_target =
        std::max(opt.pad_to, std::max(refine_n,
                  std::max(opt.offset + refine_n,
                           have_input ? opt.offset + input_lines : size_t{0})));
    if (opt.pad_to > refine_n) refine_n = opt.pad_to - opt.offset;

    if (opt.output.empty()) {
        err = "NTZ generate: --ntz-output required";
        return false;
    }

    const int batch_sz = std::max(1, opt.batch_size);
    const int n_batches = static_cast<int>((refine_n + static_cast<size_t>(batch_sz) - 1) /
                                           static_cast<size_t>(batch_sz));

    std::string shard_dir = opt.shard_dir;
    if (shard_dir.empty()) {
        shard_dir = (fs::temp_directory_path() / "marshal_ntz_shards").string();
    }
    fs::create_directories(shard_dir);

    const auto t0 = std::chrono::steady_clock::now();

    std::atomic<double> max_abs_z{0};
    std::atomic<double> max_shift{0};
    std::atomic<size_t> converged{0};

    std::vector<std::vector<double>> batch_results(static_cast<size_t>(n_batches));
    std::vector<std::vector<double>> batch_coarse(static_cast<size_t>(n_batches));

    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int bi = 0; bi < n_batches; ++bi) {
        const size_t i0 = static_cast<size_t>(bi) * static_cast<size_t>(batch_sz);
        const size_t i1 = std::min(i0 + static_cast<size_t>(batch_sz), refine_n);
        auto& coarse = batch_coarse[static_cast<size_t>(bi)];
        auto& refined = batch_results[static_cast<size_t>(bi)];
        coarse.reserve(i1 - i0);
        refined.reserve(i1 - i0);

        RSTable tab;
        for (size_t j = i0; j < i1; ++j) {
            const size_t file_idx = opt.offset + j;
            double g0 = 0;
            if (have_input && file_idx < input_lines)
                g0 = coarse_at(mv, starts, file_idx);
            else
                g0 = gamma_asymptotic(file_idx + 1);

            coarse.push_back(g0);
            const bool from_input = have_input && file_idx < input_lines;
            if (!opt.refine && from_input && file_idx < input_lines) {
                refined.push_back(g0);
                converged.fetch_add(1);
            } else {
                double lo = g0 - 1.0;
                double hi = g0 + 1.0;
                if (have_input && file_idx > 0)
                    lo = coarse_at(mv, starts, file_idx - 1);
                if (have_input && file_idx + 1 < input_lines)
                    hi = coarse_at(mv, starts, file_idx + 1);
                if (!(lo < hi)) {
                    lo = g0 - 0.5;
                    hi = g0 + 0.5;
                }
                const auto rr = refine_zero(tab, g0, opt.max_iter, opt.tol, lo, hi);
                refined.push_back(rr.gamma_out);
                max_abs_z.store(std::max(max_abs_z.load(), std::fabs(rr.z_after)));
                max_shift.store(std::max(max_shift.load(), std::fabs(rr.gamma_out - g0)));
                if (rr.converged) converged.fetch_add(1);
            }
        }
    }

    ShardWriter shards(shard_dir);
    std::unique_ptr<AsyncTextWriter> text_writer;
    if (opt.write_text) text_writer = std::make_unique<AsyncTextWriter>(opt.output);

    std::vector<double> refined_all;
    refined_all.reserve(refine_n);
    for (int bi = 0; bi < n_batches; ++bi) {
        const auto& refined = batch_results[static_cast<size_t>(bi)];
        const auto& coarse = batch_coarse[static_cast<size_t>(bi)];
        shards.WriteShard(bi, refined);
        refined_all.insert(refined_all.end(), refined.begin(), refined.end());
        if (text_writer) {
            OutBatch ob;
            ob.batch_id = bi;
            ob.gammas = refined;
            for (size_t k = 0; k < refined.size(); ++k) {
                ob.z_after.push_back(0);
                ob.shifts.push_back(std::fabs(refined[k] - coarse[k]));
            }
            text_writer->enqueue(std::move(ob));
        }
    }
    text_writer.reset();

    RepairRefinedSequence(opt, refined_all, mv, starts, input_lines, opt.offset);

    std::vector<double> merged = refined_all;
    if (merged_target > merged.size() && have_input) {
        append_coarse_tail(mv, starts, opt.offset + merged.size(), merged_target, merged);
    }

    std::string cache_path = opt.cache_path;
    if (cache_path.empty()) cache_path = opt.output + ".zerocache";

    if (!SaveZerosBinary(cache_path, merged)) {
        err = "NTZ generate: failed to write cache " + cache_path;
        return false;
    }

    if (opt.write_one_line) {
        const std::string one_line = fs::path(opt.output).parent_path().string() + "/NtzImOneLine.txt";
        std::ofstream ol(one_line, std::ios::binary);
        for (double g : refined_all) ol << std::setprecision(17) << g << "\n";
    }

    if (merged_target > refined_all.size()) {
        const std::string merged_path =
            fs::path(opt.output).parent_path().string() + "/NtzMergedOneLine.txt";
        std::ofstream ml(merged_path, std::ios::binary);
        for (double g : merged) ml << std::setprecision(17) << g << "\n";
    }

    if (opt.cleanup_shards) {
        for (const auto& p : shards.paths()) fs::remove(p);
    }

    const double wall =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    rep.engine = "marshal_native_riemann_siegel";
    rep.n_refined = refined_all.size();
    rep.merged_count = merged.size();
    rep.batch_size = batch_sz;
    rep.wall_s = wall;
    rep.max_abs_Z_after = max_abs_z.load();
    rep.max_gamma_shift = max_shift.load();
    rep.converged_count = converged.load();
    rep.throughput_per_s = rep.n_refined / std::max(wall, 1e-9);
    rep.gamma_1 = refined_all.empty() ? 0 : refined_all.front();
    rep.input = opt.input;
    rep.output = opt.output;
    rep.cache_path = cache_path;

    std::cout << "NTZ native: refined=" << rep.n_refined << " merged=" << rep.merged_count
              << " max|Z|=" << rep.max_abs_Z_after << " wall=" << rep.wall_s << "s"
              << " (" << rep.throughput_per_s << "/s)\n"
              << "  cache: " << cache_path << "\n";

    if (rep.max_abs_Z_after >= 1e-6)
        std::cerr << "NTZ warning: max|Z|=" << rep.max_abs_Z_after << " (check tolerance)\n";
    return true;
}

bool ExportNtzReportJson(const std::string& path, const NtzGenerateReport& rep) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"engine\": \"" << rep.engine << "\",\n";
    out << "  \"n_refined\": " << rep.n_refined << ",\n";
    out << "  \"merged_count\": " << rep.merged_count << ",\n";
    out << "  \"count\": " << rep.n_refined << ",\n";
    out << "  \"pad_to\": " << rep.merged_count << ",\n";
    out << "  \"batch_size\": " << rep.batch_size << ",\n";
    out << "  \"wall_s\": " << rep.wall_s << ",\n";
    out << "  \"max_abs_Z_after\": " << rep.max_abs_Z_after << ",\n";
    out << "  \"max_gamma_shift\": " << rep.max_gamma_shift << ",\n";
    out << "  \"converged_count\": " << rep.converged_count << ",\n";
    out << "  \"throughput_per_s\": " << rep.throughput_per_s << ",\n";
    out << "  \"gamma_1\": " << rep.gamma_1 << ",\n";
    out << "  \"input\": \"" << rep.input << "\",\n";
    out << "  \"output\": \"" << rep.output << "\",\n";
    out << "  \"cache_path\": \"" << rep.cache_path << "\"\n";
    out << "}\n";
    return true;
}

}  // namespace Marshal::Ntz

#include "ZeroIngest.hxx"

#include "MappedView.hxx"
#include "ZeroLoader.hxx"

#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::IO {
namespace {

namespace fs = std::filesystem;

inline bool parse_line_fast(const char* b, const char* e, double& out) {
    while (b < e && (*b == ' ' || *b == '\t' || *b == '\r' || *b == '\n')) ++b;
    if (b >= e) return false;
    char* end = nullptr;
    out = std::strtod(b, &end);
    return end != b;
}

class ShardWriter {
public:
    explicit ShardWriter(const std::string& dir) : dir_(dir) { fs::create_directories(dir_); }

    void WriteShard(int batch_id, const double* data, size_t n) {
        const std::string path = dir_ + "/ingest_shard_" + std::to_string(batch_id) + ".bin";
        std::ofstream out(path, std::ios::binary);
        const uint64_t cnt = static_cast<uint64_t>(n);
        out.write(reinterpret_cast<const char*>(&cnt), sizeof(cnt));
        out.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(n * sizeof(double)));
        shard_paths_.push_back(path);
    }

    const std::vector<std::string>& paths() const { return shard_paths_; }

private:
    std::string dir_;
    std::vector<std::string> shard_paths_;
};

}  // namespace

bool RunZeroIngest(const ZeroIngestOptions& opt, ZeroIngestReport& rep, std::string& err) {
#ifdef _OPENMP
    if (opt.threads > 0) omp_set_num_threads(opt.threads);
#endif

    if (opt.input.empty()) {
        err = "ZeroIngest: --zeros-input required";
        return false;
    }
    if (opt.cache_path.empty()) {
        err = "ZeroIngest: --zeros-cache required";
        return false;
    }

    MappedView mv;
    if (!mv.Map(opt.input)) {
        err = "ZeroIngest: cannot mmap " + opt.input;
        return false;
    }

    const auto starts = ScanLineOffsets(mv);
    const size_t n_lines = starts.size() > 1 ? starts.size() - 1 : 0;
    if (n_lines == 0) {
        err = "ZeroIngest: no lines in input";
        return false;
    }

    const size_t avail = opt.offset < n_lines ? n_lines - opt.offset : 0;
    size_t want = opt.count > 0 ? opt.count : avail;
    if (avail > 0) want = std::min(want, avail);
    if (want == 0) {
        err = "ZeroIngest: nothing to ingest";
        return false;
    }

    const int batch_sz = std::max(1024, opt.batch_size);
    const int n_batches =
        static_cast<int>((want + static_cast<size_t>(batch_sz) - 1) / static_cast<size_t>(batch_sz));

    std::string shard_dir = opt.shard_dir;
    if (shard_dir.empty())
        shard_dir = (fs::temp_directory_path() / "marshal_zero_ingest").string();

    const auto t0 = std::chrono::steady_clock::now();

    std::vector<std::vector<double>> batch_data(static_cast<size_t>(n_batches));

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (int bi = 0; bi < n_batches; ++bi) {
        const size_t i0 = static_cast<size_t>(bi) * static_cast<size_t>(batch_sz);
        const size_t i1 = std::min(i0 + static_cast<size_t>(batch_sz), want);
        auto& out = batch_data[static_cast<size_t>(bi)];
        out.reserve(i1 - i0);
        for (size_t j = i0; j < i1; ++j) {
            const size_t idx = opt.offset + j;
            double v = 0;
            const char* b = mv.data + starts[idx];
            const char* e = mv.data + starts[idx + 1];
            if (parse_line_fast(b, e, v)) out.push_back(v);
        }
    }

    ShardWriter shards(shard_dir);
    std::vector<double> merged;
    merged.reserve(want);
    for (int bi = 0; bi < n_batches; ++bi) {
        const auto& chunk = batch_data[static_cast<size_t>(bi)];
        shards.WriteShard(bi, chunk.data(), chunk.size());
        merged.insert(merged.end(), chunk.begin(), chunk.end());
    }

    fs::create_directories(fs::path(opt.cache_path).parent_path());
    if (!SaveZerosBinary(opt.cache_path, merged)) {
        err = "ZeroIngest: failed to write " + opt.cache_path;
        return false;
    }

    if (opt.cleanup_shards) {
        for (const auto& p : shards.paths()) fs::remove(p);
    }

    const double wall =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    rep.engine = "marshal_simd_mmap_ingest";
    rep.n_parsed = merged.size();
    rep.batch_size = batch_sz;
    rep.wall_s = wall;
    rep.throughput_per_s = rep.n_parsed / std::max(wall, 1e-9);
    rep.input = opt.input;
    rep.cache_path = opt.cache_path;

    std::cout << "ZeroIngest: parsed=" << rep.n_parsed << " wall=" << rep.wall_s << "s ("
              << rep.throughput_per_s << "/s)\n"
              << "  cache: " << opt.cache_path << "\n";
    return true;
}

}  // namespace Marshal::IO

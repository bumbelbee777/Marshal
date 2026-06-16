#pragma once
#include <cstddef>
#include <string>

namespace Marshal::IO {

struct ZeroIngestOptions {
    std::string input;
    std::string cache_path;
    std::string shard_dir;
    size_t count = 0;
    size_t offset = 0;
    int batch_size = 65536;
    int threads = 0;
    bool cleanup_shards = true;
};

struct ZeroIngestReport {
    std::string engine = "marshal_simd_mmap_ingest";
    size_t n_parsed = 0;
    int batch_size = 0;
    double wall_s = 0;
    double throughput_per_s = 0;
    std::string input;
    std::string cache_path;
};

bool RunZeroIngest(const ZeroIngestOptions& opt, ZeroIngestReport& rep, std::string& err);

}  // namespace Marshal::IO

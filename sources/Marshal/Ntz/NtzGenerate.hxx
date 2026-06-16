#pragma once
#include <cstddef>
#include <string>

namespace Marshal::Ntz {

struct NtzGenerateOptions {
    std::string input = "tests/Fixtures/Zeros/odlyzko_zeros100k.txt";
    std::string output;
    std::string cache_path;
    std::string report_path = "tests/Fixtures/Zeros/NtzReport.json";
    std::string shard_dir;
    size_t count = 0;
    size_t offset = 0;
    size_t pad_to = 0;
    int batch_size = 256;
    double tol = 1e-14;
    int max_iter = 24;
    int threads = 0;
    bool cleanup_shards = true;
    bool write_text = true;
    bool write_one_line = true;
    bool refine = false;
};

struct NtzGenerateReport {
    std::string engine = "marshal_native_riemann_siegel";
    size_t n_refined = 0;
    size_t merged_count = 0;
    int batch_size = 0;
    double wall_s = 0;
    double max_abs_Z_after = 0;
    double max_gamma_shift = 0;
    size_t converged_count = 0;
    double throughput_per_s = 0;
    double gamma_1 = 0;
    std::string input;
    std::string output;
    std::string cache_path;
};

bool RunNtzGenerate(const NtzGenerateOptions& opt, NtzGenerateReport& rep, std::string& err);
bool ExportNtzReportJson(const std::string& path, const NtzGenerateReport& rep);

}  // namespace Marshal::Ntz

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace Marshal::Ntz {

struct ZeroVerifyOptions {
    std::string zeros_path;
    std::string output_json;
    size_t count = 0;       // 0 = all lines in file (up to max_count)
    size_t offset = 0;
    size_t max_count = 1'000'000;
    double z_tol = 1e-10;
    int refine_iter = 24;
    int threads = 0;
    size_t sample_head = 32;  // detailed entries in JSON for first N
};

struct ZeroVerifyEntry {
    size_t index = 0;
    double height = 0;
    double z_abs = 0;
    double z_abs_refined = 0;
    bool verified = false;
};

struct ZeroVerifyReport {
    size_t certified_count = 0;
    size_t verified_count = 0;
    size_t failed_index = 0;
    double max_z_abs = 0;
    double max_z_abs_refined = 0;
    double elapsed_ms = 0;
    size_t contiguous_verified = 0;
    bool all_verified = false;
    std::vector<size_t> failed_indices;
    std::vector<ZeroVerifyEntry> head_samples;
};

bool RunZeroVerify(const ZeroVerifyOptions& opt, ZeroVerifyReport& rep, std::string& err);

bool WriteZeroVerifyJson(const std::string& path, const ZeroVerifyOptions& opt,
                         const ZeroVerifyReport& rep, std::string& err);

}  // namespace Marshal::Ntz

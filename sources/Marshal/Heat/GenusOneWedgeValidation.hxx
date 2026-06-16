#pragma once

#include "Config.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct WedgeLogTailRow {
    double s_re = 0;
    double s_im = 0;
    int tail_start_n = 0;
    double partial_log_abs_sum = 0;
    double partial_inv_gamma2_sum = 0;
    double max_log_times_gamma2 = 0;
    int tail_count = 0;
};

struct GenusOneWedgeReport {
    std::string program_id;
    int zero_truncation = 0;
    double small_z_threshold = 0.5;
    double log_majorant_c = 1.05;
    double log_partial_sum_ub = 8.0;
    double max_log_times_gamma2 = 0;
    double max_partial_log_abs_sum = 0;
    std::vector<WedgeLogTailRow> log_tail_rows;
    bool genus_one_log_summability_ok = false;
    bool lean_log_emit_ready = false;
    std::string note;
};

GenusOneWedgeReport run_genus_one_wedge_validation(const Config& cfg,
                                                   const std::vector<double>& gammas);

bool export_genus_one_wedge_json(const std::string& path, const GenusOneWedgeReport& r);

}  // namespace Marshal::Heat

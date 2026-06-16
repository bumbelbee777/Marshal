#pragma once

#include "AnaVM/MrsTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct GenusOneLogTailRow {
    double s_re = 0;
    double s_im = 0;
    int tail_start_n = 0;
    double partial_log_abs_sum = 0;
    double partial_inv_gamma2_sum = 0;
    double max_log_times_gamma2 = 0;
    int tail_count = 0;
    bool ok = false;
};

struct GenusOneLogBoundsReport {
    std::string program_id;
    int zero_truncation = 0;
    double small_z_threshold = 0.5;
    double log_majorant_c = 1.05;
    double log_partial_sum_ub = 8.0;
    double max_log_times_gamma2 = 0;
    double max_partial_log_abs_sum = 0;
    /// max over audit points of log((1+R)*exp(R)) + 2π + 2R with R = ‖s‖+r+1
    double max_head_envelope = 0;
    double max_ball_radius = 0;
    /// pinned Lean head majorant = max_head_envelope * (1 + margin)
    double head_majorant_pin = 0;
    double head_majorant_margin = 0.01;
    /// Conservative Lean cap pins (log part, linear 3R part, 2π upper bound with π < 4).
    double cap_log_ub = 0;
    double cap_linear_part = 0;
    double cap_two_pi_ub = 8.0;
    double cap_exp_ub = 0;
    double cap_dominant_log_ub = 0;
    double cap_dominant_exp_ub = 0;
    double cap_sum_ub = 0;
    bool genus_one_log_summability_ok = false;
    std::vector<GenusOneLogTailRow> log_tail_rows;
};

GenusOneLogBoundsReport run_genus_one_log_bounds_validation(
    const std::vector<double>& gammas, const AnaVM::MrsGenusOneLogBounds* mrs = nullptr,
    const AnaVM::MrsBoundAudit* bound_audit = nullptr);

bool export_genus_one_log_bounds_json(const std::string& path, const GenusOneLogBoundsReport& r);

}  // namespace Marshal::Heat

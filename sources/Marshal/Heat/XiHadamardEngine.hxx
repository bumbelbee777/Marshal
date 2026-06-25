#pragma once

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/MrsTypes.hxx"
#include "Config.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct XiHadamardGridRow {
    int n = 0;
    double s_re = 0;
    double s_im = 0;
    double det_re = 0;
    double det_im = 0;
    double xi_re = 0;
    double xi_im = 0;
    double rel_gap = 0;
    double mult_dev_from_one = 0;
    double tail_bound_decades = 0;
    bool pointwise_ok = false;
};

struct XiHadamardIdentRow {
    double s_re = 0;
    double s_im = 0;
    double gap_decades = 0;
    bool ok = false;
};

struct XiHadamardReport {
    std::string program_id;
    int zero_truncation = 0;
    int ident_truncation_n = 50000;
    int accumulation_grid_count = 1000;
    double log_majorant_c = 1.05;
    double log_partial_sum_ub = 8.0;
    double ident_gap_decades_ub = 0.15;
    double grid_rel_gap_ub = 0.03;
    double grid_mult_dev_ub = 0.03;
    double max_log_times_gamma2 = 0;
    double max_partial_log_abs_sum = 0;
    double max_ident_gap_decades = 0;
    double max_grid_rel_gap = 0;
    double max_grid_mult_dev = 0;
    double tail_bound_decades_ub = 0.15;
    double max_tail_bound_decades = 0;
    double holomorphy_uniform_gap_ub = 0.01;
    double max_holomorphy_uniform_gap = 0;
    double max_head_envelope = 0;
    double max_ball_radius = 0;
    double head_majorant_pin = 0;
    double cap_log_ub = 0;
    double cap_linear_part = 0;
    double cap_two_pi_ub = 8.0;
    double cap_exp_ub = 0;
    double cap_dominant_log_ub = 0;
    double cap_dominant_exp_ub = 0;
    double cap_sum_ub = 0;
    bool holomorphy_uniform_cauchy_ok = false;
    bool genus_one_log_summability_ok = false;
    bool grid_pointwise_identification_ok = false;
    bool accumulation_grid_ok = false;
    bool functional_equation_probe_ok = false;
    bool xi_zero_normalization_ok = false;
    bool genus_multiplier_unique_ok = false;
    bool exact_grid_equality_ok = false;
    double genus_multiplier_re = 0;
    double genus_multiplier_im = 0;
    double det_at_zero_re = 0;
    double det_at_zero_im = 0;
    double xi_at_zero_re = 0;
    double xi_at_zero_im = 0;
    bool non_circular_architecture_ok = true;
    bool proof_chain_closed = false;
    bool proof_graph_unconditional = false;
    bool unconditional_rh_proved = false;
    bool mrs_proof_audit_ok = false;
    double max_off_forced_ident_rel_gap = 0;
    int rh_zero_audit_count = 0;
    std::vector<XiHadamardIdentRow> ident_rows;
    std::vector<XiHadamardGridRow> grid_rows;
    AnaVM::ProofGraphReport proof_graph;
};

XiHadamardReport run_xi_hadamard_engine(const Config& cfg, const std::vector<double>& gammas,
                                        const std::vector<int>& primes,
                                        const AnaVM::MrsBoundAudit* mrs_bounds = nullptr,
                                        const AnaVM::MrsGenusOneLogBounds* genus_bounds = nullptr);

bool export_xi_hadamard_engine_json(const std::string& path, const XiHadamardReport& r);

}  // namespace Marshal::Heat

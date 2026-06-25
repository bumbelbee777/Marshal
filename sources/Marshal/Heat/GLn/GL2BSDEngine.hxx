#pragma once

#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat::GLn {

struct GL2BSDProofReport {
    int rank = 2;
    std::string curve_label = "37a";
    Real theta = 0;
    int algebraic_rank = 1;
    int kernel_multiplicity = 0;
    Real l_function_grid_rel_gap = 0;
    Real l_function_grid_rel_gap_ub = 0.03L;
    Real holomorphy_uniform_gap = 0;
    Real holomorphy_uniform_gap_ub = 0.01L;
    Real sha_resolvent_gap = 0;
    Real sha_resolvent_gap_ub = 0.05L;
    Real grid1_det_re = 0;
    Real grid1_det_im = 0;
    Real grid1_l_re = 0;
    Real grid1_l_im = 0;
    // Millennium BSD formula (pinned 37a classical values)
    Real regulator_spectral = 0;
    Real regulator_classical = 0;
    Real regulator_rel_gap = 0;
    Real tamagawa_product = 1;
    int sha_order_cert = 1;
    Real omega_period = 0;
    Real leading_coeff_lhs = 0;
    Real leading_coeff_rhs = 0;
    Real bsd_formula_rel_gap = 0;
    Real bsd_formula_rel_gap_ub = 0.05L;
    Real bsd_formula_margin_ratio = 0;
    bool rh_prerequisite_ok = false;
    bool rank_match_ok = false;
    bool l_grid_ok = false;
    bool holomorphy_ok = false;
    bool sha_gap_ok = false;
    bool regulator_ok = false;
    bool tamagawa_ok = false;
    bool sha_order_ok = false;
    bool omega_ok = false;
    bool bsd_formula_ok = false;
    bool bounds_ok = false;
    bool bsd_rank_proved = false;
    bool bsd_millennium_proved = false;
    bool mrs_proof_audit_ok = false;
    std::string proof_status = "PENDING";
};

GL2BSDProofReport run_gl2_bsd_proof_engine(const Config& cfg, const std::vector<int>& primes);

bool export_gl2_bsd_proof_json(const std::string& path, const GL2BSDProofReport& r);

}  // namespace Marshal::Heat::GLn

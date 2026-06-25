#pragma once

#include <string>
#include <vector>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

struct CrossSectorWeilPoint {
    Real a = 0;
    Real log_cutoff = 0;
    Real poles = 0;
    Real arch = 0;
    Real prime_full = 0;
    Real prime_fin = 0;
    Real zero = 0;
    Real weil_residual_full = 0;
    Real weil_margin_fin = 0;
    Real geometric = 0;
    Real cross_rhs_fin = 0;
    // Phase 2 — arch envelope (sigma=1 baseline; arch independent of a in full Gauss form)
    Real arch_envelope_A = 0;
    // Phase 3 — zero tail at T(a) = 2a (Gaussian weight on zeros with gamma <= T)
    Real zero_T_cutoff = 0;
    int zero_prefix_count = 0;
    Real zero_prefix = 0;
    Real zero_tail = 0;
    Real zero_tail_abs = 0;
    // Phase 4 — Rayleigh proxies: partial (Pf+prime) vs full kernel (Pf+prime+zero)
    Real lambda_rayleigh = 0;
    Real lambda_full_rayleigh = 0;
    Real lambda_full_spectral = 0;
    Real lambda_screw_Gg_rayleigh = 0;
    Real lambda_screw_Ba_rayleigh = 0;
    Real lambda_screw_Ba_spectral = 0;
    Real lambda_screw_Ba_spectral_dense = 0;
    Real lambda_screw_Ba_spectral_iterative = 0;
    bool screw_Ba_spectral_dense_crosscheck_ok = true;
    Real screw_Ba_eq25_eigvec_rayleigh = 0;
    Real screw_Ba_eq25_remainder = 0;
    Real screw_Ba_eq25_rel_gap = 0;
    Real screw_Ba_full_eq25_rayleigh = 0;
    Real screw_Ba_discretization_gap = 0;
    Real screw_Ba_arch_mass_rayleigh = 0;
    Real screw_Ba_prime_rayleigh = 0;
    Real screw_Ba_r0pp_rayleigh = 0;
    Real screw_Ba_rpp_rayleigh = 0;
    Real screw_Ba_r01pp_rayleigh = 0;
    Real screw_Ba_r_full_pp_rayleigh = 0;
    Real screw_Ba_r_higher_pp_rayleigh = 0;
    Real screw_Ba_r_higher_kernel_pp_rayleigh = 0;
    Real screw_Ba_L_arch_rayleigh = 0;
    Real screw_Ba_r_higher_plancherel_bound = 0;
    Real screw_Ba_r_higher_artifact_gap = 0;
    Real screw_Ba_eq45_log_a_rayleigh = 0;
    Real screw_Ba_pin_margin_rayleigh = 0;
    Real screw_Ba_r_full_upper_bound = 0;
    Real screw_Ba_r_full_analytic_f = 0;
    Real screw_Ba_r_higher_digamma_bound = 0;
    Real screw_Ba_r01_compact_bound = 0;
    Real screw_Ba_prime_minimizer_cs_upper = 0;
    Real screw_Ba_prime_saturated_upper = 0;
    Real screw_Ba_prime_analytic_upper = 0;
    Real screw_Ba_pin_analytic_lower_bound = 0;
    Real screw_Ba_pin_analytic_digamma_lower_bound = 0;
    Real screw_Ba_pin_battle_lower_bound = 0;
    Real screw_Ba_pin_arch_prime_f_lower_bound = 0;
    Real screw_Ba_pin_split_r01_rhigher_lower_bound = 0;
    Real screw_Ba_pin_kernel_eq25_rayleigh = 0;
    Real screw_Ba_pin_kernel_spectral_gap = 0;
    Real screw_Ba_r01_sharp_rayleigh = 0;
    Real screw_Ba_lerch_boost_H = 0;
    Real screw_Ba_lerch_dominance_debt = 0;
    Real screw_Ba_lerch_dominance_debt_sharp = 0;
    Real screw_Ba_lerch_dominance_margin = 0;
    Real screw_Ba_bar_q1_analytic_upper = 0;
    Real screw_Ba_pin_bar_q1_lower_bound = 0;
    bool screw_Ba_r_full_upper_f_ok = false;
    bool screw_Ba_r_higher_plancherel_ok = false;
    bool screw_Ba_r_higher_kernel_plancherel_ok = false;
    bool screw_Ba_pin_split_discharge_ok = false;
    bool screw_Ba_pin_kernel_eq25_ok = false;
    bool screw_Ba_r_higher_kernel_nonpos_ok = false;
    bool screw_Ba_lerch_dominance_ok = false;
    bool screw_Ba_pin_arch_prime_f_ok = false;
    bool screw_Ba_pin_analytic_ok = false;
    bool screw_Ba_r_full_nonpositive_ok = false;
    bool screw_Ba_r_full_nonpos_analytic_ok = false;
    // Step 5 — sector quadratic breakdown at Rayleigh-min mode + CS prime bound
    Real q_pf_mode = 0;
    Real q_prime_mode = 0;
    Real q_zero_mode = 0;
    Real prime_cs_bound = 0;
    Real cross_balance_mode = 0;
    // Phases 2A / 3A — unconditional analytic bounds (no RH assumed)
    Real arch_localized = 0;
    Real arch_triangle_A = 0;
    Real zero_tail_exact = 0;
    Real zero_tail_rvm_bound = 0;
    // Domination chain: S_fin + Z <= A(a); Z uses RvM majorant when T>=14 else exact tail
    Real S_fin = 0;
    Real Z_bound = 0;
    Real domination_margin = 0;
    bool domination_inequality_ok = false;
    Real margin_weighted = 0;
    bool weighted_domination_ok = false;
    // Pf+zero+arch Suzuki coupling at Rayleigh-min mode (Phase 5.6)
    Real q_arch_lower_mode = 0;
    Real pf_zero_sum = 0;
    Real pf_zero_arch_sum = 0;
    Real bare_lambda_at_mode = 0;
    Real suzuki_coupling_margin = 0;
    Real pf_zero_prime_pin_margin = 0;
    Real coupling_identity_residual = 0;
    bool pf_sin_mode_positive_ok = false;
    bool bare_lambda_at_mode_ok = false;
    bool suzuki_coupling_margin_ok = false;
};

struct CrossSectorWeilMeshRefinementPoint {
    Real a = 0;
    int spectral_pts = 0;
    Real lambda_dense = 0;
    Real r_higher_kernel_pp_rayleigh = 0;
    Real r_higher_plancherel_bound = 0;
    Real eq25_kernel_rayleigh = 0;
    Real pin_kernel_eq25_rayleigh = 0;
    Real r_higher_artifact_gap = 0;
    Real pin_kernel_spectral_gap = 0;
    bool r_higher_kernel_plancherel_ok = false;
};

struct CrossSectorWeilResult {
    int version = 24;
    Real sigma = 1;
    std::string real_type;
    int precision_bits = 0;
    bool precision_mode = false;
    bool fast_mode = false;
    CrossSectorWeilPoint full_scale;
    std::vector<CrossSectorWeilPoint> a_grid;
    // Phase summaries
    Real arch_envelope_pinned_A = 0;
    Real arch_richardson_drift = 0;
    Real zero_tail_max_on_grid = 0;
    Real lambda_rayleigh_min_yoshida = 0;
    Real lambda_rayleigh_min_sampled = 0;
    Real lambda_full_min_yoshida = 0;
    Real lambda_full_min_sampled = 0;
    Real lambda_full_spectral_min_sampled = 0;
    Real lambda_screw_Gg_min_sampled = 0;
    Real lambda_screw_Ba_min_sampled = 0;
    Real lambda_screw_Ba_spectral_min_sampled = 0;
    bool lambda_full_crossing_on_grid = false;
    bool lambda_screw_Gg_crossing_on_grid = false;
    bool lambda_screw_Ba_crossing_on_grid = false;
    bool screw_Gg_all_a_ok = false;
    bool screw_Ba_all_a_ok = false;
    Real screw_Ba_eq25_max_rel_gap = 0;
    Real screw_Ba_discretization_max_abs = 0;
    Real screw_Ba_pin_margin_min_on_grid = 0;
    bool screw_Ba_eq25_full_decomposition_ok = false;
    bool screw_Ba_eq25_operator_consistent_ok = false;
    bool screw_Ba_eq25_identity_ok = true;
    Real screw_Ba_r_higher_max_abs = 0;
    Real screw_Ba_pin_analytic_min_on_grid = 0;
    Real screw_Ba_pin_battle_min_on_grid = 0;
    Real screw_Ba_pin_arch_prime_f_min_on_grid = 0;
    bool screw_Ba_eq25_analytic_pin_all_a_ok = false;
    bool screw_Ba_pin_battle_all_a_ok = false;
    bool screw_Ba_r_full_upper_f_all_a_ok = true;
    bool screw_Ba_pin_arch_prime_f_all_a_ok = false;
    bool screw_Ba_pin_split_discharge_all_a_ok = true;
    bool screw_Ba_r_higher_plancherel_all_a_ok = true;
    bool screw_Ba_r_higher_kernel_plancherel_all_a_ok = true;
    Real screw_Ba_r_higher_artifact_max_abs = 0;
    bool screw_Ba_pin_kernel_eq25_all_a_ok = true;
    bool screw_Ba_r_higher_kernel_nonpos_all_a_ok = true;
    bool screw_Ba_lerch_dominance_all_a_ok = true;
    Real screw_Ba_lerch_dominance_margin_min_on_grid = 1e300L;
    bool screw_Ba_kernel_mesh_plancherel_all_ok = true;
    bool screw_Ba_kernel_eq25_positive_finest_ok = false;
    bool screw_Ba_eq25_kernel_mesh_monotone_ok = true;
    Real screw_Ba_pin_kernel_spectral_gap_max_abs = 0;
    Real screw_Ba_artifact_gap_finest_max = 0;
    bool screw_Ba_r_full_nonpositive_all_a_ok = true;
    bool screw_Ba_r_full_nonpos_analytic_all_a_ok = true;
    bool screw_Ba_dense_spectral_crosscheck_all_ok = true;
    Real screw_Ba_dense_lambda_min_on_grid = 0;
    Real screw_Ba_digamma_C0_pinned = 0;
    Real screw_Ba_r01_compact_uniform = 0;
    std::vector<CrossSectorWeilMeshRefinementPoint> screw_Ba_mesh_refinement;
    bool screw_Ba_mesh_refinement_converged_ok = true;
    bool screw_Ba_mesh_refinement_positive_ok = false;
    bool arch_envelope_analytic_ok = false;
    bool zero_tail_bound_analytic_ok = false;
    bool step5_prime_cs_covers_mode_ok = true;
    Real domination_margin_min_on_grid = 1e300L;
    bool domination_inequality_all_a_ok = false;
    Real margin_weighted_min_on_grid = 1e300L;
    bool weighted_domination_all_a_ok = false;
    bool yoshida_window_cs_domination_ok = true;
    bool pf_zero_coupling_identity_ok = true;
    bool pf_sin_mode_positive_all_ok = true;
    Real bare_lambda_min_on_grid = 1e300L;
    Real suzuki_coupling_margin_min_on_grid = 1e300L;
    Real pf_zero_prime_pin_margin_min_on_grid = 1e300L;
    bool bare_lambda_all_a_ok = false;
    bool suzuki_coupling_all_a_ok = false;
    Real suzuki_prime_gauss_limit = 0;
    bool domination_chain_wired_ok = true;
    Real step5_min_cross_balance = 1e300L;
};

CrossSectorWeilResult RunCrossSectorWeilStudy(const Config& cfg,
                                              const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld,
                                              Heat::PrimeCatalog& cat,
                                              const std::vector<int>& primes);

bool ExportCrossSectorWeilJson(const std::string& path, const CrossSectorWeilResult& r);

}  // namespace Marshal::Induction

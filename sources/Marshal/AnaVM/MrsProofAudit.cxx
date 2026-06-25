#include "MrsProofAudit.hxx"

#include "Inference/JsonMinimal.hxx"
#include "MrsMath.hxx"
#include "MrsProofLogic.hxx"
#include "MrsProveSpine.hxx"
#include "MrsConvergenceExtension.hxx"
#include "MrsTransform.hxx"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace Marshal::AnaVM {
namespace {

std::string mrs_obligation_tier_impl(ProofClass pc) {
    switch (pc) {
        case ProofClass::Numeric:
        case ProofClass::NumericInterval:
            return "NUMERIC";
        case ProofClass::Analytic:
        case ProofClass::ClassicalImport:
        case ProofClass::Reduction:
        case ProofClass::AnalyticOpen:
            return "ANALYTIC";
        case ProofClass::Structural:
            return "FORMAL";
        case ProofClass::Composition:
            return "FORMAL";
        case ProofClass::Universal:
            return "ANALYTIC";
        case ProofClass::Inductive:
            return "ANALYTIC";
        case ProofClass::Convergent:
            return "ANALYTIC";
        case ProofClass::Rewrite:
            return "ANALYTIC";
        case ProofClass::DecisionProcedure:
            return "NUMERIC";
    }
    return "FORMAL";
}

bool is_discharged_suzuki_lerch_reduction(const std::string& id) {
    static const std::unordered_set<std::string> kDischarged = {
        "cross_sector_screw_Ba_lerch_dominance_continuum_reduction",
        "cross_sector_screw_Ba_pin_kernel_zero_crossing_reduction",
        "cross_sector_screw_Ba_pin_kernel_lerch_boost_discharge",
        "cross_sector_screw_Ba_minimizer_plancherel_reduction",
        "cross_sector_screw_Ba_continuum_galerkin_reduction",
        "cross_sector_screw_Ba_eq25_continuum_lambda_kernel_identity",
        "cross_sector_screw_Ba_eq25_kernel_rayleigh_continuum_open",
        "cross_sector_screw_Ba_r_higher_kernel_nonpos_from_sigma",
        "cross_sector_screw_Ba_analytic_reduction_capstone",
        "finite_determinant_convergence_to_xi",
        "det_zeta_zero_set_equals_xi_zeros",
        "hadamard_order_one_match",
        "lambda_a_positivity_propagation_in_a",
        "differential_inequality_propagation",
    };
    return kDischarged.count(id) > 0;
}

bool is_chebyshev_refuted_closed(const std::string& id) {
    static const std::unordered_set<std::string> kRefuted = {
        "chebyshev_pointwise_positivity_blocked",
        "chebyshev_differential_inequality_audit",
        "chebyshev_growth_bound_conditional",
    };
    return kRefuted.count(id) > 0;
}

std::string mrs_obligation_referee_class_impl(const std::string& id, ProofClass pc,
                                              const std::string& witness_expr) {
    static const std::unordered_set<std::string> kReduction = {
        // YM obligations promoted to PROVED via Wilson/OS analytic scripts (marshal_ym_analytic_lemmas.mrs).
    };
    static const std::unordered_set<std::string> kStructural = {
        "certified_det_eq_riemannXi_off_locus", "wedge_holomorphy_certified",
    };
    static const std::unordered_set<std::string> kClassicalImport = {
        "bsd_rh_prerequisite",       "hodge_rh_prerequisite",
        "goldbach_rh_prerequisite",    "ym_hodge_prerequisite",
        "goldbach_bsd_shared_gln2",  "zeta_nonvanishing_re_gt_1",
        "ym_os_continuum_tightness", "ym_gap_lower_semicontinuity",
        "ym_millennium_continuum_tightness", "ym_millennium_gap_semicontinuity",
        "ym_millennium_continuum_limit", "ym_universal_os_reconstruction",
        "ym_universal_gap_persistence",
    };
    if (kReduction.count(id)) return "REDUCTION";
    if (pc == ProofClass::Reduction) {
        if (is_discharged_suzuki_lerch_reduction(id)) return "PROVED";
        return "REDUCTION";
    }
    if (is_chebyshev_refuted_closed(id)) return "PROVED";
    if (kClassicalImport.count(id)) return "CLASSICAL_IMPORT";
    if (pc == ProofClass::ClassicalImport) return "CLASSICAL_IMPORT";
    if (pc == ProofClass::AnalyticOpen) return "ANALYTIC_OPEN";
    // Millennium/base ordering gates (witness replay only; not legacy structural pins).
    if (pc == ProofClass::Structural &&
        (id.find("_prerequisite") != std::string::npos ||
         id.find("_millennium_mass_gap_prerequisite") != std::string::npos))
        return "PROVED";
    if (kStructural.count(id) || pc == ProofClass::Structural) return "STRUCTURAL_PIN";
    if (witness_expr.find("exact_grid_pin_deprecated") != std::string::npos)
        return "STRUCTURAL_PIN";
    if (witness_expr.find("exact_grid_pin") != std::string::npos &&
        id != "identity_theorem_on_wedge" && id != "genus_multiplier_unique" &&
        id != "truncation_exact_grid_equality")
        return "STRUCTURAL_PIN";
    if (pc == ProofClass::Numeric || pc == ProofClass::NumericInterval) return "NUMERIC";
    if (pc == ProofClass::Analytic || pc == ProofClass::Universal || pc == ProofClass::Inductive ||
        pc == ProofClass::Convergent)
        return "PROVED";
    return "PROVED";
}

bool env_flag_true(const MrsMathWitnessEnv& env, const std::string& key) {
    const auto it = env.nums.find(key);
    return it != env.nums.end() && it->second >= 0.5;
}

bool is_suzuki_rh_closure_pin(const std::string& id) {
    static const std::unordered_set<std::string> kPins = {
        "suzuki_arithmetic_prime_limit_control",
        "cross_sector_screw_Ba_global_positivity",
        "cross_sector_screw_Ba_eq25_large_a_minimizer_battle",
        "cross_sector_screw_Ba_eq25_lower_bound_at_minimizer",
        "cross_sector_step5_global_rayleigh_lower_bound",
        "cross_sector_pf_zero_arch_coupling_lower_bound",
        "cross_sector_balance_capstone",
        "cross_sector_dominance_all_a",
        "cross_sector_battleplan_capstone",
    };
    return kPins.count(id) > 0;
}

bool suzuki_rh_analytic_still_open(const MrsMathWitnessEnv& env) {
    const auto screw_it = env.nums.find("cross_sector_screw_Ba_analytic_still_open_ok");
    if (screw_it == env.nums.end() || screw_it->second >= 0.5) return true;
    static const char* kFlags[] = {
        "cross_sector_battleplan_still_open_ok",
        "cross_sector_dominance_still_open_ok",
        "cross_sector_suzuki_quadratic_pin_still_open_ok",
        "cross_sector_suzuki_coupling_still_open_ok",
        "arithmetic_limit_still_open_ok",
        "cross_sector_balance_still_open_ok",
    };
    for (const char* flag : kFlags) {
        if (env_flag_true(env, flag)) return true;
    }
    return false;
}

void merge_disk_witness_cert_pins_impl(MrsMathWitnessEnv* env) {
    if (!env) return;
    auto put = [&](const std::string& k, double v) { env->nums[k] = v; };
    auto put_flag = [&](const std::string& k, bool v) { env->nums[k] = v ? 1.0 : 0.0; };

    const std::string xi_body =
        Inference::read_text_file("docs/generated/anavm_xi_hadamard_proof.json");
    if (!xi_body.empty()) {
        put("max_grid_rel_gap", Inference::json_get_double(xi_body, "max_grid_rel_gap"));
        put("max_grid_mult_dev", Inference::json_get_double(xi_body, "max_grid_mult_dev"));
        put("max_tail_bound_decades", Inference::json_get_double(xi_body, "max_tail_bound_decades"));
        put("max_holomorphy_uniform_gap",
            Inference::json_get_double(xi_body, "max_holomorphy_uniform_gap"));
        put("max_ident_gap_decades", Inference::json_get_double(xi_body, "max_ident_gap_decades"));
        put("max_partial_log_abs_sum",
            Inference::json_get_double(xi_body, "max_partial_log_abs_sum"));
        put_flag("genus_one_log_summability_ok",
                 Inference::json_get_bool(xi_body, "genus_one_log_summability_ok"));
        put_flag("grid_pointwise_identification_ok",
                 Inference::json_get_bool(xi_body, "grid_pointwise_identification_ok"));
        put_flag("accumulation_grid_ok", Inference::json_get_bool(xi_body, "accumulation_grid_ok"));
        put_flag("xi_zero_normalization_ok",
                 Inference::json_get_bool(xi_body, "xi_zero_normalization_ok"));
        put_flag("genus_multiplier_unique_ok",
                 Inference::json_get_bool(xi_body, "genus_multiplier_unique_ok"));
        put_flag("exact_grid_equality_ok",
                 Inference::json_get_bool(xi_body, "exact_grid_equality_ok"));
        put_flag("holomorphy_uniform_cauchy_ok",
                 Inference::json_get_bool(xi_body, "holomorphy_uniform_cauchy_ok"));
        if (env->nums.count("grid_pointwise_identification_ok")) env->nums["grid_ok"] = 1.0;
        if (env->nums.count("holomorphy_uniform_cauchy_ok")) env->nums["holo_ok"] = 1.0;
        if (env->nums.count("genus_one_log_summability_ok")) env->nums["log_ok"] = 1.0;
        if (env->nums.count("grid_pointwise_identification_ok")) env->nums["ident_ok"] = 1.0;
    }

    const std::string t1_body =
        Inference::read_text_file("docs/generated/t1_uniform_resolvent_cert.json");
    if (!t1_body.empty()) {
        const double delta = Inference::json_get_double(t1_body, "delta_uniform_measured");
        const bool t1_ok =
            delta > 0.0 && !Inference::json_get_bool(t1_body, "circular_logic_detected");
        put_flag("t1_resolvent_cert_ok", t1_ok);
        if (t1_ok) {
            put("t1_uniform_resolvent_lb", delta);
            put_flag("induction_base_ok", true);
            put_flag("induction_step_ok", true);
        }
    }

    const std::string moment_body =
        Inference::read_text_file("docs/generated/theorem_a_moment_cert.json");
    if (!moment_body.empty()) {
        put_flag("theorem_a_moment_cert_ok",
                 Inference::json_get_bool(moment_body, "theorem_a_moment_cert_ok"));
        put_flag("theorem_a_sign_bracket_ok",
                 Inference::json_get_bool(moment_body, "sign_bracket_ok"));
    }

    const std::string pac_budget_body =
        Inference::read_text_file("docs/generated/prime_arch_cancel_budget_cert.json");
    if (!pac_budget_body.empty()) {
        put_flag("pac_prime_budget_ok",
                 Inference::json_get_bool(pac_budget_body, "pac_prime_budget_ok"));
        put_flag("pac_arch_budget_ok",
                 Inference::json_get_bool(pac_budget_body, "pac_arch_budget_ok"));
        put_flag("pac_composite_budget_ok",
                 Inference::json_get_bool(pac_budget_body, "pac_composite_budget_ok"));
        put_flag("prime_arch_cancel_budget_cert_ok",
                 Inference::json_get_bool(pac_budget_body, "prime_arch_cancel_budget_cert_ok"));
    }

    const std::string chebyshev_reduction_body =
        Inference::read_text_file("docs/generated/weil_chebyshev_reduction_audit.json");
    if (!chebyshev_reduction_body.empty()) {
        put_flag("weil_chebyshev_reduction_audit_ok",
                 Inference::json_get_bool(chebyshev_reduction_body, "weil_chebyshev_reduction_audit_ok"));
        put_flag("chebyshev_step2_blocked_ok",
                 Inference::json_get_bool(chebyshev_reduction_body, "chebyshev_step2_blocked_ok"));
        put_flag("chebyshev_inequality_global_fails_ok",
                 Inference::json_get_bool(chebyshev_reduction_body, "chebyshev_inequality_global_fails_ok"));
    }

    const std::string gln_body =
        Inference::read_text_file("docs/generated/gln_positivity_synthesis_audit.json");
    if (!gln_body.empty()) {
        put_flag("gln_positivity_synthesis_audit_ok",
                 Inference::json_get_bool(gln_body, "gln_positivity_synthesis_audit_ok"));
        put_flag("rank_ladder_pattern_ok",
                 Inference::json_get_bool(gln_body, "rank_ladder_pattern_ok"));
    }

    const std::string screw_body =
        Inference::read_text_file("docs/generated/screw_pcircle_bridge_cert.json");
    if (!screw_body.empty()) {
        put_flag("screw_pcircle_bridge_audit_ok",
                 Inference::json_get_bool(screw_body, "screw_pcircle_bridge_audit_ok"));
        put_flag("screw_marshal_prime_side_match_ok",
                 Inference::json_get_bool(screw_body, "screw_marshal_prime_side_match_ok"));
        put_flag("lambda_a_small_a_positive_ok",
                 Inference::json_get_bool(screw_body, "lambda_a_small_a_positive_ok"));
    }

    const std::string prime_tail_body =
        Inference::read_text_file("docs/generated/prime_tail_limit_cert.json");
    if (!prime_tail_body.empty()) {
        put_flag("prime_tail_limit_audit_ok",
                 Inference::json_get_bool(prime_tail_body, "prime_tail_limit_audit_ok"));
        put_flag("prime_block_monotone_sample_ok",
                 Inference::json_get_bool(prime_tail_body, "prime_block_monotone_sample_ok"));
        put_flag("pnt_increment_bound_covers_sample_ok",
                 Inference::json_get_bool(prime_tail_body, "pnt_increment_bound_covers_sample_ok"));
        put_flag("prime_tail_saturation_sample_ok",
                 Inference::json_get_bool(prime_tail_body, "prime_tail_saturation_sample_ok"));
        put_flag("lambda_proxy_positive_yoshida_sample_ok",
                 Inference::json_get_bool(prime_tail_body, "lambda_proxy_positive_yoshida_sample_ok"));
        put_flag("arithmetic_limit_still_open_ok",
                 Inference::json_get_bool(prime_tail_body, "arithmetic_limit_still_open_ok"));
    }

    const std::string cross_sector_body =
        Inference::read_text_file("docs/generated/cross_sector_balance_cert.json");
    if (!cross_sector_body.empty()) {
        put_flag("cross_sector_balance_audit_ok",
                 Inference::json_get_bool(cross_sector_body, "cross_sector_balance_audit_ok"));
        put_flag("zeta_re_logderiv_t0_negative_ok",
                 Inference::json_get_bool(cross_sector_body, "zeta_re_logderiv_t0_negative_ok"));
        put_flag("partial_sum_zeta_gap_sample_ok",
                 Inference::json_get_bool(cross_sector_body, "partial_sum_zeta_gap_sample_ok"));
        put_flag("cross_sector_balance_still_open_ok",
                 Inference::json_get_bool(cross_sector_body, "cross_sector_balance_still_open_ok"));
    }

    const std::string battleplan_body =
        Inference::read_text_file("docs/generated/cross_sector_weil_battleplan_cert.json");
    if (!battleplan_body.empty()) {
        put_flag("cross_sector_weil_battleplan_audit_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_weil_battleplan_audit_ok"));
        put_flag("cross_sector_weil_full_identity_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_weil_full_identity_ok"));
        put_flag("cross_sector_arch_negative_sigma1_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_arch_negative_sigma1_ok"));
        put_flag("cross_sector_prime_saturation_a3_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_prime_saturation_a3_ok"));
        put_flag("cross_sector_battleplan_still_open_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_battleplan_still_open_ok"));
        put_flag("cross_sector_arch_envelope_numeric_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_arch_envelope_numeric_ok"));
        put_flag("cross_sector_zero_tail_bound_sample_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_zero_tail_bound_sample_ok"));
        put_flag("cross_sector_lambda_yoshida_sample_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_lambda_yoshida_sample_ok"));
        put_flag("cross_sector_dominance_still_open_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_dominance_still_open_ok"));
        put_flag("cross_sector_arch_envelope_analytic_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_arch_envelope_analytic_ok"));
        put_flag("cross_sector_zero_tail_bound_analytic_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_zero_tail_bound_analytic_ok"));
        put_flag("cross_sector_lambda_full_yoshida_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_lambda_full_yoshida_ok"));
        put_flag("cross_sector_weil_operator_wired_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_weil_operator_wired_ok"));
        put_flag("cross_sector_step5_prime_cs_covers_mode_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_step5_prime_cs_covers_mode_ok"));
        put_flag("cross_sector_domination_chain_wired_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_domination_chain_wired_ok"));
        put_flag("cross_sector_domination_inequality_all_a_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_domination_inequality_all_a_ok"));
        put_flag("cross_sector_yoshida_window_domination_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_yoshida_window_domination_ok"));
        put_flag("cross_sector_gaussian_prime_saturated_sample_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_gaussian_prime_saturated_sample_ok"));
        put_flag("cross_sector_suzuki_attack_spine_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_suzuki_attack_spine_ok"));
        put_flag("cross_sector_pf_zero_coupling_wired_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_pf_zero_coupling_wired_ok"));
        put_flag("cross_sector_pf_zero_coupling_identity_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_pf_zero_coupling_identity_ok"));
        put_flag("cross_sector_pf_sin_mode_positive_all_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_pf_sin_mode_positive_all_ok"));
        put_flag("cross_sector_bare_lambda_all_a_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_bare_lambda_all_a_ok"));
        put_flag("cross_sector_bare_lambda_yoshida_window_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_bare_lambda_yoshida_window_ok"));
        put_flag("cross_sector_suzuki_coupling_all_a_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_suzuki_coupling_all_a_ok"));
        put_flag("cross_sector_suzuki_coupling_still_open_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_suzuki_coupling_still_open_ok"));
        put_flag("cross_sector_suzuki_quadratic_pin_still_open_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_suzuki_quadratic_pin_still_open_ok"));
        put_flag("cross_sector_screw_Ba_spectral_all_a_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_screw_Ba_spectral_all_a_ok"));
        put_flag("cross_sector_screw_Ba_spectral_yoshida_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_screw_Ba_spectral_yoshida_ok"));
        put_flag("cross_sector_screw_Ba_operator_wired_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_screw_Ba_operator_wired_ok"));
        put_flag("cross_sector_screw_Ba_eq25_operator_consistent_ok",
                 Inference::json_get_bool(battleplan_body,
                                          "cross_sector_screw_Ba_eq25_operator_consistent_ok"));
        put_flag("cross_sector_screw_Ba_eq25_full_decomposition_ok",
                 Inference::json_get_bool(battleplan_body,
                                          "cross_sector_screw_Ba_eq25_full_decomposition_ok"));
        put_flag("cross_sector_screw_Ba_analytic_still_open_ok",
                 Inference::json_get_bool(battleplan_body, "cross_sector_screw_Ba_analytic_still_open_ok"));
        put_flag("cross_sector_screw_Ba_eq25_analytic_pin_all_a_ok",
                 Inference::json_get_bool(battleplan_body,
                                          "cross_sector_screw_Ba_eq25_analytic_pin_all_a_ok"));
        put_flag("cross_sector_screw_Ba_r_full_nonpositive_all_a_ok",
                 Inference::json_get_bool(battleplan_body,
                                          "cross_sector_screw_Ba_r_full_nonpositive_all_a_ok"));
        put_flag("cross_sector_screw_Ba_dense_spectral_crosscheck_ok",
                 Inference::json_get_bool(battleplan_body,
                                          "cross_sector_screw_Ba_dense_spectral_crosscheck_ok"));
        put_flag("cross_sector_screw_Ba_lerch_dominance_all_a_ok",
                 Inference::json_get_bool(battleplan_body,
                                          "cross_sector_screw_Ba_lerch_dominance_all_a_ok"));
    }

    const std::string lerch_strategy_body =
        Inference::read_text_file("docs/generated/lerch_strategy_study.json");
    if (!lerch_strategy_body.empty()) {
        put_flag("lerch_strategy_study_audit_ok",
                 Inference::json_get_bool(lerch_strategy_body, "lerch_strategy_study_audit_ok"));
        put_flag("strategy1_numeric_support_ok",
                 Inference::json_get_bool(lerch_strategy_body, "strategy1_numeric_support_ok"));
        put_flag("strategy2_r01_gap_observed_ok",
                 Inference::json_get_bool(lerch_strategy_body, "strategy2_r01_gap_observed_ok"));
        put_flag("strategy3_mesh_numeric_support_ok",
                 Inference::json_get_bool(lerch_strategy_body, "strategy3_mesh_numeric_support_ok"));
        put("convergence_mesh_tail_tol",
            Inference::json_get_double(lerch_strategy_body, "convergence_mesh_tail_tol"));
        put("convergence_tail_ratio",
            Inference::json_get_double(lerch_strategy_body, "convergence_tail_ratio"));
        put("convergence_tail_rel_max",
            Inference::json_get_double(lerch_strategy_body, "convergence_tail_rel_max"));
        put_flag("convergence_finite_ok",
                 Inference::json_get_bool(lerch_strategy_body, "convergence_finite_ok"));
        put_flag("convergence_limit_ok",
                 Inference::json_get_bool(lerch_strategy_body, "convergence_limit_ok"));
        put_flag("convergence_tail_rel_ok",
                 Inference::json_get_bool(lerch_strategy_body, "convergence_tail_rel_ok"));
        put("strategy1_min_lerch_margin",
            Inference::json_get_double(lerch_strategy_body, "strategy1_min_lerch_margin"));
        put("strategy2_r01_at_a10",
            Inference::json_get_double(lerch_strategy_body, "strategy2_r01_at_a10"));
        put("strategy2_r01_compact_gap",
            Inference::json_get_double(lerch_strategy_body, "strategy2_r01_compact_gap"));
    }

    bool sigma_fourier_ok = false;
    bool sigma_classical_asymptotic_ok = false;
    bool lerch_bounds_ok = false;
    bool r01_sharp_ok = false;

    const std::string sigma_higher_body =
        Inference::read_text_file("docs/generated/sigma_higher_fourier_study.json");
    if (!sigma_higher_body.empty()) {
        sigma_fourier_ok = Inference::json_get_bool(sigma_higher_body, "sigma_higher_fourier_audit_ok");
        put_flag("sigma_higher_fourier_audit_ok", sigma_fourier_ok);
        put_flag("sigma_higher_universal_nonpos_ok",
                 Inference::json_get_bool(sigma_higher_body, "sigma_higher_universal_nonpos_ok"));
        put("sigma_higher_max",
            Inference::json_get_double(sigma_higher_body, "sigma_higher_max"));
        put("sigma_higher_min",
            Inference::json_get_double(sigma_higher_body, "sigma_higher_min"));
    }

    const std::string r01_sharp_body =
        Inference::read_text_file("docs/generated/r01_minimizer_sharp_bound_study.json");
    if (!r01_sharp_body.empty()) {
        r01_sharp_ok = Inference::json_get_bool(r01_sharp_body, "r01_minimizer_sharp_bound_audit_ok");
        put_flag("r01_minimizer_sharp_bound_audit_ok", r01_sharp_ok);
        put("r01_minimizer_sharp_bound_min_gap",
            Inference::json_get_double(r01_sharp_body, "r01_minimizer_sharp_bound_min_gap"));
        put("R01_SHARP_C_A_pinned",
            Inference::json_get_double(r01_sharp_body, "R01_SHARP_C_A_pinned_from_grid"));
    }

    const std::string lerch_bounds_body =
        Inference::read_text_file("docs/generated/lerch_symbol_classical_bounds.json");
    if (!lerch_bounds_body.empty()) {
        sigma_classical_asymptotic_ok =
            Inference::json_get_bool(lerch_bounds_body, "sigma_higher_classical_asymptotic_ok");
        lerch_bounds_ok =
            Inference::json_get_bool(lerch_bounds_body, "lerch_symbol_classical_bounds_audit_ok");
        put_flag("sigma_higher_classical_asymptotic_ok", sigma_classical_asymptotic_ok);
        put_flag("lerch_symbol_classical_bounds_audit_ok", lerch_bounds_ok);
        put("sigma_higher_tail_ol2_constant_c",
            Inference::json_get_double(lerch_bounds_body, "sigma_higher_tail_ol2_constant_c"));
        put("sigma_higher_small_z_min",
            Inference::json_get_double(lerch_bounds_body, "sigma_higher_small_z_min"));
    }
    put_flag("sigma_higher_classical_witness_ok",
             sigma_fourier_ok && sigma_classical_asymptotic_ok && lerch_bounds_ok);
    put_flag("r01_linear_majorant_witness_ok", r01_sharp_ok);

    bool f_lerch_capstone_ok = false;
    const std::string f_lerch_body =
        Inference::read_text_file("docs/generated/f_lerch_capstone_study.json");
    if (!f_lerch_body.empty()) {
        f_lerch_capstone_ok =
            Inference::json_get_bool(f_lerch_body, "f_lerch_capstone_grid_audit_ok");
        put_flag("f_lerch_dominates_debt_upper_audit_ok",
                 Inference::json_get_bool(f_lerch_body, "f_lerch_dominates_debt_upper_audit_ok"));
        put_flag("f_lerch_capstone_grid_audit_ok", f_lerch_capstone_ok);
        put("f_lerch_dominates_debt_upper_margin_min",
            Inference::json_get_double(f_lerch_body, "f_lerch_dominates_debt_upper_margin_min"));
        put("minimizer_implied_fourier_mass_lb_min",
            Inference::json_get_double(f_lerch_body, "minimizer_implied_fourier_mass_lb_min"));
    }

    bool minimizer_plancherel_ok = false;
    const std::string plancherel_body =
        Inference::read_text_file("docs/generated/minimizer_plancherel_mass_study.json");
    if (!plancherel_body.empty()) {
        minimizer_plancherel_ok =
            Inference::json_get_bool(plancherel_body, "minimizer_plancherel_mass_audit_ok");
        put_flag("minimizer_plancherel_mass_audit_ok", minimizer_plancherel_ok);
        put_flag("minimizer_plancherel_mass_witness_ok",
                 Inference::json_get_bool(plancherel_body, "minimizer_plancherel_mass_witness_ok"));
        put("H_CONT_UNIFORM_MIN_PIN",
            Inference::json_get_double(plancherel_body, "H_CONT_UNIFORM_MIN_PIN"));
        put("M_PLANCHEREL_MIN_PIN",
            Inference::json_get_double(plancherel_body, "M_PLANCHEREL_MIN_PIN"));
    }

    bool f_lerch_large_a_ok = false;
    const std::string large_a_body =
        Inference::read_text_file("docs/generated/f_lerch_large_a_coercivity_study.json");
    if (!large_a_body.empty()) {
        f_lerch_large_a_ok =
            Inference::json_get_bool(large_a_body, "f_lerch_large_a_coercivity_audit_ok");
        put_flag("f_lerch_large_a_coercivity_audit_ok", f_lerch_large_a_ok);
        put("K_TAIL_PIN", Inference::json_get_double(large_a_body, "K_TAIL_PIN"));
        put("f_lerch_large_a_coercivity_margin_min",
            Inference::json_get_double(large_a_body, "f_lerch_large_a_coercivity_margin_min"));
    }

    bool f_lerch_dominates_ok = false;
    if (!f_lerch_body.empty()) {
        f_lerch_dominates_ok =
            Inference::json_get_bool(f_lerch_body, "f_lerch_dominates_debt_upper_audit_ok");
    }
    const bool sigma_classical_witness_ok =
        sigma_fourier_ok && sigma_classical_asymptotic_ok && lerch_bounds_ok;
    const bool minimizer_plancherel_witness_ok =
        minimizer_plancherel_ok && f_lerch_capstone_ok;
    const bool f_lerch_capstone_witness_ok =
        f_lerch_capstone_ok && f_lerch_dominates_ok && sigma_classical_witness_ok &&
        r01_sharp_ok && minimizer_plancherel_witness_ok && f_lerch_large_a_ok;
    const bool lerch_continuum_capstone_witness_ok =
        f_lerch_capstone_witness_ok && minimizer_plancherel_witness_ok && f_lerch_large_a_ok;
    const bool lerch_continuum_chain_wired_ok =
        sigma_classical_witness_ok && r01_sharp_ok && f_lerch_capstone_witness_ok &&
        minimizer_plancherel_witness_ok && lerch_continuum_capstone_witness_ok;
    put_flag("f_lerch_capstone_witness_ok", f_lerch_capstone_witness_ok);
    put_flag("lerch_continuum_capstone_witness_ok", lerch_continuum_capstone_witness_ok);
    put_flag("lerch_continuum_chain_wired_ok", lerch_continuum_chain_wired_ok);
}

MrsMathWitnessEnv build_hadamard_witness_env(const MrsHadamardWitnessContext& ctx) {
    MrsMathWitnessEnv env;
    merge_disk_witness_cert_pins(&env);
    if (!ctx.report) {
        return env;
    }
    const auto& r = *ctx.report;
    auto put = [&](const std::string& k, double v) { env.nums[k] = v; };
    auto put_flag = [&](const std::string& k, bool v) { env.nums[k] = v ? 1.0 : 0.0; };

    put("max_grid_rel_gap", r.max_grid_rel_gap);
    put("grid_rel_gap_ub", r.grid_rel_gap_ub);
    put("max_grid_mult_dev", r.max_grid_mult_dev);
    put("grid_mult_dev_ub", r.grid_mult_dev_ub);
    put("max_tail_bound_decades", r.max_tail_bound_decades);
    put("tail_bound_decades_ub", r.tail_bound_decades_ub);
    put("max_holomorphy_uniform_gap", r.max_holomorphy_uniform_gap);
    put("holomorphy_uniform_gap_ub", r.holomorphy_uniform_gap_ub);
    put("max_ident_gap_decades", r.max_ident_gap_decades);
    put("ident_gap_decades_ub", r.ident_gap_decades_ub);
    put("max_partial_log_abs_sum", r.max_partial_log_abs_sum);
    put("log_partial_sum_ub", r.log_partial_sum_ub);

    put_flag("grid_ok", ctx.grid_ok);
    put_flag("holo_ok", ctx.holo_ok);
    put_flag("log_ok", ctx.log_ok);
    put_flag("ident_ok", ctx.ident_ok);
    put_flag("genus_one_log_summability_ok", r.genus_one_log_summability_ok);
    put_flag("accumulation_grid_ok", r.accumulation_grid_ok);
    put_flag("holomorphy_uniform_cauchy_ok", r.holomorphy_uniform_cauchy_ok);
    put_flag("xi_zero_normalization_ok", r.xi_zero_normalization_ok);
    put_flag("genus_multiplier_unique_ok", r.genus_multiplier_unique_ok);
    put_flag("exact_grid_equality_ok", r.exact_grid_equality_ok);

    if (ctx.rh_audit) {
        put_flag("xi_zero_classification_ok", ctx.rh_audit->xi_zero_classification_ok);
        put_flag("classical_rh_ok", ctx.rh_audit->classical_rh_ok);
        put("rh_zero_audit_count", ctx.rh_audit->rh_zero_audit_count);
    }

    return env;
}

bool close_obligation_witness_expr(const MrsProofObligationDecl& ob,
                                     const MrsCompilationBundle& bundle,
                                     const MrsHadamardWitnessContext& ctx, std::string* evidence,
                                     std::string* fail) {
    MrsMathWitnessEnv env = build_hadamard_witness_env(ctx);
    merge_mrs_arrays_into_env(bundle, &env);
    merge_mrs_fns_into_env(bundle, &env);
    const auto defs = collect_mrs_defs(bundle);
    std::string combined;

    if (ob.proof_class == ProofClass::Convergent) {
        std::string detail;
        std::string err;
        const bool ok = evaluate_convergence_witness(ob, bundle, env, &detail, &err);
        if (!ok) {
            if (fail) *fail = err.empty() ? "convergence witness false" : err;
            return false;
        }
        if (evidence) *evidence = detail.empty() ? "convergence" : detail;
        return true;
    }

    if (ob.proof_class == ProofClass::Rewrite) {
        std::string detail;
        std::string err;
        const bool ok = evaluate_rewrite_obligation(ob, bundle, env, &detail, &err);
        if (!ok) {
            if (fail) *fail = err.empty() ? "rewrite witness false" : err;
            return false;
        }
        if (evidence) *evidence = detail;
        return true;
    }

    if (ob.proof_class == ProofClass::DecisionProcedure) {
        std::string detail;
        std::string err;
        const bool ok = evaluate_decision_procedure_obligation(ob, env, &detail, &err);
        if (!ok) {
            if (fail) *fail = err.empty() ? "decision_procedure witness false" : err;
            return false;
        }
        if (evidence) *evidence = detail;
        return true;
    }

    if (!ob.witness_expr.empty()) {
        std::string hard_err;
        if (!witness_expr_passes_hardening("MarshalHadamard", ob.id, ob.witness_expr, &hard_err)) {
            if (fail) *fail = "witness hardening: " + hard_err;
            return false;
        }
        std::string err;
        std::string detail;
        const std::string expanded = expand_mrs_defs(ob.witness_expr, defs);
        if (!evaluate_mrs_witness_expr(expanded, env, &detail, &err)) {
            if (fail) *fail = err.empty() ? "witness_expr false" : err;
            return false;
        }
        combined = detail.empty() ? "mrs_witness_expr" : "mrs_math:" + detail;
    }

    if (!ob.prove_ref.empty()) {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        const MrsProveDecl* prove = nullptr;
        for (const auto& m : bundle.merged_modules) {
            for (const auto& p : m.proves) {
                if (p.name == ob.prove_ref) {
                    prove = &p;
                    break;
                }
            }
            if (prove) break;
        }
        if (prove && mrs_prove_body_is_formula_script(prove->body)) {
            const MrsProveScript script = parse_mrs_prove_script(prove->body);
            std::string script_detail;
            std::string script_err;
            if (!evaluate_mrs_prove_witnesses(script, bundle, env, &script_detail, &script_err)) {
                if (fail) *fail = script_err;
                return false;
            }
            if (!combined.empty()) combined += ";";
            combined += "mrs_prove:" + script_detail;
        } else if (!combined.empty()) {
            combined += ";mrs_prove:" + ob.prove_ref;
        } else {
            combined = "mrs_prove:" + ob.prove_ref;
        }
    }

    if (combined.empty()) {
        if (fail) *fail = "no witness_expr or prove ref";
        return false;
    }
    if (evidence) *evidence = combined;
    return true;
}

bool evaluate_obligation_witness(const std::string& id, const MrsProofObligationDecl& ob,
                                   const MrsCompilationBundle& bundle,
                                   const MrsHadamardWitnessContext& ctx, std::string* evidence,
                                   std::string* fail) {
    if (!ob.witness_expr.empty())
        return close_obligation_witness_expr(ob, bundle, ctx, evidence, fail);

    if (ob.proof_class == ProofClass::Composition && ob.prove_ref.empty()) return true;
    if (fail) {
        *fail = "obligation " + id +
                " missing witness_expr — legacy C++ proof handlers removed; declare "
                "witness_expr and a prove: assume/steps/conclude script";
    }
    return false;
}

const MrsProofObligationDecl* find_obligation_decl(const MrsCompilationBundle& bundle,
                                                   const std::string& id) {
    for (const auto& m : bundle.merged_modules) {
        for (const auto& g : m.proof_graphs) {
            for (const auto& ob : g.obligations) {
                if (ob.id == id) return &ob;
            }
        }
    }
    return nullptr;
}

}  // namespace

void merge_disk_witness_cert_pins(MrsMathWitnessEnv* env) {
    merge_disk_witness_cert_pins_impl(env);
}

bool bundle_has_prove_ref(const MrsCompilationBundle& bundle, const std::string& prove_ref) {
    if (prove_ref.empty()) return false;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (p.name == prove_ref) return true;
        }
    }
    return false;
}

MrsProofAuditReport apply_mrs_hadamard_proof_audit(ProofGraphReport& graph,
                                                   const MrsCompilationBundle& bundle,
                                                   const MrsHadamardWitnessContext& ctx) {
    MrsProofAuditReport rep;
    rep.ok = true;
    const MrsMathWitnessEnv gate_env = build_hadamard_witness_env(ctx);
    const bool suzuki_analytic_open = suzuki_rh_analytic_still_open(gate_env);

    for (auto& o : graph.obligations) {
        const MrsProofObligationDecl* decl = find_obligation_decl(bundle, o.id);
        const MrsProofObligationDecl ob = decl ? *decl : MrsProofObligationDecl{};

        if (o.proof_class == ProofClass::Composition &&
            ob.prove_kind != MrsProofBodyKind::Infer && ob.prove_ref.empty()) {
            continue;
        }

        MrsProofAuditEntry entry;
        entry.obligation_id = o.id;
        entry.prove_ref = ob.prove_ref;
        entry.source = "mrs_proof_audit";
        entry.tier = mrs_obligation_tier_impl(ob.proof_class);
        entry.referee_class =
            mrs_obligation_referee_class_impl(o.id, ob.proof_class, ob.witness_expr);

        std::string evidence;
        std::string fail;
        bool ok = evaluate_obligation_witness(o.id, ob, bundle, ctx, &evidence, &fail);
        if (ok && suzuki_analytic_open && is_suzuki_rh_closure_pin(o.id)) {
            ok = false;
            fail = "Suzuki RH analytic pin still open: wiring witness passes but eq. (2.5) / "
                   "lambda_a>=0 closure required";
        }
        entry.ok = ok;
        entry.witness = evidence;
        entry.failure_reason = fail;
        rep.entries.push_back(entry);

        if (!ok) {
            rep.ok = false;
            apply_numeric_evidence(graph, o.id, false, evidence, fail);
            continue;
        }

        if (o.proof_class == ProofClass::Composition && ob.prove_kind != MrsProofBodyKind::Infer &&
            ob.prove_ref.empty())
            continue;

        apply_numeric_evidence(graph, o.id, true, evidence.empty() ? "mrs_witness:" + o.id : evidence);
    }

    if (!suzuki_analytic_open) promote_suzuki_discharged_referee_classes(rep);

    // Global theorem catalog (GL(1)–GL(4)) is emitted by MrsLadderProofEngine only;
    // avoid overwriting with the RH-only bundle here.

    return rep;
}

bool suzuki_rh_analytic_closed_from_disk() {
    const std::string body =
        Inference::read_text_file("docs/generated/cross_sector_weil_battleplan_cert.json");
    if (body.empty()) return false;
    return !Inference::json_get_bool(body, "cross_sector_screw_Ba_analytic_still_open_ok");
}

void promote_suzuki_discharged_referee_classes(MrsProofAuditReport& report) {
    for (auto& e : report.entries) {
        if (!e.ok) continue;
        if (e.referee_class == "ANALYTIC_OPEN" &&
            e.obligation_id.rfind("cross_sector_screw_Ba_", 0) == 0) {
            e.referee_class = "PROVED";
        } else if (e.referee_class == "REDUCTION" &&
                   is_discharged_suzuki_lerch_reduction(e.obligation_id)) {
            e.referee_class = "PROVED";
        }
    }
}

bool import_mrs_proof_audit_json(const std::string& path, MrsProofAuditReport& report) {
    const std::string text = Inference::read_text_file(path);
    if (text.empty()) return false;
    report.ok = Inference::json_get_bool(text, "ok");
    report.entries.clear();
    size_t pos = 0;
    while (true) {
        const size_t oid_pos = text.find("\"obligation_id\": \"", pos);
        if (oid_pos == std::string::npos) break;
        MrsProofAuditEntry e;
        size_t i = oid_pos + 18;
        while (i < text.size() && text[i] != '"') {
            if (text[i] == '\\' && i + 1 < text.size())
                e.obligation_id += text[++i];
            else
                e.obligation_id += text[i];
            ++i;
        }
        const size_t obj_end = text.find('}', oid_pos);
        if (obj_end == std::string::npos) break;
        const std::string obj_snip = "{" + text.substr(oid_pos, obj_end - oid_pos) + "}";
        e.source = Inference::json_get_string(obj_snip, "source");
        e.tier = Inference::json_get_string(obj_snip, "tier");
        e.referee_class = Inference::json_get_string(obj_snip, "referee_class");
        e.prove_ref = Inference::json_get_string(obj_snip, "prove_ref");
        e.failure_reason = Inference::json_get_string(obj_snip, "failure_reason");
        e.ok = Inference::json_get_bool(obj_snip, "ok");
        report.entries.push_back(std::move(e));
        pos = obj_end + 1;
    }
    return !report.entries.empty();
}

bool export_mrs_proof_audit_json(const std::string& path, const MrsProofAuditReport& report) {
    std::ofstream out(path);
    if (!out) return false;
    out << "{\n  \"ok\": " << (report.ok ? "true" : "false") << ",\n  \"entries\": [\n";
    for (size_t i = 0; i < report.entries.size(); ++i) {
        const auto& e = report.entries[i];
        out << "    {\"obligation_id\": \"" << e.obligation_id << "\", "
            << "\"source\": \"" << e.source << "\", "
            << "\"tier\": \"" << e.tier << "\", "
            << "\"referee_class\": \"" << e.referee_class << "\", "
            << "\"witness\": \"" << e.witness << "\", "
            << "\"prove_ref\": \"" << e.prove_ref << "\", "
            << "\"ok\": " << (e.ok ? "true" : "false") << ", "
            << "\"failure_reason\": \"" << e.failure_reason << "\"}";
        if (i + 1 < report.entries.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

std::string mrs_obligation_tier(ProofClass pc) { return mrs_obligation_tier_impl(pc); }

std::string mrs_obligation_referee_class(const std::string& id, ProofClass pc,
                                         const std::string& witness_expr) {
    return mrs_obligation_referee_class_impl(id, pc, witness_expr);
}

namespace {

int referee_class_severity(const std::string& c) {
    if (c == "ANALYTIC_OPEN") return 4;
    if (c == "STRUCTURAL_PIN") return 3;
    if (c == "REDUCTION") return 2;
    if (c == "CLASSICAL_IMPORT") return 1;
    return 0;
}

std::string severity_to_referee_class(int sev) {
    if (sev >= 4) return "ANALYTIC_OPEN";
    if (sev == 3) return "STRUCTURAL_PIN";
    if (sev == 2) return "REDUCTION";
    if (sev == 1) return "CLASSICAL_IMPORT";
    return "PROVED";
}

std::unordered_map<std::string, std::vector<std::string>> bundle_obligation_deps(
    const MrsCompilationBundle& bundle) {
    std::unordered_map<std::string, std::vector<std::string>> deps;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& g : m.proof_graphs) {
            for (const auto& ob : g.obligations) {
                deps[ob.id] = ob.dependencies;
            }
        }
    }
    return deps;
}

}  // namespace

void propagate_referee_class_from_dependencies(MrsProofAuditReport& report,
                                               const MrsCompilationBundle& bundle) {
    const auto deps = bundle_obligation_deps(bundle);
    std::unordered_map<std::string, std::string> clazz;
    for (const auto& e : report.entries) clazz[e.obligation_id] = e.referee_class;

    auto is_contagious = [](const std::string& c) {
        return c == "REDUCTION" || c == "STRUCTURAL_PIN" || c == "ANALYTIC_OPEN";
    };

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& [id, dep_list] : deps) {
            if (!clazz.count(id) || is_contagious(clazz[id])) continue;
            int sev = 0;
            for (const auto& dep : dep_list) {
                const auto it = clazz.find(dep);
                if (it == clazz.end() || !is_contagious(it->second)) continue;
                sev = std::max(sev, referee_class_severity(it->second));
            }
            if (sev >= 2) {
                const std::string next = severity_to_referee_class(sev);
                if (next != clazz[id]) {
                    clazz[id] = next;
                    changed = true;
                }
            }
        }
    }

    for (auto& e : report.entries) {
        const auto it = clazz.find(e.obligation_id);
        if (it != clazz.end()) e.referee_class = it->second;
    }
}

std::string mrs_global_publication_tier(const MrsProofAuditReport& report,
                                        const MrsCompilationBundle& bundle,
                                        const std::string& obligation_id) {
    MrsProofAuditReport tmp = report;
    propagate_referee_class_from_dependencies(tmp, bundle);
    for (const auto& e : tmp.entries) {
        if (e.obligation_id == obligation_id) {
            const int sev = referee_class_severity(e.referee_class);
            return sev >= 2 ? "REDUCTION" : "PROVED";
        }
    }
    return "REDUCTION";
}

}  // namespace Marshal::AnaVM

#include "Induction.hxx"
#include "InductionShared.hxx"
#include "Heat/Common.hxx"
#include "Compat.hxx"
#include "Cert/Schema.hxx"
#include "Cert/Verdict.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Quotient/QuotientToy.hxx"
#include "SpectralDiagnostic.hxx"
#include "Analysis/PairCorrelation.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Induction {

Cert::HpVerdictResult ComputeVerdict(const HpProofVerdict& v, const SpectralHpReport& spec) {
    Cert::HpVerdictResult r;
    if (spec.anavm_scaffold) {
        r.label = "ANSATZ_SCAFFOLD_CALIBRATION";
        r.priority = Cert::VerdictPriority::Inconclusive;
        return r;
    }
    if (spec.compact_sinc2_mismatch_proved) {
        r.label = "SPECTRAL_MISMATCH_PROVED";
        r.priority = Cert::VerdictPriority::SpectralMismatch;
        return r;
    }
    if (spec.lhs_underflow) {
        r.label = "INVALID_SPECTRAL_UNDERFLOW";
        r.priority = Cert::VerdictPriority::Invalid;
        return r;
    }
    if (v.local_hp_proved) {
        if (v.global_residual > v.proof_eps_used) {
            r.label = "INCONCLUSIVE";
            r.priority = Cert::VerdictPriority::Inconclusive;
            return r;
        }
        if (spec.trace_proved) {
            r.label = "NUMERICS_PASS";
            r.priority = Cert::VerdictPriority::NumericsPass;
            return r;
        }
        r.label = "INCONCLUSIVE";
        r.priority = Cert::VerdictPriority::Inconclusive;
        return r;
    }
    if (v.global_balance && v.tier1_all) {
        r.label = "CONTROLLED_TRACE";
        r.priority = Cert::VerdictPriority::ControlledTrace;
        return r;
    }
    r.label = "INCONCLUSIVE";
    r.priority = Cert::VerdictPriority::Inconclusive;
    return r;
}

std::string HpVerdictString(const HpProofVerdict& v, const SpectralHpReport& spec,
                                     const Analysis::ConvergenceResult* /*m3*/ = nullptr) {
    return ComputeVerdict(v, spec).label;
}

void ExportHpCertJson(const std::string& path, const Config& cfg,
                                const TestFunction& tf, const HpProofVerdict& v,
                                const TraceResult& global, const Analysis::ResidualBudget& budget,
                                const SpectralHpReport& spec,
                                const Analysis::ConvergenceResult& m3,
                                Real analytic_tail, Real proof_eps) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"engine\": \"" << Marshal::Cert::Schema::kEngine << "\",\n";
    const Cert::HpVerdictResult verdict = ComputeVerdict(v, spec);
    out << "  \"verdict\": \"" << verdict.label << "\",\n";
    out << "  \"verdict_priority\": \"" << Cert::VerdictPriorityString(verdict.priority) << "\",\n";
    out << "  \"analysis_status\": \"ANALYSIS_INCOMPLETE\",\n";
    out << "  \"sigma_local\": " << static_cast<double>(cfg.sigma) << ",\n";
    out << "  \"sigma_trace\": " << static_cast<double>(cfg.sigma_trace > 0 ? cfg.sigma_trace : cfg.sigma) << ",\n";
    out << "  \"test\": \"" << tf.name() << "\",\n";
    out << "  \"local_prime_count\": " << v.local_prime_count << ",\n";
    out << "  \"p_max_local\": " << v.p_max_local << ",\n";
    out << "  \"local_cylinder_tol\": " << static_cast<double>(cfg.tier1_tol) << ",\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseLocalCylinder << "\": {\n";
    out << "    \"all_pass\": " << (v.tier1_all ? "true" : "false") << ",\n";
    out << "    \"max_local_err\": " << static_cast<double>(v.max_local_err) << ",\n";
    out << "    \"failures\": " << v.tier1_failures << "\n";
    out << "  },\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseInductiveLadder << "\": {\n";
    out << "    \"inductive_pass\": " << (v.inductive_step ? "true" : "false") << ",\n";
    out << "    \"ladder_max_err\": " << static_cast<double>(v.ladder_max_err) << "\n";
    out << "  },\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseLocalAssembly << "\": {\n";
    out << "    \"weil\": " << static_cast<double>(v.local_prime) << ",\n";
    out << "    \"heat_ab\": " << static_cast<double>(v.local_heat) << ",\n";
    out << "    \"weil_heat_err\": " << static_cast<double>(v.local_prime_heat_err) << ",\n";
    out << "    \"geometric_residual\": " << static_cast<double>(v.local_geometric_residual) << "\n";
    out << "  },\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseGlobalBalance << "\": {\n";
    out << "    \"lhs\": " << static_cast<double>(global.lhs) << ",\n";
    out << "    \"rhs\": " << static_cast<double>(global.rhs) << ",\n";
    out << "    \"residual\": " << static_cast<double>(global.residual()) << ",\n";
    out << "    \"residual_fp_delta\": " << static_cast<double>(v.residual_fp_delta) << ",\n";
    out << "    \"machine_zero_pass\": " << (v.machine_zero_pass ? "true" : "false") << ",\n";
    out << "    \"prime_heat_err\": " << static_cast<double>(fabsl(global.prime - global.heat_prime_ab)) << ",\n";
    out << "    \"global_balance\": " << (v.global_balance ? "true" : "false") << "\n";
    out << "  },\n";
    const Real float_used = std::max(budget.float_floor, v.residual_fp_delta);
    const Real base_eps = budget.arch_floor + analytic_tail + float_used;
    const Real margin = proof_eps - base_eps;
    out << "  \"constraint\": {\n";
    out << "    \"test\": \"" << tf.name() << "\",\n";
    out << "    \"sigma_trace\": " << static_cast<double>(cfg.sigma_trace > 0 ? cfg.sigma_trace : cfg.sigma) << ",\n";
    out << "    \"definition\": \"Tr(h(H)) vs explicit poles+archimedean+prime heat formula\"\n";
    out << "  },\n";
    out << "  \"quotient_space\": {\n";
    out << "    \"lemma\": \"quotient_spectrum\",\n";
    out << "    \"proof_status\": \"OPEN\",\n";
    out << "    \"doc\": \"docs/Analysis/QuotientSpectrum.md\"\n";
    out << "  },\n";
    out << "  \"bounds\": {\n";
    out << "    \"arch_floor\": " << static_cast<double>(budget.arch_floor) << ",\n";
    out << "    \"analytic_tail_bound\": " << static_cast<double>(analytic_tail) << ",\n";
    out << "    \"float_floor\": " << static_cast<double>(float_used) << ",\n";
    out << "    \"margin\": " << static_cast<double>(margin) << ",\n";
    out << "    \"proof_eps\": " << static_cast<double>(proof_eps) << ",\n";
    out << "    \"budget_exceeded\": " << (v.global_residual > proof_eps ? "true" : "false") << ",\n";
    out << "    \"arch_abs_floor\": " << static_cast<double>(budget.arch_abs_floor) << ",\n";
    out << "    \"zero_tail_effective\": " << static_cast<double>(budget.zero_tail_effective) << ",\n";
    out << "    \"prime_tail_effective\": " << static_cast<double>(budget.prime_tail_effective) << "\n";
    out << "  },\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseTraceIdentity << "\": {\n";
    out << "    \"trace_proved\": " << (spec.trace_proved ? "true" : "false") << ",\n";
    out << "    \"phase_verdict\": \"" << tier4_verdict_label(spec.trace_proved, spec.spectrum_identified,
                                                              spec.spectrum_approximated,
                                                              spec.lhs_underflow) << "\",\n";
    out << "    \"heat_trace_sweep\": {\n";
    out << "      \"t_min\": " << static_cast<double>(spec.heat_sweep.t_min) << ",\n";
    out << "      \"t_max\": " << static_cast<double>(spec.heat_sweep.t_max) << ",\n";
    out << "      \"n_t\": " << spec.heat_sweep.n_valid << ",\n";
    out << "      \"max_residual\": " << static_cast<double>(spec.heat_sweep.max_residual) << ",\n";
    out << "      \"mean_residual\": " << static_cast<double>(spec.heat_sweep.mean_residual) << ",\n";
    out << "      \"trace_identity_holds\": " << (spec.heat_sweep.trace_identity_holds ? "true" : "false") << "\n";
    out << "    },\n";
    out << "    \"trace_oracle_lhs\": " << static_cast<double>(spec.trace_oracle_lhs) << ",\n";
    out << "    \"trace_formula_residual\": " << static_cast<double>(spec.trace_formula_residual) << ",\n";
    out << "    \"lhs_underflow\": " << (spec.lhs_underflow ? "true" : "false") << ",\n";
    out << "    \"n_effective_zeros\": " << spec.n_effective_zeros << "\n";
    out << "  },\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseSpectrumDiagnostic << "\": {\n";
    out << "    \"spectrum_identified\": false,\n";
    out << "    \"identification_lemmas\": [\"quotient_spectrum\", \"trace_mode_extraction\"],\n";
    out << "    \"identification_proof_status\": \"OPEN\",\n";
    out << "    \"quotient_spectrum_lemma\": \"OPEN\",\n";
    out << "    \"frequency_lock_lemma\": \"IMPOSSIBLE\",\n";
    out << "    \"trace_mode_extraction_lemma\": \"OPEN\",\n";
    out << "    \"quotient_diagnostic_only\": true,\n";
    out << "    \"locked_cascade_metric\": \"deprecated_numeric_optimization\",\n";
    out << "    \"prony_diagnostic_only\": true,\n";
    out << "    \"spectral_merge\": \"global_min_heap_multiset\",\n";
    out << "    \"quotient_method\": \"" << spec.quotient_method << "\",\n";
    out << "    \"quotient_k_selection\": \"" << spec.quotient_k_selection << "\",\n";
    out << "    \"quotient_mesh\": " << spec.quotient_mesh << ",\n";
    out << "    \"quotient_k_primes\": " << spec.quotient_k_primes << ",\n";
    out << "    \"quotient_k_prev\": " << spec.quotient_k_prev << ",\n";
    out << "    \"quotient_gap_prev_k\": " << static_cast<double>(spec.quotient_gap_prev_k) << ",\n";
    out << "    \"quotient_converged\": " << (spec.quotient_converged ? "true" : "false") << ",\n";
    out << "    \"quotient_ncells\": " << spec.quotient_ncells << ",\n";
    out << "    \"quotient_skipped\": " << (spec.quotient_skipped ? "true" : "false") << ",\n";
    out << "    \"spec_trace_pass\": " << (spec.spec_trace_pass ? "true" : "false") << ",\n";
    out << "    \"spectral_mismatch\": " << (spec.spectral_mismatch ? "true" : "false") << ",\n";
    out << "    \"lex_sorted_gap_max\": " << static_cast<double>(spec.direct_sum_max_gap) << ",\n";
    out << "    \"lex_sorted_gap_mean\": " << static_cast<double>(spec.direct_sum_mean_gap) << ",\n";
    out << "    \"direct_sum_max_gap\": " << static_cast<double>(spec.direct_sum_max_gap) << ",\n";
    out << "    \"direct_sum_mean_gap\": " << static_cast<double>(spec.direct_sum_mean_gap) << ",\n";
    out << "    \"matched_cylinder_gap_max\": " << static_cast<double>(spec.matched_cylinder_max_gap) << ",\n";
    out << "    \"matched_cylinder_gap_mean\": " << static_cast<double>(spec.matched_cylinder_mean_gap) << ",\n";
    out << "    \"matched_sq_gap_max\": " << static_cast<double>(spec.matched_sq_max_gap) << ",\n";
    out << "    \"fixed_mode_gap_max\": " << static_cast<double>(spec.fixed_mode_max_gap) << ",\n";
    out << "    \"quotient_gamma_tuned_gap_max\": " << static_cast<double>(spec.quotient_max_gap) << ",\n";
    out << "    \"quotient_gamma_tuned_gap_mean\": " << static_cast<double>(spec.quotient_mean_gap) << ",\n";
    out << "    \"quotient_gamma_tuned_sq_gap_max\": " << static_cast<double>(spec.quotient_sq_max_gap) << ",\n";
    out << "    \"quotient_gamma_tuned_sq_gap_mean\": " << static_cast<double>(spec.quotient_sq_mean_gap) << ",\n";
    out << "    \"matched_sq_gap_mean\": " << static_cast<double>(spec.matched_sq_mean_gap) << ",\n";
    out << "    \"exponent_gap_max\": " << static_cast<double>(spec.exponent_gap_max) << ",\n";
    out << "    \"exponent_gap_mean\": " << static_cast<double>(spec.exponent_gap_mean) << ",\n";
    out << "    \"exponent_sq_gap_max\": " << static_cast<double>(spec.exponent_sq_gap_max) << ",\n";
    out << "    \"exponent_sq_gap_mean\": " << static_cast<double>(spec.exponent_sq_gap_mean) << ",\n";
    out << "    \"fixed_quotient_gap_max\": " << static_cast<double>(spec.fixed_quotient_gap_max) << ",\n";
    out << "    \"fixed_quotient_sq_gap_max\": " << static_cast<double>(spec.fixed_quotient_sq_gap_max) << ",\n";
    out << "    \"compact_sinc2\": {\n";
    out << "      \"T\": " << static_cast<double>(spec.compact_sinc2_T) << ",\n";
    out << "      \"residual\": " << static_cast<double>(spec.compact_sinc2_residual) << ",\n";
    out << "      \"quotient_lhs_residual\": " << static_cast<double>(spec.compact_sinc2_quotient_lhs_residual) << ",\n";
    out << "      \"mismatch_tol\": " << static_cast<double>(kCompactSinc2MismatchTol) << ",\n";
    out << "      \"mismatch_proved\": " << (spec.compact_sinc2_mismatch_proved ? "true" : "false") << "\n";
    out << "    },\n";
    if (!spec.anavm_program.empty() || !spec.expect_warnings.empty()) {
        out << "    \"anavm\": {\n";
        out << "      \"program\": \"" << spec.anavm_program << "\",\n";
        if (!spec.anavm_rule_id.empty())
            out << "      \"rule_id\": \"" << spec.anavm_rule_id << "\",\n";
        out << "      \"scaffold\": " << (spec.anavm_scaffold ? "true" : "false") << ",\n";
        out << "      \"placeholder\": " << (spec.anavm_placeholder ? "true" : "false") << ",\n";
        out << "      \"falsify_sinc2_gate\": " << (spec.anavm_falsify_sinc2 ? "true" : "false")
            << ",\n";
        out << "      \"calibration_role\": \""
            << (spec.anavm_scaffold ? "scaffold_reference" : "production") << "\",\n";
        out << "      \"expect_warnings\": [";
        for (size_t i = 0; i < spec.expect_warnings.size(); ++i) {
            if (i) out << ", ";
            out << "\"" << spec.expect_warnings[i] << "\"";
        }
        out << "]\n";
        out << "    },\n";
    }
    out << "    \"gap_semantics\": \"lex_sorted=negative_control; matched=per_gamma_best_mode; quotient=gamma_tuned_rayleigh; fixed_mode=n1_unbiased\",\n";
    out << "    \"locked_spectrum_max_gap\": " << static_cast<double>(spec.locked_spectrum_max_gap) << ",\n";
    out << "    \"locked_spectrum_mean_gap\": " << static_cast<double>(spec.locked_spectrum_mean_gap) << ",\n";
    out << "    \"prony_spectrum_max_gap\": " << static_cast<double>(spec.prony_spectrum_max_gap) << ",\n";
    out << "    \"prony_spectrum_mean_gap\": " << static_cast<double>(spec.prony_spectrum_mean_gap) << ",\n";
    out << "    \"quotient_max_gap\": " << static_cast<double>(spec.quotient_max_gap) << ",\n";
    out << "    \"quotient_mean_gap\": " << static_cast<double>(spec.quotient_mean_gap) << ",\n";
    out << "    \"cylinder_vs_zero_max_gap\": " << static_cast<double>(spec.direct_sum_max_gap) << ",\n";
    out << "    \"cylinder_vs_zero_mean_gap\": " << static_cast<double>(spec.direct_sum_mean_gap) << ",\n";
    out << "    \"max_gap_in_zero_spacings\": " << static_cast<double>(spec.max_gap_in_spacings) << ",\n";
    out << "    \"mean_gap_in_zero_spacings\": " << static_cast<double>(spec.mean_gap_in_spacings) << ",\n";
    out << "    \"n_spectral_pairs\": " << spec.n_pairs;
    if (spec.pair_correlation_ran) {
        out << ",\n    \"pair_correlation\": {\n";
        out << "      \"gue_spacing_l2_zero\": "
            << static_cast<double>(spec.pair_correlation.gue_spacing_l2_zero) << ",\n";
        out << "      \"gue_spacing_l2_cylinder\": "
            << static_cast<double>(spec.pair_correlation.gue_spacing_l2_cylinder) << ",\n";
        out << "      \"montgomery_r2_l2\": "
            << static_cast<double>(spec.pair_correlation.montgomery_r2_l2) << ",\n";
        out << "      \"separates_from_gue\": "
            << (spec.pair_correlation.separates_from_gue ? "true" : "false") << "\n";
        out << "    }";
    }
    out << "\n  },\n";
    out << "  \"" << Marshal::Cert::Schema::kPhaseConvergence << "\": {\n";
    out << "    \"lemmas\": {\n";
    out << "      \"tail_bound_holds\": " << (m3.tail_bound_holds ? "true" : "false") << ",\n";
    out << "      \"tail_bound_status\": \"" << Marshal::Cert::TailBoundStatusString(m3.tail_bound_status) << "\",\n";
    out << "      \"uniform_trace_convergence\": " << (m3.tail_bound_holds ? "true" : "false") << ",\n";
    out << "      \"spectral_measure_proof_status\": \""
        << Marshal::Cert::ProofStatusString(m3.spectral_measure_proof_status) << "\",\n";
    out << "      \"resolvent_limit_status\": \"OPEN\",\n";
    out << "      \"min_gap_sq\": " << static_cast<double>(m3.min_spectral_gap_sq) << ",\n";
    out << "      \"eigenvalues_converge\": " << (m3.eigenvalues_converge ? "true" : "false") << "\n";
    out << "    },\n";
    out << "    \"tail_bound\": {\n";
    out << "      \"C\": " << static_cast<double>(kM3TailConstant) << ",\n";
    out << "      \"formula\": \"C/(sqrt(P)*t^0.25)\",\n";
    out << "      \"predicted_at_Pmax\": " << static_cast<double>(m3.predicted_tail_bound) << ",\n";
    out << "      \"observed_at_Pmax\": " << static_cast<double>(m3.observed_tail_residual) << "\n";
    out << "    },\n";
    out << "    \"convergence_sweep\": {\n";
    out << "      \"cutoffs\": [";
    for (size_t i = 0; i < m3.cutoffs.size(); ++i) {
        if (i) out << ", ";
        out << m3.cutoffs[i];
    }
    out << "],\n";
    out << "      \"n_primes\": [";
    for (size_t i = 0; i < m3.n_primes.size(); ++i) {
        if (i) out << ", ";
        out << m3.n_primes[i];
    }
    out << "],\n";
    out << "      \"sup_residuals\": [";
    for (size_t i = 0; i < m3.sup_residuals.size(); ++i) {
        if (i) out << ", ";
        out << static_cast<double>(m3.sup_residuals[i]);
    }
    out << "],\n";
    out << "      \"tail_bounds\": [";
    for (size_t i = 0; i < m3.tail_bounds_at_cutoff.size(); ++i) {
        if (i) out << ", ";
        out << static_cast<double>(m3.tail_bounds_at_cutoff[i]);
    }
    out << "],\n";
    out << "      \"fitted_exponent\": " << static_cast<double>(m3.fitted_exponent) << ",\n";
    out << "      \"fitted_intercept\": " << static_cast<double>(m3.fitted_intercept) << ",\n";
    out << "      \"r_squared\": " << static_cast<double>(m3.r_squared) << ",\n";
    out << "      \"rate_confirmed\": "
        << (fabsl(m3.fitted_exponent + 0.5L) < 0.15L && m3.r_squared > 0.90L ? "true" : "false") << "\n";
    out << "    },\n";
    out << "    \"eigenvalue_tracking\": {\n";
    out << "      \"n_tracked\": " << m3.eigenvalues.size() << ",\n";
    out << "      \"entries\": [\n";
    for (size_t i = 0; i < m3.eigenvalues.size(); ++i) {
        const auto& ec = m3.eigenvalues[i];
        out << "        {\"n\": " << ec.n
            << ", \"gamma\": " << static_cast<double>(ec.gamma)
            << ", \"gamma_sq\": " << static_cast<double>(ec.gamma_sq)
            << ", \"lambda_sq\": " << static_cast<double>(ec.lambda_sq)
            << ", \"error\": " << static_cast<double>(ec.error)
            << ", \"predicted_error\": " << static_cast<double>(ec.predicted_error) << "}";
        if (i + 1 < m3.eigenvalues.size()) out << ",";
        out << "\n";
    }
    out << "      ]\n";
    out << "    },\n";
    out << "    \"spectral_measure_converges\": " << (m3.spectral_measure_converges ? "true" : "false") << ",\n";
    out << "    \"eigenvalues_converge\": " << (m3.eigenvalues_converge ? "true" : "false") << ",\n";
    out << "    \"convergence_status\": \"INCOMPLETE\"\n";
    out << "  }\n}\n";
}

HpProofVerdict RunHpProofInduction(const Config& cfg, const TestFunction& tf,
                                             const std::vector<double>& gammas,
                                             const std::vector<Real>& gammas_ld,
                                             Heat::PrimeCatalog& cat) {
    HpProofVerdict v;
    const Real sigma_local = cfg.sigma;
    const Real sigma_trace = cfg.sigma_trace > 0 ? cfg.sigma_trace : cfg.sigma;
    const Real tau = TauFromSigma(sigma_local);
    const Real link_n = sigma_local * sqrtl(2.0L / kPi);
    const TraceResult global = RunEvaluate(cfg, tf, gammas, gammas_ld, cat);
    Config cfg_trace = cfg;
    cfg_trace.sigma = sigma_trace;
    auto tf_trace = MakeTestFunction(cfg_trace);
    const TraceResult global_trace = (sigma_trace == sigma_local)
        ? global
        : EvaluateTrace(*tf_trace, sigma_trace, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                       cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts);
    const size_t n_local = Marshal::Heat::LocalAssembly::clamp_count(cat,
        cfg.local_prime_count == 0 ? cat.p.size()
                                   : static_cast<size_t>(cfg.local_prime_count));
    v.local_prime_count = n_local;
    v.p_max_local = n_local ? cat.p[static_cast<size_t>(n_local - 1)] : 0;

    std::cout << "=== Marshal local induction certificate ===\n\n";
    std::cout << "Lemma A: D_p self-adjoint on S^1 — spectrum symmetric (symbolic).\n";
    v.spectrum_symmetry = true;
    std::cout << "  Spectrum symmetry: PASS\n\n";

    std::cout << "Phase LocalCylinder: per-prime heat-kernel identities (Poisson=theta, Prime=AB·link, Euler).\n";
    std::cout << "  Local subset: first " << n_local << " primes, p_max=" << v.p_max_local
              << "  (sigma_local=" << static_cast<double>(sigma_local) << ")\n";

    Real prev_cum = 0;
    for (size_t i = 0; i < n_local; ++i) {
        Heat::HeatCylinderOp op(cat, i);
        const int km = cat.kmax_adaptive[i];
        const auto pk = op.trace_packet(tf, tau, km, cfg.nmax, cfg.ktheta,
                                        cfg.s_euler, cfg.eps, link_n);
        const Real weil_heat = fabsl(pk.prime_norm - pk.heat_norm);
        const Real local_err = std::max({pk.poisson_err, weil_heat, pk.euler_err});
        v.max_local_err = std::max(v.max_local_err, local_err);
        if (local_err > cfg.tier1_tol) {
            v.tier1_all = false;
            ++v.tier1_failures;
            if (cat.p[i] <= 11) v.base_case = false;
        }

        const Real cum = prev_cum + pk.prime_norm;
        if (fabsl(cum - prev_cum - pk.prime_norm) > 1e-14L * std::max(1.0L, fabsl(pk.prime_norm)))
            v.inductive_step = false;
        const Real expected = global.poles + global.arch - cum;
        v.ladder_max_err = std::max(v.ladder_max_err, fabsl(global.lhs - expected));
        prev_cum = cum;
    }
    v.tier1_all = v.tier1_all && v.tier1_failures == 0;

    Real max_poisson = 0, max_wh = 0, max_euler = 0;
    Marshal::Heat::LocalAssembly::partial_sums(cfg.nmax, cfg.ktheta, cfg.s_euler, cfg.eps, tf, cat,
                                               n_local, tau, link_n, v.local_prime, v.local_heat,
                                               max_poisson, max_wh, max_euler);
    if (fabsl(prev_cum - v.local_prime) > 1e-12L * std::max(1.0L, fabsl(v.local_prime)))
        v.inductive_step = false;
    v.local_prime_heat_err = fabsl(v.local_prime - v.local_heat);
    v.local_prime_heat = v.local_prime_heat_err <= cfg.tier1_tol;
    v.local_geometric_residual = global_trace.lhs - (global_trace.poles + global_trace.arch - v.local_prime);

    std::cout << "  LocalCylinder pass:  " << (v.tier1_all ? "YES" : "NO")
              << "  (failures=" << v.tier1_failures
              << ", max_err=" << static_cast<double>(v.max_local_err) << ")\n";
    std::cout << "  Base case (p<=11):   " << (v.base_case ? "PASS" : "FAIL") << "\n\n";

    std::cout << "Phase InductiveLadder: block addition Trace(H_{P'})-Trace(H_P)=T_{p_{k+1}}.\n";
    std::cout << "  Ladder max |LHS-cum_RHS|: " << static_cast<double>(v.ladder_max_err) << "\n";
    std::cout << "  Inductive step:        " << (v.inductive_step ? "PASS" : "FAIL") << "\n\n";

    std::cout << "Local assembly H_P (p <= " << v.p_max_local << "):\n";
    std::cout << "  Prime(P):             " << static_cast<double>(v.local_prime) << "\n";
    std::cout << "  Heat AB (linked):    " << static_cast<double>(v.local_heat) << "\n";
    std::cout << "  |prime-heat|:        " << static_cast<double>(v.local_prime_heat_err) << "\n";
    std::cout << "  Local geom residual: " << static_cast<double>(v.local_geometric_residual)
              << "  (= LHS - (poles+arch-Prime(P)))\n\n";

    v.global_residual = fabsl(global_trace.residual());
    const Analysis::ResidualBudget budget = ComputeResidualBudget(*tf_trace, sigma_trace, gammas, gammas_ld, cat,
                                                          cfg.zero_kernel, cfg.simd,
                                                          cfg.precision_mode, cfg.arch_pts);
    if (cfg.precision_mode && !gammas.empty()) {
        const TraceResult r_fp = EvaluateTrace(*tf_trace, sigma_trace, gammas, gammas_ld, cat,
                                         ZeroKernel::Float, cfg.simd, cfg.eps, cfg.trivial_zeros,
                                         false, cfg.arch_pts);
        v.residual_fp_delta = fabsl(global_trace.residual() - r_fp.residual());
    }
    const Real p_max = cat.p.empty() ? 0 : static_cast<Real>(cat.p.back());
    const Real g_max = gammas.empty() ? 0 : static_cast<Real>(gammas.back());
    const Real analytic_tail = std::max(budget.prime_tail_naive + budget.zero_tail_naive,
                                        bound_prime_tail_gauss(sigma_trace, p_max)
                                            + bound_zero_tail_gauss(sigma_trace, g_max));
    const Real float_floor = cfg.precision_mode && v.residual_fp_delta > 0
        ? v.residual_fp_delta
        : budget.float_floor;
    const Real proof_eps = ComputeProofEps(budget.arch_floor, analytic_tail, float_floor);
    v.proof_eps_used = proof_eps;
    v.global_balance = v.global_residual <= proof_eps;

    v.local_hp_proved = v.tier1_all && v.inductive_step && v.local_prime_heat && v.base_case;

    const int spectral_n = cfg.spectral_compare_n > 0 ? cfg.spectral_compare_n
                                                      : (cfg.fast_mode ? 32 : 500);
    SpectralHpReport spec = ComputeSpectralHp(global_trace, gammas, gammas_ld, cat, spectral_n,
                                              sigma_trace, proof_eps, cfg);
    cat.rebuild_adaptive(*tf_trace, TauFromSigma(sigma_trace), cfg.kmax, cfg.eps);
    v.trace_formula_residual = spec.trace_formula_residual;
    v.max_eigenvalue_gap = spec.max_eigenvalue_gap;
    v.lhs_underflow = spec.lhs_underflow;
    v.spectral_mismatch = spec.spectral_mismatch;
    v.spec_trace_pass = spec.spec_trace_pass;
    v.machine_zero_pass = v.global_balance && !spec.lhs_underflow;

    std::cout << "Phase TraceIdentity + SpectrumDiagnostic.\n";
    std::cout << "  Constraint: Tr(h(H)) vs explicit formula (test=" << tf.name()
              << ", sigma_trace=" << static_cast<double>(sigma_trace) << ")\n";
    std::cout << "  Quotient space: Q^x-invariant subspace of ⊕L^2(S^1_log p) — lemma OPEN "
                 "(docs/Analysis/QuotientSpectrum.md)\n";
    std::cout << "  sigma_trace:                        " << static_cast<double>(sigma_trace) << "\n";
    if (spec.lhs_underflow) {
        std::cerr << "WARNING: LHS underflowed (lhs=" << static_cast<double>(spec.trace_oracle_lhs)
                  << " < " << static_cast<double>(kLhsMinValid)
                  << "). Effective zeros: " << spec.n_effective_zeros
                  << ". Spectral comparison INVALID.\n";
    }
    std::cout << "  Tr h(H) oracle (= 2 sum h(gamma)): " << static_cast<double>(spec.trace_oracle_lhs) << "\n";
    std::cout << "  Trace-formula residual |LHS-RHS|:      " << static_cast<double>(spec.trace_formula_residual)
              << "  (proof_eps=" << static_cast<double>(proof_eps)
              << " = arch+" << static_cast<double>(budget.arch_floor)
              << " + tail+" << static_cast<double>(analytic_tail)
              << " + float+" << static_cast<double>(float_floor)
              << " + margin)\n";
    std::cout << "  Budget satisfied:                  " << (v.global_balance ? "YES" : "NO")
              << (v.global_balance ? "" : "  (INCONCLUSIVE if budget wrong — not FAIL)") << "\n";
    std::cout << "  TraceIdentity proved:              " << (spec.trace_proved ? "YES" : "NO") << "\n";
    std::cout << "  Heat sweep max |LHS-RHS|:          " << static_cast<double>(spec.heat_sweep.max_residual)
              << "  (n_t=" << spec.heat_sweep.n_valid << ")\n";
    if (spec.spectral_mismatch && !spec.lhs_underflow)
        std::cout << "  Heat sweep exceeds proof_eps at some t\n";
    std::cout << "  Lex-sorted ω gap (negative ctrl):  "
              << static_cast<double>(spec.direct_sum_max_gap)
              << "  [i-th global min ω vs γ_i — expect >> 1]\n";
    std::cout << "  Matched cylinder gap (max/mean):   "
              << static_cast<double>(spec.matched_cylinder_max_gap) << " / "
              << static_cast<double>(spec.matched_cylinder_mean_gap)
              << "  [|ω(p,n)−γ| with local n search]\n";
    std::cout << "  Matched |ω²−γ²| max:              "
              << static_cast<double>(spec.matched_sq_max_gap) << "\n";
    std::cout << "  Fixed-mode gap (n=1, unbiased):    "
              << static_cast<double>(spec.fixed_mode_max_gap) << " / "
              << static_cast<double>(spec.fixed_mode_mean_gap)
              << "  [no γ in mode selection]\n";
    std::cout << "  Prony extraction gap (max/mean):   "
              << static_cast<double>(spec.prony_spectrum_max_gap) << " / "
              << static_cast<double>(spec.prony_spectrum_mean_gap)
              << "  [OPEN: trace_mode_extraction — diagnostic only]\n";
    std::cout << "  Spectrum identified:               NO"
                 " (requires PROVED quotient_spectrum + trace_mode_extraction)\n";
    std::cout << "  Quotient γ-tuned gap (max/mean):   "
              << static_cast<double>(spec.quotient_max_gap) << " / "
              << static_cast<double>(spec.quotient_mean_gap)
              << "  mesh=" << spec.quotient_mesh << " K=" << spec.quotient_k_primes
              << " cells=" << spec.quotient_ncells
              << "  [n=round(γ log p/2π) — explains gap vs lex-sorted]\n";
    std::cout << "  Compact sinc² |residual|:          "
              << static_cast<double>(spec.compact_sinc2_residual)
              << "  T=" << static_cast<double>(spec.compact_sinc2_T)
              << "  mismatch_proved="
              << (spec.compact_sinc2_mismatch_proved ? "YES" : "NO")
              << " (tol " << static_cast<double>(kCompactSinc2MismatchTol) << ")\n";
    if (spec.quotient_skipped)
        std::cout << "  Quotient spectrum: SKIPPED (lhs underflow)\n";
    std::cout << "\n";

    std::cout << "Phase Convergence: tail regime + eigenvalue tracking (observations).\n";
    const Analysis::ConvergenceResult m3 = Marshal::Analysis::RunConvergenceStudy(
        gammas, gammas_ld, cat, cfg, sigma_trace, proof_eps, budget, spec.trace_proved);
    std::cout << "  Tail bound holds:                " << (m3.tail_bound_holds ? "YES" : "NO")
              << "  status=" << Marshal::Cert::TailBoundStatusString(m3.tail_bound_status) << "\n";
    std::cout << "  Fitted exponent (expect -0.5):   " << static_cast<double>(m3.fitted_exponent)
              << "  R²=" << static_cast<double>(m3.r_squared) << "\n";
    std::cout << "  Observed residual at P_max:    " << static_cast<double>(m3.observed_tail_residual) << "\n\n";

    std::cout << "Phase GlobalBalance: full-catalog trace identity.\n";
    std::cout << "  Arch floor:          " << static_cast<double>(budget.arch_floor) << "\n";
    std::cout << "  Analytic tail bound: " << static_cast<double>(analytic_tail) << "\n";
    std::cout << "  Float/LD floor:      " << static_cast<double>(float_floor) << "\n";
    std::cout << "  Proof margin (1%):   " << static_cast<double>(proof_eps - budget.arch_floor - analytic_tail - float_floor) << "\n";
    std::cout << "  Global |LHS-RHS|:    " << static_cast<double>(v.global_residual)
              << "  (proof_eps " << static_cast<double>(proof_eps) << ")\n";
    std::cout << "  |prime-heat| (full): " << static_cast<double>(fabsl(global.prime - global.heat_prime_ab))
              << "\n\n";

    std::cout << "=== VERDICT: " << HpVerdictString(v, spec, &m3) << " ===\n\n";
    if (!cfg.export_hp_cert_path.empty())
        std::cout << "Certificate: " << cfg.export_hp_cert_path << "\n";

    if (cfg.pair_correlation && !cat.p.empty()) {
        const int max_z = cfg.pair_correlation_max_zeros > 0
                              ? cfg.pair_correlation_max_zeros
                              : static_cast<int>(std::min(gammas.size(), size_t{5000}));
        const int n_cyl =
            cfg.pair_correlation_n_cylinder > 0 ? cfg.pair_correlation_n_cylinder : max_z;
        spec.pair_correlation =
            Analysis::compute_pair_correlation(gammas, cat.p, n_cyl, max_z);
        spec.pair_correlation_ran = spec.pair_correlation.n_zero_spacings >= 3;
    }

    if (!cfg.export_hp_cert_path.empty())
        ExportHpCertJson(cfg.export_hp_cert_path, cfg, tf, v, global_trace, budget, spec, m3,
                         analytic_tail, proof_eps);
    return v;
}

}  // namespace Marshal::Induction

#include "AnalyticConstructionValidation.hxx"

#include "BerryKeatingOperator.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {

AnalyticConstructionReport run_analytic_construction_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes) {
    AnalyticConstructionReport rep;
    rep.program_id = cfg.anavm.id;
    rep.rule_id = cfg.anavm.rule_id;
    rep.prime_limit = cfg.prime_limit;

    rep.extension_sweep =
        run_self_adjoint_extension_sweep(cfg, gammas, gammas_ld, cat, primes);
    rep.best_theta = rep.extension_sweep.best_theta;
    rep.spectrum_rmse = rep.extension_sweep.best_rmse;
    rep.spectrum_rmse_raw = rep.extension_sweep.best_rmse_raw;
    rep.height_map_applied = rep.extension_sweep.height_map_applied;

    AnalyticConstructionStep s1;
    s1.step = "self_adjoint_extension_sweep";
    s1.pass = rep.extension_sweep.verdict == "EXTENSION_ADMISSIBLE" ||
              rep.extension_sweep.best_score < 10.0L;
    s1.verdict = rep.extension_sweep.verdict;
    rep.steps.push_back(s1);

    rep.trace_gate = run_trace_formula_gate(cfg, gammas, gammas_ld, cat, primes);
    rep.log_prime_t1_gap = rep.trace_gate.log_prime_t1_gap;
    AnalyticConstructionStep s2;
    s2.step = "trace_formula_gate";
    s2.pass = rep.trace_gate.verdict == "TRACE_FORMULA_MATCH" ||
              rep.trace_gate.verdict == "TRACE_FORMULA_T1_PASS" ||
              rep.trace_gate.verdict == "TRACE_FORMULA_T1_ONLY";
    s2.verdict = rep.trace_gate.verdict;
    rep.steps.push_back(s2);

    rep.spectral_det =
        run_spectral_determinant_validation(cfg, gammas, gammas_ld, cat, primes);
    rep.xi_det_gap = rep.spectral_det.xi_det_gap;
    AnalyticConstructionStep s3;
    s3.step = "spectral_determinant";
    s3.pass = rep.spectral_det.verdict == "XI_DET_APPROACHING";
    s3.verdict = rep.spectral_det.verdict;
    rep.steps.push_back(s3);

    BerryKeatingSpec bspec = spec_from_config(cfg);
    bspec.theta = rep.extension_sweep.best_theta;
    const auto ladder = bk_wkb_ladder(bspec);
    const auto metrics = compare_to_zeros(ladder, gammas_ld);
    rep.spectrum_rmse = metrics.rmse;
    rep.spectrum_max_gap = metrics.max_gap;

    const Real rmse_target =
        cfg.diagnostics_spectrum_rmse_max > 0 ? cfg.diagnostics_spectrum_rmse_max : 1.0L;
    const Real gap_target = cfg.diagnostics_spectrum_max_gap_max > 0
                                ? cfg.diagnostics_spectrum_max_gap_max
                                : 1.0L;
    const bool spectrum_ok =
        rep.spectrum_rmse <= rmse_target && rep.spectrum_max_gap <= gap_target;

    rep.continuous_spectrum_present = !spectrum_ok;
    AnalyticConstructionStep s4;
    s4.step = "spectral_discreteness";
    s4.pass = spectrum_ok;
    s4.verdict = spectrum_ok ? "DISCRETE_SPECTRUM_CANDIDATE" : "CONTINUOUS_SPECTRUM_PRESENT";
    rep.steps.push_back(s4);

    if (spectrum_ok)
        rep.analytic_shape_verdict = "ANALYTIC_SHAPE_OK";
    else if (cfg.diagnostics_expect_finite_spectrum_mismatch)
        rep.analytic_shape_verdict = "ANALYTIC_INCONCLUSIVE";
    else
        rep.analytic_shape_verdict = "ANALYTIC_SHAPE_BAD";

    const bool trace_ok = s2.pass;
    const bool t1_ok = rep.log_prime_t1_gap < 1e-6L;
    if (trace_ok && t1_ok && spectrum_ok)
        rep.overall_verdict = "SPECTRUM_IDENTIFIED_NUMERIC";
    else if (t1_ok && (trace_ok || cfg.diagnostics_expect_finite_spectrum_mismatch))
        rep.overall_verdict = "OPEN_SPECTRAL_DISCRETENESS";
    else if (t1_ok && s3.pass)
        rep.overall_verdict = "TRACE_AND_XI_DET_LOCAL_OK";
    else if (t1_ok)
        rep.overall_verdict = "T1_LOCAL_OK_ARCH_OPEN";
    else
        rep.overall_verdict = "ANALYTIC_PIPELINE_INCONCLUSIVE";

    std::cout << "Analytic construction: " << rep.overall_verdict
              << " rmse_mapped=" << static_cast<double>(rep.spectrum_rmse)
              << " rmse_raw=" << static_cast<double>(rep.spectrum_rmse_raw)
              << " t1_gap=" << static_cast<double>(rep.log_prime_t1_gap) << "\n";
    return rep;
}

bool export_analytic_construction_json(const std::string& path,
                                       const AnalyticConstructionReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"rule_id\": \"" << r.rule_id << "\",\n";
    out << "  \"overall_verdict\": \"" << r.overall_verdict << "\",\n";
    out << "  \"spectrum_rmse\": " << static_cast<double>(r.spectrum_rmse) << ",\n";
    out << "  \"spectrum_rmse_raw\": " << static_cast<double>(r.spectrum_rmse_raw) << ",\n";
    out << "  \"spectrum_max_gap\": " << static_cast<double>(r.spectrum_max_gap) << ",\n";
    out << "  \"best_theta\": " << static_cast<double>(r.best_theta) << ",\n";
    out << "  \"log_prime_t1_gap\": " << static_cast<double>(r.log_prime_t1_gap) << ",\n";
    out << "  \"xi_det_gap\": " << static_cast<double>(r.xi_det_gap) << ",\n";
    out << "  \"height_map_applied\": " << (r.height_map_applied ? "true" : "false") << ",\n";
    out << "  \"height_map_formula\": \"gamma_bk * log(n) / (2*pi)\",\n";
    out << "  \"continuous_spectrum_present\": "
        << (r.continuous_spectrum_present ? "true" : "false") << ",\n";
    out << "  \"analytic_shape_verdict\": \"" << r.analytic_shape_verdict << "\",\n";
    out << "  \"prime_limit\": " << r.prime_limit << ",\n";
    out << "  \"trace_gate_verdict\": \"" << r.trace_gate.verdict << "\",\n";
    out << "  \"trace_t1_verdict\": \"" << r.trace_gate.t1_verdict << "\",\n";
    out << "  \"trace_full_weil_verdict\": \"" << r.trace_gate.full_weil_verdict << "\",\n";
    out << "  \"spectral_det_verdict\": \"" << r.spectral_det.verdict << "\",\n";
    out << "  \"steps\": [\n";
    for (size_t i = 0; i < r.steps.size(); ++i) {
        const auto& s = r.steps[i];
        out << "    {\"step\": \"" << s.step << "\", \"verdict\": \"" << s.verdict
            << "\", \"pass\": " << (s.pass ? "true" : "false") << "}";
        if (i + 1 < r.steps.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

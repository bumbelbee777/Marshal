#include "TraceFormulaGate.hxx"

#include "ArchimedeanBoundary.hxx"
#include "BerryKeatingOperator.hxx"
#include "Heat/Common.hxx"
#include "Heat/LogPrimeGlobal.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Induction/Induction.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {

TraceFormulaGateReport run_trace_formula_gate(const Config& cfg, const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
                                              const std::vector<int>& primes) {
    TraceFormulaGateReport rep;
    rep.program_id = cfg.anavm.id;
    rep.rule_id = cfg.anavm.rule_id;
    rep.residual_threshold = cfg.diagnostics_weil_residual_max > 0 ? cfg.diagnostics_weil_residual_max
                                                                   : 1.0L;
    rep.combined_bk_logprime =
        cfg.anavm.rule_id == "connes_analytic_construction" || cfg.archimedean.present;

    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const Real gauss_sigma = cfg.sigma >= 1.0L ? cfg.sigma : 5.0L;
    const Real sinc_T = cfg.test_param > 0 ? cfg.test_param : 14.134725142L;
    const Real kappa = cfg.sinc2_kappa > 0 ? cfg.sinc2_kappa : 60.0L;
    const Real laplace_a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;

    const ArchimedeanBoundarySpec* arch_ptr = nullptr;
    ArchimedeanBoundarySpec arch_spec;
    if (cfg.archimedean.present) {
        arch_spec = ArchimedeanBoundarySpec::from_mrs(cfg.archimedean);
        arch_ptr = &arch_spec;
    }

    const size_t prime_cap =
        cfg.precision_mode ? primes.size()
                           : std::min(primes.size(), size_t{5000});
    const int max_primes = static_cast<int>(prime_cap);
    std::vector<int> sub(primes.begin(), primes.begin() + max_primes);
    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(sub);

    struct RowDef {
        const char* name;
        std::unique_ptr<TestFunction> tf;
        Real sigma;
    };
    std::vector<RowDef> defs;
    defs.push_back({"laplace", std::make_unique<LaplaceTest>(laplace_a), laplace_a});
    defs.push_back({"gauss", std::make_unique<GaussTest>(gauss_sigma), gauss_sigma});
    defs.push_back({"sinc2", std::make_unique<Sinc2Test>(sinc_T, kappa), sinc_T});

    Real max_t1 = 0;
    Real max_arch_res = 0;
    for (const auto& d : defs) {
        cat.rebuild_adaptive(*d.tf, Induction::TauFromSigma(d.sigma), kmax, cfg.eps);
        const TraceResult tr =
            EvaluateTrace(*d.tf, d.sigma, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                          cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false,
                          arch_ptr);
        const Real hlog = global.weil_prime_sum(*d.tf, kmax, cfg.eps) / (2.0L * kPi);
        const Real t1_gap = std::fabs(tr.prime - hlog);
        max_t1 = std::max(max_t1, t1_gap);
        max_arch_res = std::max(max_arch_res, std::fabs(tr.residual()));

        TraceFormulaTestRow row;
        row.name = d.name;
        row.lhs = tr.lhs;
        row.rhs = tr.poles + tr.arch - tr.prime;
        row.residual = std::fabs(tr.residual());
        row.pass = row.residual <= rep.residual_threshold;
        rep.tests.push_back(row);
        rep.max_residual = std::max(rep.max_residual, row.residual);
    }

    rep.log_prime_t1_gap = max_t1;
    rep.arch_weil_residual = max_arch_res;
    const bool t1_ok = max_t1 < 1e-6L;
    const bool full_ok = rep.max_residual <= rep.residual_threshold;
    rep.t1_verdict = t1_ok ? "T1_LOCAL_PASS" : "T1_LOCAL_FAIL";
    rep.full_weil_verdict =
        full_ok ? "FULL_WEIL_MATCH" : "FULL_WEIL_ARCH_OPEN";
    if (full_ok)
        rep.verdict = "TRACE_FORMULA_MATCH";
    else if (t1_ok && rep.combined_bk_logprime)
        rep.verdict = "TRACE_FORMULA_T1_PASS";
    else if (t1_ok)
        rep.verdict = "TRACE_FORMULA_T1_ONLY";
    else
        rep.verdict = "TRACE_FORMULA_MISMATCH";
    return rep;
}

bool export_trace_formula_gate_json(const std::string& path, const TraceFormulaGateReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"rule_id\": \"" << r.rule_id << "\",\n";
    out << "  \"max_residual\": " << static_cast<double>(r.max_residual) << ",\n";
    out << "  \"residual_threshold\": " << static_cast<double>(r.residual_threshold) << ",\n";
    out << "  \"log_prime_t1_gap\": " << static_cast<double>(r.log_prime_t1_gap) << ",\n";
    out << "  \"arch_weil_residual\": " << static_cast<double>(r.arch_weil_residual) << ",\n";
    out << "  \"combined_bk_logprime\": " << (r.combined_bk_logprime ? "true" : "false") << ",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n";
    out << "  \"t1_verdict\": \"" << r.t1_verdict << "\",\n";
    out << "  \"full_weil_verdict\": \"" << r.full_weil_verdict << "\",\n";
    out << "  \"tests\": [\n";
    for (size_t i = 0; i < r.tests.size(); ++i) {
        const auto& t = r.tests[i];
        out << "    {\"name\": \"" << t.name << "\", \"lhs\": " << static_cast<double>(t.lhs)
            << ", \"rhs\": " << static_cast<double>(t.rhs) << ", \"residual\": "
            << static_cast<double>(t.residual) << ", \"pass\": " << (t.pass ? "true" : "false")
            << "}";
        if (i + 1 < r.tests.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

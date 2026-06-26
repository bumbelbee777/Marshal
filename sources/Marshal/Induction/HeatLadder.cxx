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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Induction {
// =============================================================================
// Heat trace induction (unified per-prime ladder + local checks)
// =============================================================================

struct InductionStats {
    Real max_poisson = 0, max_prime_heat = 0, max_euler = 0;
    int worst_poisson = 0, worst_weil_heat = 0, worst_euler = 0;
};

void RunHeatInduction(const Config& cfg, const TestFunction& tf,
                               const std::vector<double>& gammas,
                               const std::vector<Real>& gammas_ld,
                               const Heat::PrimeCatalog& cat) {
    const Real tau = TauFromSigma(cfg.sigma);
    const Real link_n = cfg.sigma * sqrtl(2.0L / kPi);
    const TraceResult global = RunEvaluate(cfg, tf, gammas, gammas_ld, cat);
    InductionStats st;

    std::cout << std::scientific << std::setprecision(12);
    std::cout << "Heat trace induction (test=" << tf.name()
              << ", sigma=" << static_cast<double>(cfg.sigma) << ")\n\n";

    const size_t n = cat.p.size();
    const size_t every = std::max<size_t>(1, n / 150);
    std::vector<Real> cum_prime_prefix(n);
    #ifdef _OPENMP
    #pragma omp parallel for schedule(static, kPrimeBatch)
    #endif
    for (int i = 0; i < static_cast<int>(n); ++i) {
        Heat::HeatCylinderOp op(cat, static_cast<size_t>(i));
        const int km = cat.kmax_adaptive[static_cast<size_t>(i)];
        cum_prime_prefix[static_cast<size_t>(i)] =
            op.prime_block_raw(tf, km, cfg.eps) / (2.0L * kPi);
    }
    for (size_t i = 1; i < n; ++i)
        cum_prime_prefix[i] += cum_prime_prefix[i - 1];

    for (size_t i = 0; i < n; ++i) {
        Heat::HeatCylinderOp op(cat, i);
        const int km = cat.kmax_adaptive[i];
        const auto pk = op.trace_packet(tf, tau, km, cfg.nmax, cfg.ktheta,
                                        cfg.s_euler, cfg.eps, link_n);
        const Real weil_heat_err = fabsl(pk.prime_norm - pk.heat_norm);
        if (pk.poisson_err > st.max_poisson) {
            st.max_poisson = pk.poisson_err; st.worst_poisson = cat.p[i];
        }
        if (weil_heat_err > st.max_prime_heat) {
            st.max_prime_heat = weil_heat_err; st.worst_weil_heat = cat.p[i];
        }
        if (pk.euler_err > st.max_euler) {
            st.max_euler = pk.euler_err; st.worst_euler = cat.p[i];
        }
        const Real cum_weil = cum_prime_prefix[i];
        const Real cum_rhs = global.poles + global.arch - cum_weil;
        const bool show = (i < 12) || ((i + 1) % every == 0) || (i + 1 == n);
        if (show)
            std::cout << std::setw(6) << cat.p[i]
                      << std::setw(14) << static_cast<double>(global.lhs - cum_rhs) << "\n";
    }
    PrintResult(cfg.sigma, global);
}

void ExportTraceJson(const std::string& path, const Config& cfg,
                              const TestFunction& tf, const TraceResult& r,
                              const Heat::PrimeCatalog& cat, const Analysis::ResidualBudget& b,
                              size_t n_zeros) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"sigma\": " << static_cast<double>(cfg.sigma) << ",\n";
    out << "  \"test\": \"" << tf.name() << "\",\n";
    out << "  \"poles\": " << static_cast<double>(r.poles) << ",\n";
    out << "  \"arch\": " << static_cast<double>(r.arch) << ",\n";
    out << "  \"prime\": " << static_cast<double>(r.prime) << ",\n";
    out << "  \"heat_prime_ab\": " << static_cast<double>(r.heat_prime_ab) << ",\n";
    out << "  \"prime_heat_err\": " << static_cast<double>(fabsl(r.prime - r.heat_prime_ab)) << ",\n";
    out << "  \"lhs\": " << static_cast<double>(r.lhs) << ",\n";
    out << "  \"rhs\": " << static_cast<double>(r.rhs) << ",\n";
    out << "  \"residual\": " << static_cast<double>(r.residual()) << ",\n";
    out << "  \"n_zeros\": " << n_zeros << ",\n";
    out << "  \"n_primes\": " << cat.p.size() << ",\n";
    out << "  \"bounds\": {\n";
    out << "    \"arch_floor\": " << static_cast<double>(b.arch_floor) << ",\n";
    out << "    \"arch_abs_floor\": " << static_cast<double>(b.arch_abs_floor) << ",\n";
    out << "    \"float_floor\": " << static_cast<double>(b.float_floor) << ",\n";
    out << "    \"zero_tail\": " << static_cast<double>(b.zero_tail_effective) << ",\n";
    out << "    \"prime_tail\": " << static_cast<double>(b.prime_tail_effective) << "\n";
    out << "  }\n}\n";
}

void ExportInductionJson(const std::string& path, const Config& cfg,
                                  const TestFunction& tf,
                                  const std::vector<double>& gammas,
                                  const std::vector<Real>& gammas_ld,
                                  const Heat::PrimeCatalog& cat) {
    const Real tau = TauFromSigma(cfg.sigma);
    const Real link_n = cfg.sigma * sqrtl(2.0L / kPi);
    const TraceResult global = RunEvaluate(cfg, tf, gammas, gammas_ld, cat);

    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"engine\": \"" << Marshal::Cert::Schema::kEngine << "\",\n";
    out << "  \"sigma\": " << static_cast<double>(cfg.sigma) << ",\n";
    out << "  \"test\": \"" << tf.name() << "\",\n";
    out << "  \"local_prime_count\": " << Marshal::Heat::LocalAssembly::clamp_count(cat,
        cfg.local_prime_count == 0 ? cat.p.size()
                                   : static_cast<size_t>(cfg.local_prime_count)) << ",\n";
    out << "  \"local_cylinder_tol\": " << static_cast<double>(cfg.tier1_tol) << ",\n";
    out << "  \"phase_global_balance\": {\n";
    out << "    \"lhs\": " << static_cast<double>(global.lhs) << ",\n";
    out << "    \"rhs\": " << static_cast<double>(global.rhs) << ",\n";
    out << "    \"residual\": " << static_cast<double>(global.residual()) << ",\n";
    out << "    \"poles\": " << static_cast<double>(global.poles) << ",\n";
    out << "    \"arch\": " << static_cast<double>(global.arch) << ",\n";
    out << "    \"prime\": " << static_cast<double>(global.prime) << "\n";
    out << "  },\n";
    out << "  \"per_prime\": [\n";

    Real cum = 0;
    size_t n = Marshal::Heat::LocalAssembly::clamp_count(cat,
        cfg.local_prime_count == 0 ? cat.p.size()
                                   : static_cast<size_t>(cfg.local_prime_count));
    const bool truncated = cfg.induction_export_max > 0 && n > static_cast<size_t>(cfg.induction_export_max);
    if (truncated) {
        n = static_cast<size_t>(cfg.induction_export_max);
        out << "  \"truncated\": true,\n";
        out << "  \"p_max\": " << cat.p.back() << ",\n";
    }
    for (size_t i = 0; i < n; ++i) {
        Heat::HeatCylinderOp op(cat, i);
        const int km = cat.kmax_adaptive[i];
        const auto pk = op.trace_packet(tf, tau, km, cfg.nmax, cfg.ktheta,
                                        cfg.s_euler, cfg.eps, link_n);
        cum += pk.prime_norm;
        const Real cum_rhs = global.poles + global.arch - cum;
        const Real ladder_res = global.lhs - cum_rhs;
        const Real tier1_err = std::max(pk.poisson_err,
                                         std::max(fabsl(pk.prime_norm - pk.heat_norm), pk.euler_err));
        if (i) out << ",\n";
        out << "    {\"p\": " << cat.p[i]
            << ", \"kmax\": " << km
            << ", \"poisson_err\": " << static_cast<double>(pk.poisson_err)
            << ", \"weil_heat_err\": " << static_cast<double>(fabsl(pk.prime_norm - pk.heat_norm))
            << ", \"euler_err\": " << static_cast<double>(pk.euler_err)
            << ", \"local_cylinder_pass\": " << (tier1_err <= cfg.tier1_tol ? "true" : "false")
            << ", \"T_p\": " << static_cast<double>(pk.prime_norm)
            << ", \"cum_weil\": " << static_cast<double>(cum)
            << ", \"cum_rhs\": " << static_cast<double>(cum_rhs)
            << ", \"ladder_residual\": " << static_cast<double>(ladder_res)
            << "}";
    }
    if (truncated && !cat.p.empty()) {
        const Real cum_rhs = global.poles + global.arch - global.prime;
        const Real ladder_res = global.lhs - cum_rhs;
        out << ",\n    {\"p\": " << cat.p.back()
            << ", \"kmax\": 0"
            << ", \"poisson_err\": 0"
            << ", \"weil_heat_err\": 0"
            << ", \"euler_err\": 0"
            << ", \"local_cylinder_pass\": true"
            << ", \"T_p\": 0"
            << ", \"cum_weil\": " << static_cast<double>(global.prime)
            << ", \"cum_rhs\": " << static_cast<double>(cum_rhs)
            << ", \"ladder_residual\": " << static_cast<double>(ladder_res)
            << ", \"rollup\": true}";
    }
    out << "\n  ]\n}\n";
}

void ExportSpectralJson(const std::string& path, int N,
                                 const Heat::PrimeCatalog& cat,
                                 const std::vector<double>& gammas) {
    const std::vector<Real> evals = CollectCylinderSpectrum(cat, N);

    std::ofstream out(path);
    out << std::setprecision(12);
    out << "{\n  \"operator_eigenvalues\": [";
    for (size_t i = 0; i < evals.size(); ++i) {
        if (i) out << ", ";
        out << static_cast<double>(evals[i]);
    }
    out << "],\n  \"riemann_zeros\": [";
    const size_t m = std::min(gammas.size(), static_cast<size_t>(N));
    for (size_t i = 0; i < m; ++i) {
        if (i) out << ", ";
        out << gammas[i];
    }
    out << "]\n}\n";
}

}  // namespace Marshal::Induction

#include "DualityGoldStandard.hxx"

#include "LogPrimeGlobal.hxx"
#include "TraceApi.hxx"
#include "Numerics/TestFunctions.hxx"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {

DualityGoldStandardResult run_duality_gold_standard(const Config& cfg, const double* gammas,
                                                    size_t n_zeros, const Real* gammas_ld,
                                                    size_t n_ld, PrimeCatalog& cat,
                                                    const std::vector<int>& primes) {
    DualityGoldStandardResult rep;
    rep.a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;
    rep.n_zeros = n_zeros;
    rep.n_primes = static_cast<int>(primes.size());

    const LaplaceTest lap(rep.a);
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const Real sigma = rep.a;
    cat.rebuild_adaptive(lap, 1.0L / (2.0L * sigma * sigma), kmax, cfg.eps);

    const TraceResult tr =
        Marshal::EvaluateTracePrefix(lap, sigma, gammas, n_zeros, gammas_ld, n_ld, cat,
                                     cfg.zero_kernel, cfg.simd, cfg.eps, cfg.trivial_zeros,
                                     cfg.precision_mode, cfg.arch_pts, false);

    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(primes);
    const Real hlog_prime = global.weil_prime_sum(lap, kmax, cfg.eps) / (2.0L * kPi);

    rep.lhs_zero_sum = tr.lhs;
    rep.rhs_arch = tr.arch;
    rep.rhs_poles = tr.poles;
    rep.rhs_prime = tr.prime;
    rep.rhs_total = tr.poles + tr.arch - tr.prime;
    rep.residual = std::fabs(tr.lhs - rep.rhs_total);
    rep.t1_gap = std::fabs(tr.prime - hlog_prime);
    rep.t1_pass = rep.t1_gap < 1e-18L * std::max(Real{1}, std::fabs(tr.prime));
    rep.pass = rep.t1_pass;

    std::cout << "=== Duality gold standard h(t)=exp(-a|t|), a=" << static_cast<double>(rep.a)
              << " ===\n";
    std::cout << std::scientific << std::setprecision(12);
    std::cout << "  LHS zero sum: " << static_cast<double>(rep.lhs_zero_sum) << "\n";
    std::cout << "  RHS arch: " << static_cast<double>(rep.rhs_arch)
              << "  poles: " << static_cast<double>(rep.rhs_poles)
              << "  prime: " << static_cast<double>(rep.rhs_prime) << "\n";
    std::cout << "  T1 gap: " << static_cast<double>(rep.t1_gap) << "  "
              << (rep.t1_pass ? "PASS" : "FAIL") << "\n";
    std::cout << "  full |LHS-RHS|: " << static_cast<double>(rep.residual)
              << "  (truncation-limited for non-compact h_hat)\n";
    return rep;
}

bool export_duality_gold_standard_json(const std::string& path,
                                       const DualityGoldStandardResult& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"test_function\": \"laplace_exp_abs\",\n";
    out << "  \"a\": " << static_cast<double>(r.a) << ",\n";
    out << "  \"n_zeros\": " << r.n_zeros << ",\n";
    out << "  \"n_primes\": " << r.n_primes << ",\n";
    out << "  \"lhs_zero_sum\": " << static_cast<double>(r.lhs_zero_sum) << ",\n";
    out << "  \"rhs_arch\": " << static_cast<double>(r.rhs_arch) << ",\n";
    out << "  \"rhs_poles\": " << static_cast<double>(r.rhs_poles) << ",\n";
    out << "  \"rhs_prime\": " << static_cast<double>(r.rhs_prime) << ",\n";
    out << "  \"rhs_total\": " << static_cast<double>(r.rhs_total) << ",\n";
    out << "  \"residual\": " << static_cast<double>(r.residual) << ",\n";
    out << "  \"t1_gap\": " << static_cast<double>(r.t1_gap) << ",\n";
    out << "  \"t1_pass\": " << (r.t1_pass ? "true" : "false") << ",\n";
    out << "  \"pass\": " << (r.pass ? "true" : "false") << "\n";
    out << "}\n";
    return true;
}

}  // namespace Marshal::Heat

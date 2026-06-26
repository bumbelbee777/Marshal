#include "Induction.hxx"
#include "ArchDetail.hxx"
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

const Real kPi      = 3.141592653589793238462643383279502884L;
const Real kSqrt2Pi = 2.506628274631000502415165145310982062L;

namespace Marshal::Induction {

using detail::arch_quadrature_floor;
using detail::zero_sum_h;
using detail::zero_sum_h_batched;
using detail::zero_sum_h_ld_batched;

std::unique_ptr<TestFunction> MakeTestFunction(const Config& cfg) {
    switch (cfg.test_kind) {
        case TestKind::Sinc2: {
            Real T = cfg.test_param;
            if (T <= 0) T = std::log(static_cast<Real>(cfg.prime_limit));
            const Real kappa = cfg.sinc2_kappa > 0 ? cfg.sinc2_kappa : 1.0L;
            return std::make_unique<Sinc2Test>(T, kappa);
        }
        case TestKind::Bump:
            return std::make_unique<BumpTest>(cfg.test_param > 0 ? cfg.test_param : 1.0L);
        case TestKind::Rational:
            return std::make_unique<RationalTest>(cfg.test_param > 0 ? cfg.test_param : 1.0L);
        case TestKind::Laplace:
            return std::make_unique<LaplaceTest>(cfg.duality_a > 0 ? cfg.duality_a : 1.0L);
        default:
            return std::make_unique<GaussTest>(cfg.sigma);
    }
}

Real TraceSigma(const Config& cfg, const TestFunction& tf) {
    if (std::string(tf.name()) == "sinc2") {
        if (cfg.test_param > 0) return cfg.test_param;
        return std::log(static_cast<Real>(std::max(2, cfg.prime_limit)));
    }
    return cfg.sigma;
}

TraceResult RunEvaluate(const Config& cfg, const TestFunction& tf,
                               const std::vector<double>& gammas,
                               const std::vector<Real>& gammas_ld,
                               const Heat::PrimeCatalog& cat) {
    const Real sigma = TraceSigma(cfg, tf);
    return EvaluateTrace(tf, sigma, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd, cfg.eps,
                         cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false, nullptr,
                         cfg.scale_mode);
}

TraceResult RunEvaluateView(const Config& cfg, const TestFunction& tf,
                            const IO::ZeroView& zeros, const Heat::PrimeCatalog& cat) {
    const Real sigma = TraceSigma(cfg, tf);
    return EvaluateTracePrefix(tf, sigma, zeros.ptr(), zeros.size(), zeros.ld_ptr(),
                               zeros.ld_count(), cat, cfg.zero_kernel, cfg.simd, cfg.eps,
                               cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false, nullptr,
                               cfg.scale_mode);
}

Real GammaSupportRadius(Real sigma, Real thresh = 1e-16L) {
    return sigma * sqrtl(-2.0L * logl(thresh));
}

size_t CountEffectiveZeros(Real sigma, const std::vector<double>& gammas, Real thresh) {
    const Real rad = GammaSupportRadius(sigma, thresh);
    size_t n = 0;
    for (double g : gammas)
        if (static_cast<Real>(g) <= rad) ++n;
    return n;
}

Analysis::ResidualBudget ComputeResidualBudget(const TestFunction& tf, Real sigma,
                                              const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld,
                                              const Heat::PrimeCatalog& cat, ZeroKernel zk,
                                              SimdLevel simd, bool precision_mode,
                                              int arch_pts) {
    Analysis::ResidualBudget b;
    const Real gamma_max = gammas.empty() ? 0.0L : static_cast<Real>(gammas.back());
    const Real p_max = cat.p.empty() ? 0.0L : static_cast<Real>(cat.p.back());

    b.zero_tail_naive = bound_zero_tail_gauss(sigma, gamma_max);
    b.prime_tail_naive = bound_prime_tail_gauss(sigma, p_max);

    const Real lhs_full = zero_sum_h(tf, sigma, gammas, gammas_ld, zk, simd);
    const size_t n_eff = CountEffectiveZeros(sigma, gammas);
    b.n_zeros_effective = n_eff;
    if (n_eff < gammas.size()) {
        const Real lhs_trunc = (zk == ZeroKernel::LongDouble && !gammas_ld.empty())
            ? zero_sum_h_ld_batched(tf, gammas_ld.data(), n_eff)
            : zero_sum_h_batched(tf, sigma, gammas.data(), n_eff, simd);
        b.zero_tail_effective = fabsl(lhs_full - lhs_trunc);
    }

    for (size_t i = 0; i < cat.p.size(); ++i) {
        if (tf.h_hat(cat.logp[i]) > 1e-14L) ++b.n_primes_effective;
    }
    b.prime_tail_effective = b.prime_tail_naive;

    b.arch_floor = arch_quadrature_floor(sigma, simd, precision_mode, arch_pts);
    // ψ-LUT interpolation + GH integration absolute bias (Richardson drift is arch_floor).
    b.arch_abs_floor = precision_mode ? (sigma >= 4.0L ? 1e-7L : 1e-9L) : 1e-6L;

    if (!gammas.empty()) {
        const size_t n = std::min(gammas.size(), std::min(gammas_ld.size(), size_t{20000}));
        if (n > 0) {
            const Real fsum = zero_sum_h_batched(tf, sigma, gammas.data(), n, SimdLevel::AVX2);
            const Real lsum = gammas_ld.size() >= n
                ? zero_sum_h_ld_batched(tf, gammas_ld.data(), n)
                : zero_sum_h_batched(tf, sigma, gammas.data(), n, SimdLevel::Scalar);
            b.float_floor = fabsl(lsum - fsum);
        }
    }
    return b;
}

void RunResidualScaling(const Config& cfg, const TestFunction& tf,
                                 const std::vector<double>& gammas,
                                 const std::vector<Real>& gammas_ld,
                                 const Heat::PrimeCatalog& cat,
                                 const std::vector<int>& primes) {
    std::cout << "=== Residual scaling (effective Gaussian window) ===\n";
    const Real gamma_eff = GammaSupportRadius(cfg.sigma);
    std::cout << "sigma=" << static_cast<double>(cfg.sigma)
              << "  test=" << tf.name()
              << "  gamma_eff(1e-16)=" << static_cast<double>(gamma_eff)
              << "  zeros_loaded=" << gammas.size() << "\n\n";

    const Real sigmas[] = {1.0L, 2.0L, 2.236L, 3.0L, 4.0L, 5.0L, 7.0L, 10.0L};
    for (Real s : sigmas) {
        GaussTest gt(s);
        Heat::PrimeCatalog local;
        local.set_primes(primes);
        local.rebuild_adaptive(gt, TauFromSigma(s), cfg.kmax, cfg.eps);
        const TraceResult r = EvaluateTrace(gt, s, gammas, gammas_ld, local, cfg.zero_kernel,
                                          cfg.simd, cfg.eps, false);
        const Analysis::ResidualBudget b = ComputeResidualBudget(gt, s, gammas, gammas_ld, local,
                                                         cfg.zero_kernel, cfg.simd,
                                                         cfg.precision_mode, cfg.arch_pts);
        std::cout << std::scientific << std::setprecision(4)
                  << std::setw(8) << static_cast<double>(s)
                  << std::setw(14) << static_cast<double>(fabsl(r.residual()))
                  << std::setw(14) << static_cast<double>(b.arch_floor) << "\n";
    }

    const Analysis::ResidualBudget b = ComputeResidualBudget(tf, cfg.sigma, gammas, gammas_ld, cat,
                                                     cfg.zero_kernel, cfg.simd,
                                                     cfg.precision_mode, cfg.arch_pts);
    const TraceResult r = RunEvaluate(cfg, tf, gammas, gammas_ld, cat);
    std::cout << "\nAt sigma=" << static_cast<double>(cfg.sigma) << ":\n";
    std::cout << "  Observed |residual|     = " << static_cast<double>(fabsl(r.residual())) << "\n";
    std::cout << "  Arch quadrature floor  = " << static_cast<double>(b.arch_floor) << "\n";
    std::cout << "  Analytic tail bound     = "
              << static_cast<double>(bound_total_gauss(cfg.sigma,
                  cat.p.empty() ? 0 : cat.p.back(),
                  gammas.empty() ? 0 : gammas.back(), 512)) << "\n";
}

void PrintHpAnsatz(Real sigma) {
    const Real tau = TauFromSigma(sigma);
    std::cout << "=== Hilbert-Polya heat-kernel ansatz ===\n";
    std::cout << "Formal write-up: python scripts/generate_ansatz.py --trace traces/induction.json\n";
    std::cout << "sigma=" << static_cast<double>(sigma)
              << "  tau=" << static_cast<double>(tau) << "\n\n";
}

void RunHpAnsatz(const Config& cfg, const TestFunction& tf,
                          const std::vector<double>& gammas,
                          const std::vector<Real>& gammas_ld,
                          const Heat::PrimeCatalog& cat,
                          const std::vector<int>& primes) {
    PrintHpAnsatz(cfg.sigma);
    RunResidualScaling(cfg, tf, gammas, gammas_ld, cat, primes);
    std::cout << "\n";
    RunHeatInduction(cfg, tf, gammas, gammas_ld, cat);
}


}  // namespace Marshal::Induction

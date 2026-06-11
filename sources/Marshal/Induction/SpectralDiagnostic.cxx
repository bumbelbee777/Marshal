#include "SpectralDiagnostic.hxx"

#include "Induction.hxx"
#include "InductionShared.hxx"
#include "Heat/Common.hxx"
#include "Numerics/TestFunctions.hxx"
#include "Quotient/QuotientToy.hxx"
#include "TraceApi.hxx"
#include <cmath>
#include <iomanip>
#include <iostream>

namespace Marshal::Induction {

namespace {
constexpr Real kTwoPi = 2.0L * 3.141592653589793238462643383279502884L;
}

void FillLexSortedGaps(const std::vector<Real>& sorted_omegas, const std::vector<double>& gammas,
                       size_t m, Real& max_gap, Real& mean_gap) {
    max_gap = 0;
    mean_gap = 0;
    if (!m) return;
    Real sum = 0;
    for (size_t i = 0; i < m; ++i) {
        const Real g = std::fabs(sorted_omegas[i] - static_cast<Real>(gammas[i]));
        sum += g;
        max_gap = std::max(max_gap, g);
    }
    mean_gap = sum / static_cast<Real>(m);
}

void FillMatchedCylinderGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas,
                             size_t m, int k_primes, Real& max_gap, Real& mean_gap,
                             Real& max_sq_gap, Real& mean_sq_gap) {
    max_gap = mean_gap = max_sq_gap = mean_sq_gap = 0;
    if (!m || cat.p.empty()) return;
    const int k = std::min(k_primes, static_cast<int>(cat.p.size()));
    Real sum = 0;
    Real sum_sq = 0;
    for (size_t i = 0; i < m; ++i) {
        const Real gamma = static_cast<Real>(gammas[i]);
        Real best = 1e300L;
        Real best_sq = 1e300L;
        for (int ax = 0; ax < k; ++ax) {
            const Real lp = cat.logp[static_cast<size_t>(ax)];
            const int n0 = std::max(
                1, static_cast<int>(std::lround(gamma * lp / kTwoPi)));
            for (int dn = -1; dn <= 1; ++dn) {
                const int n = std::max(1, n0 + dn);
                const Real om = kTwoPi * static_cast<Real>(n) / lp;
                best = std::min(best, std::fabs(om - gamma));
                best_sq = std::min(best_sq, std::fabs(om * om - gamma * gamma));
            }
        }
        sum += best;
        sum_sq += best_sq;
        max_gap = std::max(max_gap, best);
        max_sq_gap = std::max(max_sq_gap, best_sq);
    }
    mean_gap = sum / static_cast<Real>(m);
    mean_sq_gap = sum_sq / static_cast<Real>(m);
}

void FillFixedModeGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas,
                       size_t m, int k_primes, Real& max_gap, Real& mean_gap) {
    max_gap = mean_gap = 0;
    if (!m || cat.p.empty()) return;
    const int k = std::min(k_primes, static_cast<int>(cat.p.size()));
    Real sum = 0;
    for (size_t i = 0; i < m; ++i) {
        const Real gamma = static_cast<Real>(gammas[i]);
        Real num = 0;
        Real den = 0;
        for (int ax = 0; ax < k; ++ax) {
            const Real lp = cat.logp[static_cast<size_t>(ax)];
            const Real w = 1.0L / std::sqrt(static_cast<Real>(cat.p[static_cast<size_t>(ax)]));
            const Real om = kTwoPi / lp;
            num += w * om * om;
            den += w;
        }
        const Real rq = num / std::max(den, 1e-30L);
        const Real om_hat = std::sqrt(std::max(rq, 0.0L));
        const Real g = std::fabs(om_hat - gamma);
        sum += g;
        max_gap = std::max(max_gap, g);
    }
    mean_gap = sum / static_cast<Real>(m);
}

void FillExponentGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas,
                      size_t m, int k_primes, int kmax, Real& max_gap, Real& mean_gap,
                      Real& max_sq_gap, Real& mean_sq_gap) {
    max_gap = mean_gap = max_sq_gap = mean_sq_gap = 0;
    if (!m || cat.p.empty()) return;
    const int k = std::min(k_primes, static_cast<int>(cat.p.size()));
    Real sum = 0, sum_sq = 0;
    for (size_t i = 0; i < m; ++i) {
        const Real gamma = static_cast<Real>(gammas[i]);
        Real best = 1e300L, best_sq = 1e300L;
        for (int ax = 0; ax < k; ++ax) {
            const Real lp = cat.logp[static_cast<size_t>(ax)];
            for (int kk = 1; kk <= kmax; ++kk) {
                const Real e = static_cast<Real>(kk) * lp;
                best = std::min(best, std::fabs(e - gamma));
                best_sq = std::min(best_sq, std::fabs(e * e - gamma * gamma));
            }
        }
        sum += best;
        sum_sq += best_sq;
        max_gap = std::max(max_gap, best);
        max_sq_gap = std::max(max_sq_gap, best_sq);
    }
    mean_gap = sum / static_cast<Real>(m);
    mean_sq_gap = sum_sq / static_cast<Real>(m);
}

void FillFixedQuotientGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas,
                           size_t m, int k_primes, Real& max_gap, Real& max_sq_gap) {
    max_gap = max_sq_gap = 0;
    if (!m || cat.p.empty()) return;
    const int k = std::min(k_primes, static_cast<int>(cat.p.size()));
    weil_toy::PrimeList pl;
    std::vector<int> ps;
    for (int i = 0; i < k; ++i) ps.push_back(cat.p[static_cast<size_t>(i)]);
    pl.build(ps, k);
    weil_toy::QuotientGrid qg;
    qg.k_primes = k;
    qg.primes = pl.p;
    qg.logp = pl.logp;
    qg.haar_sqrt_inv.resize(static_cast<size_t>(k));
    for (int i = 0; i < k; ++i)
        qg.haar_sqrt_inv[static_cast<size_t>(i)] = 1.0 / std::sqrt(static_cast<double>(pl.p[i]));

    for (size_t i = 0; i < m; ++i) {
        const double gamma = gammas[i];
        const double rq = weil_toy::haar_continuum_fixed(qg, k, 1);
        const double omega = std::sqrt(std::max(rq, 0.0));
        max_gap = std::max(max_gap, static_cast<Real>(std::fabs(omega - gamma)));
        max_sq_gap = std::max(max_sq_gap, static_cast<Real>(std::fabs(rq - gamma * gamma)));
    }
}

CompactSinc2Result RunCompactSinc2Falsification(const std::vector<double>& gammas,
                                                const std::vector<Real>& gammas_ld,
                                                Heat::PrimeCatalog& cat, const Config& cfg,
                                                Real T, Real mismatch_tol) {
    CompactSinc2Result out;
    if (gammas.empty() || T <= 0) return out;
    out.T = T;
    Sinc2Test sinc(T);
    cat.rebuild_adaptive(sinc, TauFromSigma(T), cfg.kmax, cfg.eps);
    const TraceResult r = EvaluateTrace(sinc, T, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                                        cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts);
    out.lhs = r.lhs;
    out.rhs = r.rhs;
    out.residual = std::fabs(r.residual());
    out.mismatch_proved = out.residual > mismatch_tol;
    return out;
}

CompactSinc2Result RunQuotientLhsSinc2(const std::vector<double>& gammas,
                                       const std::vector<Real>& gammas_ld,
                                       Heat::PrimeCatalog& cat, const Config& cfg, Real T,
                                       int n_quotient) {
    CompactSinc2Result out;
    if (gammas.empty() || T <= 0 || n_quotient <= 0) return out;
    out.T = T;
    Sinc2Test sinc(T);
    cat.rebuild_adaptive(sinc, TauFromSigma(T), cfg.kmax, cfg.eps);
    const TraceResult base = EvaluateTrace(sinc, T, gammas, gammas_ld, cat, cfg.zero_kernel,
                                           cfg.simd, cfg.eps, cfg.trivial_zeros,
                                           cfg.precision_mode, cfg.arch_pts);
    out.rhs = base.rhs;

    const int k = std::min(kQuotientKFixedDefault, static_cast<int>(cat.p.size()));
    weil_toy::PrimeList pl;
    std::vector<int> ps;
    for (int i = 0; i < k; ++i) ps.push_back(cat.p[static_cast<size_t>(i)]);
    pl.build(ps, k);
    weil_toy::QuotientParams par;
    par.max_cells = cfg.quotient_max_cells > 0 ? cfg.quotient_max_cells : 8000000;
    weil_toy::QuotientGrid qg;
    qg.init_from(pl, k, par);

    const int m = std::min(n_quotient, static_cast<int>(gammas.size()));
    Real lhs = 0;
    for (int i = 0; i < m; ++i) {
        const double gamma = gammas[static_cast<size_t>(i)];
        const double rq = weil_toy::haar_continuum_indep(gamma, qg, qg.k_primes);
        const Real om = static_cast<Real>(std::sqrt(std::max(rq, 0.0)));
        lhs += sinc.h(om);
    }
    out.lhs = lhs;
    out.residual = std::fabs(lhs - base.rhs);
    out.mismatch_proved = out.residual > kCompactSinc2MismatchTol;
    return out;
}

void RunCompactTest(const Config& cfg, const TestFunction& /*tf*/,
                    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
                    Heat::PrimeCatalog& cat) {
    Real T = cfg.test_param > 0 ? cfg.test_param : 1.0L;
    if (cfg.test_param <= 0 && !gammas.empty()) {
        T = kTwoPi / static_cast<Real>(gammas[std::min(gammas.size() - 1, size_t{99})]);
    }
    const CompactSinc2Result r =
        RunCompactSinc2Falsification(gammas, gammas_ld, cat, cfg, T, kCompactSinc2MismatchTol);
    std::cout << "=== Compact sinc² falsification (|û| < 2π/T ⇒ finite prime sum) ===\n";
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "  T=" << static_cast<double>(r.T)
              << "  hhat_support=" << static_cast<double>(2.0L * 3.141592653589793238462643383279502884L / r.T)
              << "\n";
    std::cout << "  LHS=" << static_cast<double>(r.lhs) << "  RHS=" << static_cast<double>(r.rhs)
              << "  |residual|=" << static_cast<double>(r.residual) << "\n";
    std::cout << "  mismatch_tol=" << static_cast<double>(kCompactSinc2MismatchTol) << "\n";
    std::cout << "  SPECTRAL_MISMATCH_PROVED: " << (r.mismatch_proved ? "YES" : "NO") << "\n";
    if (r.mismatch_proved) {
        std::cout << "  Compact sinc² residual exceeds tolerance — trace identity fails with\n";
        std::cout << "  compactly supported ĥ; cylinder spectrum cannot match zero oracle.\n";
    }
}

}  // namespace Marshal::Induction

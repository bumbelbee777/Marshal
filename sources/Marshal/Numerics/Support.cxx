#include "Support.hxx"
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace Marshal::Numerics {

static constexpr Real kPi = MARSHAL_LIT(3.141592653589793238462643383279502884);
static constexpr Real kSqrt2Pi = MARSHAL_LIT(2.506628274631000502415165145310982062);

bool ValidateConfig(Real sigma, int prime_limit, bool do_sweep,
                    Real sweep_min, Real sweep_max, int sweep_steps,
                    size_t max_zeros, int kmax, int nmax, int ktheta,
                    std::string& err) {
    if (sigma <= 0) { err = "sigma must be positive"; return false; }
    if (prime_limit < 2) { err = "prime_limit must be >= 2"; return false; }
    if (do_sweep) {
        if (sweep_min <= 0 || sweep_max <= 0) { err = "sweep bounds must be positive"; return false; }
        if (sweep_min >= sweep_max) { err = "sweep_min must be < sweep_max"; return false; }
        if (sweep_steps < 1) { err = "sweep_steps must be >= 1"; return false; }
    }
    if (kmax < 1 || nmax < 1 || ktheta < 1) {
        err = "kmax, nmax, ktheta must be >= 1"; return false;
    }
    (void)max_zeros;
    return true;
}

bool ParseZeroLineFc(const char* b, const char* e, double& val) {
    while (b < e && (*b == ' ' || *b == '\t' || *b == '\r')) ++b;
    if (b >= e) return false;
    std::string s(b, e);
    char* end = nullptr;
    val = std::strtod(s.c_str(), &end);
    return end != s.c_str();
}

bool ParseZeroLineLd(const char* b, const char* e, Real& val) {
    while (b < e && (*b == ' ' || *b == '\t' || *b == '\r')) ++b;
    if (b >= e) return false;
    std::string s(b, e);
    char* end = nullptr;
    val = std::strtold(s.c_str(), &end);
    return end != s.c_str();
}

Real BoundPrimeTailGauss(Real sigma, Real p_max) {
    if (p_max <= 2) return 0;
    const Real lp = MarshalLog(p_max);
    const Real exponent = -sigma * sigma * lp * lp / 2.0L;
    return MarshalExp(exponent) / (2.0L * kSqrt2Pi * sigma * sigma * lp);
}

Real BoundZeroTailGauss(Real sigma, Real gamma_max) {
    if (gamma_max <= 0) return 0;
    const Real density = MarshalLog(gamma_max / (2.0L * kPi)) / (2.0L * kPi);
    const Real gauss = MarshalExp(-gamma_max * gamma_max / (2.0L * sigma * sigma));
    return gauss * density * sigma * sigma / gamma_max;
}

Real BoundArchQuadrature(Real sigma, int gh_order) {
    const Real h = 1.0L / static_cast<Real>(gh_order);
    return 1e-4L * h * h * MarshalExp(sigma * sigma * 0.01L);
}

Real BoundTotalGauss(Real sigma, Real p_max, Real gamma_max, int gh_order) {
    return BoundPrimeTailGauss(sigma, p_max)
         + BoundZeroTailGauss(sigma, gamma_max)
         + BoundArchQuadrature(sigma, gh_order);
}

Real BoundOmittedPrimeTail(Real t, Real P_max) {
    if (P_max <= 2.0L || t <= 0.0L) return 0.0L;
    if (!ConvergenceRegime(t, P_max)) return -1.0L;  // sentinel: bound not valid
    return kM3TailConstant / (MarshalSqrt(P_max) * std::pow(static_cast<double>(t), 0.25));
}

Real BoundOmittedPrimeTailUniform(Real t_min, Real t_max, Real P_max) {
    (void)t_max;
    if (t_min <= 0.0L) t_min = t_max > 0.0L ? t_max : 1.0L;
    return BoundOmittedPrimeTail(t_min, P_max);
}

Real EvaluateOmittedPrimeTail(Real t, Real P_max,
                              const int* primes, size_t n_primes, int k_cap) {
    if (t <= 0.0L || !primes || n_primes == 0) return 0.0L;
    Real sum = 0;
    for (size_t i = 0; i < n_primes; ++i) {
        const int p = primes[i];
        if (static_cast<Real>(p) <= P_max) continue;
        const Real lp = MarshalLog(static_cast<Real>(p));
        Real ppow = MarshalSqrt(static_cast<Real>(p));
        for (int k = 1; k <= k_cap; ++k) {
            const Real u = static_cast<Real>(k) * lp;
            sum += (lp / ppow) * MarshalExp(-t * u * u);
            ppow *= MarshalSqrt(static_cast<Real>(p));
        }
    }
    return sum / (2.0L * kPi);
}

TailBoundStatus ClassifyTailBound(Real t, Real P_max,
                                  Real exact_tail, Real analytic_bound) {
    if (!ConvergenceRegime(t, P_max)) return TailBoundStatus::Divergent;
    if (analytic_bound < 0.0L) return TailBoundStatus::Unproved;
    if (exact_tail > 1e-30L && analytic_bound < exact_tail * 0.95L)
        return TailBoundStatus::Unproved;
    return TailBoundStatus::Valid;
}

int AdaptiveNmax(Real inv_logp, Real tau, Real eps) {
    for (int n = 1; n <= 100000; ++n) {
        const Real lam = 2.0L * kPi * static_cast<Real>(n) * inv_logp;
        if (MarshalExp(-tau * lam * lam) < eps) return n;
    }
    return 100000;
}

int AdaptiveKtheta(Real logp, Real tau, Real eps) {
    const Real lp2 = logp * logp;
    const Real inv4t = 1.0L / (4.0L * tau);
    for (int k = 1; k <= 100000; ++k) {
        if (MarshalExp(-static_cast<Real>(k * k) * lp2 * inv4t) < eps) return k;
    }
    return 100000;
}

}  // namespace Marshal::Numerics

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Real.hxx"

namespace Marshal::Numerics {

inline Real PairwiseSumVec(const std::vector<Real>& v) {
    if (v.empty()) return 0;
    std::vector<Real> cur = v;
    while (cur.size() > 1) {
        std::vector<Real> nxt;
        nxt.reserve((cur.size() + 1) / 2);
        for (size_t i = 0; i < cur.size(); i += 2) {
            if (i + 1 < cur.size()) nxt.push_back(cur[i] + cur[i + 1]);
            else nxt.push_back(cur[i]);
        }
        cur.swap(nxt);
    }
    return cur[0];
}

inline uint32_t Crc32Bytes(const void* data, size_t len) {
    const auto* p = static_cast<const uint8_t*>(data);
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        c ^= p[i];
        for (int k = 0; k < 8; ++k)
            c = (c >> 1) ^ (0xEDB88320u & (~((c & 1u) - 1u)));
    }
    return c ^ 0xFFFFFFFFu;
}

bool ValidateConfig(Real sigma, int prime_limit, bool do_sweep,
                    Real sweep_min, Real sweep_max, int sweep_steps,
                    size_t max_zeros, int kmax, int nmax, int ktheta,
                    std::string& err);

bool ParseZeroLineFc(const char* b, const char* e, double& val);
bool ParseZeroLineLd(const char* b, const char* e, Real& val);

Real BoundPrimeTailGauss(Real sigma, Real p_max);
Real BoundZeroTailGauss(Real sigma, Real gamma_max);
Real BoundArchQuadrature(Real sigma, int gh_order);
Real BoundTotalGauss(Real sigma, Real p_max, Real gamma_max, int gh_order);

constexpr Real kM3TailConstant = 4.0L * 2.506628274631000502415165145310982062L;

enum class TailBoundStatus { Valid, Divergent, Unproved };

// Effective series exponent for k=1 term: -1/2 - t*log(P).
inline Real EffectiveTailExponent(Real t, Real P) {
    if (P <= 1.0L || t <= 0.0L) return 0.0L;
    return -0.5L - t * MarshalLog(P);
}

// Regime: absolute convergence of omitted-prime tail requires exponent < -1.
inline bool ConvergenceRegime(Real t, Real P) {
    return EffectiveTailExponent(t, P) < -1.0L;
}

// Smallest t at which the k=1 tail is absolutely convergent at prime cutoff P.
inline Real RegimeTMin(Real P, Real safety = 1.02L) {
    if (P <= 2.0L) return 1.0L;
    return safety * 0.5L / MarshalLog(P);
}

Real BoundOmittedPrimeTail(Real t, Real P_max);
Real BoundOmittedPrimeTailUniform(Real t_min, Real t_max, Real P_max);
Real EvaluateOmittedPrimeTail(Real t, Real P_max,
                              const int* primes, size_t n_primes, int k_cap);

TailBoundStatus ClassifyTailBound(Real t, Real P_max,
                                  Real exact_tail, Real analytic_bound);

int AdaptiveNmax(Real inv_logp, Real tau, Real eps);
int AdaptiveKtheta(Real logp, Real tau, Real eps);

}  // namespace Marshal::Numerics

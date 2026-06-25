#include "CrossSectorStep5Bounds.hxx"

#include <algorithm>
#include <cmath>

#include "Heat/Common.hxx"

namespace Marshal::Induction {
namespace {

using Heat::Kahan;

Real kernel_zero_cos(Real x, Real y, Real zero_T, const std::vector<double>& gammas) {
    Real s = 0;
    const Real d = x - y;
    for (double g : gammas) {
        if (static_cast<Real>(g) > zero_T) break;
        s += 2.0L * std::cos(static_cast<Real>(g) * d);
    }
    return s;
}

}  // namespace

Real weil_prime_weight_sum(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps) {
    (void)a;
    Kahan acc;
    for (size_t pi = 0; pi < cat.p.size(); ++pi) {
        const Real lp = cat.logp[pi];
        Real ppow = cat.sqrtp[pi];
        const int km = cat.kmax_adaptive[pi];
        for (int kk = 1; kk <= km; ++kk) {
            const Real u = static_cast<Real>(kk) * lp;
            if (u > log_cutoff) break;
            acc.add(lp / ppow);
            ppow *= cat.sqrtp[pi];
            if (eps > 0 && lp / ppow < eps) break;
        }
    }
    return acc.total();
}

Real weil_quadratic_prime_cs_bound(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps) {
    return 2.0L * weil_prime_weight_sum(a, cat, log_cutoff, eps);
}

WeilQuadraticSectorBreakdown weil_rayleigh_sector_breakdown(
    Real a, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat, Real log_cutoff,
    Real zero_T, Real eps, const WeilOperatorRayleighOpts& opts) {
    WeilQuadraticSectorBreakdown out;
    if (a <= 0) return out;

    Real best = 1e300L;
    const int n_quad = std::max(16, opts.n_quad);
    const Real dx = 2.0L * a / static_cast<Real>(n_quad);

    for (int j = 1; j <= opts.sin_modes; ++j) {
        const Real scale = Heat::kPi * static_cast<Real>(j) / (2.0L * a);
        Real l2 = 0;
        for (int i = 0; i <= n_quad; ++i) {
            const Real x = -a + dx * static_cast<Real>(i);
            const Real v = std::sin(scale * (x + a));
            l2 += v * v * dx;
        }
        if (l2 <= 0) continue;

        Real q_pf = 0;
        Real q_prime = 0;
        Real q_zero = 0;
        for (int i = 0; i <= n_quad; ++i) {
            const Real xi = -a + dx * static_cast<Real>(i);
            const Real vx = std::sin(scale * (xi + a));
            for (int k = 0; k <= n_quad; ++k) {
                const Real xj = -a + dx * static_cast<Real>(k);
                const Real vy = std::sin(scale * (xj + a));
                const Real d = MarshalFabs(xi - xj);
                if (d >= 1e-8L) q_pf += 0.5L / d * vx * vy * dx * dx;
                if (opts.include_zero) {
                    q_zero += kernel_zero_cos(xi, xj, zero_T, gammas) * vx * vy * dx * dx;
                }
            }
            for (size_t pi = 0; pi < cat.p.size(); ++pi) {
                const Real lp = cat.logp[pi];
                Real ppow = cat.sqrtp[pi];
                const int km = cat.kmax_adaptive[pi];
                for (int kk = 1; kk <= km; ++kk) {
                    const Real u = static_cast<Real>(kk) * lp;
                    if (u > log_cutoff) break;
                    const Real coeff = lp / ppow;
                    for (int sign : {1, -1}) {
                        const Real y = xi - static_cast<Real>(sign) * u;
                        if (MarshalFabs(y) <= a) {
                            const Real vy = std::sin(scale * (y + a));
                            q_prime += coeff * vx * vy * dx;
                        }
                    }
                    ppow *= cat.sqrtp[pi];
                    if (eps > 0 && coeff < eps) break;
                }
            }
        }
        const Real q_total = q_pf + q_prime + q_zero;
        const Real ray = q_total / l2;
        if (ray < best) {
            best = ray;
            out.q_pf = q_pf / l2;
            out.q_prime = q_prime / l2;
            out.q_zero = q_zero / l2;
            out.q_total = ray;
            out.rayleigh = ray;
            out.mode_index = j;
        }
    }
    return out;
}

}  // namespace Marshal::Induction

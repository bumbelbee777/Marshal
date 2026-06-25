#include "CrossSectorWeilOperator.hxx"

#include <algorithm>
#include <cmath>

#include "Heat/Common.hxx"

namespace Marshal::Induction {
namespace {

Real kernel_pf(Real x, Real y) {
    const Real d = std::fabsl(x - y);
    if (d < 1e-10L) return 0;
    return 0.5L / d;
}

Real kernel_zero_cos(Real x, Real y, Real zero_T, const std::vector<double>& gammas) {
    Real s = 0;
    const Real d = x - y;
    for (double g : gammas) {
        if (static_cast<Real>(g) > zero_T) break;
        s += 2.0L * std::cos(static_cast<Real>(g) * d);
    }
    return s;
}

Real rayleigh_on_modes(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real zero_T,
                       const std::vector<double>& gammas, Real eps, bool include_zero,
                       const WeilOperatorRayleighOpts& opts) {
    if (a <= 0) return 0;
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

        Real q = 0;
        for (int i = 0; i <= n_quad; ++i) {
            const Real xi = -a + dx * static_cast<Real>(i);
            const Real vx = std::sin(scale * (xi + a));
            for (int k = 0; k <= n_quad; ++k) {
                const Real xj = -a + dx * static_cast<Real>(k);
                const Real vy = std::sin(scale * (xj + a));
                const Real d = std::fabsl(xi - xj);
                if (d >= 1e-8L) q += 0.5L / d * vx * vy * dx * dx;
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
                        if (std::fabsl(y) <= a) {
                            const Real vy = std::sin(scale * (y + a));
                            q += coeff * vx * vy * dx;
                        }
                    }
                    ppow *= cat.sqrtp[pi];
                    if (eps > 0 && coeff < eps) break;
                }
            }
            if (include_zero) {
                for (int k = 0; k <= n_quad; ++k) {
                    const Real xj = -a + dx * static_cast<Real>(k);
                    const Real vy = std::sin(scale * (xj + a));
                    q += kernel_zero_cos(xi, xj, zero_T, gammas) * vx * vy * dx * dx;
                }
            }
        }
        best = std::min(best, q / l2);
    }
    return best;
}

Real jacobi_smallest_eigenvalue(std::vector<std::vector<Real>> a, int max_iter = 120) {
    const int n = static_cast<int>(a.size());
    if (n == 0) return 0;
    std::vector<Real> v(n, 1.0L / static_cast<Real>(n));
    Real mu = 0;
    for (int it = 0; it < max_iter; ++it) {
        std::vector<Real> w(n, 0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) w[i] += a[i][j] * v[j];
        }
        Real norm = 0;
        for (Real x : w) norm += x * x;
        norm = std::sqrt(norm);
        if (norm <= 0) return mu;
        for (int i = 0; i < n; ++i) v[i] = w[i] / norm;
        Real num = 0, den = 0;
        for (int i = 0; i < n; ++i) {
            Real row = 0;
            for (int j = 0; j < n; ++j) row += a[i][j] * v[j];
            num += v[i] * row;
            den += v[i] * v[i];
        }
        mu = num / den;
    }
    return mu;
}

Real kernel_prime_at_pair(Real x, Real y, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps) {
    Real s = 0;
    const Real dxy = x - y;
    for (size_t pi = 0; pi < cat.p.size(); ++pi) {
        const Real lp = cat.logp[pi];
        Real ppow = cat.sqrtp[pi];
        const int km = cat.kmax_adaptive[pi];
        for (int kk = 1; kk <= km; ++kk) {
            const Real u = static_cast<Real>(kk) * lp;
            if (u > log_cutoff) break;
            const Real coeff = lp / ppow;
            if (std::fabsl(dxy - u) < 1e-6L || std::fabsl(dxy + u) < 1e-6L) s += coeff;
            ppow *= cat.sqrtp[pi];
            if (eps > 0 && coeff < eps) break;
        }
    }
    return s;
}

}  // namespace

Real weil_rayleigh_partial(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                           const WeilOperatorRayleighOpts& opts) {
    return rayleigh_on_modes(a, cat, log_cutoff, 0, {}, eps, false, opts);
}

Real weil_rayleigh_full(Real a, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat,
                        Real log_cutoff, Real zero_T, Real eps,
                        const WeilOperatorRayleighOpts& opts) {
    return rayleigh_on_modes(a, cat, log_cutoff, zero_T, gammas, eps, true, opts);
}

Real weil_spectral_min_full(Real a, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat,
                            Real log_cutoff, Real zero_T, Real eps,
                            const WeilOperatorRayleighOpts& opts) {
    (void)eps;
    if (a <= 0) return 0;
    const int n = std::max(8, opts.spectral_pts);
    const Real dx = 2.0L * a / static_cast<Real>(n - 1);
    std::vector<std::vector<Real>> mat(n, std::vector<Real>(n, 0));
    for (int i = 0; i < n; ++i) {
        const Real xi = -a + dx * static_cast<Real>(i);
        for (int j = 0; j < n; ++j) {
            const Real xj = -a + dx * static_cast<Real>(j);
            Real k = kernel_pf(xi, xj) + kernel_prime_at_pair(xi, xj, cat, log_cutoff, 0) +
                     kernel_zero_cos(xi, xj, zero_T, gammas);
            mat[i][j] = k * dx;
        }
    }
    return jacobi_smallest_eigenvalue(std::move(mat));
}

}  // namespace Marshal::Induction

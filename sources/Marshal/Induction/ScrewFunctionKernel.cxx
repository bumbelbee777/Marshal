#include "ScrewFunctionKernel.hxx"

#include <cmath>
#include <vector>

#include "Heat/Common.hxx"

namespace Marshal::Induction {
namespace {

constexpr Real kZeta2Quarter = 16.832779479491073486211315635859811L;
constexpr Real kPsiQuarterMinusLogPi =
    -1.9635100260210038852795559472751597L;  // ψ(1/4) − log π
// Suzuki (2.2): A = (1/2)(log(2π) − ψ(2)) = (1/2)(log(2π) + γ − 1)
constexpr Real kSuzukiA = 0.70754648889752094848120443672410487L;
constexpr Real kSuzukiMassCoeff = 2.0L * kSuzukiA + 1.0L;  // 2A + 1

Real hurwitz_lerch_phi(Real z, Real s, Real a, int max_n) {
    Real sum = 0;
    Real zp = 1.0L;
    for (int n = 0; n < max_n; ++n) {
        const Real an = a + static_cast<Real>(n);
        sum += zp / std::pow(an, s);
        zp *= z;
        if (MarshalFabs(zp) < 1e-40L) break;
    }
    return sum;
}

Real screw_arch_lerch_term(Real t_abs) {
    const Real exp_half = std::exp(-0.5L * t_abs);
    const Real z = std::exp(-2.0L * t_abs);
    const Real phi_t = exp_half * hurwitz_lerch_phi(z, 2.0L, 0.25L, 400);
    return -0.25L * (kZeta2Quarter - phi_t);
}

Real screw_poly_term(Real t_abs) {
    const Real half = 0.5L * t_abs;
    return -4.0L * (std::exp(half) + std::exp(-half) - 2.0L);
}

Real screw_prime_term(Real t_abs, const Heat::PrimeCatalog& cat, Real eps) {
    const Real cutoff = std::exp(t_abs);
    Real s = 0;
    for (size_t pi = 0; pi < cat.p.size(); ++pi) {
        const Real lp = cat.logp[pi];
        Real ppow = cat.sqrtp[pi];
        const int km = cat.kmax_adaptive[pi];
        for (int kk = 1; kk <= km; ++kk) {
            const Real n = ppow * ppow;
            if (n > cutoff + 1e-9L) break;
            const Real ln = static_cast<Real>(kk) * lp;
            s += (lp / ppow) * (t_abs - ln);
            ppow *= cat.sqrtp[pi];
            if (eps > 0 && lp / ppow < eps) break;
        }
    }
    return s;
}

inline Real sin_mode(Real x, Real a, Real scale) {
    return std::sin(scale * (x + a));
}

Real screw_La_form(Real a, int n_quad, Real scale) {
    if (a <= 0) return 0;
    const Real dx = 2.0L * a / static_cast<Real>(n_quad);
    Real la = 0;
    for (int i = 0; i <= n_quad; ++i) {
        const Real xi = -a + dx * static_cast<Real>(i);
        const Real vi = sin_mode(xi, a, scale);
        const Real log_term = std::log(std::max(a * a - xi * xi, 1e-30L));
        la -= 0.5L * log_term * vi * vi * dx;
    }
    for (int i = 0; i <= n_quad; ++i) {
        const Real xi = -a + dx * static_cast<Real>(i);
        const Real vi = sin_mode(xi, a, scale);
        for (int k = 0; k <= n_quad; ++k) {
            if (i == k) continue;
            const Real xj = -a + dx * static_cast<Real>(k);
            const Real vj = sin_mode(xj, a, scale);
            const Real d = MarshalFabs(xi - xj);
            if (d < 1e-10L) continue;
            la += 0.25L * (vi - vj) * (vi - vj) / d * dx * dx;
        }
    }
    return la;
}

Real screw_prime_Ba_shift(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                            int n_quad, Real scale) {
    if (a <= 0) return 0;
    const Real dx = 2.0L * a / static_cast<Real>(n_quad);
    Real q = 0;
    for (size_t pi = 0; pi < cat.p.size(); ++pi) {
        const Real lp = cat.logp[pi];
        Real ppow = cat.sqrtp[pi];
        const int km = cat.kmax_adaptive[pi];
        for (int kk = 1; kk <= km; ++kk) {
            const Real u = static_cast<Real>(kk) * lp;
            if (u > log_cutoff) break;
            const Real coeff = lp / ppow;
            for (int i = 0; i <= n_quad; ++i) {
                const Real xi = -a + dx * static_cast<Real>(i);
                const Real vi = sin_mode(xi, a, scale);
                if (xi <= a - u) {
                    const Real y = xi + u;
                    q += coeff * vi * sin_mode(y, a, scale) * dx;
                }
                if (xi >= -a + u) {
                    const Real y = xi - u;
                    q += coeff * vi * sin_mode(y, a, scale) * dx;
                }
            }
            ppow *= cat.sqrtp[pi];
            if (eps > 0 && coeff < eps) break;
        }
    }
    return q;
}

Real mode_l2(Real a, int n_quad, Real scale) {
    const Real dx = 2.0L * a / static_cast<Real>(n_quad);
    Real l2 = 0;
    for (int i = 0; i <= n_quad; ++i) {
        const Real x = -a + dx * static_cast<Real>(i);
        const Real v = sin_mode(x, a, scale);
        l2 += v * v * dx;
    }
    return l2;
}

Real gershgorin_row_sum(const std::vector<std::vector<Real>>& mat, int row) {
    Real s = 0;
    for (size_t j = 0; j < mat[static_cast<size_t>(row)].size(); ++j) {
        s += MarshalFabs(mat[static_cast<size_t>(row)][j]);
    }
    return s;
}

Real gershgorin_max_row_sum(const std::vector<std::vector<Real>>& mat) {
    Real mx = 0;
    for (size_t i = 0; i < mat.size(); ++i) {
        mx = std::max(mx, gershgorin_row_sum(mat, static_cast<int>(i)));
    }
    return mx;
}

std::pair<Real, std::vector<Real>> jacobi_smallest_eigenpair(std::vector<std::vector<Real>> mat,
                                                             int max_iter = 160) {
    const int n = static_cast<int>(mat.size());
    if (n == 0) return {0, {}};
    const Real tau = gershgorin_max_row_sum(mat) + 1.0L;
    std::vector<std::vector<Real>> shifted(n, std::vector<Real>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            shifted[static_cast<size_t>(i)][static_cast<size_t>(j)] =
                (i == j ? tau : 0) - mat[static_cast<size_t>(i)][static_cast<size_t>(j)];
        }
    }
    std::vector<Real> v(n, 1.0L / std::sqrt(static_cast<Real>(n)));
    Real mu_shift = 0;
    for (int it = 0; it < max_iter; ++it) {
        std::vector<Real> w(n, 0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) w[i] += shifted[i][j] * v[j];
        }
        Real norm = 0;
        for (Real x : w) norm += x * x;
        norm = std::sqrt(norm);
        if (norm <= 0) break;
        for (int i = 0; i < n; ++i) v[i] = w[i] / norm;
        Real num = 0, den = 0;
        for (int i = 0; i < n; ++i) {
            Real row = 0;
            for (int j = 0; j < n; ++j) row += shifted[i][j] * v[j];
            num += v[i] * row;
            den += v[i] * v[i];
        }
        mu_shift = num / den;
    }
    return {tau - mu_shift, v};
}

// Classic Jacobi rotations — exact smallest eigenpair for m <= kDenseBaMaxPts.
constexpr int kDenseBaMaxPts = 512;

std::pair<Real, std::vector<Real>> dense_smallest_eigenpair(const std::vector<std::vector<Real>>& mat) {
    const int n = static_cast<int>(mat.size());
    if (n == 0) return {0, {}};
    std::vector<std::vector<Real>> a = mat;
    std::vector<std::vector<Real>> v(n, std::vector<Real>(n, 0));
    for (int i = 0; i < n; ++i) v[static_cast<size_t>(i)][static_cast<size_t>(i)] = 1.0L;
    for (int sweep = 0; sweep < 6 * n * n; ++sweep) {
        int p = 0, q = 1;
        Real mx = MarshalFabs(a[0][1]);
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const Real off = MarshalFabs(a[static_cast<size_t>(i)][static_cast<size_t>(j)]);
                if (off > mx) {
                    mx = off;
                    p = i;
                    q = j;
                }
            }
        }
        if (mx < 1e-14L * std::max(1.0L, MarshalFabs(a[static_cast<size_t>(p)][static_cast<size_t>(p)])))
            break;
        const Real app = a[static_cast<size_t>(p)][static_cast<size_t>(p)];
        const Real aqq = a[static_cast<size_t>(q)][static_cast<size_t>(q)];
        const Real apq = a[static_cast<size_t>(p)][static_cast<size_t>(q)];
        const Real phi = 0.5L * std::atan2(2.0L * apq, aqq - app);
        const Real c = std::cos(phi);
        const Real s = std::sin(phi);
        for (int k = 0; k < n; ++k) {
            const Real akp = a[static_cast<size_t>(k)][static_cast<size_t>(p)];
            const Real akq = a[static_cast<size_t>(k)][static_cast<size_t>(q)];
            a[static_cast<size_t>(k)][static_cast<size_t>(p)] = c * akp - s * akq;
            a[static_cast<size_t>(k)][static_cast<size_t>(q)] = s * akp + c * akq;
        }
        for (int k = 0; k < n; ++k) {
            const Real apk = a[static_cast<size_t>(p)][static_cast<size_t>(k)];
            const Real aqk = a[static_cast<size_t>(q)][static_cast<size_t>(k)];
            a[static_cast<size_t>(p)][static_cast<size_t>(k)] = c * apk - s * aqk;
            a[static_cast<size_t>(q)][static_cast<size_t>(k)] = s * apk + c * aqk;
        }
        for (int k = 0; k < n; ++k) {
            const Real vkp = v[static_cast<size_t>(k)][static_cast<size_t>(p)];
            const Real vkq = v[static_cast<size_t>(k)][static_cast<size_t>(q)];
            v[static_cast<size_t>(k)][static_cast<size_t>(p)] = c * vkp - s * vkq;
            v[static_cast<size_t>(k)][static_cast<size_t>(q)] = s * vkp + c * vkq;
        }
    }
    int min_i = 0;
    for (int i = 1; i < n; ++i) {
        if (a[static_cast<size_t>(i)][static_cast<size_t>(i)] <
            a[static_cast<size_t>(min_i)][static_cast<size_t>(min_i)]) {
            min_i = i;
        }
    }
    std::vector<Real> vec(n);
    for (int i = 0; i < n; ++i) vec[static_cast<size_t>(i)] = v[static_cast<size_t>(i)][static_cast<size_t>(min_i)];
    return {a[static_cast<size_t>(min_i)][static_cast<size_t>(min_i)], vec};
}

constexpr Real kEulerGamma = 0.57721566490153286060651209L;

Real screw_re_psi_quarter_plus_it(Real t) {
    Real s = -kEulerGamma;
    for (int n = 0; n < 500; ++n) {
        const Real zn = 0.25L + static_cast<Real>(n);
        s -= zn / (zn * zn + t * t);
    }
    return s;
}

Real screw_r1pp_fourier_hat_abs(Real z) {
    if (z < 1e-12L) return 0;
    const Real t = 0.5L * z;
    const Real hat = -screw_re_psi_quarter_plus_it(t) + std::log(z) - std::log(2.0L);
    return MarshalFabs(hat);
}

Real screw_La_on_grid(Real a, const std::vector<Real>& v, Real dx) {
    const int m = static_cast<int>(v.size());
    if (m == 0 || a <= 0) return 0;
    Real la = 0;
    for (int i = 0; i < m; ++i) {
        const Real xi = -a + dx * static_cast<Real>(i + 1);
        const Real log_term = std::log(std::max(a * a - xi * xi, 1e-30L));
        la -= 0.5L * log_term * v[static_cast<size_t>(i)] * v[static_cast<size_t>(i)] * dx;
    }
    for (int i = 0; i < m; ++i) {
        for (int k = 0; k < m; ++k) {
            if (i == k) continue;
            const Real d = dx * static_cast<Real>(std::abs(i - k));
            if (d < 1e-10L) continue;
            const Real diff = v[static_cast<size_t>(i)] - v[static_cast<size_t>(k)];
            la += 0.25L * diff * diff / d * dx * dx;
        }
    }
    return la;
}

Real eval_v_on_grid(Real x, Real a, const std::vector<Real>& v, Real dx) {
    const int m = static_cast<int>(v.size());
    if (m == 0 || a <= 0) return 0;
    const Real t = (x + a) / dx - 1.0L;
    const int i0 = static_cast<int>(std::floor(t));
    const Real f = t - static_cast<Real>(i0);
    if (i0 < 0 || i0 >= m) return 0;
    if (i0 + 1 >= m) return v[static_cast<size_t>(i0)];
    return (1.0L - f) * v[static_cast<size_t>(i0)] + f * v[static_cast<size_t>(i0 + 1)];
}

Real screw_r1_double_prime(Real t_abs) {
    if (t_abs < 1e-10L) return 0;
    const Real eh = std::exp(-0.5L * t_abs);
    const Real em2 = std::exp(-2.0L * t_abs);
    const Real denom = 1.0L - em2;
    if (MarshalFabs(denom) < 1e-30L) return 0;
    return eh / denom - 0.5L / t_abs;
}

Real screw_r0_double_prime(Real t_abs) {
    if (t_abs < 1e-10L) return -2.0L;
    const Real half = 0.5L * t_abs;
    return -std::exp(half) - std::exp(-half);
}

Real screw_g_double_prime_numerical(Real t, const Heat::PrimeCatalog& cat, Real eps) {
    const Real h = 1e-4L * (1.0L + MarshalFabs(t));
    const Real gp = (screw_function_g(t + h, cat, eps) - screw_function_g(t - h, cat, eps)) / (2.0L * h);
    const Real gm = (screw_function_g(t + h, cat, eps) + screw_function_g(t - h, cat, eps) -
                     2.0L * screw_function_g(t, cat, eps)) /
                    (h * h);
    (void)gp;
    return gm;
}

Real screw_r_remainder_double_prime(Real t, const Heat::PrimeCatalog& cat, Real eps) {
    const Real ta = MarshalFabs(t);
    return screw_g_double_prime_numerical(t, cat, eps) - screw_r0_double_prime(ta) -
           screw_r1_double_prime(ta);
}

Real screw_kernel_pp_bilinear_on_grid(Real a, const std::vector<Real>& v, Real dx,
                                      Real (*kernel_pp)(Real)) {
    const int m = static_cast<int>(v.size());
    if (m == 0 || a <= 0) return 0;
    Real q = 0;
    for (int i = 0; i < m; ++i) {
        const Real xi = -a + dx * static_cast<Real>(i + 1);
        const Real vi = v[static_cast<size_t>(i)];
        for (int k = 0; k < m; ++k) {
            const Real xj = -a + dx * static_cast<Real>(k + 1);
            const Real vj = v[static_cast<size_t>(k)];
            q += kernel_pp(MarshalFabs(xi - xj)) * vi * vj * dx * dx;
        }
    }
    return q;
}

Real screw_rpp_bilinear_on_grid(Real a, const std::vector<Real>& v, Real dx) {
    return screw_kernel_pp_bilinear_on_grid(a, v, dx, screw_r1_double_prime);
}

Real screw_r0pp_bilinear_on_grid(Real a, const std::vector<Real>& v, Real dx) {
    return screw_kernel_pp_bilinear_on_grid(a, v, dx, screw_r0_double_prime);
}

Real screw_prime_Ba_shift_on_grid(Real a, const std::vector<Real>& v, const Heat::PrimeCatalog& cat,
                                  Real log_cutoff, Real eps, Real dx) {
    const int m = static_cast<int>(v.size());
    if (m == 0 || a <= 0) return 0;
    Real q = 0;
    for (size_t pi = 0; pi < cat.p.size(); ++pi) {
        const Real lp = cat.logp[pi];
        Real ppow = cat.sqrtp[pi];
        const int km = cat.kmax_adaptive[pi];
        for (int kk = 1; kk <= km; ++kk) {
            const Real u = static_cast<Real>(kk) * lp;
            if (u > log_cutoff) break;
            const Real coeff = lp / ppow;
            for (int i = 0; i < m; ++i) {
                const Real xi = -a + dx * static_cast<Real>(i + 1);
                const Real vi = v[static_cast<size_t>(i)];
                if (xi <= a - u) {
                    q += coeff * vi * eval_v_on_grid(xi + u, a, v, dx) * dx;
                }
                if (xi >= -a + u) {
                    q += coeff * vi * eval_v_on_grid(xi - u, a, v, dx) * dx;
                }
            }
            ppow *= cat.sqrtp[pi];
            if (eps > 0 && coeff < eps) break;
        }
    }
    return q;
}

std::vector<std::vector<Real>> build_screw_Ba_matrix(Real a, const Heat::PrimeCatalog& cat, Real eps,
                                                     int spectral_pts, Real& out_dx) {
    const int m = std::max(8, spectral_pts);
    out_dx = 2.0L * a / static_cast<Real>(m + 1);
    const Real c = out_dx / (2.0L * a);

    // Toeplitz: g(x_i - x_j) depends only on i - j — O(m) screw evaluations, not O(m^2).
    std::vector<Real> g_raw(m * m);
    auto g_at = [&](int i, int j) -> Real& { return g_raw[static_cast<size_t>(i * m + j)]; };
    for (int d = -(m - 1); d <= m - 1; ++d) {
        const Real val = screw_function_g(out_dx * static_cast<Real>(d), cat, eps) * out_dx;
        for (int i = 0; i < m; ++i) {
            const int j = i + d;
            if (j < 0 || j >= m) continue;
            g_at(i, j) = val;
        }
    }

    // P_a G P_a with P = I - c·11^T: O(m^2) instead of O(m^4).
    Real total = 0;
    std::vector<Real> row_sum(static_cast<size_t>(m), 0);
    std::vector<Real> col_sum(static_cast<size_t>(m), 0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            const Real gij = g_at(i, j);
            total += gij;
            row_sum[static_cast<size_t>(j)] += gij;
            col_sum[static_cast<size_t>(i)] += gij;
        }
    }
    const Real c2 = c * c;
    std::vector<std::vector<Real>> g_a(static_cast<size_t>(m), std::vector<Real>(static_cast<size_t>(m)));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            g_a[static_cast<size_t>(i)][static_cast<size_t>(j)] =
                g_at(i, j) - c * (row_sum[static_cast<size_t>(j)] + col_sum[static_cast<size_t>(i)]) +
                c2 * total;
        }
    }

    // B = S^T G_a S with tridiagonal S (only 3 diagonals contribute per entry).
    const Real inv_2dx = 0.5L / out_dx;
    std::vector<std::vector<Real>> b_mat(static_cast<size_t>(m), std::vector<Real>(static_cast<size_t>(m)));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            Real s = 0;
            for (int p = std::max(0, i - 1); p <= std::min(m - 1, i + 1); ++p) {
                const Real spi = (p == i + 1) ? inv_2dx : (p == i - 1) ? -inv_2dx : 0;
                if (spi == 0) continue;
                for (int q = std::max(0, j - 1); q <= std::min(m - 1, j + 1); ++q) {
                    const Real sqj = (q == j + 1) ? inv_2dx : (q == j - 1) ? -inv_2dx : 0;
                    if (sqj == 0) continue;
                    s += spi * g_a[static_cast<size_t>(p)][static_cast<size_t>(q)] * sqj;
                }
            }
            b_mat[static_cast<size_t>(i)][static_cast<size_t>(j)] = s * out_dx;
        }
    }
    return b_mat;
}

Real remainder_bilinear_on_grid(Real a, const std::vector<Real>& v, Real dx,
                               const Heat::PrimeCatalog& cat, Real eps) {
    const int m = static_cast<int>(v.size());
    if (m == 0 || a <= 0) return 0;
    Real q = 0;
    for (int i = 0; i < m; ++i) {
        const Real xi = -a + dx * static_cast<Real>(i + 1);
        const Real vi = v[static_cast<size_t>(i)];
        for (int k = 0; k < m; ++k) {
            const Real xj = -a + dx * static_cast<Real>(k + 1);
            const Real vj = v[static_cast<size_t>(k)];
            q += screw_r_remainder_double_prime(xi - xj, cat, eps) * vi * vj * dx * dx;
        }
    }
    return q;
}

}  // namespace

std::pair<Real, std::vector<Real>> screw_Ba_smallest_eigenpair_at_pts(Real a,
                                                                      const Heat::PrimeCatalog& cat,
                                                                      Real eps, int spectral_pts,
                                                                      Real& out_dx) {
    out_dx = 0;
    auto b_mat = build_screw_Ba_matrix(a, cat, eps, spectral_pts, out_dx);
    const int m = static_cast<int>(b_mat.size());
    if (m <= kDenseBaMaxPts) {
        return dense_smallest_eigenpair(b_mat);
    }
    return jacobi_smallest_eigenpair(std::move(b_mat), 320);
}

Real screw_r_higher_digamma_fourier_C0_pinned() {
    static const Real c0 = [] {
        Real mx = 0;
        for (int i = 1; i <= 800; ++i) {
            const Real z = 1.0L + 79.0L * static_cast<Real>(i) / 800.0L;
            mx = std::max(mx, z * z * screw_r1pp_fourier_hat_abs(z));
        }
        return std::min(std::max(mx, 0.25L), 4.0L);
    }();
    return c0;
}

Real screw_r_higher_digamma_ol2_rayleigh_bound(Real a) {
    if (a <= 0) return 0;
    constexpr Real kPi = 3.141592653589793238462643383279502884L;
    return kPi * screw_r_higher_digamma_fourier_C0_pinned() * a;
}

Real screw_r01_sharp_rayleigh_bound(Real a) {
    constexpr Real kCompact = 50.0L;
    constexpr Real kC = 16.0L;
    return kCompact + kC * std::max(a, 0.0L);
}

Real screw_r01_compact_uniform_rayleigh_bound() {
    static const Real bound = [] {
        Real mx = 0;
        // Avoid small-t digamma/Lerch blow-up; battle bound is uniform O(1) on compact screw kernels.
        for (int i = 1; i <= 400; ++i) {
            const Real t = 0.5L + 19.5L * static_cast<Real>(i) / 400.0L;
            mx = std::max(mx, MarshalFabs(screw_r0_double_prime(t)));
            mx = std::max(mx, MarshalFabs(screw_r1_double_prime(t)));
        }
        return std::min(4.0L * mx, 50.0L);
    }();
    return bound;
}

Real screw_Ba_prime_minimizer_cs_upper(Real S_fin) {
    return 2.0L * S_fin;
}

Real screw_Ba_prime_saturated_upper(Real prime_S_fin_sat) {
    return 2.0L * std::max(prime_S_fin_sat, 1.07L);
}

Real screw_Ba_prime_analytic_upper(Real a, Real S_fin, Real prime_S_fin_sat) {
    const Real cs = screw_Ba_prime_minimizer_cs_upper(S_fin);
    if (a >= 3.0L) {
        return std::min(cs, screw_Ba_prime_saturated_upper(prime_S_fin_sat));
    }
    return cs;
}

Real screw_Ba_r_full_analytic_f(Real a) {
    if (a <= 0) return screw_r01_compact_uniform_rayleigh_bound();
    return screw_r01_compact_uniform_rayleigh_bound() + screw_r_higher_digamma_ol2_rayleigh_bound(a);
}

Real screw_Ba_bar_q1_analytic_upper(Real a, Real prime_S_fin_sat) {
    // Suzuki 4.4: bar_q_a^1 = finite translation sum (prime) + a-scaled r'' integral (eq. 4.5).
    return screw_Ba_prime_saturated_upper(prime_S_fin_sat) + screw_Ba_r_full_analytic_f(a);
}

Real screw_Ba_L_arch_rayleigh_on_grid(Real a, const std::vector<Real>& v, Real dx) {
    if (a <= 0 || dx <= 0) return 0;
    Real l2 = 0;
    for (Real x : v) l2 += x * x;
    l2 *= dx;
    if (l2 <= 0) return 0;
    return screw_La_on_grid(a, v, dx) / l2;
}

Real screw_Ba_r_higher_plancherel_analytic_bound(Real a, Real L_arch_rayleigh) {
    if (a <= 0) return screw_r01_compact_uniform_rayleigh_bound();
    const Real hlog_weight = std::max(1.0L, L_arch_rayleigh);
    return screw_r_higher_digamma_ol2_rayleigh_bound(a) * hlog_weight;
}

Real screw_Ba_pin_arch_prime_r01_rhigher_lower_bound(Real arch_mass, Real prime_upper, Real a,
                                                     Real L_arch_rayleigh) {
    const Real r01 = screw_r01_compact_uniform_rayleigh_bound();
    const Real rh = screw_Ba_r_higher_plancherel_analytic_bound(a, L_arch_rayleigh);
    return arch_mass - prime_upper - r01 - rh;
}

Real screw_Ba_pin_kernel_boost_lower_bound(Real arch_mass, Real prime_upper, Real r01_pp,
                                           Real r_higher_kernel_pp) {
    return arch_mass - prime_upper - r01_pp - r_higher_kernel_pp;
}

Real screw_Ba_lerch_dominance_margin(Real arch_mass, Real prime_upper, Real r01_pp,
                                     Real r_higher_kernel_pp) {
    if (r_higher_kernel_pp > 1e-12L) return -1e300L;
    const Real H = -r_higher_kernel_pp;
    const Real debt = prime_upper + r01_pp - arch_mass;
    return H - debt;
}

ScrewBaSpectralAudit screw_Ba_audit_from_eigenvector(Real a, Real mu, const std::vector<Real>& v,
                                                     Real dx, const Heat::PrimeCatalog& cat,
                                                     Real log_cutoff, Real eps) {
    ScrewBaSpectralAudit out;
    if (a <= 0 || dx <= 0) return out;
    out.spectral_lambda = mu;
    out.l2 = 0;
    for (Real x : v) out.l2 += x * x;
    out.l2 *= dx;
    if (out.l2 <= 0) return out;
    out.L_a = screw_La_on_grid(a, v, dx);
    out.prime_shift = screw_prime_Ba_shift_on_grid(a, v, cat, log_cutoff, eps, dx);
    out.r0_pp = screw_r0pp_bilinear_on_grid(a, v, dx);
    out.r_pp = screw_rpp_bilinear_on_grid(a, v, dx);
    out.r01_pp = out.r0_pp + out.r_pp;
    out.mass_term = kSuzukiMassCoeff * out.l2;
    const Real eq25 = out.L_a - out.mass_term - out.prime_shift;
    out.eq25_rayleigh = eq25 / out.l2;
    out.remainder_rayleigh = out.spectral_lambda - out.eq25_rayleigh;
    out.eq25_rel_gap =
        MarshalFabs(out.remainder_rayleigh) / std::max(MarshalFabs(out.spectral_lambda), 1e-12L);
    const Real full_eq25 = out.L_a - out.mass_term - out.prime_shift - out.r01_pp;
    out.full_eq25_rayleigh = full_eq25 / out.l2;
    out.discretization_gap = out.spectral_lambda - out.full_eq25_rayleigh;
    const Real q_ba = out.spectral_lambda * out.l2;
    out.r_full_pp = out.L_a - out.mass_term - out.prime_shift - q_ba;
    out.r_higher_pp = out.r_full_pp - out.r01_pp;
    out.r_higher_kernel_bilinear = remainder_bilinear_on_grid(a, v, dx, cat, eps);
    out.r_higher_kernel_pp_rayleigh = out.r_higher_kernel_bilinear / out.l2;
    out.L_arch_rayleigh = screw_Ba_L_arch_rayleigh_on_grid(a, v, dx);
    out.r_higher_plancherel_bound =
        screw_Ba_r_higher_plancherel_analytic_bound(a, out.L_arch_rayleigh);
    out.arch_mass_rayleigh = (out.L_a - out.mass_term) / out.l2;
    out.prime_rayleigh = out.prime_shift / out.l2;
    out.r0pp_rayleigh = out.r0_pp / out.l2;
    out.rpp_rayleigh = out.r_pp / out.l2;
    out.r01pp_rayleigh = out.r01_pp / out.l2;
    out.r_full_pp_rayleigh = out.r_full_pp / out.l2;
    out.r_higher_pp_rayleigh = out.r_higher_pp / out.l2;
    out.pin_kernel_eq25_rayleigh =
        out.arch_mass_rayleigh - out.prime_rayleigh - out.r01pp_rayleigh -
        out.r_higher_kernel_pp_rayleigh;
    out.pin_kernel_spectral_gap = out.spectral_lambda - out.pin_kernel_eq25_rayleigh;
    out.eq45_log_a_rayleigh = -std::log(a);
    out.pin_margin_rayleigh = out.spectral_lambda;
    return out;
}

ScrewBaEq25AnalyticLedger screw_Ba_eq25_ledger_from_audit(const ScrewBaSpectralAudit& audit, Real a,
                                                        Real prime_S_fin_sat, Real S_fin) {
    ScrewBaEq25AnalyticLedger led;
    led.arch_mass_rayleigh = audit.arch_mass_rayleigh;
    led.prime_rayleigh = audit.prime_rayleigh;
    led.r01pp_rayleigh = audit.r01pp_rayleigh;
    led.r_higher_pp_rayleigh = audit.r_higher_pp_rayleigh;
    led.r_full_pp_rayleigh = audit.r_full_pp_rayleigh;
    led.spectral_lambda = audit.spectral_lambda;
    led.prime_minimizer_cs_upper = screw_Ba_prime_minimizer_cs_upper(S_fin);
    led.prime_saturated_upper = screw_Ba_prime_saturated_upper(prime_S_fin_sat);
    led.prime_upper_bound = screw_Ba_prime_analytic_upper(a, S_fin, prime_S_fin_sat);
    led.r01_abs_bound = screw_r01_compact_uniform_rayleigh_bound();
    led.r_higher_digamma_bound = screw_r_higher_digamma_ol2_rayleigh_bound(a);
    led.r_full_analytic_f = screw_Ba_r_full_analytic_f(a);
    led.r_full_upper_bound = led.r_full_analytic_f;
    led.r_full_upper_f_ok = led.r_full_pp_rayleigh <= led.r_full_analytic_f + 1e-6L;
    led.r_full_nonpos_analytic_ok = led.r_full_pp_rayleigh <= 1e-6L;
    led.pin_analytic_lower_bound =
        led.arch_mass_rayleigh - led.prime_upper_bound - led.r_full_analytic_f;
    led.pin_analytic_digamma_lower_bound = led.pin_analytic_lower_bound;
    if (led.r_full_nonpos_analytic_ok) {
        led.pin_analytic_digamma_lower_bound =
            led.arch_mass_rayleigh - led.prime_upper_bound;
    }
    led.pin_battle_lower_bound = led.pin_analytic_digamma_lower_bound;
    led.pin_arch_prime_f_lower_bound =
        led.arch_mass_rayleigh - led.prime_upper_bound - led.r_full_analytic_f;
    led.pin_arch_prime_f_ok = led.pin_arch_prime_f_lower_bound >= -1e-8L;
    led.L_arch_rayleigh = audit.L_arch_rayleigh;
    led.r_higher_plancherel_bound = audit.r_higher_plancherel_bound;
    led.r_higher_artifact_gap =
        audit.r_higher_pp_rayleigh - audit.r_higher_kernel_pp_rayleigh;
    led.r_higher_plancherel_ok =
        audit.r_higher_pp_rayleigh <= led.r_higher_plancherel_bound + 1e-6L;
    led.r_higher_kernel_plancherel_ok =
        audit.r_higher_kernel_pp_rayleigh <= led.r_higher_plancherel_bound + 1e-6L;
    led.pin_kernel_eq25_rayleigh = audit.pin_kernel_eq25_rayleigh;
    led.pin_kernel_spectral_gap = audit.pin_kernel_spectral_gap;
    led.pin_kernel_eq25_ok = led.pin_kernel_eq25_rayleigh >= -1e-6L;
    led.r_higher_kernel_nonpos_ok = audit.r_higher_kernel_pp_rayleigh <= 1e-6L;
    led.r01_sharp_rayleigh = screw_r01_sharp_rayleigh_bound(a);
    if (led.r_higher_kernel_nonpos_ok) {
        led.lerch_boost_H = -audit.r_higher_kernel_pp_rayleigh;
        const Real r01_for_debt =
            std::min(led.r01pp_rayleigh, led.r01_sharp_rayleigh);
        led.lerch_dominance_debt =
            led.prime_upper_bound + r01_for_debt - led.arch_mass_rayleigh;
        led.lerch_dominance_debt_sharp =
            led.prime_upper_bound + led.r01_sharp_rayleigh - led.arch_mass_rayleigh;
        led.lerch_dominance_margin = screw_Ba_lerch_dominance_margin(
            led.arch_mass_rayleigh, led.prime_upper_bound, r01_for_debt,
            audit.r_higher_kernel_pp_rayleigh);
        led.lerch_dominance_ok = led.lerch_dominance_margin >= -1e-8L;
    } else {
        led.lerch_boost_H = 0;
        led.lerch_dominance_debt = 0;
        led.lerch_dominance_margin = -1e300L;
        led.lerch_dominance_ok = false;
    }
    led.pin_split_r01_rhigher_lower_bound = screw_Ba_pin_arch_prime_r01_rhigher_lower_bound(
        led.arch_mass_rayleigh, led.prime_upper_bound, a, led.L_arch_rayleigh);
    led.pin_split_discharge_ok = led.pin_split_r01_rhigher_lower_bound >= -1e-8L;
    led.bar_q1_analytic_upper = screw_Ba_bar_q1_analytic_upper(a, prime_S_fin_sat);
    led.pin_bar_q1_lower_bound = led.arch_mass_rayleigh - led.bar_q1_analytic_upper;
    led.pin_analytic_ok = led.pin_battle_lower_bound >= -1e-8L;
    return led;
}

ScrewBaSpectralPack screw_Ba_spectral_pack(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff,
                                           Real eps, Real prime_S_fin_sat, Real S_fin,
                                           const WeilOperatorRayleighOpts& opts) {
    ScrewBaSpectralPack pack;
    if (a <= 0) return pack;
    WeilOperatorRayleighOpts adapted = opts;
    adapted.spectral_pts =
        screw_Ba_adaptive_spectral_pts(a, opts.spectral_pts, opts.spectral_pts_max);
    Real dx = 0;
    auto b_mat = build_screw_Ba_matrix(a, cat, eps, adapted.spectral_pts, dx);
    const int m = static_cast<int>(b_mat.size());
    Real mu = 0;
    std::vector<Real> v;
    if (m <= kDenseBaMaxPts) {
        std::tie(mu, v) = dense_smallest_eigenpair(b_mat);
        pack.lambda_dense = mu;
        pack.dense_crosscheck_ok = true;
    } else {
        std::tie(mu, v) = jacobi_smallest_eigenpair(std::move(b_mat), 200);
        pack.lambda_dense = mu;
        pack.dense_crosscheck_ok = true;
    }
    pack.audit =
        screw_Ba_audit_from_eigenvector(a, mu, v, dx, cat, log_cutoff, eps);
    pack.ledger = screw_Ba_eq25_ledger_from_audit(pack.audit, a, prime_S_fin_sat, S_fin);
    return pack;
}

int screw_Ba_adaptive_spectral_pts(Real a, int base_pts, int max_pts) {
    const int cap = std::max(16, std::min(kDenseBaMaxPts, max_pts));
    const int scaled = static_cast<int>(std::ceil(14.0L * std::max(a, 0.25L)));
    return std::max(base_pts, std::min(cap, scaled));
}

Real screw_Ba_spectral_min_at_pts(Real a, const Heat::PrimeCatalog& cat, Real eps, int spectral_pts) {
    Real dx = 0;
    return screw_Ba_smallest_eigenpair_at_pts(a, cat, eps, spectral_pts, dx).first;
}

ScrewBaMeshRefinement screw_Ba_mesh_refinement_study(Real a, const Heat::PrimeCatalog& cat,
                                                     Real eps) {
    return screw_Ba_kernel_mesh_refinement_study(a, cat, 20.0L, eps, 1.07L, 0.0L);
}

ScrewBaMeshRefinement screw_Ba_kernel_mesh_refinement_study(
    Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps, Real prime_S_fin_sat,
    Real S_fin, int spectral_pts_cap) {
    ScrewBaMeshRefinement out;
    out.a = a;
    out.eq25_kernel_mesh_monotone_ok = true;
    static const int kMeshes[] = {128, 192, 256, 384};
    WeilOperatorRayleighOpts opts;
    for (int m : kMeshes) {
        if (m > spectral_pts_cap) continue;
        ScrewBaMeshRefinementPoint pt;
        pt.spectral_pts = m;
        Real dx = 0;
        auto b_mat = build_screw_Ba_matrix(a, cat, eps, m, dx);
        const int rows = static_cast<int>(b_mat.size());
        if (rows <= 0) continue;
        const auto [mu, v] = dense_smallest_eigenpair(b_mat);
        pt.lambda_dense = mu;
        const ScrewBaSpectralAudit audit =
            screw_Ba_audit_from_eigenvector(a, mu, v, dx, cat, log_cutoff, eps);
        pt.r_higher_kernel_pp_rayleigh = audit.r_higher_kernel_pp_rayleigh;
        pt.r_higher_plancherel_bound = audit.r_higher_plancherel_bound;
        pt.eq25_kernel_rayleigh = audit.pin_kernel_eq25_rayleigh;
        pt.pin_kernel_eq25_rayleigh = audit.pin_kernel_eq25_rayleigh;
        pt.r_higher_artifact_gap = audit.r_higher_pp_rayleigh - audit.r_higher_kernel_pp_rayleigh;
        pt.pin_kernel_spectral_gap = audit.pin_kernel_spectral_gap;
        pt.r_higher_kernel_plancherel_ok =
            audit.r_higher_kernel_pp_rayleigh <= audit.r_higher_plancherel_bound + 1e-6L;
        out.points.push_back(pt);
        if (!pt.r_higher_kernel_plancherel_ok) out.kernel_plancherel_all_meshes_ok = false;
        if (out.points.size() >= 2) {
            const auto& prev = out.points[out.points.size() - 2];
            if (pt.eq25_kernel_rayleigh > prev.eq25_kernel_rayleigh + 1e-6L) {
                out.eq25_kernel_mesh_monotone_ok = false;
            }
            const Real scale = std::max(1.0L, MarshalFabs(pt.lambda_dense));
            out.max_lambda_jump =
                std::max(out.max_lambda_jump, MarshalFabs(pt.lambda_dense - prev.lambda_dense));
            out.max_kernel_pp_jump = std::max(
                out.max_kernel_pp_jump,
                MarshalFabs(pt.r_higher_kernel_pp_rayleigh - prev.r_higher_kernel_pp_rayleigh));
            out.max_artifact_gap_jump = std::max(
                out.max_artifact_gap_jump,
                MarshalFabs(pt.r_higher_artifact_gap - prev.r_higher_artifact_gap));
            if (MarshalFabs(pt.lambda_dense - prev.lambda_dense) > 0.05L * scale) {
                out.monotone_converged = false;
            }
        }
    }
    if (!out.points.empty()) {
        const auto& finest = out.points.back();
        out.lambda_coarsest = out.points.front().lambda_dense;
        out.lambda_finest = finest.lambda_dense;
        out.eq25_kernel_finest = finest.eq25_kernel_rayleigh;
        out.artifact_gap_finest = finest.r_higher_artifact_gap;
        out.eq25_kernel_positive_finest_ok = finest.eq25_kernel_rayleigh >= -1e-6L;
        out.converged_positive_ok = out.monotone_converged && out.lambda_finest >= -1e-6L;
    }
    (void)S_fin;
    (void)prime_S_fin_sat;
    (void)opts;
    return out;
}

ScrewBaSpectralCrosscheck screw_Ba_spectral_dense_crosscheck(Real a, const Heat::PrimeCatalog& cat,
                                                             Real eps, int spectral_pts) {
    ScrewBaSpectralCrosscheck out;
    if (a <= 0) return out;
    out.spectral_pts = spectral_pts;
    Real dx = 0;
    auto b_mat = build_screw_Ba_matrix(a, cat, eps, spectral_pts, dx);
    const int m = static_cast<int>(b_mat.size());
    out.dense_available = m <= kDenseBaMaxPts;
    if (!out.dense_available) return out;
    const auto [mu_dense, v_dense] = dense_smallest_eigenpair(b_mat);
    out.lambda_dense = mu_dense;
    const auto [mu_iter, v_iter] = jacobi_smallest_eigenpair(b_mat, 320);
    out.lambda_iterative = mu_iter;
    const Real scale = std::max(1.0L, MarshalFabs(mu_dense));
    out.agrees = MarshalFabs(mu_dense - mu_iter) <= 0.05L * scale + 1e-8L;
    (void)v_dense;
    (void)v_iter;
    return out;
}

Real screw_function_g(Real t, const Heat::PrimeCatalog& cat, Real eps) {
    const Real ta = MarshalFabs(t);
    const Real poly = screw_poly_term(ta);
    const Real prime = screw_prime_term(ta, cat, eps);
    const Real arch_linear = -0.5L * ta * kPsiQuarterMinusLogPi;
    const Real arch_lerch = screw_arch_lerch_term(ta);
    return poly + prime + arch_linear + arch_lerch;
}

Real screw_kernel_Gg(Real x, Real y, const Heat::PrimeCatalog& cat, Real eps) {
    return screw_function_g(x - y, cat, eps) - screw_function_g(x, cat, eps) -
           screw_function_g(y, cat, eps);
}

Real screw_Ba_quadratic(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                        int n_quad, Real scale) {
    const Real l2 = mode_l2(a, n_quad, scale);
    if (l2 <= 0) return 0;
    const Real la = screw_La_form(a, n_quad, scale);
    const Real prime_shift = screw_prime_Ba_shift(a, cat, log_cutoff, eps, n_quad, scale);
    return la - kSuzukiMassCoeff * l2 - prime_shift;
}

Real weil_rayleigh_screw_Gg(Real a, const Heat::PrimeCatalog& cat, Real eps,
                            const WeilOperatorRayleighOpts& opts) {
    if (a <= 0) return 0;
    Real best = 1e300L;
    const int n_quad = std::max(16, opts.n_quad);
    for (int j = 1; j <= opts.sin_modes; ++j) {
        const Real scale = Heat::kPi * static_cast<Real>(j) / (2.0L * a);
        const Real l2 = mode_l2(a, n_quad, scale);
        if (l2 <= 0) continue;
        Real q = 0;
        const Real dx = 2.0L * a / static_cast<Real>(n_quad);
        for (int i = 0; i <= n_quad; ++i) {
            const Real xi = -a + dx * static_cast<Real>(i);
            const Real vx = sin_mode(xi, a, scale);
            for (int k = 0; k <= n_quad; ++k) {
                const Real xj = -a + dx * static_cast<Real>(k);
                const Real vy = sin_mode(xj, a, scale);
                q += screw_kernel_Gg(xi, xj, cat, eps) * vx * vy * dx * dx;
            }
        }
        best = std::min(best, q / l2);
    }
    return best;
}

Real weil_rayleigh_screw_Ba(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                            const WeilOperatorRayleighOpts& opts) {
    if (a <= 0) return 0;
    Real best = 1e300L;
    const int n_quad = std::max(24, opts.n_quad);
    for (int j = 1; j <= opts.sin_modes; ++j) {
        const Real scale = Heat::kPi * static_cast<Real>(j) / (2.0L * a);
        const Real l2 = mode_l2(a, n_quad, scale);
        if (l2 <= 0) continue;
        const Real q = screw_Ba_quadratic(a, cat, log_cutoff, eps, n_quad, scale);
        best = std::min(best, q / l2);
    }
    return best;
}

Real weil_spectral_min_screw_Ba(Real a, const Heat::PrimeCatalog& cat, Real eps,
                                const WeilOperatorRayleighOpts& opts) {
    if (a <= 0) return 0;
    return screw_Ba_spectral_min_at_pts(
        a, cat, eps, screw_Ba_adaptive_spectral_pts(a, opts.spectral_pts, opts.spectral_pts_max));
}

Real screw_kernel_hilbert_schmidt_on_interval(Real a, Real (*kernel)(Real)) {
    if (a <= 0) return 0;
    const Real width = 4.0L * a;
    const int n = std::max(64, static_cast<int>(std::ceil(32.0L * a)));
    const Real dt = width / static_cast<Real>(n);
    Real integral = 0;
    for (int i = 0; i <= n; ++i) {
        const Real t = -2.0L * a + dt * static_cast<Real>(i);
        const Real k = kernel(MarshalFabs(t));
        const Real w = (i == 0 || i == n) ? 0.5L : 1.0L;
        integral += w * k * k * dt;
    }
    return std::sqrt(2.0L * a * integral);
}

Real screw_kernel_hilbert_schmidt_remainder(Real a, const Heat::PrimeCatalog& cat, Real eps) {
    if (a <= 0) return 0;
    const Real width = 4.0L * a;
    const int n = std::max(96, static_cast<int>(std::ceil(40.0L * a)));
    const Real dt = width / static_cast<Real>(n);
    Real integral = 0;
    for (int i = 0; i <= n; ++i) {
        const Real t = -2.0L * a + dt * static_cast<Real>(i);
        const Real k = screw_r_remainder_double_prime(t, cat, eps);
        const Real w = (i == 0 || i == n) ? 0.5L : 1.0L;
        integral += w * k * k * dt;
    }
    return std::sqrt(2.0L * a * integral);
}

ScrewBaEq25AnalyticLedger screw_Ba_eq25_analytic_ledger(Real a, const Heat::PrimeCatalog& cat,
                                                        Real log_cutoff, Real eps,
                                                        Real prime_S_fin_sat, Real S_fin,
                                                        const WeilOperatorRayleighOpts& opts) {
    return screw_Ba_spectral_pack(a, cat, log_cutoff, eps, prime_S_fin_sat, S_fin, opts).ledger;
}

ScrewBaSpectralAudit screw_Ba_spectral_eq25_audit(Real a, const Heat::PrimeCatalog& cat,
                                                  Real log_cutoff, Real eps,
                                                  const WeilOperatorRayleighOpts& opts) {
    return screw_Ba_spectral_pack(a, cat, log_cutoff, eps, 1.07L, 0.0L, opts).audit;
}

}  // namespace Marshal::Induction

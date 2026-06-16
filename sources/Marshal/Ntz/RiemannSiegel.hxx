#pragma once
// Riemann-Siegel Hardy Z(t) with Gabcke remainder (libHGT / Gabcke 1979).
#include <cmath>
#include <cstdint>
#include <vector>
#include "GabckeCoeffs.hxx"

namespace weil_rs {

inline constexpr double kPi = 3.141592653589793238462643383279502884;
inline constexpr int kAdjPPowers = 88;

inline double siegel_theta(double t) {
    if (t <= 0) return 0;
    const double th = 0.5 * t * (std::log(t / (2.0 * kPi)) - 1.0) - kPi / 8.0;
    const double inv_t = 1.0 / t;
    return th + inv_t * (1.0 / 48.0 + inv_t * inv_t * (-7.0 / 23040.0));
}

inline int rs_main_terms(double t) {
    if (t < 1.0) return 1;
    return std::max(1, static_cast<int>(std::floor(std::sqrt(t / (2.0 * kPi)))));
}

struct RSTable {
    mutable std::vector<double> inv_sqrt;
    mutable std::vector<double> log_n;
    mutable int nmax = 0;

    void ensure(int N) const {
        if (N <= nmax) return;
        inv_sqrt.resize(static_cast<size_t>(N) + 1);
        log_n.resize(static_cast<size_t>(N) + 1);
        for (int n = nmax + 1; n <= N; ++n) {
            inv_sqrt[static_cast<size_t>(n)] = 1.0 / std::sqrt(static_cast<double>(n));
            log_n[static_cast<size_t>(n)] = std::log(static_cast<double>(n));
        }
        nmax = N;
    }
};

inline double rs_remainder(double t) {
    const double t_over_2pi = t / (2.0 * kPi);
    const double T = std::sqrt(t_over_2pi);
    const int N = static_cast<int>(std::floor(T));
    const double P = T - static_cast<double>(N);
    const double tf = std::pow(t_over_2pi, -0.25);
    double factor = tf;
    if (N % 2 == 0) factor = -factor;

    const double adj_p = 1.0 - 2.0 * P;
    double pow_p[kAdjPPowers];
    pow_p[0] = 1.0;
    for (int k = 1; k < kAdjPPowers; ++k) pow_p[k] = pow_p[k - 1] * adj_p;

    double total = 0.0;
    for (int j = 0; j < kGabckeTerms; ++j) {
        const int even_odd = j % 2;
        double cj = 0.0;
        for (int i = 0; i < kGabckeCoeffsPerTerm; ++i)
            cj += kGabcke[j][i] * pow_p[2 * i + even_odd];
        total += cj * std::pow(tf, 2.0 * static_cast<double>(j));
    }
    return factor * total;
}

inline double hardy_Z_sum(const RSTable& tab, double t, int N) {
    const double theta = siegel_theta(t);
    double sum = 0;
    for (int n = 1; n <= N; ++n) {
        const double phase = theta - t * tab.log_n[static_cast<size_t>(n)];
        sum += tab.inv_sqrt[static_cast<size_t>(n)] * std::cos(phase);
    }
    return 2.0 * sum;
}

inline double hardy_Z(const RSTable& tab, double t) {
    const int N = rs_main_terms(t);
    tab.ensure(N);
    return hardy_Z_sum(tab, t, N) + rs_remainder(t);
}

inline double hardy_Z_deriv(const RSTable& tab, double t) {
    const double h = std::max(1e-6, t * 1e-8);
    return (hardy_Z(tab, t + h) - hardy_Z(tab, t - h)) / (2.0 * h);
}

struct RefineResult {
    double gamma_in = 0;
    double gamma_out = 0;
    double z_before = 0;
    double z_after = 0;
    int iterations = 0;
    bool converged = false;
};

inline RefineResult refine_zero(RSTable& tab, double gamma, int max_iter = 12, double tol = 1e-12,
                               double bracket_lo = 0.0, double bracket_hi = 0.0) {
    RefineResult r;
    r.gamma_in = gamma;
    r.gamma_out = gamma;
    r.z_before = hardy_Z(tab, gamma);

    if (bracket_hi > bracket_lo) {
        double best_t = 0.5 * (bracket_lo + bracket_hi);
        double best_z = 1e300;
        for (int k = 0; k <= 64; ++k) {
            const double t =
                bracket_lo + (bracket_hi - bracket_lo) * static_cast<double>(k) / 64.0;
            const double z = std::fabs(hardy_Z(tab, t));
            if (z < best_z) {
                best_z = z;
                best_t = t;
            }
        }
        r.gamma_out = best_t;
        r.z_before = hardy_Z(tab, r.gamma_out);
    }

    for (int it = 0; it < max_iter; ++it) {
        const double z = hardy_Z(tab, r.gamma_out);
        if (std::fabs(z) < tol) {
            r.z_after = z;
            r.iterations = it + 1;
            r.converged = true;
            return r;
        }
        const double zp = hardy_Z_deriv(tab, r.gamma_out);
        if (std::fabs(zp) < 1e-30) break;
        const double step = z / zp;
        r.gamma_out -= std::max(-0.25, std::min(0.25, step));
        if (bracket_hi > bracket_lo) {
            r.gamma_out = std::max(bracket_lo, std::min(bracket_hi, r.gamma_out));
        }
        r.iterations = it + 1;
    }
    r.z_after = hardy_Z(tab, r.gamma_out);
    const double zb = std::fabs(r.z_before);
    double za = std::fabs(r.z_after);
    bool accept =
        za < tol || za < 1e-4 || (za < zb && za < 0.01 * std::max(zb, 1e-12));
    if (!accept && bracket_hi > bracket_lo) {
        double lo = bracket_lo;
        double hi = bracket_hi;
        double zlo = hardy_Z(tab, lo);
        double zhi = hardy_Z(tab, hi);
        if (zlo * zhi <= 0.0) {
            for (int it = 0; it < 80; ++it) {
                const double mid = 0.5 * (lo + hi);
                const double zm = hardy_Z(tab, mid);
                if (std::fabs(zm) < tol) {
                    r.gamma_out = mid;
                    r.z_after = zm;
                    za = std::fabs(zm);
                    accept = true;
                    r.converged = true;
                    break;
                }
                if (zlo * zm <= 0.0) {
                    hi = mid;
                    zhi = zm;
                } else {
                    lo = mid;
                    zlo = zm;
                }
            }
            if (!accept) {
                r.gamma_out = 0.5 * (lo + hi);
                r.z_after = hardy_Z(tab, r.gamma_out);
                za = std::fabs(r.z_after);
                accept = za < tol || za < 1e-4;
                r.converged = accept;
            }
        }
    }
    if (!accept) {
        r.gamma_out = gamma;
        r.z_after = r.z_before;
        r.converged = zb < 1e-3;
        return r;
    }
    r.converged = za < std::max(tol * 10.0, 1e-3);
    return r;
}

}  // namespace weil_rs

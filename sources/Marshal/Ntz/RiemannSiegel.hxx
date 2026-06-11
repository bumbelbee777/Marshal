#pragma once
// Riemann-Siegel Hardy Z(t) for zero refinement (leading sum + theta asymptotic).
#include <cmath>
#include <cstdint>
#include <vector>

namespace weil_rs {

inline constexpr double kPi = 3.141592653589793238462643383279502884;

inline double siegel_theta(double t) {
    if (t <= 0) return 0;
    // θ(t) = (t/2)(log(t/2π) - 1) - π/8 + 1/(48t) - 7/(23040 t³) + ...
    const double th = 0.5 * t * (std::log(t / (2.0 * kPi)) - 1.0) - kPi / 8.0;
    const double inv_t = 1.0 / t;
    return th + inv_t * (1.0 / 48.0 + inv_t * inv_t * (-7.0 / 23040.0));
}

inline int rs_main_terms(double t) {
    if (t < 1.0) return 1;
    return std::max(1, static_cast<int>(std::floor(std::sqrt(t / (2.0 * kPi)))));
}

// Precomputed 1/sqrt(n), log(n) for n=1..N_max — reused across a batch.
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

inline double hardy_Z_sum(const RSTable& tab, double t, int N) {
    const double theta = siegel_theta(t);
    double sum = 0;
    #ifdef _OPENMP
    #pragma omp simd reduction(+ : sum)
    #endif
    for (int n = 1; n <= N; ++n) {
        const double phase = theta - t * tab.log_n[static_cast<size_t>(n)];
        sum += tab.inv_sqrt[static_cast<size_t>(n)] * std::cos(phase);
    }
    // Leading Riemann-Siegel main sum ≈ Z(t) near the critical line.
    double z = 2.0 * sum;
    // First remainder correction (Gabcke) for t ≳ 100.
    if (t > 100.0 && N >= 1) {
        const double rt = std::pow(t / (2.0 * kPi), -0.25);
        const double phase = theta - t * tab.log_n[static_cast<size_t>(N)];
        z += rt * tab.inv_sqrt[static_cast<size_t>(N)] * std::cos(phase) * 0.5;
    }
    return z;
}

inline double hardy_Z(const RSTable& tab, double t) {
    const int N = rs_main_terms(t);
    tab.ensure(N);
    return hardy_Z_sum(tab, t, N);
}

inline double hardy_Z_deriv(const RSTable& tab, double t) {
    const double h = std::max(1e-6, t * 1e-10);
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

inline RefineResult refine_zero(RSTable& tab, double gamma, int max_iter = 8, double tol = 1e-14) {
    RefineResult r;
    r.gamma_in = gamma;
    r.gamma_out = gamma;
    r.z_before = hardy_Z(tab, gamma);
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
        r.gamma_out -= z / zp;
        r.iterations = it + 1;
    }
    r.z_after = hardy_Z(tab, r.gamma_out);
    r.converged = std::fabs(r.z_after) < tol * 10.0;
    return r;
}

}  // namespace weil_rs

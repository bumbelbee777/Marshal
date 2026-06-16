#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "Numerics/Real.hxx"

namespace Marshal::Heat {

inline constexpr int kConnesBasisCap = 2500;

struct ConnesBasisPlan {
    int n_primes = 0;
    int k_twist = 0;
    int n_modes = 0;
};

inline ConnesBasisPlan plan_connes_basis(int n_primes, int kmax,
                                         int basis_cap = kConnesBasisCap) {
    ConnesBasisPlan plan;
    if (n_primes <= 0 || kmax <= 0 || basis_cap <= 0) return plan;

    int k = std::min(kmax, 12);
    while (k > 1 && n_primes * k > basis_cap) --k;

    int n = n_primes;
    if (n * k > basis_cap) n = basis_cap / k;
    if (n < 1) {
        n = std::min(n_primes, basis_cap);
        k = std::max(1, std::min(k, basis_cap / std::max(n, 1)));
    }

    plan.n_primes = n;
    plan.k_twist = k;
    plan.n_modes = n * k;
    return plan;
}

inline std::vector<int> cap_primes_for_connes(const std::vector<int>& primes, int n_cap) {
    if (n_cap <= 0 || primes.size() <= static_cast<size_t>(n_cap)) return primes;
    return std::vector<int>(primes.begin(), primes.begin() + n_cap);
}

inline int sturm_count_below(const std::vector<Real>& diag, const std::vector<Real>& off,
                             Real mu) {
    const int n = static_cast<int>(diag.size());
    if (n == 0) return 0;
    int count = 0;
    Real p_prev = 1;
    Real p = diag[0] - mu;
    if (p <= 0) ++count;
    for (int k = 1; k < n; ++k) {
        const Real b = off[static_cast<size_t>(k - 1)];
        const Real p_next = (diag[static_cast<size_t>(k)] - mu) * p - b * b * p_prev;
        if (p_next <= 0) ++count;
        p_prev = p;
        p = p_next;
    }
    return count;
}

/// All eigenvalues of symmetric tridiagonal (diag, off) via Sturm bisection — O(n^2 log 1/eps).
inline std::vector<Real> tridiagonal_symmetric_eigenvalues(std::vector<Real> diag,
                                                          std::vector<Real> off,
                                                          Real tol = 1e-12L) {
    const int n = static_cast<int>(diag.size());
    if (n == 0) return {};
    if (n == 1) return {diag[0]};

    Real lo = diag[0] - std::fabs(off[0]);
    Real hi = diag[0] + std::fabs(off[0]);
    for (int i = 0; i < n; ++i) {
        Real rad = std::fabs(diag[static_cast<size_t>(i)]);
        if (i > 0) rad += std::fabs(off[static_cast<size_t>(i - 1)]);
        if (i + 1 < n) rad += std::fabs(off[static_cast<size_t>(i)]);
        lo = std::min(lo, diag[static_cast<size_t>(i)] - rad);
        hi = std::max(hi, diag[static_cast<size_t>(i)] + rad);
    }
    lo -= 1;
    hi += 1;

    std::vector<Real> eig(static_cast<size_t>(n), 0);
    for (int i = 0; i < n; ++i) {
        Real a = lo;
        Real b = hi;
        for (int iter = 0; iter < 200; ++iter) {
            const Real mid = 0.5L * (a + b);
            const int c = sturm_count_below(diag, off, mid);
            if (c <= i)
                a = mid;
            else
                b = mid;
            if (b - a <= tol * std::max(Real{1}, std::fabs(mid))) break;
        }
        eig[static_cast<size_t>(i)] = 0.5L * (a + b);
    }
    return eig;
}

}  // namespace Marshal::Heat

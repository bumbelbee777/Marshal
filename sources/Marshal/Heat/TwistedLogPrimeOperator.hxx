#pragma once

#include <cmath>
#include <vector>
#include "LogPrimeOperator.hxx"

namespace Marshal::Heat {

/// Finite Q^× coupling: modes (p,k) and (q,l) interact when p^k = q^l.
struct TwistedLogPrimeOperator {
    std::vector<LogPrimeOperator> local_ops;
    Real coupling_strength = 0;

    struct ModeIndex {
        int prime_idx = 0;
        int k = 0;
    };

    std::vector<ModeIndex> build_basis(int kmax) const {
        std::vector<ModeIndex> basis;
        for (size_t pi = 0; pi < local_ops.size(); ++pi) {
            for (int k = 1; k <= kmax; ++k) basis.push_back({static_cast<int>(pi), k});
        }
        return basis;
    }

    static bool same_prime_power(int p, int k, int q, int l) {
        if (p == q) return k == l;
        const Real lhs = static_cast<Real>(k) * std::log(static_cast<Real>(p));
        const Real rhs = static_cast<Real>(l) * std::log(static_cast<Real>(q));
        return std::fabs(lhs - rhs) <= 1e-12L * std::max(Real{1}, std::max(lhs, rhs));
    }

    std::vector<Real> compute_eigenvalues(int kmax) const {
        const auto basis = build_basis(kmax);
        const int n = static_cast<int>(basis.size());
        if (n == 0) return {};
        std::vector<Real> H(static_cast<size_t>(n) * static_cast<size_t>(n), 0);

        auto idx = [&](int i, int j) { return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j); };

        for (int i = 0; i < n; ++i) {
            const auto& mi = basis[static_cast<size_t>(i)];
            const Real diag = local_ops[static_cast<size_t>(mi.prime_idx)].eigenvalue(mi.k);
            H[idx(i, i)] = diag;
            for (int j = i + 1; j < n; ++j) {
                const auto& mj = basis[static_cast<size_t>(j)];
                const int p = local_ops[static_cast<size_t>(mi.prime_idx)].prime;
                const int q = local_ops[static_cast<size_t>(mj.prime_idx)].prime;
                Real off = 0;
                if (mi.prime_idx != mj.prime_idx &&
                    same_prime_power(p, mi.k, q, mj.k)) {
                    off = coupling_strength;
                }
                H[idx(i, j)] = off;
                H[idx(j, i)] = off;
            }
        }

        // Symmetric Jacobi-style diagonalization via naive power iteration on smallest/largest
        // For v1: use std::vector + simple QR not available — use analytic diagonal approx
        // when coupling is small: return diagonal + perturbation estimate.
        // Full diagonalization: Jacobi eigenvalue algorithm for small n.
        std::vector<Real> a = H;
        const int max_iter = 100 * n * n;
        std::vector<Real> eig(n, 0);
        std::vector<Real> V(static_cast<size_t>(n) * static_cast<size_t>(n), 0);
        for (int i = 0; i < n; ++i) V[idx(i, i)] = 1.0L;

        for (int iter = 0; iter < max_iter; ++iter) {
            Real max_off = 0;
            int p = 0, q = 1;
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    const Real v = std::fabs(a[idx(i, j)]);
                    if (v > max_off) {
                        max_off = v;
                        p = i;
                        q = j;
                    }
                }
            }
            if (max_off < 1e-14L) break;

            const Real app = a[idx(p, p)];
            const Real aqq = a[idx(q, q)];
            const Real apq = a[idx(p, q)];
            const Real phi = 0.5L * std::atan2(2.0L * apq, aqq - app);
            const Real c = std::cos(phi);
            const Real s = std::sin(phi);

            for (int k = 0; k < n; ++k) {
                const Real akp = a[idx(k, p)];
                const Real akq = a[idx(k, q)];
                a[idx(k, p)] = c * akp - s * akq;
                a[idx(p, k)] = a[idx(k, p)];
                a[idx(k, q)] = s * akp + c * akq;
                a[idx(q, k)] = a[idx(k, q)];
            }
            const Real new_pp = c * c * app - 2.0L * s * c * apq + s * s * aqq;
            const Real new_qq = s * s * app + 2.0L * s * c * apq + c * c * aqq;
            a[idx(p, p)] = new_pp;
            a[idx(q, q)] = new_qq;
            a[idx(p, q)] = a[idx(q, p)] = 0;

            for (int k = 0; k < n; ++k) {
                const Real vkp = V[idx(k, p)];
                const Real vkq = V[idx(k, q)];
                V[idx(k, p)] = c * vkp - s * vkq;
                V[idx(k, q)] = s * vkp + c * vkq;
            }
        }

        for (int i = 0; i < n; ++i) eig[static_cast<size_t>(i)] = a[idx(i, i)];
        std::sort(eig.begin(), eig.end());
        return eig;
    }

    Real sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax) const {
        if (T <= 0 || gammas.empty()) return 0;
        const auto evals = compute_eigenvalues(kmax);
        Kahan lhs, rhs;
        for (Real lambda : evals) lhs.add(LogPrimeGlobal::sinc_sq(lambda / T));
        for (Real gamma : gammas) rhs.add(LogPrimeGlobal::sinc_sq(gamma / T));
        return std::fabs(lhs.total() - rhs.total());
    }
};

}  // namespace Marshal::Heat

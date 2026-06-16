#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "ConnesBasisCap.hxx"
#include "ConnesCouplingMode.hxx"
#include "LogPrimeGlobal.hxx"
#include "LogPrimeOperator.hxx"

namespace Marshal::Heat {

/// Finite Q× coupling on log-prime modes (p,k) with eigenvalue k log p.
struct TwistedLogPrimeOperator {
    std::vector<LogPrimeOperator> local_ops;
    Real coupling_strength = 0;
    ConnesCouplingMode coupling_mode = ConnesCouplingMode::LogLadder;

    struct ModeIndex {
        int prime_idx = 0;
        int k = 0;
        Real u = 0;
        Real weil_weight = 0;
    };

    std::vector<ModeIndex> build_basis(int kmax) const {
        std::vector<ModeIndex> basis;
        for (size_t pi = 0; pi < local_ops.size(); ++pi) {
            const auto& op = local_ops[pi];
            Real ppow = op.sqrt_p;
            for (int k = 1; k <= kmax; ++k) {
                const Real u = op.eigenvalue(k);
                const Real w = op.log_p / ppow;
                basis.push_back({static_cast<int>(pi), k, u, w});
                ppow *= op.sqrt_p;
            }
        }
        std::sort(basis.begin(), basis.end(),
                  [](const ModeIndex& a, const ModeIndex& b) { return a.u < b.u; });
        return basis;
    }

    static bool same_prime_power(int p, int k, int q, int l) {
        if (p == q) return k == l;
        const Real lhs = static_cast<Real>(k) * std::log(static_cast<Real>(p));
        const Real rhs = static_cast<Real>(l) * std::log(static_cast<Real>(q));
        return std::fabs(lhs - rhs) <= 1e-12L * std::max(Real{1}, std::max(lhs, rhs));
    }

    static void jacobi_diagonalize(int n, std::vector<Real>& H, std::vector<Real>& eig) {
        auto idx = [&](int i, int j) {
            return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j);
        };
        const int max_iter = 100 * n * n;
        for (int iter = 0; iter < max_iter; ++iter) {
            Real max_off = 0;
            int p = 0, q = 1;
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    const Real v = std::fabs(H[idx(i, j)]);
                    if (v > max_off) {
                        max_off = v;
                        p = i;
                        q = j;
                    }
                }
            }
            if (max_off < 1e-14L) break;

            const Real app = H[idx(p, p)];
            const Real aqq = H[idx(q, q)];
            const Real apq = H[idx(p, q)];
            const Real phi = 0.5L * std::atan2(2.0L * apq, aqq - app);
            const Real c = std::cos(phi);
            const Real s = std::sin(phi);

            for (int k = 0; k < n; ++k) {
                const Real akp = H[idx(k, p)];
                const Real akq = H[idx(k, q)];
                H[idx(k, p)] = c * akp - s * akq;
                H[idx(p, k)] = H[idx(k, p)];
                H[idx(k, q)] = s * akp + c * akq;
                H[idx(q, k)] = H[idx(k, q)];
            }
            const Real new_pp = c * c * app - 2.0L * s * c * apq + s * s * aqq;
            const Real new_qq = s * s * app + 2.0L * s * c * apq + c * c * aqq;
            H[idx(p, p)] = new_pp;
            H[idx(q, q)] = new_qq;
            H[idx(p, q)] = H[idx(q, p)] = 0;
        }
        for (int i = 0; i < n; ++i) eig[static_cast<size_t>(i)] = H[idx(i, i)];
        std::sort(eig.begin(), eig.end());
    }

    std::vector<Real> compute_eigenvalues(int kmax) const {
        const auto plan = plan_connes_basis(static_cast<int>(local_ops.size()), kmax);
        TwistedLogPrimeOperator sub = *this;
        if (plan.n_primes < static_cast<int>(sub.local_ops.size()))
            sub.local_ops.resize(static_cast<size_t>(plan.n_primes));
        const auto basis = sub.build_basis(plan.k_twist);
        const int n = static_cast<int>(basis.size());
        if (n == 0) return {};

        if (coupling_mode == ConnesCouplingMode::LogLadder) {
            std::vector<Real> diag(static_cast<size_t>(n));
            std::vector<Real> off(static_cast<size_t>(n > 1 ? n - 1 : 0));
            for (int i = 0; i < n; ++i)
                diag[static_cast<size_t>(i)] = basis[static_cast<size_t>(i)].u;
            if (coupling_strength != 0) {
                for (int i = 0; i + 1 < n; ++i) {
                    const Real wi = basis[static_cast<size_t>(i)].weil_weight;
                    const Real wj = basis[static_cast<size_t>(i + 1)].weil_weight;
                    off[static_cast<size_t>(i)] =
                        coupling_strength * std::sqrt(std::max(wi * wj, Real{0}));
                }
            }
            return tridiagonal_symmetric_eigenvalues(diag, off);
        }

        std::vector<Real> H(static_cast<size_t>(n) * static_cast<size_t>(n), 0);
        auto idx = [&](int i, int j) {
            return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j);
        };

        for (int i = 0; i < n; ++i) {
            H[idx(i, i)] = basis[static_cast<size_t>(i)].u;
        }

        for (int i = 0; i < n; ++i) {
            const auto& mi = basis[static_cast<size_t>(i)];
            for (int j = i + 1; j < n; ++j) {
                const auto& mj = basis[static_cast<size_t>(j)];
                const int p = sub.local_ops[static_cast<size_t>(mi.prime_idx)].prime;
                const int q = sub.local_ops[static_cast<size_t>(mj.prime_idx)].prime;
                if (mi.prime_idx != mj.prime_idx && same_prime_power(p, mi.k, q, mj.k)) {
                    H[idx(i, j)] = coupling_strength;
                    H[idx(j, i)] = coupling_strength;
                }
            }
        }

        std::vector<Real> eig(static_cast<size_t>(n), 0);
        jacobi_diagonalize(n, H, eig);
        return eig;
    }

    Real sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax,
                        Real kappa = 1.0L) const {
        if (T <= 0 || gammas.empty()) return 0;
        const auto evals = compute_eigenvalues(kmax);
        Kahan lhs, rhs;
        for (Real lambda : evals)
            lhs.add(LogPrimeGlobal::sinc_sq_weil(kappa * lambda / T));
        for (Real gamma : gammas)
            rhs.add(LogPrimeGlobal::sinc_sq_weil(kappa * gamma / T));
        return std::fabs(lhs.total() - rhs.total());
    }
};

}  // namespace Marshal::Heat

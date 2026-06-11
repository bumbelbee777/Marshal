#pragma once

#include <cmath>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "Compat.hxx"
#include "Common.hxx"
#include "LogPrimeOperator.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Heat {

struct LogPrimeGlobal {
    std::vector<LogPrimeOperator> operators;

    static LogPrimeGlobal from_primes(const std::vector<int>& primes) {
        LogPrimeGlobal g;
        g.operators.reserve(primes.size());
        for (int p : primes) g.operators.push_back(LogPrimeOperator::from_prime(p));
        return g;
    }

    Real weil_prime_sum(const TestFunction& tf, int kmax, Real eps) const {
        Real total = 0;
        const int n = static_cast<int>(operators.size());
        #ifdef _OPENMP
        #pragma omp parallel for schedule(static, kPrimeBatch) reduction(+ : total)
        #endif
        for (int i = 0; i < n; ++i)
            total += operators[static_cast<size_t>(i)].weil_prime_sum(tf, kmax, eps);
        return total;
    }

    Real p_weight_sum(const TestFunction& tf, int kmax, Real eps) const {
        Real total = 0;
        const int n = static_cast<int>(operators.size());
        #ifdef _OPENMP
        #pragma omp parallel for schedule(static, kPrimeBatch) reduction(+ : total)
        #endif
        for (int i = 0; i < n; ++i)
            total += operators[static_cast<size_t>(i)].p_weight_sum(tf, kmax, eps);
        return total;
    }

    Real weil_prime_sum(Real t, int kmax, Real eps = 0) const {
        Real total = 0;
        for (const auto& op : operators) total += op.weil_prime_block(t, kmax, eps);
        return total;
    }

    static Real sinc_sq(Real x) {
        if (std::fabs(x) < 1e-30L) return 1.0L;
        const Real s = std::sin(kPi * x) / (kPi * x);
        return s * s;
    }

    Real zero_sinc2_sum(const std::vector<Real>& gammas, Real T) const {
        Real total = 0;
        for (Real gamma : gammas) total += sinc_sq(gamma / T);
        return total;
    }

    Real sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax) const {
        if (T <= 0 || gammas.empty()) return 0;
        Real lhs = 0;
        const int n = static_cast<int>(operators.size());
        #ifdef _OPENMP
        #pragma omp parallel for schedule(static, kPrimeBatch) reduction(+ : lhs)
        #endif
        for (int i = 0; i < n; ++i) {
            const auto& op = operators[static_cast<size_t>(i)];
            for (int k = 1; k <= kmax; ++k) lhs += sinc_sq(op.eigenvalue(k) / T);
        }
        return std::fabs(lhs - zero_sinc2_sum(gammas, T));
    }

    Real p_weight_sinc2_sum(Real T, int kmax, Real eps = 0) const {
        Real sum = 0;
        const int n = static_cast<int>(operators.size());
        #ifdef _OPENMP
        #pragma omp parallel for schedule(static, kPrimeBatch) reduction(+ : sum)
        #endif
        for (int i = 0; i < n; ++i) {
            const auto& op = operators[static_cast<size_t>(i)];
            Real ppow = op.sqrt_p;
            for (int k = 1; k <= kmax; ++k) {
                const Real term = (1.0L / ppow) * sinc_sq(op.eigenvalue(k) / T);
                sum += term;
                ppow *= op.sqrt_p;
                if (eps > 0 && term < eps) break;
            }
        }
        return sum;
    }

    Real p_weight_sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax,
                                 Real eps = 0) const {
        return std::fabs(p_weight_sinc2_sum(T, kmax, eps) - zero_sinc2_sum(gammas, T));
    }

    Real weil_prime_sinc2_sum(Real T, int kmax, Real eps = 0) const {
        Real sum = 0;
        const int n = static_cast<int>(operators.size());
        #ifdef _OPENMP
        #pragma omp parallel for schedule(static, kPrimeBatch) reduction(+ : sum)
        #endif
        for (int i = 0; i < n; ++i) {
            const auto& op = operators[static_cast<size_t>(i)];
            Real ppow = op.sqrt_p;
            for (int k = 1; k <= kmax; ++k) {
                const Real term = (op.log_p / ppow) * sinc_sq(op.eigenvalue(k) / T);
                sum += term;
                ppow *= op.sqrt_p;
                if (eps > 0 && term < eps) break;
            }
        }
        return sum;
    }

    Real weil_prime_sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax,
                                   Real eps = 0) const {
        return std::fabs(weil_prime_sinc2_sum(T, kmax, eps) - zero_sinc2_sum(gammas, T));
    }

    int count_below(Real T, int kmax) const {
        int n = 0;
        for (const auto& op : operators) {
            for (int k = 1; k <= kmax; ++k) {
                if (op.eigenvalue(k) <= T) ++n;
            }
        }
        return n;
    }
};

}  // namespace Marshal::Heat

#pragma once

#include <cmath>
#include "Common.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Heat {

/// Local log-prime block: spectrum lambda_k = k log p.
struct LogPrimeOperator {
    int prime = 0;
    Real log_p = 0;
    Real sqrt_p = 0;

    LogPrimeOperator() = default;
    LogPrimeOperator(int p, Real lp, Real sp) : prime(p), log_p(lp), sqrt_p(sp) {}

    static LogPrimeOperator from_prime(int p) {
        const Real lp = std::log(static_cast<Real>(p));
        return LogPrimeOperator(p, lp, std::sqrt(static_cast<Real>(p)));
    }

    Real eigenvalue(int k) const { return static_cast<Real>(k) * log_p; }

    /// p^{-k/2} heat trace (NOT the Weil prime side — missing log p).
    Real p_weight_heat_trace(Real t, int kmax, Real eps = 0) const {
        Kahan acc;
        Real ppow = sqrt_p;
        for (int k = 1; k <= kmax; ++k) {
            const Real lambda = eigenvalue(k);
            const Real term = (1.0L / ppow) * std::exp(-t * lambda * lambda);
            acc.add(term);
            ppow *= sqrt_p;
            if (eps > 0 && term < eps) break;
        }
        return acc.total();
    }

    /// Actual Weil prime block: sum (log p)/p^{k/2} exp(-t (k log p)^2).
    Real weil_prime_block(Real t, int kmax, Real eps = 0) const {
        Kahan acc;
        Real ppow = sqrt_p;
        for (int k = 1; k <= kmax; ++k) {
            const Real u = static_cast<Real>(k) * log_p;
            const Real term = (log_p / ppow) * std::exp(-t * u * u);
            acc.add(term);
            ppow *= sqrt_p;
            if (eps > 0 && term < eps) break;
        }
        return acc.total();
    }

    /// Weil prime sum for test function h: sum (log p)/p^{k/2} * 2*h_hat(k log p).
    /// Matches HeatCylinderOp::prime_block_raw / Marshal trace prime convention.
    Real weil_prime_sum(const TestFunction& tf, int kmax, Real eps) const {
        Kahan acc;
        Real ppow = sqrt_p;
        for (int k = 1; k <= kmax; ++k) {
            const Real u = static_cast<Real>(k) * log_p;
            const Real term = (log_p / ppow) * 2.0L * tf.h_hat(u);
            acc.add(term);
            if (eps > 0 && std::fabs(term) < eps) break;
            ppow *= sqrt_p;
        }
        return acc.total();
    }

    /// p^{-k/2} trace: sum p^{-k/2} * 2*h_hat(k log p) — differs from Weil by factor log p.
    Real p_weight_sum(const TestFunction& tf, int kmax, Real eps) const {
        Kahan acc;
        Real ppow = sqrt_p;
        for (int k = 1; k <= kmax; ++k) {
            const Real u = static_cast<Real>(k) * log_p;
            const Real term = (1.0L / ppow) * 2.0L * tf.h_hat(u);
            acc.add(term);
            if (eps > 0 && std::fabs(term) < eps) break;
            ppow *= sqrt_p;
        }
        return acc.total();
    }

};

}  // namespace Marshal::Heat

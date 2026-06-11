#pragma once

#include "LogPrimeGlobal.hxx"
#include "TwistedLogPrimeOperator.hxx"
#include <vector>

namespace Marshal::Heat {

/// Finite Q^× crossed-product sketch: local H_log blocks coupled when p^k = q^l.
struct ConnesCrossedProduct {
    std::vector<LogPrimeOperator> local_ops;
    Real coupling_strength = 0;

    static ConnesCrossedProduct from_primes(const std::vector<int>& primes, Real lambda = 0) {
        ConnesCrossedProduct cp;
        cp.local_ops = LogPrimeGlobal::from_primes(primes).operators;
        cp.coupling_strength = lambda;
        return cp;
    }

    TwistedLogPrimeOperator as_twisted() const {
        TwistedLogPrimeOperator t;
        t.local_ops = local_ops;
        t.coupling_strength = coupling_strength;
        return t;
    }

    std::vector<Real> spectrum(int kmax) const { return as_twisted().compute_eigenvalues(kmax); }

    Real weil_heat_trace(Real t, int kmax, Real eps = 0) const {
        LogPrimeGlobal g;
        g.operators = local_ops;
        return g.weil_prime_sum(t, kmax, eps);
    }

    Real sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax) const {
        return as_twisted().sinc2_residual(gammas, T, kmax);
    }

    Real weil_sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax, Real eps = 0) const {
        LogPrimeGlobal g;
        g.operators = local_ops;
        return g.weil_prime_sinc2_residual(gammas, T, kmax, eps);
    }
};

}  // namespace Marshal::Heat

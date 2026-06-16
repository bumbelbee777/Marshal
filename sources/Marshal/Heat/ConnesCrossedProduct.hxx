#pragma once

#include "ConnesCouplingMode.hxx"
#include "ConnesCrossedValidation.hxx"
#include "LogPrimeGlobal.hxx"
#include "TwistedLogPrimeOperator.hxx"
#include <algorithm>
#include <cmath>
#include <vector>

namespace Marshal::Heat {

/// Finite Q^× crossed-product: local H_log blocks + Connes-style coupling on the log ladder.
struct ConnesCrossedProduct {
    std::vector<LogPrimeOperator> local_ops;
    Real coupling_strength = 0;
    ConnesCouplingMode coupling_mode = ConnesCouplingMode::LogLadder;

    static ConnesCrossedProduct from_primes(const std::vector<int>& primes, Real lambda = 0,
                                            ConnesCouplingMode mode = ConnesCouplingMode::LogLadder) {
        ConnesCrossedProduct cp;
        cp.local_ops = LogPrimeGlobal::from_primes(primes).operators;
        cp.coupling_strength = lambda;
        cp.coupling_mode = mode;
        return cp;
    }

    TwistedLogPrimeOperator as_twisted() const {
        TwistedLogPrimeOperator t;
        t.local_ops = local_ops;
        t.coupling_strength = coupling_strength;
        t.coupling_mode = coupling_mode;
        return t;
    }

    std::vector<Real> spectrum(int kmax) const { return as_twisted().compute_eigenvalues(kmax); }

    static ConnesSpectrumMetrics spectrum_vs_zeros(const std::vector<Real>& evals,
                                                   const std::vector<Real>& gammas) {
        ConnesSpectrumMetrics m;
        m.n_modes = static_cast<int>(evals.size());
        m.n_zeros_matched = static_cast<int>(std::min(evals.size(), gammas.size()));
        if (m.n_zeros_matched == 0) return m;

        Real sum_sq = 0;
        Real max_g = 0;
        Real mean_g = 0;
        for (int i = 0; i < m.n_zeros_matched; ++i) {
            const Real d = evals[static_cast<size_t>(i)] - gammas[static_cast<size_t>(i)];
            sum_sq += d * d;
            const Real ad = std::fabs(d);
            max_g = std::max(max_g, ad);
            mean_g += ad;
        }
        m.rmse = std::sqrt(sum_sq / static_cast<Real>(m.n_zeros_matched));
        m.max_gap = max_g;
        m.mean_gap = mean_g / static_cast<Real>(m.n_zeros_matched);
        return m;
    }

    ConnesSpectrumMetrics spectrum_metrics(const std::vector<Real>& gammas, int kmax) const {
        return spectrum_vs_zeros(spectrum(kmax), gammas);
    }

    Real weil_heat_trace(Real t, int kmax, Real eps = 0) const {
        LogPrimeGlobal g;
        g.operators = local_ops;
        return g.weil_prime_sum(t, kmax, eps);
    }

    Real sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax,
                        Real kappa = 1.0L) const {
        return as_twisted().sinc2_residual(gammas, T, kmax, kappa);
    }

    Real weil_sinc2_residual(const std::vector<Real>& gammas, Real T, int kmax, Real kappa = 1.0L,
                             Real eps = 0) const {
        LogPrimeGlobal g;
        g.operators = local_ops;
        return g.weil_prime_sinc2_residual(gammas, T, kmax, kappa, eps);
    }

    Real spectrum_rmse(const std::vector<Real>& gammas, int kmax) const {
        return spectrum_metrics(gammas, kmax).rmse;
    }
};

}  // namespace Marshal::Heat

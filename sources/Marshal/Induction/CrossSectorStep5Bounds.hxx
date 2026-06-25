#pragma once

#include <vector>

#include "CrossSectorWeilOperator.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

/// Per-sector quadratic form values at the Rayleigh-minimizing sin mode (Step 5 audit).
struct WeilQuadraticSectorBreakdown {
    Real q_pf = 0;
    Real q_prime = 0;
    Real q_zero = 0;
    Real q_total = 0;
    Real rayleigh = 0;
    int mode_index = 0;
};

/// Unconditional Cauchy–Schwarz upper bound: Q_prime^a(f) <= 2 * S_fin(a) * ||f||^2
/// with S_fin(a) = sum_{n <= exp(2a)} Lambda(n)/sqrt(n).
Real weil_prime_weight_sum(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps);

Real weil_quadratic_prime_cs_bound(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps);

/// Full-kernel Rayleigh min + sector breakdown at the winning mode.
WeilQuadraticSectorBreakdown weil_rayleigh_sector_breakdown(
    Real a, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat, Real log_cutoff,
    Real zero_T, Real eps, const WeilOperatorRayleighOpts& opts = {});

}  // namespace Marshal::Induction

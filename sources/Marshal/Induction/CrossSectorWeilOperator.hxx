#pragma once

#include <vector>

#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

/// Cross-sector Phase 4A: full Weil kernel K_a = K_arch(Pf) + K_prime + K_zero on [-a,a].
/// T_a f(x) = integral K_a(x,y) f(y) dy; lambda_a = inf Rayleigh quotient on L2(-a,a).

struct WeilOperatorRayleighOpts {
    int sin_modes = 6;
    int n_quad = 40;
    int spectral_pts = 24;
    /// Cap for screw_Ba adaptive mesh (512 = full audit; 96 = CI fast path).
    int spectral_pts_max = 512;
    bool include_zero = true;
};

/// Rayleigh min over sin modes on Pf + prime-delta kernel (legacy partial proxy).
Real weil_rayleigh_partial(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                           const WeilOperatorRayleighOpts& opts = {});

/// Rayleigh min over sin modes on full kernel (Pf + prime + zero cos sum up to T=2a).
Real weil_rayleigh_full(Real a, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat,
                       Real log_cutoff, Real zero_T, Real eps,
                       const WeilOperatorRayleighOpts& opts = {});

/// Smallest eigenvalue of symmetric discretization of full K_a (n x n on uniform grid).
Real weil_spectral_min_full(Real a, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat,
                            Real log_cutoff, Real zero_T, Real eps,
                            const WeilOperatorRayleighOpts& opts = {});

}  // namespace Marshal::Induction

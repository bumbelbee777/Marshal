#pragma once

#include "ArchimedeanBoundary.hxx"
#include "BerryKeatingOperator.hxx"
#include "ConnesBasisCap.hxx"
#include "ConnesCouplingMode.hxx"
#include "TwistedLogPrimeOperator.hxx"
#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat {

/// Finite Connes Dirac on adele class truncation: BK archimedean sector ⊗ coupled log-prime
/// crossed-product generator (not a bare BK heat-kernel proxy).
struct CombinedConnesDiracSpec {
    BerryKeatingSpec bk;
    Real theta = 0;
    ArchimedeanBoundarySpec arch;
    Real coupling_lambda = 0.5L;
    ConnesCouplingMode coupling_mode = ConnesCouplingMode::LogLadder;
    int kmax = 12;
    int arch_cap = 80;
    int combined_cap = 400;
};

struct CombinedConnesDiracResult {
    std::vector<Real> eigenvalues;
    int n_arch = 0;
    int n_prime = 0;
    Real spectral_action_heat = 0;
};

/// Λ_D(f) proxy: multi-scale Tr(exp(−D²/σ²)) on the **combined** generator spectrum.
Real combined_spectral_action_heat(const std::vector<Real>& eigenvalues, Real scale_base,
                                   int n_scales);

CombinedConnesDiracResult build_combined_dirac_spectrum(const CombinedConnesDiracSpec& spec,
                                                        const std::vector<int>& primes);

}  // namespace Marshal::Heat

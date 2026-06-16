#pragma once

#include "GLnArchimedeanOperator.hxx"
#include "GLnTwistedCoupling.hxx"
#include "ArchimedeanBoundary.hxx"
#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat {

struct CombinedConnesDiracSpec;

}  // namespace Marshal::Heat

namespace Marshal::Heat::GLn {

/// Rank-parametric Connes Dirac generator spec.
struct MarshalGLnDiracSpec {
    int rank = 1;
    GLnArchimedeanSpec arch;
    GLnTwistedCouplingSpec coupling;
    int combined_cap = 400;

    MarshalGLnDiracSpec with_rank(int r) const {
        MarshalGLnDiracSpec out = *this;
        out.rank = r;
        out.arch.rank = r;
        out.coupling.rank = r;
        if (r == 1) {
            out.arch.preset = GLnArchPreset::BerryKeating;
            out.coupling.coupling_lambda = coupling.coupling_lambda;
        } else if (r == 2) {
            out.arch.preset = GLnArchPreset::MaassH2;
        } else if (r == 3) {
            out.arch.preset = GLnArchPreset::HitchinK3Stub;
            out.coupling.coupling_lambda = 0;
        } else if (r >= 4) {
            out.arch.preset = GLnArchPreset::CliffordStub;
            out.coupling.coupling_lambda = 0;
        }
        return out;
    }
};

struct MarshalGLnDiracResult {
    int rank = 1;
    std::vector<Real> eigenvalues;
    int n_arch = 0;
    int n_prime = 0;
    Real spectral_action_heat = 0;
    int kernel_multiplicity = 0;
};

Real gln_spectral_action_heat(const std::vector<Real>& eigenvalues, Real scale_base, int n_scales);

MarshalGLnDiracResult build_gln_dirac_spectrum(const MarshalGLnDiracSpec& spec,
                                               const std::vector<int>& primes);

/// Map legacy GL(1) CombinedConnesDiracSpec → MarshalGLnDiracSpec.
MarshalGLnDiracSpec marshal_gln_spec_from_combined(const Marshal::Heat::CombinedConnesDiracSpec& spec);

}  // namespace Marshal::Heat::GLn

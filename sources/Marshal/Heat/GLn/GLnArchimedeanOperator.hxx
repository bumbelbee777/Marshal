#pragma once

#include "BerryKeatingOperator.hxx"
#include "ArchimedeanBoundary.hxx"
#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat::GLn {

/// Rank-parametric archimedean sector preset.
enum class GLnArchPreset {
    BerryKeating = 1,
    MaassH2 = 2,
    MaassEllipseHeegner = 5,
    HitchinK3Stub = 3,
    CliffordStub = 4,
};

struct GLnArchimedeanSpec {
    int rank = 1;
    GLnArchPreset preset = GLnArchPreset::BerryKeating;
    BerryKeatingSpec bk;
    Real theta = 0;
    ArchimedeanBoundarySpec arch;
    int arch_cap = 80;
};

/// Build archimedean eigenvalue ladder for the given rank preset.
std::vector<Real> build_gln_archimedean_ladder(const GLnArchimedeanSpec& spec);

}  // namespace Marshal::Heat::GLn

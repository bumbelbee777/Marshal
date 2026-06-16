#pragma once

#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat::GLn {

/// Count eigenvalues with |λ| < tol (kernel multiplicity witness).
int ladder_count_kernel(const std::vector<Real>& eig, Real tol);

/// Maass H² grid relative gap: compare excited arch ladder modes to λ_j = sqrt(1/4 + (jπ/θ)²).
Real ladder_maass_grid_rel_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol);

/// Smallest |λ| among non-kernel archimedean modes (Sha/resolvent gap witness).
Real ladder_min_positive_arch(const std::vector<Real>& arch_ladder, Real tol);

/// Major/minor arc split from spectral heat weights exp(-|λ|/θ).
struct LadderArcSplit {
    Real major_arc_mass = 0;
    Real minor_arc_bound = 0;
};

LadderArcSplit ladder_arc_split(const std::vector<Real>& eigenvalues, Real theta, Real major_arc_lambda_ub);

/// Maass-index major arc: kernel + first `major_maass_levels` excited arch modes.
Real ladder_maass_major_arc_mass(const std::vector<Real>& arch_ladder, Real theta, Real tol,
                                 int major_maass_levels);

/// Analytic tail upper bound: exp(-2π·levels/θ) (circle-method minor arc decay).
Real ladder_maass_minor_arc_tail_bound(Real theta, int major_maass_levels);

}  // namespace Marshal::Heat::GLn

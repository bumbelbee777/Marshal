#pragma once

#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat::GLn {

/// Count eigenvalues with |λ| < tol (kernel multiplicity witness).
int ladder_count_kernel(const std::vector<Real>& eig, Real tol);

/// Maass H² grid relative gap: compare excited arch ladder modes to λ_j = sqrt(1/4 + (jπ/θ)²).
Real ladder_maass_grid_rel_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol);

/// Grid pointwise det vs L relative gap at `s = s_re + i/n` (genus-1 partial products).
Real ladder_maass_grid_det_l_rel_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol,
                                     Real s_re, int grid_count = 8);

/// Uniform Cauchy holomorphy gap: |gap_decades(det) - gap_decades(L)| on Maass grid.
Real ladder_maass_holomorphy_uniform_gap(const std::vector<Real>& arch_ladder, Real theta,
                                         Real tol, Real s_re, int stride = 2);

std::vector<Real> ladder_excited_arch_gammas(const std::vector<Real>& arch_ladder, Real tol,
                                            int max_levels);

std::vector<Real> ladder_maass_predicted_gammas(Real theta, int max_levels);

void ladder_partial_genus_one_det(Real s_re, Real s_im, const std::vector<Real>& gammas,
                                  Real* out_re, Real* out_im);

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

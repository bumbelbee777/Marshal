#pragma once

#include <vector>
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

// Localized Gauss arch on [-a,a]: (1/2π) ∫ h(t)(Re ψ(1/4+it/2) − log π) dt
Real arch_localized_gauss(Real a, Real sigma, int n_pts = 8001);

// Triangle envelope A(a): (1/2π) ∫_{-a}^{a} h(t)|Re ψ − log π| dt  (always ≥ |arch_localized|)
Real arch_triangle_envelope_gauss(Real a, Real sigma, int n_pts = 8001);

// Exact zero tail Σ_{γ > T} h(γ) from ordinate list
Real zero_tail_exact_gauss(Real T, Real sigma, const std::vector<double>& gammas);

// Unconditional RvM majorant: ∫_T^∞ exp(−x²/2σ²) · (log(x/2π))/(2π) dx  (dominates tail via dN bound)
Real zero_tail_rvm_majorant(Real T, Real sigma, int n_pts = 4001);

// Re ψ(1/4 + i y) for arch / screw-kernel terms
Real re_psi_quarter_plus_iy(Real y);

}  // namespace Marshal::Induction

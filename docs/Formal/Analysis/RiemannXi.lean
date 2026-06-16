import Mathlib.NumberTheory.LSeries.RiemannZeta
import Mathlib.Analysis.SpecialFunctions.Gamma.Basic

/-!
# Completed Riemann ξ function (Landau convention)

`ξ(s) = ½ s(s−1) π^{-s/2} Γ(s/2) ζ(s) = ½ s(s−1) Λ(s)` with `Λ = completedRiemannZeta`.
-/

namespace HPAnalysis

open Complex

/-- Critical-line spectral parameter `s = ½ + it` (Connes/Marshal convention). -/
noncomputable def criticalLineParam (t : ℝ) : ℂ :=
  (1 / 2 : ℂ) + t * Complex.I

/-- Completed Riemann ξ function (entire). -/
noncomputable def riemannXi (s : ℂ) : ℂ :=
  (1 / 2 : ℂ) * s * (s - 1) * completedRiemannZeta s

private theorem riemannXi_polynomial_differentiable : Differentiable ℂ fun s =>
    (1 / 2 : ℂ) * s * (s - 1) := by
  refine Differentiable.mul ?_ ?_
  · exact Differentiable.mul (differentiable_const (1 / 2 : ℂ)) differentiable_id
  · exact Differentiable.sub differentiable_id (differentiable_const (1 : ℂ))

theorem riemannXi_one_sub (s : ℂ) : riemannXi (1 - s) = riemannXi s := by
  unfold riemannXi
  rw [completedRiemannZeta_one_sub]
  congr 1
  ring

end HPAnalysis

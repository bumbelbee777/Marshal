import Analysis.RiemannXi
import Mathlib.NumberTheory.LSeries.RiemannZeta
import Mathlib.Analysis.Analytic.Basic
import Mathlib.Analysis.Complex.CauchyIntegral

/-!
# Completed ξ — holomorphy on the wedge `{1 < Re s}`

On `marshalWedgeIdentityDomain`, `s ≠ 0` and `s ≠ 1`, so `completedRiemannZeta` is differentiable at each point.
-/

namespace HPAnalysis

open Complex Set

def marshalWedgeIdentityDomain : Set ℂ :=
  {s : ℂ | (1 : ℝ) < s.re}

theorem marshalWedgeIdentityDomain_isOpen : IsOpen marshalWedgeIdentityDomain := by
  simpa [marshalWedgeIdentityDomain] using
    isOpen_lt continuous_const continuous_re

theorem riemannXi_differentiable_on_wedge : DifferentiableOn ℂ riemannXi marshalWedgeIdentityDomain := by
  intro s hs
  unfold marshalWedgeIdentityDomain at hs
  simp only [Set.mem_setOf_eq] at hs
  have hs0 : s ≠ 0 := by
    intro h0
    have hre := congrArg Complex.re h0
    simp at hre
    linarith [hs]
  have hs1 : s ≠ 1 := by
    intro h1
    have hre := congrArg Complex.re h1
    simp at hre
    linarith [hs]
  unfold riemannXi
  refine DifferentiableAt.differentiableWithinAt ?_
  refine DifferentiableAt.mul ?_ ?_
  · refine DifferentiableAt.mul ?_ ?_
    · exact DifferentiableAt.mul (differentiableAt_const (1 / 2 : ℂ)) differentiableAt_id
    · exact DifferentiableAt.sub differentiableAt_id (differentiableAt_const (1 : ℂ))
  · exact differentiableAt_completedZeta hs0 hs1

theorem riemannXi_analytic_on_wedge : AnalyticOnNhd ℂ riemannXi marshalWedgeIdentityDomain := by
  rw [analyticOnNhd_iff_differentiableOn marshalWedgeIdentityDomain_isOpen]
  exact riemannXi_differentiable_on_wedge

end HPAnalysis

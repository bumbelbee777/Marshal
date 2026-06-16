import Analysis.RiemannXi
import Mathlib.NumberTheory.LSeries.RiemannZeta

/-!
# ξ zeros from ζ zeros on the critical line

`IsRiemannZeroOrdinate` links certified Odlyzko heights to Mathlib `riemannZeta`.
ξ-zero on the Marshal spectrum is proved in `MarshalHadamardClosure`.
-/

namespace HPAnalysis

open Complex

theorem criticalLineParam_ne_zero (t : ℝ) : criticalLineParam t ≠ 0 := by
  intro h
  have hre := congrArg Complex.re h
  simp [criticalLineParam] at hre

/-- A Riemann zero ordinate: ζ vanishes on the critical line at height `t`. -/
def IsRiemannZeroOrdinate (t : ℝ) : Prop :=
  riemannZeta (criticalLineParam t) = 0

end HPAnalysis

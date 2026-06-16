import Mathlib.NumberTheory.LSeries.HurwitzZeta
import Mathlib.Analysis.SpecialFunctions.Pow.Real
import Mathlib.Data.Real.Basic
import Mathlib.Data.Real.Pi.Bounds
import Analysis.CertifiedBounds

/-!
# Archimedean spectral action — pure scaling Berry–Keating model
-/

namespace HPAnalysis

open scoped Real
open HurwitzZeta Set

noncomputable def twoPi : ℝ := 2 * π

theorem twoPi_pos : 0 < twoPi := by
  unfold twoPi
  linarith [Real.pi_pos]

/-- BK scaling eigenvalue on mode `n`. -/
noncomputable def archEigenvalue (n : ℤ) (θ logRatio : ℝ) : ℝ :=
  (θ + twoPi * n) / logRatio

/-- Hurwitz shift `a = θ/(2π)` (use on `θ ∈ (0, 2π)`). -/
noncomputable def hurwitzShift (θ : ℝ) : ℝ := θ / twoPi

/-- Real part of Hurwitz zeta at real exponent `s > 1`. -/
noncomputable def hurwitzZetaReal (a s : ℝ) : ℝ :=
  (hurwitzZeta a (s : ℂ)).re

/-- Spectral zeta `ζ_{D_θ}(s)` for `s > 1`, pure scaling model. -/
noncomputable def spectralZeta (s θ logRatio : ℝ) : ℝ :=
  |logRatio| ^ s * twoPi ^ (-s) *
    (hurwitzZetaReal (hurwitzShift θ) s + hurwitzZetaReal (1 - hurwitzShift θ) s)

/-- Closed-form second θ-derivative of `spectralZeta s · logRatio`. -/
noncomputable def spectralZetaDeriv2 (s θ logRatio : ℝ) : ℝ :=
  |logRatio| ^ s * twoPi ^ (-(s + 2)) * s * (s + 1) *
    (hurwitzZetaReal (hurwitzShift θ) (s + 2) +
      hurwitzZetaReal (1 - hurwitzShift θ) (s + 2))

/-- Mode-sum spectral action proxy (Schwartz cutoff `f`, scale `Λ`). -/
noncomputable def archSpectralAction (f : ℝ → ℝ) (θ Λ logRatio : ℝ) : ℝ :=
  ∑' n : ℤ, f (archEigenvalue n θ logRatio / Λ)

def fundamentalDomain : Set ℝ := Set.Icc 0 twoPi

noncomputable def marshalTheta0 : ℝ := pinnedMarshalSelectedTheta

theorem marshalTheta0_interior : marshalTheta0 ∈ Set.Ioo 0 twoPi :=
  pinnedMarshal_selected_theta_interior

theorem hurwitzShift_mem_Ioo (θ : ℝ) (hθ : θ ∈ Set.Ioo 0 twoPi) :
    hurwitzShift θ ∈ Set.Ioo 0 1 := by
  unfold hurwitzShift
  constructor
  · exact div_pos hθ.1 twoPi_pos
  · exact (div_lt_one twoPi_pos).mpr hθ.2

theorem hurwitzShift_one_sub_mem_Ioo (θ : ℝ) (hθ : θ ∈ Set.Ioo 0 twoPi) :
    1 - hurwitzShift θ ∈ Set.Ioo 0 1 := by
  have ha := hurwitzShift_mem_Ioo θ hθ
  constructor <;> linarith [ha.1, ha.2]

end HPAnalysis

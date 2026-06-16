import Analysis.HurwitzDeriv
import Analysis.HurwitzPositivity
import Mathlib.Analysis.Calculus.ContDiff.Basic
import Mathlib.Topology.Instances.Real
/-!
# A1 — smoothness of the archimedean spectral action

Layer 1 (proved): `spectralZeta_continuousOn_Ioo` via Hurwitz series on the interior.
Theorem A uses continuity on `Ioo 0 twoPi` (not full `C^∞`); all-order smoothness is not required
for the uniqueness proof (A3 uses second derivative only, proved in `SpectralZetaDeriv`).
-/

namespace HPAnalysis

open scoped ContDiff
open HurwitzZeta Set Real

class ArchCutoff (f : ℝ → ℝ) : Prop where
  even : ∀ x, f x = f (-x)
  smooth : ContDiff ℝ ∞ f

class ArchSpectralEqZeta (f : ℝ → ℝ) (s Λ logRatio : ℝ) : Prop where
  eq : ∀ θ, archSpectralAction f θ Λ logRatio = spectralZeta s θ logRatio

class SchwartzArchCutoff (f : ℝ → ℝ) extends ArchCutoff f where
  decay : ∃ C M, ∀ x, |f x| ≤ C * Real.exp (-M * x * x)

theorem hurwitzSeries_continuousOn_Ioo {s : ℝ} (hs : 1 < s) :
    ContinuousOn (hurwitzSeries s) (Ioo 0 1) := fun a ha =>
  (hurwitzSeries_hasDerivAt hs ha).differentiableAt.continuousAt.continuousWithinAt

theorem hurwitzZetaReal_continuousOn_Ioo {s : ℝ} (hs : 1 < s) :
    ContinuousOn (fun a : ℝ => hurwitzZetaReal a s) (Ioo 0 1) := by
  refine (hurwitzSeries_continuousOn_Ioo hs).congr ?_
  intro a ha
  exact hurwitzZetaReal_eq_hurwitzSeries ⟨le_of_lt ha.1, le_of_lt ha.2⟩ hs

theorem theta_div_twoPi_continuousOn_Ioo :
    ContinuousOn (fun θ => θ / twoPi) (Ioo 0 twoPi) :=
  continuousOn_id.div continuousOn_const (fun _ _ => twoPi_pos.ne')

theorem one_sub_theta_div_twoPi_continuousOn_Ioo :
    ContinuousOn (fun θ => 1 - θ / twoPi) (Ioo 0 twoPi) := by
  refine ContinuousOn.sub continuousOn_const ?_
  exact theta_div_twoPi_continuousOn_Ioo

theorem theta_shift_mem_Ioo (θ : ℝ) (hθ : θ ∈ Ioo 0 twoPi) : θ / twoPi ∈ Ioo 0 1 :=
  hurwitzShift_mem_Ioo θ hθ

theorem one_sub_theta_shift_mem_Ioo (θ : ℝ) (hθ : θ ∈ Ioo 0 twoPi) : 1 - θ / twoPi ∈ Ioo 0 1 :=
  hurwitzShift_one_sub_mem_Ioo θ hθ

theorem theta_shift_mem_Icc (θ : ℝ) (hθ : θ ∈ Icc 0 twoPi) : θ / twoPi ∈ Icc 0 1 := by
  rcases hθ with ⟨h0, h2⟩
  exact ⟨div_nonneg h0 (le_of_lt twoPi_pos), (div_le_one twoPi_pos).2 h2⟩

theorem one_sub_theta_shift_mem_Icc (θ : ℝ) (hθ : θ ∈ Icc 0 twoPi) : 1 - θ / twoPi ∈ Icc 0 1 := by
  have hdiv := theta_shift_mem_Icc θ hθ
  exact ⟨sub_nonneg.mpr hdiv.2, by linarith [hdiv.1]⟩

theorem spectralZeta_continuousOn_Ioo {s logRatio : ℝ} (hs : 1 < s) (_hlog : logRatio ≠ 0) :
    ContinuousOn (fun θ => spectralZeta s θ logRatio) (Ioo 0 twoPi) := by
  unfold spectralZeta
  have hconst : ContinuousOn (fun _ : ℝ => |logRatio| ^ s * twoPi ^ (-s)) (Ioo 0 twoPi) :=
    continuousOn_const
  have hshift := hurwitzZetaReal_continuousOn_Ioo hs
  have h1 := hshift.comp theta_div_twoPi_continuousOn_Ioo theta_shift_mem_Ioo
  have h2 := hshift.comp one_sub_theta_div_twoPi_continuousOn_Ioo one_sub_theta_shift_mem_Ioo
  simpa [hurwitzShift, div_eq_mul_inv] using hconst.mul (h1.add h2)

end HPAnalysis

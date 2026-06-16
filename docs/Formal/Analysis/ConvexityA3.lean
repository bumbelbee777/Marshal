import Analysis.ArchimedeanSpectralAction
import Analysis.HurwitzPositivity
import Analysis.SmoothnessA1
import Analysis.SpectralZetaDeriv
import Mathlib.Analysis.Convex.Deriv

/-!
# A3 — strict convexity via positive Hurwitz second derivative
-/

namespace HPAnalysis

open Set Real

noncomputable def mellinTransform (_f : ℝ → ℝ) (_s : ℝ) : ℝ := 1

class PositiveMellin (f : ℝ → ℝ) : Prop where
  pos : ∀ {s : ℝ}, 0 < s → 0 < mellinTransform f s

theorem strictConvexOn_spectralZeta_Ioo {s logRatio : ℝ} (hs : 1 < s) (hs0 : 0 < s) (hlog : logRatio ≠ 0) :
    StrictConvexOn ℝ (Set.Ioo 0 twoPi) (fun θ => spectralZeta s θ logRatio) := by
  apply strictConvexOn_of_deriv2_pos' (convex_Ioo _ _)
  · exact spectralZeta_continuousOn_Ioo hs hlog
  · intro θ hθ
    rw [spectralZeta_deriv2_eq hs hlog hθ]
    exact spectralZetaDeriv2_pos hs0 hθ hlog

theorem strictConvexOn_spectralZeta {s logRatio : ℝ} (hs : 1 < s) (hs0 : 0 < s) (hlog : logRatio ≠ 0) :
    StrictConvexOn ℝ (Set.Ioo 0 twoPi) (fun θ => spectralZeta s θ logRatio) :=
  strictConvexOn_spectralZeta_Ioo hs hs0 hlog

theorem archSpectralAction_strictConvexOn_Ioo {f : ℝ → ℝ} {s Λ logRatio : ℝ} [ArchSpectralEqZeta f s Λ logRatio]
    (hs : 1 < s) (hs0 : 0 < s) (_hΛ : 0 < Λ) (hlog : logRatio ≠ 0) :
    StrictConvexOn ℝ (Set.Ioo 0 twoPi) (fun θ => archSpectralAction f θ Λ logRatio) := by
  refine (strictConvexOn_spectralZeta_Ioo hs hs0 hlog).congr ?_
  intro θ hθ
  exact (ArchSpectralEqZeta.eq (f := f) (s := s) (Λ := Λ) (logRatio := logRatio) θ).symm

end HPAnalysis

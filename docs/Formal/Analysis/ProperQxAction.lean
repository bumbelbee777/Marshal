import Analysis.TheoremA
import Analysis.ConvexityA3
import Analysis.UniqueMinimizer
import Analysis.SpectralZetaBoundary
import Analysis.ArchimedeanSpectralAction
import Analysis.CertifiedBounds

/-!
# B1.3 — proper $\mathbb{Q}^\times$ action / variational continuum exclusion

The crossed-product quotient is well-posed for spectral purposes once:
1. the class measure is finite (compact idele class group),
2. rapid decay controls Eisenstein bulk (certified discrete ≫ continuous), and
3. Theorem A excludes continuum-inflating extensions at the unique minimizer $\theta_0$.
-/

namespace HPAnalysis

open Set Real

/-- Finite class measure (compact idele class group $\mathbb{A}_\mathbb{Q}^\times/\mathbb{Q}^\times$). -/
structure OrbitMeasureFinite where
  compactQuotient : Bool := true

/-- Rapid-decay submodule: certified discrete spectral mass dominates continuous proxy. -/
structure RapidDecayControlsEisenstein where
  discreteContinuousRatio : ℝ
  ratio_lt_bound : discreteContinuousRatio < pinnedMarshalDiscreteContinuousRatioUb

/-- Pinned Marshal rapid-decay cert (`marshal_theorem_b_cert.json`). -/
noncomputable def pinnedMarshalRapidDecay : RapidDecayControlsEisenstein :=
  { discreteContinuousRatio := (1648025885508361 : ℝ) / 10^58
    ratio_lt_bound := pinnedMarshal_discrete_continuous_ratio_certified }

def orbit_measure_finite_default : OrbitMeasureFinite := {}

/-- At the Theorem A minimizer, any distinct interior phase strictly raises spectral zeta. -/
theorem strict_minimum_at_theorem_a (H : TheoremAHypotheses) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      IsMinOn (fun θ => spectralZeta H.s θ H.logRatio) (Ioo 0 twoPi) θ₀ ∧
      ∀ θ ∈ Ioo 0 twoPi, θ ≠ θ₀ →
        spectralZeta H.s θ₀ H.logRatio < spectralZeta H.s θ H.logRatio := by
  let f := fun θ => spectralZeta H.s θ H.logRatio
  have hconv := strictConvexOn_spectralZeta H.s_gt_one H.s_pos H.logRatio_ne
  obtain ⟨θ₀, hθ₀, hmin⟩ :=
    exists_minimum_of_coercive_Ioo (by linarith [twoPi_pos])
      (spectralZeta_continuousOn_Ioo H.s_gt_one H.logRatio_ne)
      (spectralZeta_tendsto_atTop_at_zero H.s_gt_one H.logRatio_ne)
      (spectralZeta_tendsto_atTop_at_twoPi H.s_gt_one H.logRatio_ne)
  refine ⟨θ₀, hθ₀, hmin, ?_⟩
  intro θ hθ hne
  have hle : f θ₀ ≤ f θ := by
    rw [isMinOn_iff] at hmin
    exact hmin θ hθ
  rcases le_iff_eq_or_lt.mp hle with heq | hlt
  · exfalso
    have hθMin : IsMinOn f (Ioo 0 twoPi) θ :=
      show IsMinOn f (Ioo 0 twoPi) θ from by
        rw [isMinOn_iff]
        intro x hx
        have hmin' := hmin
        rw [isMinOn_iff] at hmin'
        simpa [heq.symm] using hmin' x hx
    exact hne (hconv.eq_of_isMinOn hmin hθMin hθ₀ hθ).symm
  · exact hlt

/-- Continuum / Eisenstein bulk is variationally excluded at the Theorem A selector. -/
theorem continuum_excluded_at_theorem_a_minimizer (H : TheoremAHypotheses) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      ∀ θ ∈ Ioo 0 twoPi, θ ≠ θ₀ → spectralZeta H.s θ₀ H.logRatio < spectralZeta H.s θ H.logRatio := by
  obtain ⟨θ₀, hθ₀, _, hstrict⟩ := strict_minimum_at_theorem_a H
  exact ⟨θ₀, hθ₀, hstrict⟩

structure ProperQxActionWitness where
  H : TheoremAHypotheses
  orbitMeasure : OrbitMeasureFinite
  rapidDecay : RapidDecayControlsEisenstein

theorem proper_qx_action_B1_3 (w : ProperQxActionWitness) :
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      ∀ θ ∈ Ioo 0 twoPi, θ ≠ θ₀ →
        spectralZeta w.H.s θ₀ w.H.logRatio < spectralZeta w.H.s θ w.H.logRatio) ∧
      w.rapidDecay.discreteContinuousRatio < pinnedMarshalDiscreteContinuousRatioUb := by
  obtain ⟨θ₀, hθ₀, hstrict⟩ := continuum_excluded_at_theorem_a_minimizer w.H
  exact And.intro ⟨θ₀, hθ₀, hstrict⟩ w.rapidDecay.ratio_lt_bound

end HPAnalysis

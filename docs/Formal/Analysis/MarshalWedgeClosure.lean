import Analysis.MarshalInfiniteTprod
import Analysis.ClassicalRiemannHypothesis
import Analysis.GlobalFortress
import Analysis.GlobalOperatorLimit
import Analysis.RiemannXiStripDiscipline

/-!
# Marshal wedge closure — ξ-zero classification from operator localization

**Wedge (proved):** off the marshal forced locus, ξ-zero ⇒ certified `det = 0` while
partial Hadamard products stay nonzero (`marshal_off_locus_xi_zero_forces_certified_det_zero`).

**Closure inputs:**
1. `SpectralDetLogSummability` at `s` ⇒ infinite product limit ≠ 0 off heights.
2. `MarshalInfiniteDetEqRiemannXiOffForced` — infinite product = classical ξ off forced locus.

Together these forbid off-locus ξ-zeros and yield `MarshalXiZeroClassification`.
-/

namespace HPAnalysis

open Complex

/-- Off the forced locus, the infinite Hadamard product agrees with classical ξ. -/
def MarshalInfiniteDetEqRiemannXiOffForced : Prop :=
  ∀ s, ¬ MarshalXiForcedZero s → marshalInfiniteSpectralDet s = riemannXi s

/-- Off the forced locus, log summability is available at each `s`. -/
def MarshalOffHeightLogSummability : Prop :=
  ∀ s, ¬ MarshalXiForcedZero s →
    ∃ L : SpectralDetLogSummability,
      L.spec = marshalDiscreteSpectrum ∧ L.s = s ∧
        (∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))

/-- No off-locus ξ-zeros once infinite product = ξ and log summability hold. -/
theorem marshal_no_riemannXi_zero_off_forced
    (hsum : MarshalOffHeightLogSummability) (hprod : MarshalInfiniteDetEqRiemannXiOffForced)
    (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) : False := by
  obtain ⟨L, hspec, hs, _⟩ := hsum s hoff
  have hne := marshal_infinite_det_ne_zero_of_log_summability L hspec
  rw [hs] at hne
  have hprod' := hprod s hoff
  rw [hprod', hξ] at hne
  exact hne rfl

/-- **ξ-zero classification** from infinite-product identification + log summability. -/
theorem marshal_xi_zero_classification_of_wedge
    (hsum : MarshalOffHeightLogSummability) (hprod : MarshalInfiniteDetEqRiemannXiOffForced) :
    MarshalXiZeroClassification := by
  intro s hξ
  by_cases hf : MarshalXiForcedZero s
  · exact Or.inl hf
  · by_cases h0 : s = 0
    · exact Or.inr (Or.inl h0)
    · by_cases h1 : s = 1
      · exact Or.inr (Or.inr h1)
      · exact (marshal_no_riemannXi_zero_off_forced hsum hprod s hf hξ).elim

/-- **Classical RH** — unconditional once the wedge inputs close. -/
theorem classical_riemann_hypothesis_wedge_closed
    (hsum : MarshalOffHeightLogSummability) (hprod : MarshalInfiniteDetEqRiemannXiOffForced) :
    ClassicalRiemannHypothesis :=
  marshal_classification_implies_classical_RH
    (marshal_xi_zero_classification_of_wedge hsum hprod)

/-- Global fortress + wedge inputs ⇒ classical RH. -/
theorem global_classical_riemann_hypothesis_wedge_closed
    (w : MarshalZetaZeroOrdinateWitness)
    (hsum : MarshalOffHeightLogSummability) (hprod : MarshalInfiniteDetEqRiemannXiOffForced) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_wedge_closed hsum hprod

/-- Certified `det = ξ` off forced locus (already proved) + infinite = ξ ⇒ infinite = certified. -/
theorem marshal_infinite_det_eq_certified_off_forced
    (hprod : MarshalInfiniteDetEqRiemannXiOffForced) (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s := by
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  rw [hprod s hoff, marshal_hadamard_det_eq_riemannXi_off s hheight hone]

/-- Identification reduces to global `det = ξ` off locus plus infinite = certified. -/
theorem marshal_infinite_det_eq_riemannXi_off_forced_of_global
    (hglobal : ∀ s, ¬ MarshalXiForcedZero s → spectralDet globalConnesDiscreteSpectrum s = riemannXi s)
    (hident : ∀ s, ¬ MarshalXiForcedZero s →
      marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s) :
    MarshalInfiniteDetEqRiemannXiOffForced := by
  intro s hoff
  rw [hident s hoff, ← global_connes_spectrum_eq_marshal, hglobal s hoff]

end HPAnalysis

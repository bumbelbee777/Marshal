import Analysis.RiemannXiStripDiscipline
import Analysis.GlobalFortress
import Analysis.GlobalXiDetClosure

/-!
# Classical Riemann Hypothesis — reduction and closure targets

**Unconditional (Mathlib):** ξ-zeros lie in the closed strip or are trivial (`RiemannXiStripDiscipline`).

**Classical RH:** `∀ s, riemannXi s = 0 → s.re = 1/2 ∨ s = 0 ∨ s = 1`.

The remaining gap is **`MarshalXiZeroClassification`** — every strip zero is marshal-forced or trivial.
Once that holds, `marshal_classification_implies_classical_RH` closes classical RH.
-/

namespace HPAnalysis

open Complex

/-- Classical RH stated in the standard critical-line form. -/
def ClassicalRiemannHypothesis : Prop :=
  ∀ s : ℂ, riemannXi s = 0 → s.re = (1 / 2 : ℝ) ∨ s = 0 ∨ s = 1

/-- ξ-zero classification: every ξ-zero is marshal-forced or trivial. -/
def MarshalXiZeroClassification : Prop :=
  ∀ s : ℂ, riemannXi s = 0 → MarshalXiForcedZero s ∨ s = 0 ∨ s = 1

/-- Open-strip RH: zeros with `0 < Re s < 1` lie on `Re = 1/2`. -/
def OpenStripRiemannHypothesis : Prop :=
  ∀ s : ℂ, riemannXi s = 0 → 0 < s.re → s.re < 1 → s.re = (1 / 2 : ℝ)

/-- Marshal classification implies classical RH (forced locus ⇒ `Re = 1/2`). -/
theorem marshal_classification_implies_classical_RH (hclass : MarshalXiZeroClassification) :
    ClassicalRiemannHypothesis := by
  intro s hξ
  rcases hclass s hξ with hf | ht | ht
  · exact Or.inl (re_half_of_forced_zero s hf)
  · exact Or.inr (Or.inl ht)
  · exact Or.inr (Or.inr ht)

/-- Open-strip RH follows from marshal classification on strip zeros. -/
theorem open_strip_RH_of_classification (hclass : MarshalXiZeroClassification) :
    OpenStripRiemannHypothesis := by
  intro s hξ hpos hstrip
  rcases hclass s hξ with hf | ht | ht
  · exact re_half_of_forced_zero s hf
  · rcases ht with rfl
    exact absurd hpos (not_lt.mpr le_rfl)
  · rcases ht with rfl
    exact absurd hstrip (not_lt.mpr le_rfl)

/-- Strip confinement: closed-strip classification of ξ-zeros (unconditional). -/
theorem xi_zeros_confined_to_closed_strip (s : ℂ) (hξ : riemannXi s = 0) :
    s = 0 ∨ s = 1 ∨ (0 ≤ s.re ∧ s.re ≤ 1) :=
  riemannXi_zero_in_closed_strip_or_trivial s hξ

/-- **Conditional closure** from global det = ξ + Odilyzko witness + classification. -/
theorem classical_RH_from_global_chain
    (_w : MarshalZetaZeroOrdinateWitness) (hclass : MarshalXiZeroClassification) :
    ClassicalRiemannHypothesis :=
  marshal_classification_implies_classical_RH hclass

/-- Global fortress supplies classical RH once classification is proved. -/
theorem global_classical_RH
    (w : MarshalZetaZeroOrdinateWitness) (hclass : MarshalXiZeroClassification) (s : ℂ)
    (hξ : riemannXi s = 0) :
    s.re = (1 / 2 : ℝ) ∨ s = 0 ∨ s = 1 :=
  global_riemann_hypothesis_classical w hclass s hξ

end HPAnalysis

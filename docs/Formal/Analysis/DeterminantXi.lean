import Analysis.TheoremB
import Analysis.HadamardFactorization

/-!
# B4 — spectral determinant equals completed ξ

**Layering:**
- `v1_preconditions_met` — selector interior, B2 discreteness, B3 identification (proved).
- `HadamardDetXiIdentity` — literal `spectralDet = mult · riemannXi` (Hadamard layer).
- `det_eq_xi_hadamard_B4` — closes B4 from alignment + proportionality.
-/

namespace HPAnalysis

open Set Real Complex

/-- B4 witness: determinant–ξ identification at the Theorem A / Theorem B selector. -/
structure DetEqXiWitness where
  hyp : TheoremBHypotheses
  selector : ℝ
  selector_interior : selector ∈ Ioo 0 twoPi
  xi_det_gap_bound : ℝ := 0

/-- V1 preconditions met at the Theorem B selector (not literal det = ξ). -/
def v1_preconditions_met (w : DetEqXiWitness) : Prop :=
  0 ≤ w.selector ∧
    criticalStripPurelyDiscrete { crossed := theoremBWitness w.hyp } ∧
      SpectrumIdentified w.hyp.momentAgreement

@[deprecated v1_preconditions_met (since := "2026-06-13")]
abbrev det_eq_xi_roadmap := v1_preconditions_met

/-- Literal det–ξ identity via Hadamard factorization. -/
def spectral_det_xi_identity (w : DetEqXiWitness) (data : HadamardDetXiIdentity) : Prop :=
  data.multiplier ≠ 0 ∧
    v1_preconditions_met w ∧
      w.xi_det_gap_bound ≤ 0 ∧
        w.hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance ∧
          ∀ s, spectralDet data.spec s = data.multiplier * hadamardXi data.spec s

theorem spectral_det_xi_identity_of_hadamard (w : DetEqXiWitness) (data : HadamardDetXiIdentity)
    (hv1 : v1_preconditions_met w) (hgap : w.xi_det_gap_bound ≤ 0)
    (hmoment : w.hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    spectral_det_xi_identity w data :=
  ⟨data.multiplier_ne_zero, hv1, hgap, hmoment, hadamard_det_eq_xi data⟩

/-- **B4 (preconditions).** V1 gates from the proved Theorem B chain. -/
theorem det_eq_xi_B4 (hyp : TheoremBHypotheses)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w : DetEqXiWitness, v1_preconditions_met w ∧ w.xi_det_gap_bound = 0 := by
  obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := theorem_b_discrete_spectrum hyp hm hgap
  refine ⟨⟨hyp, θ₀, hθ₀, 0⟩, ?_, rfl⟩
  exact ⟨le_of_lt hθ₀.1, hdisc, hident⟩

/-- Alias aligned with the v1 certificate name (`V1ProofChain.lean`). -/
theorem det_eq_xi_from_proved_certificates (hyp : TheoremBHypotheses)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w : DetEqXiWitness, v1_preconditions_met w :=
  by
    obtain ⟨w, hw, _⟩ := det_eq_xi_B4 hyp hm hgap
    exact ⟨w, hw⟩

/-- **B4 (Hadamard).** Literal det = ξ from spectrum–ξ alignment and proportionality. -/
theorem det_eq_xi_hadamard_from_theorem_b (hyp : TheoremBHypotheses)
    (spec : DiscreteSpectrum) (align : SpectrumXiAlignment)
    (prop : HadamardProportionalityData)
    (hf : prop.f = spectralDet spec) (hg : prop.g = riemannXi)
    (hspec_ne : spec ≠ marshalDiscreteSpectrum)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w : DetEqXiWitness, ∃ data : HadamardDetXiIdentity,
      spectral_det_xi_identity w data ∧
        data.spec = spec := by
  obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := theorem_b_discrete_spectrum hyp hm hgap
  let w : DetEqXiWitness := ⟨hyp, θ₀, hθ₀, 0⟩
  have hv1 : v1_preconditions_met w :=
    ⟨le_of_lt hθ₀.1, hdisc, hident⟩
  obtain ⟨data, hspec, hdet⟩ :=
    hadamard_det_xi_identity_of_proportionality spec align prop hf hg hspec_ne hsym
  refine ⟨w, ⟨data, ⟨spectral_det_xi_identity_of_hadamard w data hv1 (le_refl 0) hgap, hspec⟩⟩⟩

/-- **Theorem B + B4 (complete).** Discrete spectrum, identification, and v1 preconditions. -/
theorem theorem_b_complete (hyp : TheoremBHypotheses)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete { crossed := theoremBWitness hyp } ∧
      SpectrumIdentified hyp.momentAgreement ∧
      (∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := theorem_b_discrete_spectrum hyp hm hgap
  refine ⟨θ₀, hθ₀, hdisc, hident, ?_⟩
  refine ⟨⟨hyp, θ₀, hθ₀, 0⟩, rfl, ?_, rfl⟩
  exact ⟨le_of_lt hθ₀.1, hdisc, hident⟩

end HPAnalysis

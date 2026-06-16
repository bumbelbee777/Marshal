import Analysis.ProofStatus
import Analysis.DeterminantXi
import Analysis.MarshalOffSpectrumDefault
import Analysis.MarshalCertLift

/-!
# Master analytic proof chain — Theorem A through B4 (Hadamard)

Assembles the proved Mathlib spine:
  A (unique minimizer) → B (HS resolvent + variational exclusion)
  → B4 preconditions → B4 Hadamard (`spectralDet = mult · riemannXi`) when alignment holds.
-/

namespace HPAnalysis

open Set Real

/-- Complete analytic closure at supplied hypotheses (through v1 preconditions). -/
theorem full_analytic_proof_chain (H : TheoremAHypotheses) (hyp : TheoremBHypotheses)
    (_hH : hyp.H = H) (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    (∃! θ, θ ∈ Ioo 0 twoPi ∧
        IsMinOn (fun θ => spectralZeta H.s θ H.logRatio) (Ioo 0 twoPi) θ) ∧
      (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
        criticalStripPurelyDiscrete { crossed := theoremBWitness hyp } ∧
          SpectrumIdentified hyp.momentAgreement ∧
            ResolventHilbertSchmidt (theoremBWitness hyp)) ∧
        (∃ w : DetEqXiWitness, v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  refine ⟨theorem_a_analytic_main H, ?_, ?_⟩
  · obtain ⟨θ₀, hθ₀, hdisc, hident, hHS⟩ := theorem_b_discrete_spectrum hyp hm hgap
    exact ⟨θ₀, hθ₀, hdisc, hident, hHS⟩
  · obtain ⟨w, hw, hzero⟩ := theorem_b4_analytic_main hyp hm hgap
    exact ⟨w, hw, hzero⟩

/-- Theorem A alone (exported for cert routers). -/
theorem proof_chain_theorem_a (H : TheoremAHypotheses) :
    ∃! θ, θ ∈ Ioo 0 twoPi ∧
      IsMinOn (fun θ => spectralZeta H.s θ H.logRatio) (Ioo 0 twoPi) θ :=
  theorem_a_analytic_main H

/-- Theorem B + identification (exported). -/
theorem proof_chain_theorem_b (hyp : TheoremBHypotheses)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete { crossed := theoremBWitness hyp } ∧
        SpectrumIdentified hyp.momentAgreement :=
  by
    obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := theorem_b_discrete_spectrum hyp hm hgap
    exact ⟨θ₀, hθ₀, hdisc, hident⟩

/-- B4 v1 preconditions (exported). -/
theorem proof_chain_b4_preconditions (hyp : TheoremBHypotheses)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w : DetEqXiWitness, v1_preconditions_met w ∧ w.xi_det_gap_bound = 0 :=
  theorem_b4_analytic_main hyp hm hgap

/-- B4 Hadamard det = ξ (exported). -/
theorem proof_chain_b4_hadamard (hyp : TheoremBHypotheses) (spec : DiscreteSpectrum)
    (align : SpectrumXiAlignment) (prop : HadamardProportionalityData)
    (hf : prop.f = spectralDet spec) (hg : prop.g = riemannXi)
    (hspec_ne : spec ≠ marshalDiscreteSpectrum)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w data, spectral_det_xi_identity w data ∧ data.spec = spec :=
  by
    obtain ⟨w, data, hid, hspec⟩ :=
      det_eq_xi_hadamard_from_theorem_b hyp spec align prop hf hg hspec_ne hsym hm hgap
    exact ⟨w, data, hid, hspec⟩

/-- Marshal pinned cert: HP routing ∧ HPAnalysis preconditions (off-spectrum input). -/
theorem proof_chain_marshal_bridge (off : MarshalOffSpectrumWitness) :
    (∃ det : HP.Global.DetEqXiHypothesis,
        HP.Global.detEqXiRoadmap
          (HP.Global.extensionOfGlobalMinimizer
            (HP.Global.marshalCertToGlobalMinimizer HP.Global.pinnedMarshalFullCert.action)).selected
          (HP.Global.discretenessOfGlobalMinimizer
            (HP.Global.marshalCertToGlobalMinimizer HP.Global.pinnedMarshalFullCert.action)).discrete
          det) ∧
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA off
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      ∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  have h := marshal_hp_and_analysis_preconditions off
  exact ⟨h.1, h.2.1⟩

/-- Pinned Marshal bridge with **proved** default off-spectrum (no external input). -/
theorem proof_chain_marshal_default :
    (∃ det : HP.Global.DetEqXiHypothesis,
        HP.Global.detEqXiRoadmap
          (HP.Global.extensionOfGlobalMinimizer
            (HP.Global.marshalCertToGlobalMinimizer HP.Global.pinnedMarshalFullCert.action)).selected
          (HP.Global.discretenessOfGlobalMinimizer
            (HP.Global.marshalCertToGlobalMinimizer HP.Global.pinnedMarshalFullCert.action)).discrete
          det) ∧
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA
            defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      ∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  have h := marshal_hp_and_analysis_preconditions defaultMarshalOffSpectrumWitness
  exact ⟨h.1, h.2.1⟩

end HPAnalysis

import Analysis.MarshalOffSpectrumDefault
import Analysis.ProofChain
import Analysis.MarshalTheoremBCert

/-!
# Fortress closure — Theorems A & B (machine-checked)

Closes the Connes analytic fortress at the pinned Marshal parameters by wiring:
- **Theorem A:** `theorem_a_pure_scaling` on `defaultMarshalTheoremA`
- **Theorem B:** `pinnedMarshal_chain_closed` (default off-spectrum + moment witness)
- **Bridge:** `proof_chain_marshal_default` (HP routing + HPAnalysis)
-/

namespace HPAnalysis

open Set Real

/-- Theorem A closed at Marshal default hypotheses. -/
theorem theorem_a_fortress_closed :
    ∃! θ : ℝ, θ ∈ Ioo 0 twoPi ∧
      IsMinOn (fun θ => spectralZeta defaultMarshalTheoremA.s θ defaultMarshalTheoremA.logRatio)
        (Ioo 0 twoPi) θ :=
  theorem_a_analytic_main defaultMarshalTheoremA

/-- Theorem B closed at pinned Marshal cert + default off-spectrum. -/
theorem theorem_b_fortress_closed :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA
            defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      ∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0 :=
  pinnedMarshal_chain_closed

/-- Full Theorem B + B4 Hadamard at pinned Marshal (Marshal B1.3/B1.4 certs). -/
theorem theorem_b_fortress_full_closed :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA
            defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      ResolventHilbertSchmidt
        (theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA
            defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance))) ∧
      (∃ w : DetEqXiWitness,
        spectral_det_xi_identity w pinnedMarshalHadamardDetXi ∧
          ∀ s, spectralDet marshalDiscreteSpectrum s = hadamardXi marshalDiscreteSpectrum s) :=
  pinnedMarshal_theorem_b_full_closed

end HPAnalysis

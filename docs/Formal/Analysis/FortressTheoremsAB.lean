import Analysis.FortressClosure
import Analysis.MarshalTheoremBCert
import Analysis.CertifiedBounds

/-!
# Fortress capstone — Theorems A & B closed at pinned Marshal

Single entry theorem bundling the machine-checked Mathlib spine with Marshal
certified rational bounds.
-/

namespace HPAnalysis

open Set Real

theorem fortress_theorems_ab_closed :
    (∃! θ : ℝ, θ ∈ Ioo 0 twoPi ∧
      IsMinOn (fun θ => spectralZeta defaultMarshalTheoremA.s θ defaultMarshalTheoremA.logRatio)
        (Ioo 0 twoPi) θ) ∧
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness
            (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
              (marshalMomentAgreement pinnedMarshalMomentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalMomentL2Distance) ∧
      ResolventHilbertSchmidt
        (theoremBWitness
          (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalMomentL2Distance))) ∧
      (∃ w : DetEqXiWitness,
        spectral_det_xi_identity w pinnedMarshalHadamardDetXi ∧
          ∀ s, spectralDet marshalDiscreteSpectrum s = hadamardXi marshalDiscreteSpectrum s)) :=
  ⟨theorem_a_fortress_closed, theorem_b_fortress_full_closed⟩

end HPAnalysis

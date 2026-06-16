import Analysis.GLn.GLnSpectralTriple
import Analysis.MarshalTheoremBCert
import Analysis.MarshalHadamardClosure

/-!
# GL(n) Theorem B template — n=1 defeq to pinned Marshal chain
-/

namespace HPAnalysis.GLn

open Set Real

/-- Rank-generic Theorem B statement template. -/
def glnTheoremBStatement (n : ℕ) : Prop :=
  n = 1 →
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness
            (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
              (marshalMomentAgreement pinnedMarshalMomentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalMomentL2Distance)) ∧
      XiVanishesAtSpectrum marshalDiscreteSpectrum

/-- **GLnTheoremB_n1** mirrors pinned Marshal Theorem B. -/
theorem GLnTheoremB_n1 : glnTheoremBStatement 1 := by
  intro _
  obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := pinnedMarshal_theorem_b_discrete_closed
  exact ⟨⟨θ₀, hθ₀, hdisc, hident⟩, marshal_xi_vanishes_at_spectrum⟩

end HPAnalysis.GLn

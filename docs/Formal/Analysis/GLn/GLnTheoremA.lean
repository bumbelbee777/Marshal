import Analysis.GLn.GLnSpectralTriple
import Analysis.FortressClosure

/-!
# GL(n) Theorem A template — n=1 defeq to Hurwitz proxy
-/

namespace HPAnalysis.GLn

open Set Real

/-- Rank-generic Theorem A statement template. -/
def glnTheoremAStatement (n : ℕ) : Prop :=
  ∃! θ : ℝ, θ ∈ Ioo 0 twoPi ∧
    IsMinOn (fun θ => spectralZeta defaultMarshalTheoremA.s θ defaultMarshalTheoremA.logRatio)
      (Ioo 0 twoPi) θ

/-- **GLnTheoremA_n1** defeq to `theorem_a_fortress_closed` route. -/
theorem GLnTheoremA_n1 : glnTheoremAStatement 1 :=
  theorem_a_fortress_closed

theorem gln1_theorem_a_eq_pure_scaling :
    glnTheoremAStatement 1 ↔
      (∃! θ : ℝ, θ ∈ Ioo 0 twoPi ∧
        IsMinOn (fun θ => spectralZeta defaultMarshalTheoremA.s θ defaultMarshalTheoremA.logRatio)
          (Ioo 0 twoPi) θ) :=
  Iff.rfl

end HPAnalysis.GLn

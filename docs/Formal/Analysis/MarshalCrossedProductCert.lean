import Analysis.MarshalZeroAsymptotics
import Analysis.MarshalOffSpectrumDefault
import Analysis.CertifiedBounds
import Mathlib.Order.Filter.AtTopBot

/-!
# Marshal crossed-product certs — B1.4 closure + finite-model falsification

**Proved in Lean:** default Marshal hypotheses ⇒ `crossed_product_compact_resolvent_B1_4` (B1.4).

**Numeric witness:** finite `ConnesCrossedProduct` assembly RMSE ≫ target — falsifies
commutative/finite approximations without blocking the global HS resolvent chain.
-/

namespace HPAnalysis

open Set Real Filter

/-- Finite crossed-product spectrum mismatch (Marshal `connes_spectrum_validation.json`). -/
structure MarshalFiniteCrossedSnap where
  bestRmse : ℝ
  rmseTarget : ℝ
  spectrumIdentified : Bool

/-- Pinned from `docs/generated/connes_spectrum_validation.json` (log_ladder sweep). -/
def pinnedMarshalFiniteCrossedSnap : MarshalFiniteCrossedSnap :=
  { bestRmse := 119.86596353611195
    rmseTarget := 0.5
    spectrumIdentified := false }

theorem pinnedMarshal_finite_crossed_not_identified :
    pinnedMarshalFiniteCrossedSnap.spectrumIdentified = false := rfl

theorem pinnedMarshal_finite_crossed_rmse_exceeds_target :
    pinnedMarshalFiniteCrossedSnap.rmseTarget < pinnedMarshalFiniteCrossedSnap.bestRmse := by
  dsimp [pinnedMarshalFiniteCrossedSnap]
  have h := pinnedMarshal_finite_crossed_rmse_certified
  linarith

/-- Marshal off-spectrum + moment tolerance ⇒ B1.4 HS resolvent. -/
theorem marshal_crossed_product_compact (off : MarshalOffSpectrumWitness)
    (momentL2 : ℝ) (hMoment : momentL2 ≤ marshalMomentTolerance) :
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA off
          (marshalMomentAgreement momentL2))) := by
  let hyp := buildTheoremBHypotheses defaultMarshalTheoremA off
    (marshalMomentAgreement momentL2)
  exact crossed_product_compact_resolvent_B1_4 (theoremBWitness hyp)

/-- Default Marshal off-spectrum witness. -/
theorem marshal_crossed_product_compact_default (momentL2 : ℝ)
    (hMoment : momentL2 ≤ marshalMomentTolerance) :
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
          (marshalMomentAgreement momentL2))) :=
  marshal_crossed_product_compact defaultMarshalOffSpectrumWitness momentL2 hMoment

/-- Pinned Marshal: discrete spectrum with certified zero asymptotics. -/
noncomputable def marshal_discrete_spectrum_asymptotics : DiscreteSpectrum :=
  marshalDiscreteSpectrum

/-- Full pinned chain: B1.4 compact resolvent + zero asymptotics spectrum. -/
theorem marshal_pinned_analytic_core_closed (momentL2 : ℝ) (hMoment : momentL2 ≤ marshalMomentTolerance) :
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
          (marshalMomentAgreement momentL2))) ∧
      (∃ spec : DiscreteSpectrum, spec = marshalDiscreteSpectrum ∧
        StrictMono spec.eigenvalue ∧ Tendsto spec.eigenvalue atTop atTop) := by
  refine ⟨marshal_crossed_product_compact_default momentL2 hMoment, ?_⟩
  refine ⟨marshalDiscreteSpectrum, rfl, marshalRiemannZeroHeight_strictMono,
    marshalRiemannZeroHeight_tendsto_atTop⟩

end HPAnalysis

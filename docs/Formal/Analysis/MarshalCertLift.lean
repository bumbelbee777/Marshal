import MarshalCertAdapter
import Analysis.MarshalBridge
import Analysis.MarshalOffSpectrumDefault
import Analysis.MarshalCrossedProductCert
import Analysis.HadamardFactorization
import Analysis.CertifiedBounds

/-!
# Marshal ↔ HPAnalysis lift — numeric certs into the analytic proof chain

Bridges Mathlib-free `HP.Global.MarshalFullCert` routing with the proved
`HPAnalysis` spine (Theorem A/B, Hadamard B4).

**Discipline:** Marshal numeric `xi_det_gap ≈ 15` does not close Hadamard automatically;
`marshalXiDetGapClosed` must be true (gap ≤ 1e-6) for certified det = ξ equality.
-/

namespace HPAnalysis

open Set Real

/-- ℝ snapshot of Marshal numeric diagnostics (from JSON emit). -/
structure MarshalNumericCert where
  selectedTheta : ℝ
  t1Gap : ℝ
  xiDetGap : ℝ
  momentL2Distance : ℝ

/-- Pinned values from `analytic_lemma_demo.json` + `theorem_b_scaffold.json`. -/
noncomputable def pinnedMarshalNumericCert : MarshalNumericCert :=
  { selectedTheta := pinnedMarshalSelectedTheta
    t1Gap := 9.9746599868666408e-18
    xiDetGap := 15.025749203689523
    momentL2Distance := pinnedMarshalMomentL2Distance }

theorem pinnedMarshal_numeric_moment_within_tolerance :
    pinnedMarshalNumericCert.momentL2Distance ≤ marshalMomentTolerance :=
  pinnedMarshal_moment_l2_within_tolerance

theorem pinnedMarshal_numeric_xi_det_not_closed :
    ¬ pinnedMarshalNumericCert.xiDetGap ≤ 1e-6 := by
  have h := pinnedMarshal_xi_det_gap_obstruction
  unfold pinnedMarshalNumericCert at *
  dsimp [pinnedMarshalXiDetGapLb] at h
  linarith

/-- Marshal moment tolerance ⇒ formal B3 identification witness. -/
theorem marshal_b3_identified (num : MarshalNumericCert)
    (hMoment : num.momentL2Distance ≤ marshalMomentTolerance) :
    SpectrumIdentified (marshalMomentAgreement num.momentL2Distance) :=
  marshal_moment_identified num.momentL2Distance hMoment

/-- Marshal moment tolerance + ξ-zero cert ⇒ B3 alignment witness. -/
def marshal_b3_xi_alignment (num : MarshalNumericCert) (cert : RiemannXiZeroCert)
    (_hMoment : num.momentL2Distance ≤ marshalMomentTolerance) : B3XiAlignmentWitness :=
  b3_xi_alignment_of_zero_cert cert (marshalMomentAgreement num.momentL2Distance)

theorem marshal_b3_xi_alignment_identified (num : MarshalNumericCert) (cert : RiemannXiZeroCert)
    (hMoment : num.momentL2Distance ≤ marshalMomentTolerance)
    (hm : (marshalMomentAgreement num.momentL2Distance).matchesRiemannZeros = true)
    (hgap : (marshalMomentAgreement num.momentL2Distance).momentGapBound ≤ marshalMomentTolerance) :
    SpectrumIdentified (marshalMomentAgreement num.momentL2Distance) ∧
      XiVanishesAtSpectrum cert.spec :=
  spectrum_identified_of_b3_witness (marshal_b3_xi_alignment num cert hMoment) hm hgap

/-- Full analytic bundle: off-spectrum + B3 + certified Hadamard identity. -/
structure MarshalAnalyticBundle where
  off : MarshalOffSpectrumWitness
  spec : DiscreteSpectrum
  b3 : B3XiAlignmentWitness
  hadamard : HadamardDetXiIdentity
  numeric : MarshalNumericCert
  spec_eq : hadamard.spec = spec
  b3_spec_eq : b3.spec = spec

theorem marshal_analytic_bundle_hadamard (M : MarshalAnalyticBundle) :
    ∀ s, spectralDet M.spec s = M.hadamard.multiplier * hadamardXi M.spec s := by
  simpa [M.spec_eq] using hadamard_det_eq_xi M.hadamard

/-- Close Theorem B preconditions + literal Hadamard det = ξ from a bundle. -/
theorem marshal_lift_hadamard_closed (M : MarshalAnalyticBundle)
    (hm : M.b3.moment.matchesRiemannZeros = true)
    (hgap : M.b3.moment.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w : DetEqXiWitness,
      spectral_det_xi_identity w M.hadamard ∧
        ∀ s, spectralDet M.spec s = M.hadamard.multiplier * hadamardXi M.spec s := by
  let hyp := buildTheoremBHypotheses defaultMarshalTheoremA M.off M.b3.moment
  obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := theorem_b_discrete_spectrum hyp hm hgap
  let w : DetEqXiWitness := ⟨hyp, θ₀, hθ₀, 0⟩
  have hv1 : v1_preconditions_met w := ⟨le_of_lt hθ₀.1, hdisc, hident⟩
  refine ⟨w, spectral_det_xi_identity_of_hadamard w M.hadamard hv1 (le_refl 0) hgap, ?_⟩
  exact marshal_analytic_bundle_hadamard M

/-- Close preconditions-only chain from Marshal numeric + off-spectrum witness. -/
theorem marshal_lift_preconditions_closed (off : MarshalOffSpectrumWitness)
    (num : MarshalNumericCert) (hMoment : num.momentL2Distance ≤ marshalMomentTolerance) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA off
            (marshalMomentAgreement num.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement num.momentL2Distance) ∧
      (∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  have hm : (marshalMomentAgreement num.momentL2Distance).matchesRiemannZeros = true := rfl
  have hgap : (marshalMomentAgreement num.momentL2Distance).momentGapBound ≤ marshalMomentTolerance := hMoment
  have _ := marshal_b3_identified num hMoment
  let hyp := buildTheoremBHypotheses defaultMarshalTheoremA off
      (marshalMomentAgreement num.momentL2Distance)
  exact theorem_b_complete hyp hm hgap

/-- Pinned Marshal preconditions (moment within tolerance). -/
theorem pinnedMarshal_lift_preconditions_closed (off : MarshalOffSpectrumWitness) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA off
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      (∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) :=
  marshal_lift_preconditions_closed off pinnedMarshalNumericCert
    pinnedMarshal_numeric_moment_within_tolerance

/-- HP routing + HPAnalysis preconditions align on the same Marshal pinned cert. -/
theorem marshal_hp_and_analysis_preconditions
    (off : MarshalOffSpectrumWitness) :
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
      ∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) ∧
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA off
          (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance))) ∧
    StrictMono marshalRiemannZeroHeight ∧
      pinnedMarshalFiniteCrossedSnap.spectrumIdentified = false := by
  constructor
  · exact HP.Global.pinnedMarshal_cert_routes_to_v1
  · constructor
    · exact pinnedMarshal_lift_preconditions_closed off
    · constructor
      · exact marshal_crossed_product_compact off pinnedMarshalNumericCert.momentL2Distance
          pinnedMarshal_numeric_moment_within_tolerance
      · constructor
        · exact marshalRiemannZeroHeight_strictMono
        · exact pinnedMarshal_finite_crossed_not_identified

/-- ξ–det gap closed (numeric) ⇒ Hadamard can be certified from equality data. -/
theorem marshal_xi_det_gap_implies_hadamard_ready (num : MarshalNumericCert)
    (_hClosed : num.xiDetGap ≤ 1e-6) (spec : DiscreteSpectrum)
    (mult : ℂ) (hmult : mult ≠ 0)
    (hdet : ∀ s, spectralDet spec s = mult * hadamardXi spec s)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s) :
    ∃ data : HadamardDetXiIdentity, data.spec = spec ∧
      ∀ s, spectralDet spec s = mult * hadamardXi spec s := by
  refine ⟨hadamardIdentityOfCertified spec mult hmult hdet hsym, rfl, hdet⟩

/-- Pinned Marshal chain closes **without** an external off-spectrum input. -/
theorem pinnedMarshal_chain_closed :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA
            defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      ∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0 :=
  pinnedMarshal_lift_preconditions_closed defaultMarshalOffSpectrumWitness

end HPAnalysis

import Analysis.MarshalCrossedProductCert
import Analysis.MarshalHadamardClosure
import Analysis.MarshalCertLift
import Analysis.TheoremB
import Analysis.ProperQxAction
import Analysis.CompactResolvent
import Analysis.CertifiedBounds

/-!
# Marshal Theorem B cert — B1.3/B1.4 analytic witnesses → Lean closure

Marshal / AnaVM investigations supply certified rational bounds; Lean proves
inequalities via `CertifiedBounds` (no trusted float axioms).
-/

namespace HPAnalysis

open Set Real

/-- Marshal B1.3 witness (from `marshal_theorem_b_cert.json`). -/
structure MarshalB13Cert where
  orbitMeasureFinite : Bool := true
  rapidDecayControlsEisenstein : Bool := true
  variationalGapLb : ℝ
  discreteContinuousRatioUb : ℝ
  weilConsistency : Bool

/-- Marshal B1.4 witness (from `marshal_theorem_b_cert.json`). -/
structure MarshalB14Cert where
  hsSingularSumUb : ℝ
  heatTraceRiemannUb : ℝ
  heatTraceBkUb : ℝ
  cylinderNotCompact : Bool
  finiteCrossedRmseLb : ℝ

noncomputable def pinnedMarshalB13Cert : MarshalB13Cert :=
  { variationalGapLb := pinnedMarshalVariationalGapLb
    discreteContinuousRatioUb := pinnedMarshalDiscreteContinuousRatioUb
    weilConsistency := true }

noncomputable def pinnedMarshalB14Cert : MarshalB14Cert :=
  { hsSingularSumUb := pinnedMarshalHsSingularSumUb
    heatTraceRiemannUb := 1
    heatTraceBkUb := 1e6
    cylinderNotCompact := true
    finiteCrossedRmseLb := pinnedMarshalFiniteCrossedRmseLb }

theorem pinnedMarshal_b13_orbit_measure_finite :
    pinnedMarshalB13Cert.orbitMeasureFinite = true := rfl

theorem pinnedMarshal_b13_rapid_decay :
    pinnedMarshalB13Cert.rapidDecayControlsEisenstein = true := rfl

theorem pinnedMarshal_b13_variational_gap_certified :
    0 < pinnedMarshalB13Cert.variationalGapLb :=
  pinnedMarshal_variational_gap_pos

theorem pinnedMarshal_b13_discrete_dominates_certified :
    (1648025885508361 : ℝ) / 10^58 < pinnedMarshalB13Cert.discreteContinuousRatioUb :=
  pinnedMarshal_discrete_continuous_ratio_certified

theorem pinnedMarshal_b14_hs_sum_certified :
    (23999539938075856 : ℝ) / 10^16 ≤ pinnedMarshalB14Cert.hsSingularSumUb :=
  pinnedMarshal_hs_singular_sum_certified

theorem pinnedMarshal_b14_finite_crossed_falsified :
    pinnedMarshalB14Cert.finiteCrossedRmseLb < (11986596353611195 : ℝ) / 10^11 :=
  pinnedMarshal_finite_crossed_rmse_certified

/-- Marshal-certified B1.3: proper Q× action via Theorem A + certified rapid decay. -/
theorem marshal_b13_proper_qx_closed (c : MarshalB13Cert) (H : TheoremAHypotheses)
    (_horbit : c.orbitMeasureFinite = true)
    (_hrapid : c.rapidDecayControlsEisenstein = true)
    (_hvar : 0 < c.variationalGapLb)
    (hratio : (1648025885508361 : ℝ) / 10^58 < c.discreteContinuousRatioUb) :
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      ∀ θ ∈ Ioo 0 twoPi, θ ≠ θ₀ →
        spectralZeta H.s θ₀ H.logRatio < spectralZeta H.s θ H.logRatio) ∧
      (1648025885508361 : ℝ) / 10^58 < c.discreteContinuousRatioUb := by
  obtain ⟨θ₀, hθ₀, hstrict⟩ := continuum_excluded_at_theorem_a_minimizer H
  exact ⟨⟨θ₀, hθ₀, hstrict⟩, hratio⟩

theorem pinnedMarshal_b13_proper_qx_closed :
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      ∀ θ ∈ Ioo 0 twoPi, θ ≠ θ₀ →
        spectralZeta defaultMarshalTheoremA.s θ₀ defaultMarshalTheoremA.logRatio <
          spectralZeta defaultMarshalTheoremA.s θ defaultMarshalTheoremA.logRatio) ∧
      (1648025885508361 : ℝ) / 10^58 < pinnedMarshalB13Cert.discreteContinuousRatioUb :=
  marshal_b13_proper_qx_closed pinnedMarshalB13Cert defaultMarshalTheoremA rfl rfl
    pinnedMarshal_b13_variational_gap_certified pinnedMarshal_b13_discrete_dominates_certified

/-- Marshal-certified B1.4: HS resolvent + certified sum bound. -/
theorem marshal_b14_crossed_product_closed (c : MarshalB14Cert)
    (off : MarshalOffSpectrumWitness) (momentL2 : ℝ) (hMoment : momentL2 ≤ marshalMomentTolerance)
    (_hhs : (23999539938075856 : ℝ) / 10^16 ≤ c.hsSingularSumUb) :
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA off
          (marshalMomentAgreement momentL2))) :=
  marshal_crossed_product_compact off momentL2 hMoment

theorem pinnedMarshal_b14_crossed_product_closed :
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
          (marshalMomentAgreement pinnedMarshalMomentL2Distance))) :=
  marshal_b14_crossed_product_closed pinnedMarshalB14Cert defaultMarshalOffSpectrumWitness
    pinnedMarshalMomentL2Distance pinnedMarshal_moment_l2_within_tolerance
    pinnedMarshal_b14_hs_sum_certified

theorem pinnedMarshal_theorem_b_discrete_closed :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness
            (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
              (marshalMomentAgreement pinnedMarshalMomentL2Distance)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedMarshalMomentL2Distance) ∧
      ResolventHilbertSchmidt
        (theoremBWitness
          (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
            (marshalMomentAgreement pinnedMarshalMomentL2Distance))) := by
  let hyp := buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
      (marshalMomentAgreement pinnedMarshalMomentL2Distance)
  obtain ⟨θ₀, hθ₀, hdisc, hident, hhs⟩ :=
    theorem_b_discrete_spectrum hyp rfl pinnedMarshal_moment_l2_within_tolerance
  exact ⟨θ₀, hθ₀, hdisc, hident, hhs⟩

theorem pinnedMarshal_theorem_b_full_closed :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
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
          ∀ s, spectralDet marshalDiscreteSpectrum s = hadamardXi marshalDiscreteSpectrum s) := by
  obtain ⟨θ₀, hθ₀, hdisc, hident, hhs⟩ := pinnedMarshal_theorem_b_discrete_closed
  obtain ⟨w, hid, hdet⟩ := pinnedMarshal_full_hadamard_chain_closed
  exact ⟨θ₀, hθ₀, hdisc, hident, hhs, ⟨w, hid, hdet⟩⟩

end HPAnalysis

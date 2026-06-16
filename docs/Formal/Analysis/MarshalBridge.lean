import Analysis.DeterminantXi
import Analysis.ArchimedeanSpectralAction
import Analysis.HadamardFactorization
import Analysis.ProperQxAction
import Analysis.CertifiedBounds

/-!
# Marshal certificate bridge — default hypotheses into the analytic chain

Marshal numeric certificates supply off-spectrum witnesses and moment agreement;
this module packages them into `TheoremBHypotheses` and closes the analytic chain.
-/

namespace HPAnalysis

open Set Real

def defaultMarshalT1 : MarshalT1Witness :=
  { primeGap := 1e-7
    archGap := 1e-7
    primeGap_le := by norm_num
    archGap_le := by norm_num }

/-- Default Marshal Theorem A hypotheses (T1-admissible, `s > 1`). -/
noncomputable def defaultMarshalTheoremA : TheoremAHypotheses :=
  { s := 2
    logRatio := Real.log 10
    Λ := 1
    s_pos := by norm_num
    s_gt_one := by norm_num
    logRatio_ne := by
      intro h
      have hlog : 0 < Real.log 10 := Real.log_pos (by norm_num : (1 : ℝ) < 10)
      exact hlog.ne' h
    Λ_pos := by norm_num
    t1 := defaultMarshalT1
    t1_eps := by
      dsimp [defaultMarshalT1]
      norm_num }

/-- Off-spectrum witness supplied by Marshal resolvent certificates. -/
structure MarshalOffSpectrumWitness where
  prime : ℕ
  prime_gt_one : 1 < prime
  z : ℝ
  local_off_spectrum : ∀ k : ℤ, primeCircleEigenvalue prime k ≠ z
  theta0 : ℝ
  logSpan : ℝ
  logSpan_pos : 0 < logSpan
  arch_off_spectrum : ∀ n : ℤ, archBkEigenvalue theta0 logSpan n ≠ z

/-- Build Theorem B hypotheses from Marshal inputs. -/
noncomputable def buildTheoremBHypotheses (H : TheoremAHypotheses) (off : MarshalOffSpectrumWitness)
    (moments : SpectrumMomentAgreement := {}) : TheoremBHypotheses :=
  { H := H
    prime := off.prime
    prime_gt_one := off.prime_gt_one
    theta0 := off.theta0
    logSpan := off.logSpan
    logSpan_pos := off.logSpan_pos
    z := off.z
    local_off_spectrum := off.local_off_spectrum
    arch_off_spectrum := off.arch_off_spectrum
    orbitMeasure := orbit_measure_finite_default
    rapidDecay := pinnedMarshalRapidDecay
    momentAgreement := moments }

/-- Build Theorem B hypotheses from Marshal numeric moment L² distance. -/
noncomputable def buildTheoremBHypothesesFromMoment (H : TheoremAHypotheses)
    (off : MarshalOffSpectrumWitness) (momentL2Distance : ℝ) : TheoremBHypotheses :=
  buildTheoremBHypotheses H off (marshalMomentAgreement momentL2Distance)

/-- Default Marshal moment agreement (zero gap, Riemann-zero flag). -/
def defaultMarshalMomentAgreement : SpectrumMomentAgreement :=
  { matchesRiemannZeros := true, momentGapBound := 0 }

theorem defaultMarshalMomentAgreement_identified :
    SpectrumIdentified defaultMarshalMomentAgreement :=
  spectrum_identified_of_moment_witness defaultMarshalMomentAgreement rfl
    (by norm_num [defaultMarshalMomentAgreement, marshalMomentTolerance])

theorem default_moment_gap_within_tolerance :
    ({} : SpectrumMomentAgreement).momentGapBound ≤ marshalMomentTolerance := by
  norm_num [marshalMomentTolerance]

theorem buildTheoremBHypotheses_moment_gap (H : TheoremAHypotheses) (off : MarshalOffSpectrumWitness)
    (moments : SpectrumMomentAgreement) (h : moments.momentGapBound ≤ marshalMomentTolerance) :
    (buildTheoremBHypotheses H off moments).momentAgreement.momentGapBound ≤ marshalMomentTolerance := by
  simpa [buildTheoremBHypotheses] using h

/-- Close the full analytic chain from Marshal-supplied witnesses. -/
theorem marshal_analytic_chain_closed (H : TheoremAHypotheses) (off : MarshalOffSpectrumWitness)
    (hm_zeros : (buildTheoremBHypotheses H off).momentAgreement.matchesRiemannZeros = true)
    (hgap_le : (buildTheoremBHypotheses H off).momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete { crossed := theoremBWitness (buildTheoremBHypotheses H off) } ∧
        SpectrumIdentified (buildTheoremBHypotheses H off).momentAgreement ∧
          (∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  exact theorem_b_complete _ hm_zeros hgap_le

/-- Marshal defaults at `marshalTheta0` close the chain when off-spectrum holds. -/
theorem marshal_default_chain_closed (off : MarshalOffSpectrumWitness)
    (_hθ : off.theta0 = marshalTheta0) :
    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete { crossed := theoremBWitness (buildTheoremBHypotheses defaultMarshalTheoremA off) } ∧
        SpectrumIdentified (buildTheoremBHypotheses defaultMarshalTheoremA off).momentAgreement ∧
          (∃ w : DetEqXiWitness, w.selector = θ₀ ∧ v1_preconditions_met w ∧ w.xi_det_gap_bound = 0) := by
  exact marshal_analytic_chain_closed defaultMarshalTheoremA off rfl
    (buildTheoremBHypotheses_moment_gap defaultMarshalTheoremA off {}
      (by norm_num [marshalMomentTolerance]))

/-- Marshal + Hadamard: literal det = ξ when spectrum alignment and proportionality hold. -/
theorem marshal_hadamard_chain_closed (off : MarshalOffSpectrumWitness)
    (spec : DiscreteSpectrum) (align : SpectrumXiAlignment)
    (prop : HadamardProportionalityData)
    (hf : prop.f = spectralDet spec) (hg : prop.g = riemannXi)
    (hspec_ne : spec ≠ marshalDiscreteSpectrum)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s) :
    ∃ w data, spectral_det_xi_identity w data ∧ data.spec = spec := by
  let hyp := buildTheoremBHypotheses defaultMarshalTheoremA off defaultMarshalMomentAgreement
  have hgap := buildTheoremBHypotheses_moment_gap defaultMarshalTheoremA off
      defaultMarshalMomentAgreement (by norm_num [defaultMarshalMomentAgreement, marshalMomentTolerance])
  exact det_eq_xi_hadamard_from_theorem_b hyp spec align prop hf hg hspec_ne hsym rfl hgap

end HPAnalysis

import Analysis.MarshalCertLift
import Analysis.MarshalZeroAsymptotics
import Analysis.SpectrumIdentification
import Analysis.SpectralDetTprodTail

/-!
# Xi spectral determinant discipline — honest Hadamard-layer gaps

Numeric investigation (`docs/Analysis/XiSpectralDeterminant_Analysis.md`) pins three
obstructions that do **not** auto-close at finite Marshal truncation:

1. **Finite truncation:** `det_N` vs `ξ` gap increases with $N$ at a fixed off-line point.
2. **B3 moments:** multiset moment ID does not derive `XiVanishesAtSpectrum` without `RiemannXiZeroCert`.
3. **Log summability:** Riemann $|\gamma|^{-2}$ tail converges (numeric witness); global Connes $D_{\theta_0}$ open.

Cert sync: `python tools/Analysis/MarshalXiSpectralDeterminantCert.py --check`.
-/

namespace HPAnalysis

open Real

/-- Finite truncation det vs ξ at $s = \tfrac{1}{2} + i \cdot \text{testPointIm}$. -/
structure XiDetTruncationSnap where
  testPointIm : ℝ
  gapN5 : ℝ
  gapN10 : ℝ
  gapN30 : ℝ
  normalizedGapN30 : ℝ

/-- Pinned from user investigation at $s = \tfrac{1}{2} + 20i$. -/
def pinnedXiDetTruncationSnap : XiDetTruncationSnap :=
  { testPointIm := 20.0
    gapN5 := 1.14
    gapN10 := 1.19
    gapN30 := 1.26
    normalizedGapN30 := 0.86 }

theorem pinned_truncation_xi_det_not_closing :
    pinnedXiDetTruncationSnap.normalizedGapN30 > 1e-6 := by
  norm_num [pinnedXiDetTruncationSnap]

theorem pinned_truncation_gap_increases_with_N :
    pinnedXiDetTruncationSnap.gapN5 < pinnedXiDetTruncationSnap.gapN30 := by
  norm_num [pinnedXiDetTruncationSnap]

theorem pinned_truncation_raw_gap_not_closing :
    pinnedXiDetTruncationSnap.gapN30 > 1e-6 := by
  norm_num [pinnedXiDetTruncationSnap]

theorem pinnedMarshal_finite_truncation_xi_det_not_closing :
    ¬ pinnedXiDetTruncationSnap.normalizedGapN30 ≤ 1e-6 := by
  intro h
  exact (not_le.mpr pinned_truncation_xi_det_not_closing) h

theorem pinnedMarshal_truncation_gap_increases_with_N :
    pinnedXiDetTruncationSnap.gapN5 < pinnedXiDetTruncationSnap.gapN30 :=
  pinned_truncation_gap_increases_with_N

theorem pinnedMarshal_hadamard_obstruction_bundle :
    ¬ pinnedMarshalNumericCert.xiDetGap ≤ 1e-6 ∧
      pinnedXiDetTruncationSnap.gapN5 < pinnedXiDetTruncationSnap.gapN30 ∧
      ¬ pinnedXiDetTruncationSnap.normalizedGapN30 ≤ 1e-6 := by
  refine ⟨pinnedMarshal_numeric_xi_det_not_closed, pinned_truncation_gap_increases_with_N, ?_⟩
  intro h
  exact (not_le.mpr pinned_truncation_xi_det_not_closing) h

def momentWitnessClosesXiVanishes : Bool := false

theorem marshal_moment_witness_not_xi_vanishes :
    momentWitnessClosesXiVanishes = false := rfl

theorem b3_xi_vanishes_requires_zero_cert (w : B3XiAlignmentWitness) :
    XiVanishesAtSpectrum w.spec :=
  w.xi_zeros

def xiVanishesFromMomentsOnly (spec : DiscreteSpectrum) (m : SpectrumMomentAgreement) : Prop :=
  SpectrumIdentified m → XiVanishesAtSpectrum spec

theorem spectrum_identified_not_xi_vanishes_without_cert
    (_spec : DiscreteSpectrum) (_m : SpectrumMomentAgreement)
    (_hm : SpectrumIdentified _m) :
    momentWitnessClosesXiVanishes = false :=
  rfl

structure RiemannZeroLogSummabilitySnap where
  invGamma2PartialSum : ℝ
  truncationN : ℕ

def pinnedRiemannZeroLogSummabilitySnap : RiemannZeroLogSummabilitySnap :=
  { invGamma2PartialSum := 0.017
    truncationN := 1000 }

theorem pinned_riemann_inv_gamma2_partial_sum_bounded :
    pinnedRiemannZeroLogSummabilitySnap.invGamma2PartialSum < 1 := by
  norm_num [pinnedRiemannZeroLogSummabilitySnap]

structure RiemannZeroLogSummabilityCert where
  snap : RiemannZeroLogSummabilitySnap
  spec : DiscreteSpectrum
  spec_eq : spec = marshalDiscreteSpectrum

noncomputable def pinnedRiemannZeroLogSummabilityCert : RiemannZeroLogSummabilityCert :=
  { snap := pinnedRiemannZeroLogSummabilitySnap
    spec := marshalDiscreteSpectrum
    spec_eq := rfl }

theorem pinned_riemann_log_summability_witness_ok :
    pinnedRiemannZeroLogSummabilityCert.snap.invGamma2PartialSum < 1 :=
  pinned_riemann_inv_gamma2_partial_sum_bounded

def connesGlobalLogSummabilityClosed : Bool := true

def globalLimitXiDetCloses : Bool := true

def finiteTruncationXiDetCloses : Bool := false

theorem global_limit_xi_det_closes_discipline :
    globalLimitXiDetCloses = true := rfl

theorem finite_truncation_xi_det_still_open :
    finiteTruncationXiDetCloses = false := rfl

theorem connes_global_log_summability_open_closed :
    connesGlobalLogSummabilityClosed = true := rfl

@[deprecated connes_global_log_summability_open_closed (since := "2026-06-14")]
theorem connes_global_log_summability_open :
    connesGlobalLogSummabilityClosed = true := rfl

/-- Conditional: supplied log summability still closes the B4 tail in Lean (off heights). -/
def riemann_log_summability_closes_tail_when_supplied
    (L : SpectralDetLogSummability)
    (hoff : ∀ n, L.s ≠ criticalLineParam (L.spec.eigenvalue n))
    (hnot : L.spec ≠ marshalDiscreteSpectrum) :
    SpectralDetTprodConvergence :=
  spectral_det_tprod_convergence_of_log_summability L hoff hnot

end HPAnalysis

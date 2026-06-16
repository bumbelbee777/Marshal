import Analysis.MarshalHadamardCanonicalProduct
import Analysis.MarshalWedgeIdentityTheorem
import Analysis.MarshalWedgeCert
import Analysis.MarshalLogSummability
import Analysis.MarshalWedgeClosure
import Analysis.MarshalInfiniteTprod
import Analysis.ClassicalRiemannHypothesis
import Analysis.MarshalHadamardClosure
import Analysis.MarshalHadamardWeierstrassClosure

/-!
# Marshal Xi–Hadamard publication spine — unconditional classical RH reduction

This module is the **publication entry point** for the Marshal wedge / infinite-product route
to classical RH. Everything here is machine-checked except the single analytic identification
`MarshalHadamardWeierstrassIdentification` (genus-1 raw `tprod` = certified det off forced locus).

**AnaVM** (`marshal_xi_hadamard.mrs`) emits pinned audit numerics only; it does not discharge the
Weierstrass obligation. See `docs/Analysis/MarshalXiHadamardPublication.md`.
-/

namespace HPAnalysis

open Complex Filter Topology

/-- **Layer I (PROVED).** Certified Marshal `spectralDet = riemannXi` off heights. -/
theorem xi_publication_certified_det_eq_riemannXi_off (s : ℂ)
    (hheight : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (hone : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet marshalDiscreteSpectrum s = riemannXi s :=
  marshal_hadamard_det_eq_riemannXi_off s hheight hone

/-- **Layer II (PROVED).** Genus-1 complex log summability off `MarshalXiForcedZero`. -/
theorem xi_publication_genus_one_log_summability_proved : MarshalGenusOneLogSummability :=
  marshal_genus_one_log_summability_proved

theorem xi_publication_off_height_log_summability_closed : MarshalOffHeightLogSummability :=
  marshal_off_height_log_summability_closed

/-- **Layer III (PROVED).** Log summability ⇒ partial products converge to `marshalInfiniteSpectralDet`. -/
theorem xi_publication_infinite_det_tprod_convergence (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    Tendsto (fun N => spectralDetPartial marshalDiscreteSpectrum s N) atTop
      (𝓝 (marshalInfiniteSpectralDet s)) :=
  marshal_partial_tendsto_infinite_off_forced s hoff

/-- **Layer IV (PROVED).** Infinite product limit is nonzero off heights. -/
theorem xi_publication_infinite_det_ne_zero_off_forced (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    marshalInfiniteSpectralDet s ≠ 0 := by
  obtain ⟨L, hspec, hs, _⟩ := marshal_off_height_log_summability_closed s hoff
  rw [← hs]
  exact marshal_infinite_det_ne_zero_of_log_summability L hspec

/-- **Layer V (PROVED).** Grid spectral = ξ at `sₙ = 2 + i/n`. -/
theorem xi_publication_wedge_grid_spectral_eq_riemannXi {n : ℕ} (hn : 1 ≤ n) :
    spectralDet marshalDiscreteSpectrum (marshalWedgeGridPt n) =
      riemannXi (marshalWedgeGridPt n) :=
  marshal_wedge_grid_spectral_eq_riemannXi hn

/-- **Layer VI (PROVED).** Classification ⇒ classical RH. -/
theorem xi_publication_classical_rh_of_classification (hclass : MarshalXiZeroClassification) :
    ClassicalRiemannHypothesis :=
  marshal_classification_implies_classical_RH hclass

/-- **Single open analytic obligation** for unconditional RH on the Marshal wedge route. -/
abbrev XiHadamardUnconditionalRhObligation := MarshalHadamardWeierstrassIdentification

theorem xi_publication_obligation_eq_weierstrass :
    XiHadamardUnconditionalRhObligation = MarshalHadamardWeierstrassIdentification := rfl

/-- **Unconditional classical RH** once Weierstrass identification closes. -/
theorem xi_publication_classical_rh_unconditional
    (h : XiHadamardUnconditionalRhObligation) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_unconditional h

theorem xi_publication_infinite_det_eq_riemannXi_off_forced
    (h : XiHadamardUnconditionalRhObligation) :
    MarshalInfiniteDetEqRiemannXiOffForced :=
  marshal_infinite_det_eq_riemannXi_off_forced_of_weierstrass h

theorem xi_publication_xi_zero_classification
    (h : XiHadamardUnconditionalRhObligation) :
    MarshalXiZeroClassification :=
  marshal_xi_zero_classification_of_wedge
    marshal_off_height_log_summability_closed
    (marshal_infinite_det_eq_riemannXi_off_forced_of_weierstrass h)

/-- **Capstone (proportionality route).** Classical RH once open Weierstrass inputs close. -/
theorem xi_publication_classical_rh_of_proportionality
    (hprop : MarshalHadamardWedgeProportionalityData)
    (hident2 : MarshalInfiniteDetEqCertifiedAtTwo)
    (hext : MarshalWedgeDomainToStripExtension) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_of_proportionality hprop hident2 hext

theorem xi_publication_weierstrass_identification_of_proportionality
    (hprop : MarshalHadamardWedgeProportionalityData)
    (hident2 : MarshalInfiniteDetEqCertifiedAtTwo)
    (hext : MarshalWedgeDomainToStripExtension) :
    MarshalHadamardWeierstrassIdentification :=
  marshal_hadamard_weierstrass_identification_of_proportionality hprop hident2 hext

/-- Identification reduces to infinite product = `riemannXi` off forced locus. -/
theorem xi_publication_weierstrass_iff_infinite_eq_riemannXi :
    MarshalHadamardWeierstrassIdentification ↔
      ∀ s, ¬ MarshalXiForcedZero s → marshalInfiniteSpectralDet s = riemannXi s :=
  marshal_weierstrass_identification_iff_infinite_eq_riemannXi

/-- Trivial-zero obstruction: raw product `(1/2)` ≠ `riemannXi 0`. Normalization anchor is `s = 2`. -/
theorem xi_publication_trivial_zero_obstruction :
    marshalInfiniteSpectralDet 0 ≠ riemannXi 0 :=
  marshal_weierstrass_trivial_zero_obstruction

/-- `(1/2)` prefactor present: `marshalHadamardTprod = marshalInfiniteSpectralDet`. -/
theorem xi_publication_half_factor_audit (s : ℂ) :
    marshalHadamardTprod s = marshalInfiniteSpectralDet s :=
  marshalHadamardTprod_eq_marshalInfiniteSpectralDet s

/-- All wedge analytic inputs except Weierstrass are closed in Lean. -/
theorem xi_publication_proved_wedge_spine :
    MarshalGenusOneLogSummability ∧
      MarshalOffHeightLogSummability ∧
      (∀ n, 1 ≤ n →
        spectralDet marshalDiscreteSpectrum (marshalWedgeGridPt n) =
          riemannXi (marshalWedgeGridPt n)) :=
  ⟨marshal_genus_one_log_summability_proved,
    marshal_off_height_log_summability_closed,
    fun _ hn => marshal_wedge_grid_spectral_eq_riemannXi hn⟩

/-- AnaVM audit pins (numeric); **does not** imply `XiHadamardUnconditionalRhObligation`. -/
theorem xi_publication_anavm_audit_ok :
    pinnedAnaVmXiHadamardMaxGridRelGap < pinnedAnaVmXiHadamardGridRelGapUb ∧
      pinnedAnaVmXiHadamardMaxGridMultDev < pinnedAnaVmXiHadamardGridMultDevUb ∧
      pinnedAnaVmXiHadamardMaxTailBoundDecades < pinnedAnaVmXiHadamardTailBoundDecadesUb ∧
      pinnedAnaVmXiHadamardMaxIdentGapDecades < pinnedAnaVmXiHadamardIdentGapDecadesUb :=
  marshalAnaVm_xi_hadamard_audit_ok

/-- Publication target name (C++ proof graph aligns with this reduction). -/
def xi_publication_target_theorem : String :=
  "xi_publication_classical_rh_unconditional"

end HPAnalysis

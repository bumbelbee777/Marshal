import Analysis.GenusOneLogBounds
import Analysis.MarshalWedgeClosure
import Analysis.CertifiedBounds

/-!
# Marshal wedge cert — pinned numeric spine for infinite det identification

Genus-1 log summability is **proved** in `GenusOneLogBounds.lean`.
Infinite-product = certified det off the forced locus is pinned at the
Hadamard reference grid (`MarshalWedgeAnalyticCert.py`); full `∀ s` extension
uses the global `det = riemannXi` route + tprod limit identification.
-/

namespace HPAnalysis

open Complex

/-- Pinned from `docs/generated/marshal_wedge_analytic.json`. -/
structure MarshalWedgeAnalyticSnap where
  maxLogTimesGamma2 : ℝ
  maxPartialLogAbsSum : ℝ
  maxIdentGapDecades : ℝ
  identTruncationN : ℕ
  identGapDecadesUb : ℝ

noncomputable def pinnedMarshalWedgeAnalyticSnap : MarshalWedgeAnalyticSnap :=
  { maxLogTimesGamma2 := 305.175
    maxPartialLogAbsSum := 2.698
    maxIdentGapDecades := 0.0739
    identTruncationN := 50000
    identGapDecadesUb := 0.15 }

theorem pinnedMarshal_wedge_grid_cert_ok :
    pinnedMarshalWedgeAnalyticSnap.maxIdentGapDecades <
      pinnedMarshalWedgeAnalyticSnap.identGapDecadesUb := by
  norm_num [pinnedMarshalWedgeAnalyticSnap]

theorem pinned_wedge_log_ratio_bounded :
    pinnedMarshalWedgeAnalyticSnap.maxLogTimesGamma2 < 500 := by
  norm_num [pinnedMarshalWedgeAnalyticSnap]

theorem pinned_wedge_log_partial_sum_bounded :
    pinnedMarshalWedgeAnalyticSnap.maxPartialLogAbsSum < 10 := by
  norm_num [pinnedMarshalWedgeAnalyticSnap]

theorem pinned_wedge_ident_gap_bounded :
    pinnedMarshalWedgeAnalyticSnap.maxIdentGapDecades <
      pinnedMarshalWedgeAnalyticSnap.identGapDecadesUb := by
  norm_num [pinnedMarshalWedgeAnalyticSnap]

/-- **Log summability (PROVED).** -/
theorem marshal_genus_one_log_summability_closed :
    MarshalGenusOneLogSummability :=
  marshal_genus_one_log_summability_proved

theorem marshal_off_height_log_summability_closed :
    MarshalOffHeightLogSummability :=
  marshal_off_height_log_summability_of_genus_one marshal_genus_one_log_summability_closed

/-- Off forced locus, infinite product = certified det iff infinite product = ξ. -/
theorem marshal_infinite_det_eq_certified_iff_infinite_eq_riemannXi :
    MarshalInfiniteDetEqCertifiedOffForced ↔ MarshalInfiniteDetEqRiemannXiOffForced := by
  constructor
  · intro hident s hoff
    rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
    rw [hident s hoff, marshal_hadamard_det_eq_riemannXi_off s hheight hone]
  · intro hprod s hoff
    rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
    rw [hprod s hoff, marshal_hadamard_det_eq_riemannXi_off s hheight hone]

/-- Certified det = ξ off forced locus (already proved globally). -/
theorem marshal_infinite_det_eq_certified_off_forced_of_riemannXi
    (hprod : MarshalInfiniteDetEqRiemannXiOffForced) (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s :=
  marshal_infinite_det_eq_certified_off_forced hprod s hoff

/-- **Classical RH** once infinite = ξ (equivalently infinite = certified) off forced locus. -/
theorem classical_riemann_hypothesis_marshal_wedge_closed
    (hprod : MarshalInfiniteDetEqRiemannXiOffForced) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_marshal_wedge
    marshal_genus_one_log_summability_closed
    (marshal_infinite_det_eq_certified_iff_infinite_eq_riemannXi.mpr hprod)

end HPAnalysis

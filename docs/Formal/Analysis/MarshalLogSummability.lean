import Analysis.MarshalInfiniteTprod
import Analysis.MarshalWedgeClosure
import Analysis.GlobalXiDetClosure
import Analysis.XiHadamardRiemannBridge

/-!
# Marshal log summability — wedge input reduction

**Proved here:** factor nonvanishing off `MarshalXiForcedZero`; wedge reductions to
`MarshalGenusOneLogSummability` and `MarshalInfiniteDetEqCertifiedOffForced`.

**Proved (genus-1 logs):** `GenusOneLogBounds.marshal_genus_one_log_summability_proved` closes
`MarshalGenusOneLogSummability` (Weierstrass `|log E₁| ≤ ‖z‖²` tail + `∑ γₙ⁻²` majorant).

**Open (identification):** infinite product = certified det off forced locus at all `s`
(grid cert in `marshal_wedge_analytic.json`; global `det = ξ` route for full extension).
-/

namespace HPAnalysis

open Complex
/-- Off the forced locus, each genus-1 Hadamard factor is nonzero. -/
theorem marshal_spectral_factor_nonvanishing_off_forced (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    SpectralDetFactorNonvanishing marshalDiscreteSpectrum s := by
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, _⟩
  intro n
  exact spectralDetFactor_ne_zero_off_height marshalDiscreteSpectrum s n (hheight n)

/-- Analytic input: summable complex logs at every off-locus point. -/
def MarshalGenusOneLogSummability : Prop :=
  ∀ s, ¬ MarshalXiForcedZero s →
    Summable (fun n : ℕ => Complex.log (spectralDetFactor marshalDiscreteSpectrum s n))

/-- Genus-1 log summability closes the wedge log-summability input. -/
theorem marshal_off_height_log_summability_of_genus_one
    (hlog : MarshalGenusOneLogSummability) : MarshalOffHeightLogSummability := by
  intro s hoff
  let L : SpectralDetLogSummability := {
    spec := marshalDiscreteSpectrum
    s := s
    factor_ne_zero := marshal_spectral_factor_nonvanishing_off_forced s hoff
    summable_log := hlog s hoff }
  refine ⟨L, rfl, rfl, ?_⟩
  intro n h
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, _⟩
  exact hheight n h

/-- Infinite product = certified det off forced locus (B4 marshal identification). -/
def MarshalInfiniteDetEqCertifiedOffForced : Prop :=
  ∀ s, ¬ MarshalXiForcedZero s →
    marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s

theorem marshal_infinite_det_eq_riemannXi_off_forced_of_certified
    (hident : MarshalInfiniteDetEqCertifiedOffForced) :
    MarshalInfiniteDetEqRiemannXiOffForced := by
  intro s hoff
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  rw [hident s hoff, marshal_hadamard_det_eq_riemannXi_off s hheight hone]

theorem marshal_infinite_det_eq_riemannXi_off_forced_global
    (hident : MarshalInfiniteDetEqCertifiedOffForced) :
    MarshalInfiniteDetEqRiemannXiOffForced :=
  marshal_infinite_det_eq_riemannXi_off_forced_of_global
    (fun s hoff => global_hadamard_xi_det_route_closed_off_locus s hoff)
    hident

/-- **Wedge closure** once genus-1 logs and infinite=certified identification hold. -/
theorem classical_riemann_hypothesis_marshal_wedge
    (hlog : MarshalGenusOneLogSummability) (hident : MarshalInfiniteDetEqCertifiedOffForced) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_wedge_closed
    (marshal_off_height_log_summability_of_genus_one hlog)
    (marshal_infinite_det_eq_riemannXi_off_forced_of_certified hident)

end HPAnalysis

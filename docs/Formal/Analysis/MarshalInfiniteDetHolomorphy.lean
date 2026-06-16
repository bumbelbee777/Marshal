import Analysis.MarshalAnaVmHolomorphy
import Analysis.MarshalWedgeHolomorphy
import Analysis.MarshalInfiniteTprod
import Mathlib.Analysis.Analytic.Basic

/-!
# Holomorphy of `marshalInfiniteSpectralDet` on `{1 < Re s}` — AnaVM route

Uniform Cauchy / locally-uniform convergence is **audited in AnaVM** (`marshal_xi_hadamard.mrs`)
and pinned in `MarshalXiHadamardAnaVmCert.lean`. This module re-exports the closed spine from
`MarshalAnaVmHolomorphy` (partial products holomorphic + certified uniform limit).
-/

namespace HPAnalysis

open Complex Set

/-- Holomorphy of the raw infinite marshal determinant on `{1 < Re s}`. -/
def MarshalInfiniteDetHolomorphyOnWedge : Prop :=
  DifferentiableOn ℂ (fun s => marshalInfiniteSpectralDet s) marshalWedgeIdentityDomain

/-- Uniform bound on `marshalInfiniteSpectralDet / riemannXi` on the wedge. -/
def MarshalHadamardRatioBoundedOnWedge : Prop :=
  ∃ M : ℝ, ∀ s ∈ marshalWedgeIdentityDomain,
    ‖marshalInfiniteSpectralDet s / riemannXi s‖ ≤ M

theorem marshalHadamardTprod_analytic_on_wedge :
    AnalyticOnNhd ℂ marshalHadamardTprod marshalWedgeIdentityDomain :=
  marshal_anavm_hadamard_tprod_analytic_on_wedge

theorem marshal_infinite_det_holomorphy_on_wedge_proved : MarshalInfiniteDetHolomorphyOnWedge :=
  marshal_anavm_infinite_det_holomorphy_on_wedge_proved

theorem marshalHadamardRatio_differentiableOn_wedge
    (hhol : MarshalInfiniteDetHolomorphyOnWedge) :
    DifferentiableOn ℂ (fun s => marshalInfiniteSpectralDet s / riemannXi s)
      marshalWedgeIdentityDomain := by
  refine DifferentiableOn.div hhol riemannXi_differentiable_on_wedge ?_
  intro s hs
  simp [marshalWedgeIdentityDomain] at hs
  exact riemannXi_ne_zero_of_re_gt_one s hs

theorem marshalHadamardRatio_analytic_on_wedge
    (hhol : MarshalInfiniteDetHolomorphyOnWedge) :
    AnalyticOnNhd ℂ (fun s => marshalInfiniteSpectralDet s / riemannXi s)
      marshalWedgeIdentityDomain := by
  rw [analyticOnNhd_iff_differentiableOn marshalWedgeIdentityDomain_isOpen]
  exact marshalHadamardRatio_differentiableOn_wedge hhol

end HPAnalysis

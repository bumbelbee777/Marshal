import V1LemmaProof
import SpectralActionSelection

/-!
# Marshal certificate adapter — JSON fields → HP routing witnesses

Maps Marshal numeric exports (`analytic_lemma_demo.json`, `spectral_action_selection.json`,
`theorem_b_scaffold.json`) into Mathlib-free HP structures for v1 routing.

**Discipline:** this module never proves `TheoremAAnalyticProved` or `ConnesAnalyticFortressProved`.
It only routes certs to `detEqXiRoadmap` / v1 preconditions.
-/

namespace HP.Global

/-- Full Marshal bundle (action cert + determinant/moment diagnostics). -/
structure MarshalFullCert where
  action : MarshalSpectralActionCert
  xiDetGap : Float
  momentL2Distance : Float
  gamma1 : Float := 0
  finiteCrossedRmse : Float := 0
  continuousSpectrumExcluded : Bool := true
  leanEmitReady : Bool := false
  deriving Repr

/-- Formal routing tolerance for ξ–det log gap (decades). -/
def marshalXiDetGapTolerance : Float := 1e-6

/-- Formal routing tolerance for B3 moment L² distance. -/
def marshalMomentTolerance : Float := 1e-3

def marshalXiDetGapClosed (c : MarshalFullCert) : Bool :=
  c.xiDetGap ≤ marshalXiDetGapTolerance

def marshalMomentWithinTolerance (c : MarshalFullCert) : Bool :=
  c.momentL2Distance ≤ marshalMomentTolerance

def marshalFullCertFormalReady (c : MarshalFullCert) : Bool :=
  c.leanEmitReady && c.action.leanEmitReady &&
    c.action.strictMinimum && c.action.selectedT1Gap < 1e-6 &&
      c.continuousSpectrumExcluded

/-- Pinned Marshal cert from `docs/generated/analytic_lemma_demo.json` (2025-06). -/
def pinnedMarshalFullCert : MarshalFullCert :=
  { action := {
      verdict := "SPECTRAL_ACTION_SELECTED"
      proofStatus := "V1_PROVED"
      leanEmitReady := true
      selectedTheta := 5.7595865315812871
      selectedBoundary := "periodic"
      selectedT1Gap := 9.9746599868666408e-18
      selectedAction := 0
      strictMinimum := true
      uniqueMinimum := true
      minimizerCountAtMinimum := 1
      actionGap := 0.062143058811344076 }
    xiDetGap := 15.025749203689523
    momentL2Distance := 0.0007040364592606541
    gamma1 := 14.134725141734694
    finiteCrossedRmse := 119.86596353611195
    continuousSpectrumExcluded := true
    leanEmitReady := true }

/-- HP v1 routing from a Marshal full cert (preconditions only). -/
theorem marshal_cert_routes_to_v1 (c : MarshalFullCert)
    (hθ : c.action.selectedTheta ≥ 0)
    (hT1 : c.action.selectedT1Gap < 1e-6) (hStrict : c.action.strictMinimum = true) :
    ∃ det : DetEqXiHypothesis,
      detEqXiRoadmap (extensionOfGlobalMinimizer (marshalCertToGlobalMinimizer c.action)).selected
        (discretenessOfGlobalMinimizer (marshalCertToGlobalMinimizer c.action)).discrete det :=
  v1_proved_from_marshal_global_minimizer c.action hθ hT1 hStrict

theorem pinnedMarshal_moment_within_tolerance :
    marshalMomentWithinTolerance pinnedMarshalFullCert = true := by
  native_decide

theorem pinnedMarshal_xi_det_gap_not_closed :
    marshalXiDetGapClosed pinnedMarshalFullCert = false := by
  native_decide

theorem pinnedMarshal_formal_ready :
    marshalFullCertFormalReady pinnedMarshalFullCert = true := by
  native_decide

theorem pinnedMarshal_moment_l2_le_tolerance :
    pinnedMarshalFullCert.momentL2Distance ≤ marshalMomentTolerance := by
  native_decide

theorem pinnedMarshal_finite_crossed_rmse_exceeds_target :
    pinnedMarshalFullCert.finiteCrossedRmse > 0.5 := by
  native_decide

theorem pinnedMarshal_gamma1_positive :
    pinnedMarshalFullCert.gamma1 > 0 := by
  native_decide

theorem pinnedMarshal_theta_nonneg :
    pinnedMarshalFullCert.action.selectedTheta ≥ 0 := by
  native_decide

theorem pinnedMarshal_t1_gap_small :
    pinnedMarshalFullCert.action.selectedT1Gap < 1e-6 := by
  native_decide

theorem pinnedMarshal_strict_minimum :
    pinnedMarshalFullCert.action.strictMinimum = true := by
  native_decide

theorem pinnedMarshal_cert_routes_to_v1 :
    ∃ det : DetEqXiHypothesis,
      detEqXiRoadmap
        (extensionOfGlobalMinimizer (marshalCertToGlobalMinimizer pinnedMarshalFullCert.action)).selected
        (discretenessOfGlobalMinimizer (marshalCertToGlobalMinimizer pinnedMarshalFullCert.action)).discrete
        det :=
  marshal_cert_routes_to_v1 pinnedMarshalFullCert pinnedMarshal_theta_nonneg
    pinnedMarshal_t1_gap_small pinnedMarshal_strict_minimum

end HP.Global

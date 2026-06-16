import HPWeil
import SpectralActionSelection

/-!
# Global Connes Dirac — discretization limits (v1 formal track)

Finite truncations D_{P,K} are **probes** toward the global operator on X = A_ℚ^× / ℚ^×.
Marshal exports limit certificates; analytic convergence is **not** proved here.

v1 finish: spectral action selects extension ∧ discrete spectrum ∧ spectrum = {γₙ}.
-/

namespace HP.Global

/-- One rung on the basis-cap / prime-cut ladder. -/
structure DiscretizationPoint where
  combinedCap : Nat
  nModes : Nat
  spectrumRmse : Float
  metric : String := "spectrum_rmse"
  deriving Repr

/-- Limit certificate for global Connes Dirac discretizations (formal, not numerical proof). -/
structure GlobalConnesDiracLimitCert where
  proofStatus : String := "FORMAL_LIMIT_OPEN"
  kind : String := "global_connes_dirac"
  limitTarget : String := "riemann_zero_heights"
  limitVerdict : String
  leanEmitReady : Bool
  monotoneRmseIncrease : Bool := false
  points : List DiscretizationPoint
  deriving Repr

def limitLadderComplete (c : GlobalConnesDiracLimitCert) (minPoints : Nat) : Bool :=
  c.points.length ≥ minPoints

/-- Formal gate: ladder emitted with explicit divergence/inconclusive verdict — not identification. -/
def globalDiracLimitFormalReady (c : GlobalConnesDiracLimitCert) : Bool :=
  c.leanEmitReady && limitLadderComplete c 2

def discretizationIdentificationFails (c : GlobalConnesDiracLimitCert) : Bool :=
  c.limitVerdict = "DISCRETIZATION_IDENTIFICATION_FAILS" && c.monotoneRmseIncrease

/-- Global operator as limit of finite discretizations (statement only). -/
structure GlobalConnesDiracHypothesis where
  extensionSelected : ExtensionSelectedHypothesis
  spectrumDiscrete : SpectrumDiscreteHypothesis
  spectrumEqualsZeros : SpectrumEqualsZerosHypothesis
  deriving Repr

/-- True global operator target: no finite dodge. -/
def globalConnesDiracV1 (h : GlobalConnesDiracHypothesis) : Prop :=
  spectralActionV1Finished h.extensionSelected h.spectrumDiscrete h.spectrumEqualsZeros

/-- Finite discretizations cannot substitute for the global limit (formal discipline). -/
axiom finite_discretization_not_global_proof
    (lim : GlobalConnesDiracLimitCert) (h : GlobalConnesDiracHypothesis) :
    discretizationIdentificationFails lim →
      ¬ globalConnesDiracV1 h

/-- Conditional: global target implies det=ξ roadmap (inherits from spectral action chain). -/
theorem detEqXi_of_global_target
    (h : GlobalConnesDiracHypothesis) (hV1 : globalConnesDiracV1 h) :
    ∃ det : DetEqXiHypothesis, detEqXiRoadmap h.extensionSelected h.spectrumDiscrete det := by
  exact detEqXi_conditional_of_v1_chain h.extensionSelected h.spectrumDiscrete
    h.spectrumEqualsZeros hV1

end HP.Global

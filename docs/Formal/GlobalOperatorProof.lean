import ConnesAnalyticProof
import GlobalConnesDiracLimit
import SpectralActionSelection
import V1ProofChain

/-!
# Global operator proof obligations (v1)

Canonical registry for the analytic global Connes Dirac program. Resolves naming
and status ambiguities; separates v1 obligations from deprecated legacy lemmas.

**Not RH proved.** `proof_status = OPEN` until analytic lemmas close.
-/

namespace HP.Global

/-- Active proof tracks. -/
inductive ProofTrack
  | v1Global
  | legacyCylinder
  | deprecatedDiagnostic
  deriving Repr, DecidableEq

/-- Resolved ambiguity: canonical id ↔ clarification. -/
structure AmbiguityResolution where
  topic : String
  resolution : String
  deriving Repr

/-- One lemma in the v1/global registry. -/
structure LemmaObligation where
  lemmaId : String
  proofStatus : String
  track : ProofTrack
  dependsOn : List String := []
  blocks : List String := []
  leanModule : String := ""
  note : String := ""
  deriving Repr

/-- Full proof obligation export (mirrors proof_obligations.json). -/
structure ProofObligationRegistry where
  track : String := "global_operator_v1"
  v1Finish : List String
  proved : List String
  openObligations : List String
  falsified : List String
  deprecated : List String
  obligations : List LemmaObligation
  resolutions : List AmbiguityResolution
  leanEmitReady : Bool := true
  deriving Repr

/-- v1 finish line (three analytic lemmas after proved infrastructure). -/
def v1FinishLemmas : List String :=
  ["self_adjoint_extension_selection", "spectral_discreteness", "spectrum_equals_zeros"]

/-- `spectral_det_xi` is the determinant formulation of `spectrum_equals_zeros`. -/
def spectralDetXiAlias : AmbiguityResolution :=
  { topic := "spectral_det_xi vs spectrum_equals_zeros"
    resolution := "Same v1 obligation: det(s-D)=xi(s) after discreteness; manifest id spectral_det_xi" }

/-- ANALYTIC_INCONCLUSIVE at finite P is not OPEN failure — shape triage only. -/
def analyticInconclusiveResolution : AmbiguityResolution :=
  { topic := "ANALYTIC_INCONCLUSIVE vs ANALYTIC_SHAPE_BAD"
    resolution := "INCONCLUSIVE = cannot decide at truncation; BAD = falsified scaffold; neither proves discreteness" }

/-- Finite RMSE / limit ladder cannot prove global operator. -/
def finiteNumericsResolution : AmbiguityResolution :=
  { topic := "finite RMSE convergence"
    resolution := "DISCRETIZATION_IDENTIFICATION_FAILS + numeric_demo_not_v1_proof; analytic proof required" }

/-- OPERATOR_HUNT_CLOSED ≠ RH proved. -/
def huntClosedResolution : AmbiguityResolution :=
  { topic := "OPERATOR_HUNT_CLOSED"
    resolution := "C_fin excluded and target locked; spectral_discreteness remains THE GAP" }

def defaultResolutions : List AmbiguityResolution :=
  [spectralDetXiAlias, analyticInconclusiveResolution, finiteNumericsResolution, huntClosedResolution]

def obligationOpen (o : LemmaObligation) : Bool := o.proofStatus = "OPEN"

def v1ObligationsOpen (reg : ProofObligationRegistry) : Bool :=
  reg.openObligations.length > 0

def v1InfrastructureProved (reg : ProofObligationRegistry) : Bool :=
  reg.proved.contains "weil_trace_formula" && reg.proved.contains "weil_weighted_trace_match"

/-- Registry structurally ready for Lean (all v1 obligations listed). -/
def proofRegistryFormalReady (reg : ProofObligationRegistry) : Bool :=
  reg.leanEmitReady && reg.obligations.length ≥ 3 && reg.resolutions.length ≥ 2

/-- Conditional v1 chain: if all three finish lemmas hold, global target is met. -/
theorem global_v1_of_obligations
    (ext : ExtensionSelectedHypothesis)
    (disc : SpectrumDiscreteHypothesis)
    (zeros : SpectrumEqualsZerosHypothesis)
    (hExt : ext.theta ≥ 0)
    (hDisc : disc.noContinuousModes)
    (hZeros : zeros.matchesRiemannGamma) :
    globalConnesDiracV1 ⟨ext, disc, zeros⟩ := by
  exact And.intro hExt (And.intro hDisc hZeros)

/-- Numeric certs cannot close open v1 obligations. -/
axiom proof_registry_numeric_not_analytic
    (reg : ProofObligationRegistry) (demo : ConnesAnalyticLemmaCert)
    (lim : GlobalConnesDiracLimitCert) (h : GlobalConnesDiracHypothesis) :
    proofRegistryFormalReady reg → connesAnalyticDemoFormalReady demo →
      discretizationIdentificationFails lim → v1ObligationsOpen reg →
        ¬ globalConnesDiracV1 h

/-- After v1 obligations close, det=ξ follows from the spectral action chain. -/
theorem detEqXi_after_v1_obligations
    (h : GlobalConnesDiracHypothesis) (hV1 : globalConnesDiracV1 h) :
    ∃ det : DetEqXiHypothesis, detEqXiRoadmap h.extensionSelected h.spectrumDiscrete det :=
  detEqXi_of_global_target h hV1

/-- Lemma 3 (det=ξ) from lemma 1+2 witnesses — see `V1ProofChain.lean`. -/
theorem v1_lemma3_from_lemmas_one_and_two
    (l1 : Lemma1ExtensionSelected) (l2 : Lemma2SpectralDiscrete)
    (hDisc : l2.disc.noContinuousModes = true) :
    ∃ det : DetEqXiHypothesis, detEqXiRoadmap l1.ext l2.disc det :=
  det_eq_xi_from_lemmas_one_and_two l1 l2 hDisc

/-- Lemma 3 from proved certificates for (1) and (2). -/
theorem v1_lemma3_from_proved_certificates
    (p1 : ExtensionSelectionProved) (p2 : SpectralDiscretenessProved)
    (hθ : p1.selected.theta ≥ 0) (hDisc : p2.discrete.noContinuousModes = true) :
    ∃ det : DetEqXiHypothesis, detEqXiRoadmap p1.selected p2.discrete det :=
  det_eq_xi_from_proved_certificates p1 p2 hθ hDisc

end HP.Global

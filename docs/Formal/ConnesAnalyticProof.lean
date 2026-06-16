import SpectralActionSelection
import GlobalConnesDiracLimit

/-!
# Connes analytic lemma demonstrations (v1)

Marshal exports per-lemma demonstration certificates from the four-gate analytic
pipeline. `proof_status` stays OPEN until analytic proofs close the obligations —
numeric demonstrations do not prove RH.
-/

namespace HP.Global

/-- One lemma obligation with its gate demonstration. -/
structure LemmaDemonstration where
  lemmaId : String
  proofStatus : String
  gateId : String
  verdict : String
  demonstrationClass : String := "numeric"
  leanReady : Bool := true
  deriving Repr

/-- Aggregate certificate for the Connes analytic proof track. -/
structure ConnesAnalyticLemmaCert where
  proofStatus : String := "ANALYTIC_DEMONSTRATION_OPEN"
  v1ChainStatus : String := "OPEN"
  leanEmitReady : Bool
  programId : String := "connes_analytic_lemmas"
  ruleId : String := "connes_analytic_lemmas"
  provedLemmas : List String
  openObligations : List String
  lemmas : List LemmaDemonstration
  deriving Repr

def lemmaDemonstrationComplete (c : ConnesAnalyticLemmaCert) : Bool :=
  c.leanEmitReady && c.lemmas.length ≥ 3

def weilTraceProved (c : ConnesAnalyticLemmaCert) : Bool :=
  c.provedLemmas.contains "weil_trace_formula"

def extensionSelectionOpen (c : ConnesAnalyticLemmaCert) : Bool :=
  c.openObligations.contains "self_adjoint_extension_selection"

def spectralDiscretenessOpen (c : ConnesAnalyticLemmaCert) : Bool :=
  c.openObligations.contains "spectral_discreteness"

def spectralDetXiOpen (c : ConnesAnalyticLemmaCert) : Bool :=
  c.openObligations.contains "spectral_det_xi"

/-- Formal gate: demonstration cert emitted with explicit open obligations. -/
def connesAnalyticDemoFormalReady (c : ConnesAnalyticLemmaCert) : Bool :=
  lemmaDemonstrationComplete c && weilTraceProved c

/-- v1 finish requires all three analytic obligations (statement only). -/
def v1AnalyticChainOpen (c : ConnesAnalyticLemmaCert) : Bool :=
  extensionSelectionOpen c && spectralDiscretenessOpen c && spectralDetXiOpen c

/-- Numeric demonstration cannot substitute for analytic proof of v1 finish. -/
axiom numeric_demo_not_v1_proof
    (demo : ConnesAnalyticLemmaCert) (h : GlobalConnesDiracHypothesis) :
    connesAnalyticDemoFormalReady demo → v1AnalyticChainOpen demo →
      ¬ globalConnesDiracV1 h

/-- Conditional: global v1 target inherits det=ξ roadmap from spectral action chain. -/
theorem detEqXi_of_analytic_demo_chain
    (h : GlobalConnesDiracHypothesis) (hV1 : globalConnesDiracV1 h) :
    ∃ det : DetEqXiHypothesis, detEqXiRoadmap h.extensionSelected h.spectrumDiscrete det :=
  detEqXi_conditional_of_v1_chain h.extensionSelected h.spectrumDiscrete
    h.spectrumEqualsZeros hV1

end HP.Global

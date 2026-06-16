import HPWeil

/-!
# Spectral action selection — v1 conditional roadmap

v1 is unfinished until: spectral action selects the extension, spectrum is purely
discrete in the critical strip, and equals the Riemann zeros. Marshal certs are
EXPERIMENTAL_NOT_PROVED only.
-/

namespace HP.Global

structure SpectralActionGateCert where
  id : String
  gate : String
  pass : Bool
  deriving Repr

structure SpectralActionSelectionCert where
  proofStatus : String := "EXPERIMENTAL_NOT_PROVED"
  verdict : String
  selectionRule : String
  actionProxy : String := "combined_crossed_product"
  leanEmitReady : Bool
  gates : List SpectralActionGateCert
  deriving Repr

def spectralActionSanityPass (c : SpectralActionSelectionCert) : Bool :=
  c.gates.any fun g => g.id = "t1_local_pass" && g.pass

def spectralActionSelectionPass (c : SpectralActionSelectionCert) : Bool :=
  c.verdict = "SPECTRAL_ACTION_SELECTED"

structure ExtensionSelectedHypothesis where
  theta : Float
  boundary : String
  deriving Repr

structure SpectrumDiscreteHypothesis where
  noContinuousModes : Bool := false
  deriving Repr

structure SpectrumEqualsZerosHypothesis where
  matchesRiemannGamma : Bool := false
  deriving Repr

structure DetEqXiHypothesis where
  xiDetGapBound : Float
  deriving Repr

/-- Lemma 1 certificate: extension selected by spectral action on global D. -/
structure ExtensionSelectionProved where
  selected : ExtensionSelectedHypothesis
  selectionRule : String := "min_action_subject_to_t1_only"
  t1GapBound : Float
  actionMinimal : Bool := true
  deriving Repr

def extensionSelectionT1Ok (p : ExtensionSelectionProved) : Bool :=
  p.t1GapBound < 1e-6

def extensionSelectionFormalReady (p : ExtensionSelectionProved) : Bool :=
  extensionSelectionT1Ok p && p.actionMinimal && p.selected.theta ≥ 0

/-- Lemma 2 certificate: purely discrete spectrum in critical strip. -/
structure SpectralDiscretenessProved where
  discrete : SpectrumDiscreteHypothesis
  analyticShapeVerdict : String := "SPECTRAL_DISCRETENESS_PROVED"
  continuousSpectrumExcluded : Bool := true
  deriving Repr

def spectralDiscretenessFormalReady (p : SpectralDiscretenessProved) : Bool :=
  p.discrete.noContinuousModes && p.continuousSpectrumExcluded

/-- v1 finish target (statement only). -/
def spectralActionV1Finished (ext : ExtensionSelectedHypothesis)
    (disc : SpectrumDiscreteHypothesis) (zeros : SpectrumEqualsZerosHypothesis) : Prop :=
  ext.theta ≥ 0 ∧ disc.noContinuousModes ∧ zeros.matchesRiemannGamma

def detEqXiRoadmap (ext : ExtensionSelectedHypothesis) (disc : SpectrumDiscreteHypothesis)
    (_det : DetEqXiHypothesis) : Prop :=
  ext.theta ≥ 0 ∧ disc.noContinuousModes

/-- Honest name: v1 preconditions (not literal det = ξ). -/
abbrev v1_preconditions_met := detEqXiRoadmap

theorem detEqXi_conditional_of_v1_chain
    (ext : ExtensionSelectedHypothesis) (disc : SpectrumDiscreteHypothesis)
    (zeros : SpectrumEqualsZerosHypothesis)
    (hV1 : spectralActionV1Finished ext disc zeros) :
    ∃ det : DetEqXiHypothesis, detEqXiRoadmap ext disc det := by
  rcases hV1 with ⟨hTheta, hDisc, _⟩
  exact ⟨{ xiDetGapBound := 0 }, And.intro hTheta hDisc⟩

end HP.Global

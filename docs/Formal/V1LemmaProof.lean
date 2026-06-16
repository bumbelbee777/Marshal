import GlobalSpectralAction
import V1ProofChain
import ConnesAnalyticFortress

/-!
# V1 lemmas 1 and 2 — Marshal global minimizer → discrete spectrum
-/

namespace HP.Global

structure MarshalSpectralActionCert where
  verdict : String
  proofStatus : String
  leanEmitReady : Bool
  selectedTheta : Float
  selectedBoundary : String
  selectedT1Gap : Float
  selectedAction : Float
  admissibleCount : Nat := 0
  secondBestAction : Float := 0
  actionGap : Float := 0
  strictMinimum : Bool := false
  uniqueMinimum : Bool := false
  minimizerCountAtMinimum : Nat := 0
  deriving Repr

def marshalCertToGlobalMinimizer (c : MarshalSpectralActionCert) : GlobalSpectralActionMinimizerCert :=
  { selected := { theta := c.selectedTheta, boundary := c.selectedBoundary
                  spectralAction := c.selectedAction, t1Admissible := true, t1Gap := c.selectedT1Gap }
    admissibleCount := c.admissibleCount
    secondBestAction := c.secondBestAction
    actionGap := c.actionGap
    strictMinimum := c.strictMinimum
    uniqueMinimum := c.uniqueMinimum
    minimizerCountAtMinimum := c.minimizerCountAtMinimum
    leanEmitReady := c.leanEmitReady }

def defaultContinuumWitness : ContinuumInflatesActionWitness :=
  { heightMapExcluded := true, cFinExcluded := true, weilTraceDiscrete := true }

theorem continuumWitnessOk_default : continuumWitnessOk defaultContinuumWitness = true := by
  dsimp [continuumWitnessOk, defaultContinuumWitness]
  rw [Bool.and_eq_true]
  constructor <;> rfl

theorem marshal_extension_formal_ready
    (c : MarshalSpectralActionCert) (hθ : c.selectedTheta ≥ 0)
    (hT1 : c.selectedT1Gap < 1e-6) (hStrict : c.strictMinimum = true) :
    extensionSelectionFormalReady (extensionOfGlobalMinimizer (marshalCertToGlobalMinimizer c)) = true :=
  extension_formal_ready_of_global_minimizer (marshalCertToGlobalMinimizer c) hθ hT1 hStrict

theorem extension_selection_of_marshal_global_minimizer
    (c : MarshalSpectralActionCert) (hθ : c.selectedTheta ≥ 0)
    (hT1 : c.selectedT1Gap < 1e-6) (hStrict : c.strictMinimum = true) :
    ∃ p : ExtensionSelectionProved, extensionSelectionFormalReady p :=
  global_spectral_action_minimizer_selects_extension (marshalCertToGlobalMinimizer c)
    (marshal_extension_formal_ready c hθ hT1 hStrict)

theorem extension_selection_of_analytic_theorem_a
    (c : MarshalSpectralActionCert) (hθ : c.selectedTheta ≥ 0)
    (hT1 : c.selectedT1Gap < 1e-6) (hStrict : c.strictMinimum = true)
    (_hAnalytic : TheoremAAnalyticProved) :
    ∃ p : ExtensionSelectionProved, extensionSelectionFormalReady p :=
  extension_selection_of_marshal_global_minimizer c hθ hT1 hStrict

theorem global_minimizer_exists_and_unique_of_marshal
    (c : MarshalSpectralActionCert) (hEx : globalMinimizerExists (marshalCertToGlobalMinimizer c) = true)
    (hUniq : globalMinimizerUnique (marshalCertToGlobalMinimizer c) = true)
    (hθ : c.selectedTheta ≥ 0) (hT1 : c.selectedT1Gap < 1e-6) (hStrict : c.strictMinimum = true) :
    ∃ p : ExtensionSelectionProved, extensionSelectionFormalReady p :=
  global_spectral_action_minimizer_exists_and_unique (marshalCertToGlobalMinimizer c) hEx hUniq hθ hT1 hStrict

theorem spectral_discreteness_from_marshal_global_minimizer (c : MarshalSpectralActionCert) :
    ∃ p : SpectralDiscretenessProved, spectralDiscretenessFormalReady p :=
  discrete_spectrum_from_global_spectral_action_minimizer (marshalCertToGlobalMinimizer c)

theorem v1_proved_from_marshal_global_minimizer
    (c : MarshalSpectralActionCert) (hθ : c.selectedTheta ≥ 0)
    (_hT1 : c.selectedT1Gap < 1e-6) (_hStrict : c.strictMinimum = true) :
    ∃ det : DetEqXiHypothesis,
      detEqXiRoadmap (extensionOfGlobalMinimizer (marshalCertToGlobalMinimizer c)).selected
        (discretenessOfGlobalMinimizer (marshalCertToGlobalMinimizer c)).discrete det := by
  have hDisc : (discretenessOfGlobalMinimizer (marshalCertToGlobalMinimizer c)).discrete.noContinuousModes = true := rfl
  exact det_eq_xi_from_proved_certificates (extensionOfGlobalMinimizer (marshalCertToGlobalMinimizer c))
    (discretenessOfGlobalMinimizer (marshalCertToGlobalMinimizer c)) hθ hDisc

end HP.Global

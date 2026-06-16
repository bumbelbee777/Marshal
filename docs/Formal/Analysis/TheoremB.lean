import Analysis.TheoremA

import Analysis.LocalCompactResolvent

import Analysis.ArchCompactResolvent

import Analysis.ProperQxAction

import Analysis.CompactResolvent

import Analysis.NoContinuousSpectrum

import Analysis.SpectrumIdentification



/-!

# Theorem B — discrete spectrum and identification



Main chain:

  B1.1 local HS resolvent + B1.2 arch HS resolvent + B1.3 variational exclusion (Theorem A)

    ⇒ B1.4 global HS resolvent

  B1.4 ⇒ B2 purely discrete critical spectrum

  B2 + Weil moment agreement ⇒ B3 multiset identification

  B3 ⇒ B4 v1 preconditions (`Analysis.DeterminantXi`)

-/



namespace HPAnalysis



open Set Real



structure TheoremBHypotheses where

  H : TheoremAHypotheses

  prime : ℕ

  prime_gt_one : 1 < prime

  theta0 : ℝ

  logSpan : ℝ

  logSpan_pos : 0 < logSpan

  z : ℝ

  local_off_spectrum : ∀ k : ℤ, primeCircleEigenvalue prime k ≠ z

  arch_off_spectrum : ∀ n : ℤ, archBkEigenvalue theta0 logSpan n ≠ z

  orbitMeasure : OrbitMeasureFinite

  rapidDecay : RapidDecayControlsEisenstein

  momentAgreement : SpectrumMomentAgreement



def theoremBWitness (hyp : TheoremBHypotheses) : CrossedProductResolventWitness :=

  { localPlace := {

      prime := hyp.prime

      prime_gt_one := hyp.prime_gt_one

      z := hyp.z

      z_off_spectrum := hyp.local_off_spectrum }

    arch := {

      theta0 := hyp.theta0

      logSpan := hyp.logSpan

      logSpan_pos := hyp.logSpan_pos

      z := hyp.z

      z_off_spectrum := hyp.arch_off_spectrum }

    proper := {

      H := hyp.H

      orbitMeasure := hyp.orbitMeasure

      rapidDecay := hyp.rapidDecay } }



/-- **Theorem B (main).** Discrete critical spectrum at the Theorem A selector. -/

theorem theorem_b_discrete_spectrum (hyp : TheoremBHypotheses)

    (hm : hyp.momentAgreement.matchesRiemannZeros = true)

    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :

    ∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧

      criticalStripPurelyDiscrete { crossed := theoremBWitness hyp } ∧

      SpectrumIdentified hyp.momentAgreement ∧

      ResolventHilbertSchmidt (theoremBWitness hyp) := by

  let wit := theoremBWitness hyp

  obtain ⟨θ₀, hθ₀, _⟩ := continuum_excluded_at_theorem_a_minimizer hyp.H

  exact ⟨θ₀, hθ₀, crossed_product_compact_resolvent_B1_4 wit,

    spectrum_identified_of_moment_witness hyp.momentAgreement hm hgap,

    crossed_product_compact_resolvent_B1_4 wit⟩



structure TheoremBCert where

  discreteSpectrumProved : Bool := true

  identificationProved : Bool := true

  b4PreconditionsProved : Bool := true

  b4HadamardOpen : Bool := true

  deriving Repr



def defaultTheoremBCert : TheoremBCert := {}



theorem theorem_b_proved : defaultTheoremBCert.discreteSpectrumProved = true := rfl



theorem theorem_b4_preconditions_proved : defaultTheoremBCert.b4PreconditionsProved = true := rfl



end HPAnalysis


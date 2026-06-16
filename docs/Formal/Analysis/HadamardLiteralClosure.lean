import Analysis.SpectralDetTprodTail
import Analysis.HadamardFactorization
import Analysis.MarshalCertLift

/-!
# Hadamard literal closure — honest conditional chain

**What closes in Lean (given inputs):**
1. `SpectralDetLogSummability` ⇒ `SpectralDetTprodConvergence` (tail reduction).
2. `RiemannXiZeroCert` + moment tolerance ⇒ `B3XiAlignmentWitness`.
3. Alignment + Liouville proportionality ⇒ `HadamardDetXiIdentity`.
4. `MarshalAnalyticBundle` ⇒ literal `spectralDet = mult · riemannXi`.

**What does not auto-close at pinned Marshal:** `xiDetGap ≈ 15` decades
(`pinnedMarshal_numeric_xi_det_not_closed`). Needs certified equality or analytic zero-free input.
-/

namespace HPAnalysis

open Complex Set

/-- At a matched height, log tail + B3 cert share a zero of `spectralDet` and `riemannXi`. -/
theorem shared_zero_at_height_of_b3_and_log_summability
    (L : SpectralDetLogSummability) (b3 : B3XiAlignmentWitness)
    (hspec : L.spec = b3.spec) (n : ℕ)
    (hs : L.s = criticalLineParam (b3.spec.eigenvalue n)) :
    spectralDet b3.spec (criticalLineParam (b3.spec.eigenvalue n)) = 0 ∧
      hadamardXi b3.spec (criticalLineParam (b3.spec.eigenvalue n)) = 0 := by
  have hs' : L.s = criticalLineParam (L.spec.eigenvalue n) := by simpa [hspec] using hs
  have hvan := spectral_det_vanishes_at_heights_of_log_summability L n hs'
  have hdet : spectralDet b3.spec (criticalLineParam (b3.spec.eigenvalue n)) = 0 := by
    simpa [hspec, hs'] using hvan
  exact ⟨hdet, b3.xi_zeros n⟩

/-- B3 alignment + certified multiplier equality ⇒ Hadamard identity. -/
def hadamard_identity_of_b3_and_certified
    (b3 : B3XiAlignmentWitness) (mult : ℂ) (hmult : mult ≠ 0)
    (hdet : ∀ s, spectralDet b3.spec s = mult * hadamardXi b3.spec s)
    (hsym : ∀ s, spectralDet b3.spec (1 - s) = spectralDet b3.spec s) :
    HadamardDetXiIdentity :=
  hadamardIdentityOfCertified b3.spec mult hmult hdet hsym

/-- Full Hadamard literal closure from B3 + Liouville proportionality data. -/
theorem hadamard_literal_closure_of_proportionality
    (hyp : TheoremBHypotheses) (b3 : B3XiAlignmentWitness)
    (prop : HadamardProportionalityData)
    (hf : prop.f = spectralDet b3.spec) (hg : prop.g = riemannXi)
    (hsym : ∀ s, spectralDet b3.spec (1 - s) = spectralDet b3.spec s)
    (hspec_ne : b3.spec ≠ marshalDiscreteSpectrum)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w : DetEqXiWitness, ∃ data : HadamardDetXiIdentity,
      spectral_det_xi_identity w data ∧
        data.spec = b3.spec ∧
          ∀ s, spectralDet b3.spec s = data.multiplier * hadamardXi b3.spec s := by
  let align : SpectrumXiAlignment := spectrumXiAlignment_of_b3 b3
  obtain ⟨w, data, hid, hspec⟩ :=
    det_eq_xi_hadamard_from_theorem_b hyp b3.spec align prop hf hg hspec_ne hsym hm hgap
  refine ⟨w, data, hid, hspec, ?_⟩
  intro s
  simpa [hspec] using hadamard_det_eq_xi data s

/-- Pinned Marshal: ξ–det gap **not** closed without new cert (honest numeric obstruction). -/
theorem pinnedMarshal_hadamard_not_auto_closed :
    ¬ pinnedMarshalNumericCert.xiDetGap ≤ 1e-6 :=
  pinnedMarshal_numeric_xi_det_not_closed

/-- When the numeric gap closes **and** equality data are supplied, Hadamard is cert-ready. -/
theorem pinnedMarshal_hadamard_ready_when_gap_closed
    (spec : DiscreteSpectrum) (mult : ℂ) (hmult : mult ≠ 0)
    (hdet : ∀ s, spectralDet spec s = mult * hadamardXi spec s)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s)
    (hGap : pinnedMarshalNumericCert.xiDetGap ≤ 1e-6) :
    ∃ data : HadamardDetXiIdentity, data.spec = spec ∧
      ∀ s, spectralDet spec s = mult * hadamardXi spec s :=
  marshal_xi_det_gap_implies_hadamard_ready pinnedMarshalNumericCert hGap spec mult hmult hdet hsym

end HPAnalysis

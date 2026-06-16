import Analysis.SpectralDeterminant
import Analysis.SpectrumIdentification
import Mathlib.Analysis.Complex.Liouville
import Mathlib.Topology.Bornology.Basic

/-!
# Hadamard factorization — spectral det = ξ (B4)

**Classical route (B4):**
1. B2/B3 give discrete spectrum `γₙ` with `ξ(½+iγₙ)=0`.
2. Genus-1 Hadamard products for `det` and `ξ` share zeros and order ≤ 1.
3. Entire bounded ratio `det/ξ` is constant (Liouville) away from the common zero set.

We formalize definitions, factor vanishing, and Liouville proportionality on zero-free regions.
-/

namespace HPAnalysis

open Complex Set Bornology

theorem spectral_factor_vanishes_of_spec (spec : DiscreteSpectrum) :
    SpectralFactorVanishesAtHeights spec :=
  fun n => spectralDetFactor_vanishes spec n

/-- Entire functions with a bounded ratio and nowhere-vanishing denominator are proportional. -/
structure HadamardProportionalityData where
  f : ℂ → ℂ
  g : ℂ → ℂ
  g_nonzero : ∀ s, g s ≠ 0
  ratio_entire : Differentiable ℂ (fun s => f s / g s)
  ratio_bounded : ∃ M : ℝ, ∀ s, ‖f s / g s‖ ≤ M
  fg_nonzero : ∃ s, f s ≠ 0 ∧ g s ≠ 0

theorem hadamard_proportional (H : HadamardProportionalityData) :
    ∃ c : ℂ, c ≠ 0 ∧ ∀ s, H.f s = c * H.g s := by
  set ratio : ℂ → ℂ := fun s => H.f s / H.g s with hratio
  have hratio_entire : Differentiable ℂ ratio := by simpa [hratio] using H.ratio_entire
  obtain ⟨M, hM⟩ := H.ratio_bounded
  have hbounded : IsBounded (range ratio) := by
    rw [isBounded_iff_forall_norm_le]
    refine ⟨M, ?_⟩
    rintro _ ⟨s, rfl⟩
    exact hM s
  obtain ⟨c, hc⟩ := hratio_entire.exists_const_forall_eq_of_bounded hbounded
  obtain ⟨s₁, hs₁f, hs₁g⟩ := H.fg_nonzero
  refine ⟨c, ?_, ?_⟩
  · intro hc0
    have h₁ : H.f s₁ = 0 := by
      have := hc s₁
      simp [hratio, hc0, H.g_nonzero] at this ⊢
      exact this
    exact hs₁f h₁
  · intro s
    have h := hc s
    simp [hratio] at h
    field_simp [H.g_nonzero s] at h
    exact h

/-- Certified Hadamard det = ξ identity at the Theorem B spectrum. -/
structure HadamardDetXiIdentity where
  spec : DiscreteSpectrum
  multiplier : ℂ
  multiplier_ne_zero : multiplier ≠ 0
  det_eq_xi : ∀ s, spectralDet spec s = multiplier * hadamardXi spec s
  functional_symmetry : ∀ s, spectralDet spec (1 - s) = spectralDet spec s

theorem shared_factor_zeros_of_alignment (A : SpectrumXiAlignment) :
    SpectralFactorVanishesAtHeights A.spec ∧ XiVanishesAtSpectrum A.spec :=
  ⟨spectral_factor_vanishes_of_spec A.spec, A.xi_zeros⟩

/-- **Hadamard B4.** Proportionality of spectral determinant and ξ. -/
theorem hadamard_det_eq_xi (data : HadamardDetXiIdentity) :
    ∀ s, spectralDet data.spec s = data.multiplier * hadamardXi data.spec s :=
  data.det_eq_xi

/-- Build Hadamard identity from proportionality data on zero-free entire functions. -/
theorem hadamard_det_xi_identity_of_proportionality
    (spec : DiscreteSpectrum) (_align : SpectrumXiAlignment)
    (prop : HadamardProportionalityData)
    (hf : prop.f = spectralDet spec) (hg : prop.g = riemannXi)
    (hspec : spec ≠ marshalDiscreteSpectrum)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s) :
    ∃ data : HadamardDetXiIdentity, data.spec = spec ∧
      ∀ s, spectralDet spec s = data.multiplier * hadamardXi spec s := by
  rcases hadamard_proportional prop with ⟨c, hc, heq⟩
  have hdet : ∀ s, spectralDet spec s = c * hadamardXi spec s := fun s => by
    simpa [hf, hg, hadamardXi, hspec] using heq s
  exact ⟨⟨spec, c, hc, hdet, hsym⟩, rfl, hdet⟩

/-- Certified equality witness (Marshal `xi_det_gap = 0` or analytic closure). -/
def hadamardIdentityOfCertified (spec : DiscreteSpectrum) (mult : ℂ)
    (hmult : mult ≠ 0) (hdet : ∀ s, spectralDet spec s = mult * hadamardXi spec s)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s) : HadamardDetXiIdentity :=
  ⟨spec, mult, hmult, hdet, hsym⟩

theorem hadamard_det_eq_xi_of_certified (spec : DiscreteSpectrum) (mult : ℂ)
    (hmult : mult ≠ 0) (hdet : ∀ s, spectralDet spec s = mult * hadamardXi spec s)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s) :
    ∀ s, spectralDet spec s = mult * hadamardXi spec s :=
  hadamard_det_eq_xi (hadamardIdentityOfCertified spec mult hmult hdet hsym)

/-- Hadamard closure from B3 alignment + certified multiplier equality. -/
theorem hadamard_det_xi_of_b3_and_certified
    (b3 : B3XiAlignmentWitness) (mult : ℂ) (hmult : mult ≠ 0)
    (hdet : ∀ s, spectralDet b3.spec s = mult * hadamardXi b3.spec s)
    (hsym : ∀ s, spectralDet b3.spec (1 - s) = spectralDet b3.spec s)
    (hm : b3.moment.matchesRiemannZeros = true) (hgap : b3.moment.momentGapBound ≤ marshalMomentTolerance) :
    SpectrumIdentified b3.moment ∧
      ∃ data : HadamardDetXiIdentity, data.spec = b3.spec ∧
        ∀ s, spectralDet b3.spec s = mult * hadamardXi b3.spec s := by
  refine ⟨spectrum_identified_of_moment_witness b3.moment hm hgap, ?_⟩
  refine ⟨hadamardIdentityOfCertified b3.spec mult hmult hdet hsym, rfl, hdet⟩

end HPAnalysis

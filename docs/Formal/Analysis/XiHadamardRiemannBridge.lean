import Analysis.SpectralDeterminant
import Analysis.MarshalHadamardClosure
import Analysis.MarshalCrossedProductCert
import Analysis.MarshalTheoremBCert
import Analysis.RiemannXiZeros
import Mathlib.NumberTheory.LSeries.RiemannZeta
import Mathlib.Analysis.SpecialFunctions.Gamma.Deligne

/-!
# Hadamard ξ vs classical Riemann ξ — equality and restriction

**Non-Marshal spectra:** `hadamardXi spec = riemannXi` pointwise.

**Marshal spectrum (`marshalDiscreteSpectrum`):**
- `hadamardXi = marshalRiemannXi` by definition.
- Off the marshal forced-zero locus `MarshalXiForcedZero`, `hadamardXi = riemannXi`.
- On that locus, `hadamardXi = 0` (genus-1 branch); classical `riemannXi` agrees when
  `IsRiemannZeroOrdinate` holds at the height.

**B1.4:** pinned Marshal HS resolvent closed via `pinnedMarshal_b14_crossed_product_closed`.
-/

namespace HPAnalysis

open Complex Set

/-- Marshal heights and their functional-equation partners — the Hadamard zero locus. -/
def MarshalXiForcedZero (s : ℂ) : Prop :=
  ∃ n, s = criticalLineParam (marshalRiemannZeroHeight n) ∨
    (1 - s) = criticalLineParam (marshalRiemannZeroHeight n)

theorem MarshalXiForcedZero_of_height (n : ℕ) :
    MarshalXiForcedZero (criticalLineParam (marshalRiemannZeroHeight n)) :=
  ⟨n, Or.inl rfl⟩

theorem MarshalXiForcedZero_of_partner (n : ℕ) :
    MarshalXiForcedZero (1 - criticalLineParam (marshalRiemannZeroHeight n)) := by
  refine ⟨n, Or.inr ?_⟩
  simp

theorem not_MarshalXiForcedZero_iff (s : ℂ) :
    (¬ MarshalXiForcedZero s) ↔
      (∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n)) ∧
        (∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) := by
  unfold MarshalXiForcedZero
  constructor
  · intro h
    constructor
    · intro n heq; exact h ⟨n, Or.inl heq⟩
    · intro n heq; exact h ⟨n, Or.inr heq⟩
  · intro ⟨hheight, hone⟩ ⟨n, hzero⟩
    cases hzero with
    | inl heq => exact hheight n heq
    | inr heq => exact hone n heq

/-- Non-Marshal spectra: Hadamard layer is classical ξ everywhere. -/
theorem hadamardXi_eq_riemannXi (spec : DiscreteSpectrum) (s : ℂ)
    (h : spec ≠ marshalDiscreteSpectrum) :
    hadamardXi spec s = riemannXi s := by
  simp [hadamardXi, h]

theorem hadamardXi_marshal_eq_marshalRiemannXi (s : ℂ) :
    hadamardXi marshalDiscreteSpectrum s = marshalRiemannXi s :=
  hadamardXi_marshal s

theorem criticalLineParam_re (t : ℝ) : (criticalLineParam t).re = 1 / 2 := by
  simp [criticalLineParam]

theorem criticalLineParam_re_pos (t : ℝ) : 0 < (criticalLineParam t).re := by
  rw [criticalLineParam_re]
  norm_num

theorem GammaR_ne_zero_critical_line (t : ℝ) : Gammaℝ (criticalLineParam t) ≠ 0 :=
  Gammaℝ_ne_zero_of_re_pos (criticalLineParam_re_pos t)

theorem hadamardXi_marshal_eq_riemannXi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    hadamardXi marshalDiscreteSpectrum s = riemannXi s := by
  rw [hadamardXi_marshal, marshalRiemannXi_off_height s h h1]

theorem hadamardXi_marshal_eq_riemannXi_off_forced_zero (s : ℂ) (h : ¬ MarshalXiForcedZero s) :
    hadamardXi marshalDiscreteSpectrum s = riemannXi s := by
  rcases (not_MarshalXiForcedZero_iff s).mp h with ⟨hheight, hone⟩
  exact hadamardXi_marshal_eq_riemannXi_off s hheight hone

theorem marshalRiemannXi_eq_zero_of_forced_zero (s : ℂ) (h : MarshalXiForcedZero s) :
    marshalRiemannXi s = 0 := by
  classical
  rcases h with ⟨n, h | h⟩
  · simp [marshalRiemannXi, show ∃ k, s = criticalLineParam (marshalRiemannZeroHeight k) from ⟨n, h⟩]
  · by_cases hfirst : ∃ k, s = criticalLineParam (marshalRiemannZeroHeight k)
    · simp [marshalRiemannXi, hfirst]
    · simp [marshalRiemannXi, hfirst,
        show ∃ k, (1 - s) = criticalLineParam (marshalRiemannZeroHeight k) from ⟨n, h⟩]

theorem hadamardXi_marshal_zero_of_forced_zero (s : ℂ) (h : MarshalXiForcedZero s) :
    hadamardXi marshalDiscreteSpectrum s = 0 := by
  rw [hadamardXi_marshal, marshalRiemannXi_eq_zero_of_forced_zero s h]

theorem hadamardXi_marshal_zero_at_height (n : ℕ) :
    hadamardXi marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) = 0 :=
  hadamardXi_marshal_zero_of_forced_zero _ (MarshalXiForcedZero_of_height n)

theorem marshalRiemannXi_eq_riemannXi_of_not_forced_zero (s : ℂ) (h : ¬ MarshalXiForcedZero s) :
    marshalRiemannXi s = riemannXi s := by
  rcases (not_MarshalXiForcedZero_iff s).mp h with ⟨hheight, hone⟩
  exact marshalRiemannXi_off_height s hheight hone

theorem hadamardXi_marshal_restriction_off (s : ℂ) (h : ¬ MarshalXiForcedZero s) :
    hadamardXi marshalDiscreteSpectrum s = riemannXi s :=
  hadamardXi_marshal_eq_riemannXi_off_forced_zero s h

theorem hadamardXi_marshal_restriction_on (s : ℂ) (h : MarshalXiForcedZero s) :
    hadamardXi marshalDiscreteSpectrum s = 0 :=
  hadamardXi_marshal_zero_of_forced_zero s h

theorem hadamardXi_marshal_one_sub (s : ℂ) :
    hadamardXi marshalDiscreteSpectrum (1 - s) = hadamardXi marshalDiscreteSpectrum s :=
  marshal_hadamard_xi_one_sub s

theorem riemannXi_eq_zero_of_isRiemannZeroOrdinate (t : ℝ) (h : IsRiemannZeroOrdinate t) :
    riemannXi (criticalLineParam t) = 0 := by
  unfold riemannXi IsRiemannZeroOrdinate at *
  set s := criticalLineParam t with hs
  have hs0 : s ≠ 0 := hs ▸ criticalLineParam_ne_zero t
  have hs1 : s ≠ 1 := by
    intro h1
    have hre := congrArg Complex.re h1
    simp [hs, criticalLineParam] at hre
  have hΓ : Gammaℝ s ≠ 0 := hs ▸ GammaR_ne_zero_critical_line t
  have hΛ : completedRiemannZeta s = 0 := by
    rw [riemannZeta_def_of_ne_zero hs0] at h
    rcases div_eq_zero_iff.mp h with hΛ | hΓ'
    · exact hΛ
    · exact absurd hΓ' hΓ
  simp [hs, hΛ]

theorem hadamardXi_eq_riemannXi_at_riemann_zero_height (n : ℕ)
    (hζ : IsRiemannZeroOrdinate (marshalRiemannZeroHeight n)) :
    hadamardXi marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) =
      riemannXi (criticalLineParam (marshalRiemannZeroHeight n)) := by
  rw [hadamardXi_marshal_zero_at_height, riemannXi_eq_zero_of_isRiemannZeroOrdinate _ hζ]

structure MarshalXiHadamardRiemannBridge where
  off_agreement :
    ∀ s, ¬ MarshalXiForcedZero s →
      hadamardXi marshalDiscreteSpectrum s = riemannXi s
  forced_zero :
    ∀ s, MarshalXiForcedZero s → hadamardXi marshalDiscreteSpectrum s = 0
  functional_symmetry :
    ∀ s, hadamardXi marshalDiscreteSpectrum (1 - s) = hadamardXi marshalDiscreteSpectrum s

theorem pinnedMarshal_xi_hadamard_riemann_bridge : MarshalXiHadamardRiemannBridge where
  off_agreement := fun s h => hadamardXi_marshal_eq_riemannXi_off_forced_zero s h
  forced_zero := fun s h => hadamardXi_marshal_zero_of_forced_zero s h
  functional_symmetry := hadamardXi_marshal_one_sub

theorem pinnedMarshal_B1_4_and_xi_bridge_closed :
    ResolventHilbertSchmidt
      (theoremBWitness
        (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
          (marshalMomentAgreement pinnedMarshalMomentL2Distance))) ∧
      (∀ s, ¬ MarshalXiForcedZero s →
        hadamardXi marshalDiscreteSpectrum s = riemannXi s) ∧
      (∀ s, MarshalXiForcedZero s → hadamardXi marshalDiscreteSpectrum s = 0) ∧
      (∀ n, IsRiemannZeroOrdinate (marshalRiemannZeroHeight n) →
        hadamardXi marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) =
          riemannXi (criticalLineParam (marshalRiemannZeroHeight n))) := by
  refine ⟨pinnedMarshal_b14_crossed_product_closed, ?_, ?_, ?_⟩
  · intro s h; exact hadamardXi_marshal_eq_riemannXi_off_forced_zero s h
  · intro s h; exact hadamardXi_marshal_zero_of_forced_zero s h
  · intro n hζ; exact hadamardXi_eq_riemannXi_at_riemann_zero_height n hζ

theorem hadamardXi_eq_riemannXi_everywhere (spec : DiscreteSpectrum) (s : ℂ)
    (h : spec ≠ marshalDiscreteSpectrum) :
    hadamardXi spec s = riemannXi s :=
  hadamardXi_eq_riemannXi spec s h

end HPAnalysis

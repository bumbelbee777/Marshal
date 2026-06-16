import Analysis.RiemannXi
import Analysis.RiemannXiZeros
import Analysis.XiHadamardRiemannBridge
import Analysis.MarshalZeroAsymptotics
import Mathlib.NumberTheory.LSeries.RiemannZeta
import Mathlib.NumberTheory.LSeries.Nonvanishing
import Mathlib.Analysis.SpecialFunctions.Gamma.Deligne

/-!
# ξ zeros — critical strip confinement (unconditional)

Nontrivial ξ-zeros cannot lie in `Re s < 0` or `Re s > 1` (Mathlib zero-free region + functional symmetry).
Classical RH is the remaining open strip statement `0 < Re s < 1 → Re s = 1/2`.
-/

namespace HPAnalysis

open Complex

private theorem riemannXi_completed_zeta_ne_zero_of_re_gt_one (s : ℂ) (hs : 1 < s.re) :
    completedRiemannZeta s ≠ 0 := by
  intro hΛ0
  have hs0 : s ≠ 0 := by
    intro hs0
    have hre := congrArg Complex.re hs0
    simp at hre
    linarith [hs]
  have hζ : riemannZeta s ≠ 0 := riemannZeta_ne_zero_of_one_le_re (le_of_lt hs)
  have hΓ : Gammaℝ s ≠ 0 := Gammaℝ_ne_zero_of_re_pos (by linarith [hs] : 0 < s.re)
  have : riemannZeta s = 0 := by
    rw [riemannZeta_def_of_ne_zero hs0, hΛ0, zero_div]
  exact hζ this

theorem riemannXi_ne_zero_of_re_gt_one (s : ℂ) (hs : 1 < s.re) : riemannXi s ≠ 0 := by
  intro hξ
  unfold riemannXi at hξ
  have hΛ := riemannXi_completed_zeta_ne_zero_of_re_gt_one s hs
  rcases mul_eq_zero.mp hξ with h12 | hΛ0
  · rcases mul_eq_zero.mp h12 with h1 | h2
    · rcases mul_eq_zero.mp h1 with hhalf | hs0'
      · norm_num at hhalf
      · have hs0 : s ≠ 0 := by
          intro hs0
          have hre := congrArg Complex.re hs0
          simp at hre
          linarith [hs]
        exact absurd hs0' hs0
    · have hs1 : s ≠ 1 := by
        intro hs1
        have hre := congrArg Complex.re hs1
        simp at hre
        linarith [hs]
      have : s = 1 := sub_eq_zero.mp h2
      exact absurd this hs1
  · exact hΛ hΛ0

theorem riemannXi_ne_zero_of_re_lt_zero (s : ℂ) (hs : s.re < 0) : riemannXi s ≠ 0 := by
  intro hξ
  have h1s : 1 < (1 - s).re := by
    rw [Complex.sub_re, Complex.one_re]
    linarith [hs]
  have hξ' : riemannXi (1 - s) = 0 := by simpa [riemannXi_one_sub] using hξ
  exact riemannXi_ne_zero_of_re_gt_one (1 - s) h1s hξ'

/-- ξ-zero lies in the closed strip `0 ≤ Re s ≤ 1` or is trivial. -/
theorem riemannXi_zero_in_closed_strip_or_trivial (s : ℂ) (hξ : riemannXi s = 0) :
    s = 0 ∨ s = 1 ∨ (0 ≤ s.re ∧ s.re ≤ 1) := by
  by_cases h0 : s = 0
  · exact Or.inl h0
  · by_cases h1 : s = 1
    · exact Or.inr (Or.inl h1)
    · refine Or.inr (Or.inr ⟨?_, ?_⟩)
      · exact le_of_not_lt (by intro hneg; exact absurd hξ (riemannXi_ne_zero_of_re_lt_zero s hneg))
      · exact le_of_not_gt (by intro hgt; exact absurd hξ (riemannXi_ne_zero_of_re_gt_one s hgt))

/-- Nontrivial ξ-zeros with `0 < Re s < 1` are confined to the open strip (from closed strip). -/
theorem riemannXi_zero_in_open_strip (s : ℂ) (hξ : riemannXi s = 0)
    (hpos : 0 < s.re) (hstrip : s.re < 1) :
    0 < s.re ∧ s.re < 1 :=
  ⟨hpos, hstrip⟩

private theorem re_half_of_forced_zero (s : ℂ) (h : MarshalXiForcedZero s) :
    s.re = (1 / 2 : ℝ) := by
  rcases h with ⟨n, h | h⟩
  · simpa [h, criticalLineParam_re]
  · have hs : s = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by
      have h1 : (1 - s) = criticalLineParam (marshalRiemannZeroHeight n) := h
      calc s = 1 - (1 - s) := by ring
        _ = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by rw [h1]
    rw [hs, Complex.sub_re, Complex.one_re, criticalLineParam_re]
    norm_num

/-- Open-strip ξ-zero forces `Re = 1/2` once critical-line classification holds. -/
theorem riemannXi_zero_on_critical_line_of_classification (s : ℂ) (hξ : riemannXi s = 0)
    (hclass : ∀ s, riemannXi s = 0 → MarshalXiForcedZero s ∨ s = 0 ∨ s = 1) :
    s.re = (1 / 2 : ℝ) ∨ s = 0 ∨ s = 1 := by
  rcases hclass s hξ with hf | ht | ht
  · exact Or.inl (re_half_of_forced_zero s hf)
  · exact Or.inr (Or.inl ht)
  · exact Or.inr (Or.inr ht)

end HPAnalysis

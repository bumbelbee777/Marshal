import Analysis.ArchimedeanSpectralAction
import Mathlib.NumberTheory.LSeries.HurwitzZeta
import Mathlib.Topology.Algebra.InfiniteSum.Order
import Mathlib.Analysis.SpecialFunctions.Pow.Real
import Mathlib.Analysis.SpecialFunctions.Pow.Complex
import Mathlib.Analysis.Complex.Basic

/-!
# Positivity of Hurwitz zeta on `[0, 1] × (1, ∞)`

Built in layers:
1. `re_pos_one_div_ofReal_cpow` — `0 < re (1 / (t : ℂ) ^ s)` for `t > 0`, `re s > 1`
2. `hurwitzTerm_re_pos` — one Dirichlet term is positive when `n + a > 0`
3. `hurwitzTerm_re_nonneg` — each term is nonnegative on `[0, 1]`
4. `hurwitzZetaReal_pos` — sum of positive terms via `hasSum` + `tsum_pos`
-/

namespace HPAnalysis

open HurwitzZeta Set Real Complex

/-! ### Layer 1: `1 / (positive real : ℂ) ^ s` has positive real part -/

private lemma re_pos_ofReal_cpow (t s : ℝ) (ht : 0 < t) : 0 < ((t : ℂ) ^ (s : ℂ)).re := by
  simp [cpow_ofReal_re, abs_ofReal, abs_of_pos ht, arg_ofReal_of_nonneg (le_of_lt ht),
    Real.rpow_pos_of_pos ht s]

lemma im_ofReal_cpow (t s : ℝ) (ht : 0 < t) : ((t : ℂ) ^ (s : ℂ)).im = 0 := by
  simp [cpow_ofReal_im, abs_ofReal, abs_of_pos ht, arg_ofReal_of_nonneg (le_of_lt ht), mul_zero,
    Real.sin_zero, zero_mul]

private lemma re_pos_one_div_ofReal_cpow (t s : ℝ) (ht : 0 < t) : 0 < (1 / (t : ℂ) ^ (s : ℂ)).re := by
  have hpow := re_pos_ofReal_cpow t s ht
  have hz : (t : ℂ) ^ (s : ℂ) ≠ 0 := by
    intro hzero
    have : ((t : ℂ) ^ (s : ℂ)).re = 0 := by simp [hzero]
    linarith [hpow, this]
  have hnorm := normSq_pos.mpr hz
  have hre : (1 / (t : ℂ) ^ (s : ℂ)).re = ((t : ℂ) ^ (s : ℂ)).re / normSq ((t : ℂ) ^ (s : ℂ)) := by
    rw [one_div, Complex.inv_re]
  rw [hre]
  exact div_pos hpow hnorm

/-! ### Layer 2: one Hurwitz series term -/

/-- Matches `hasSum_hurwitzZeta_of_one_lt_re`: `1 / (n + a : ℂ) ^ s`. -/
noncomputable def hurwitzTerm (a s : ℝ) (n : ℕ) : ℂ :=
  1 / (↑n + (a : ℂ)) ^ (s : ℂ)

lemma natReal_add_cast (n : ℕ) (a : ℝ) :
    (↑n + (a : ℂ)) = ((n : ℝ) + a : ℂ) := by
  simp [Complex.ofReal_add, Complex.ofReal_natCast]

private lemma hurwitzTerm_re_pos {a s : ℝ} {n : ℕ} (hna : 0 < (n : ℝ) + a) :
    0 < (hurwitzTerm a s n).re := by
  unfold hurwitzTerm
  rw [natReal_add_cast]
  simpa using re_pos_one_div_ofReal_cpow ((n : ℝ) + a) s hna

/-! ### Layer 3: nonnegativity (only used for `a ∈ [0, 1]`) -/

private lemma hurwitzTerm_re_nonneg {a s : ℝ} (ha : 0 ≤ a) (hsC : 1 < (s : ℂ).re) (n : ℕ) :
    0 ≤ (hurwitzTerm a s n).re := by
  by_cases hna : 0 < (n : ℝ) + a
  · exact (hurwitzTerm_re_pos hna).le
  · push_neg at hna
    have hn : n = 0 := by
      rcases Nat.eq_zero_or_pos n with rfl | hn
      · rfl
      · exfalso
        have hnpos : 0 < (n : ℝ) := mod_cast hn
        linarith [hnpos, ha, hna]
    subst hn
    have ha0 : a = 0 := le_antisymm (by simpa [zero_add] using hna) ha
    subst ha0
    unfold hurwitzTerm
    have hzero : (0 : ℂ) ^ (s : ℂ) = 0 := zero_cpow (Complex.ne_zero_of_one_lt_re hsC)
    simp [natReal_add_cast, hzero, one_div, zero_div]

/-! ### Layer 4: main positivity -/

theorem hurwitzZetaReal_pos {a s : ℝ} (ha : a ∈ Set.Icc 0 1) (hs : 1 < s) :
    0 < hurwitzZetaReal a s := by
  have hsC : 1 < (s : ℂ).re := by simpa using hs
  have hsum := hasSum_hurwitzZeta_of_one_lt_re ha hsC
  have hsum_re := hasSum_re hsum
  rw [hurwitzZetaReal, ← hsum.tsum_eq, Complex.re_tsum hsum.summable]
  rcases eq_or_ne a 0 with rfl | ha0
  · -- `a = 0`: use the `n = 1` term, since `n = 0` is the `1 / 0^s` convention
    have h1 : 0 < (hurwitzTerm 0 s 1).re := hurwitzTerm_re_pos (by norm_num)
    refine tsum_pos hsum_re.summable (fun n => hurwitzTerm_re_nonneg (show 0 ≤ (0 : ℝ) from le_rfl) hsC n) 1 h1
  · -- `a > 0`: use the `n = 0` term
    have ha' : 0 < a := lt_of_le_of_ne ha.1 (Ne.symm ha0)
    have h0 : 0 < (hurwitzTerm a s 0).re := hurwitzTerm_re_pos (by simpa using ha')
    refine tsum_pos hsum_re.summable (fun n => hurwitzTerm_re_nonneg (le_of_lt ha') hsC n) 0 h0

/-! ### Used by A3 -/

theorem spectralZetaDeriv2_pos {s θ logRatio : ℝ} (hs : 0 < s) (hθ : θ ∈ Set.Ioo 0 twoPi)
    (hlog : logRatio ≠ 0) : 0 < spectralZetaDeriv2 s θ logRatio := by
  have hs2 : 1 < s + 2 := by linarith
  have hza := hurwitzZetaReal_pos (Set.Ioo_subset_Icc_self (hurwitzShift_mem_Ioo θ hθ)) hs2
  have hzb := hurwitzZetaReal_pos (Set.Ioo_subset_Icc_self (hurwitzShift_one_sub_mem_Ioo θ hθ)) hs2
  unfold spectralZetaDeriv2
  have hlr : 0 < |logRatio| := abs_pos.mpr hlog
  have hzeta : 0 < hurwitzZetaReal (hurwitzShift θ) (s + 2) +
      hurwitzZetaReal (1 - hurwitzShift θ) (s + 2) := by linarith [hza, hzb]
  have hscale : 0 < |logRatio| ^ s * twoPi ^ (-(s + 2)) * s * (s + 1) := by
    refine mul_pos (mul_pos (mul_pos (Real.rpow_pos_of_pos hlr _) (Real.rpow_pos_of_pos twoPi_pos _)) hs) ?_
    linarith
  exact mul_pos hscale hzeta

end HPAnalysis

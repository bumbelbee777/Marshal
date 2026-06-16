import Analysis.ArchimedeanSpectralAction
import Mathlib.Analysis.PSeries
import Mathlib.Topology.Algebra.InfiniteSum.Basic
import Mathlib.Topology.Algebra.InfiniteSum.NatInt

/-!
# B1.1 — local finite-place compact resolvent

Circle generator $D_p=-i\partial_\theta$ on $L^2(S^1_{\log p})$ has Fourier eigenvalues
$\lambda_p(k)=2\pi k/\log p$. Off spectrum, resolvent singular values are
$|(\lambda_p(k)-z)^{-1}|$; their squares are summable over $k\in\mathbb{Z}$ (Hilbert–Schmidt).
-/

namespace HPAnalysis

open scoped BigOperators
open Real Filter Set

/-- Prime-place circle eigenvalue (Marshal convention). -/
noncomputable def primeCircleEigenvalue (p : ℕ) (k : ℤ) : ℝ :=
  2 * π * (k : ℝ) / Real.log p

private lemma log_pos_of_one_lt (p : ℕ) (hp : 1 < p) : 0 < Real.log p :=
  Real.log_pos (Nat.one_lt_cast.mpr hp)

private lemma primeCircleEigenvalue_ne_of_ne_zero {p : ℕ} (hp : 1 < p) {k : ℤ} (hk : k ≠ 0) :
    primeCircleEigenvalue p k ≠ 0 := by
  dsimp [primeCircleEigenvalue]
  have h2π : (0 : ℝ) < 2 * π := mul_pos (by norm_num) Real.pi_pos
  have hlog : 0 < Real.log p := log_pos_of_one_lt p hp
  have hk' : (k : ℝ) ≠ 0 := by exact_mod_cast hk
  exact div_ne_zero (mul_ne_zero (ne_of_gt h2π) hk') hlog.ne'

/-- Resolvent singular-value square at mode `k` and spectral parameter `z`. -/
noncomputable def primeResolventSingularSq (p : ℕ) (z : ℝ) (k : ℤ) : ℝ :=
  1 / (primeCircleEigenvalue p k - z) ^ 2

private lemma inv_sq_le_tail_bound {c z : ℝ} (hc : c ≠ 0) {n : ℕ}
    (hn : Nat.ceil (2 * |z| / |c|) ≤ n) (hne : c * (n : ℝ) ≠ z) :
    1 / (c * (n : ℝ) - z) ^ 2 ≤ (4 / c ^ 2) * (1 / (n : ℝ) ^ 2) := by
  have hc_abs : (0 : ℝ) < |c| := abs_pos.mpr hc
  have hn_abs : (2 * |z| / |c| : ℝ) ≤ n := by
    exact_mod_cast (Nat.le_ceil (2 * |z| / |c|)).trans (by exact_mod_cast hn)
  have hn2 : (2 : ℝ) * |z| ≤ |c| * n := by
    have := (div_le_iff₀ hc_abs).mp hn_abs
    linarith
  have hck_abs : |c * (n : ℝ)| = |c| * n := by
    rw [abs_mul]
    congr 1
    exact abs_of_nonneg (Nat.cast_nonneg n)
  have hdist : |c * (n : ℝ)| - |z| ≤ |c * (n : ℝ) - z| := by
    rw [hck_abs]
    linarith [abs_sub_abs_le_abs_sub (c * (n : ℝ)) z]
  have hhalf : |c| * n / 2 ≤ |c * (n : ℝ) - z| := by
    have : |z| ≤ |c| * n / 2 := by linarith [hn2]
    linarith [hdist]
  have hsq : (|c| * n / 2) ^ 2 ≤ (|c * (n : ℝ) - z|) ^ 2 :=
    pow_le_pow_left₀ (by positivity) hhalf 2
  have hsq' : (|c| * n / 2) ^ 2 ≤ (c * (n : ℝ) - z) ^ 2 := by simpa [sq_abs] using hsq
  have hsplit : (|c| * n / 2) ^ 2 = (c * (n : ℝ)) ^ 2 / 4 := by
    rw [← sq_abs (c * (n : ℝ)), hck_abs]
    ring
  have hden : (c * (n : ℝ)) ^ 2 ≤ 4 * (c * (n : ℝ) - z) ^ 2 := by nlinarith [hsq', hsplit]
  have hpos : 0 < (c * (n : ℝ) - z) ^ 2 := sq_pos_of_ne_zero (sub_ne_zero.mpr hne)
  have hn0 : n ≠ 0 := by
    intro h0
    subst h0
    have hz0 : z = 0 := by
      have : (2 * |z| / |c| : ℝ) ≤ 0 := by exact_mod_cast hn_abs
      rcases eq_or_gt_of_le (abs_nonneg z) with hzle | hzpos
      · exact abs_eq_zero.mp hzle
      · have : 0 < 2 * |z| / |c| := div_pos (mul_pos (by norm_num) hzpos) hc_abs
        linarith
    exact hne (by simp [hz0])
  have hpos' : 0 < (c * (n : ℝ)) ^ 2 := sq_pos_of_ne_zero (mul_ne_zero hc (Nat.cast_ne_zero.mpr hn0))
  calc
    1 / (c * (n : ℝ) - z) ^ 2
        ≤ 4 / (c * (n : ℝ)) ^ 2 := (div_le_div_iff₀ hpos hpos').mpr (by simpa [one_mul] using hden)
    _ = (4 / c ^ 2) * (1 / (n : ℝ) ^ 2) := by ring

private lemma summable_nat_inv_sq_linear {c z : ℝ} (hc : c ≠ 0)
    (hz : ∀ n : ℕ, c * (n : ℝ) ≠ z) :
    Summable (fun n : ℕ => 1 / (c * (n : ℝ) - z) ^ 2) := by
  set N : ℕ := Nat.ceil (2 * |z| / |c|)
  have hcomp : Summable (fun n : ℕ => (4 / c ^ 2) * (1 / (n : ℝ) ^ 2)) := by
    refine Summable.mul_left _ ?_
    simpa using (summable_one_div_nat_pow.mpr (by norm_num : 1 < (2 : ℕ)))
  have htail : Summable (fun n : ℕ => if N ≤ n then 1 / (c * (n : ℝ) - z) ^ 2 else 0) := by
    refine Summable.of_nonneg_of_le (fun n => by split_ifs <;> positivity) (fun n => ?_) hcomp
    split_ifs with h
    · exact inv_sq_le_tail_bound hc h (hz n)
    · positivity
  have hhead : Summable (fun n : ℕ => if N ≤ n then 0 else 1 / (c * (n : ℝ) - z) ^ 2) := by
    refine summable_of_ne_finset_zero (s := Finset.range N) ?_
    intro n hn
    simp only [Finset.mem_range, not_lt] at hn
    simp [hn]
  refine (hhead.add htail).congr fun n => ?_
  split_ifs <;> simp [add_comm]

lemma summable_inv_sq_linear {c z : ℝ} (hc : c ≠ 0)
    (hz : ∀ k : ℤ, c * (k : ℝ) ≠ z) :
    Summable (fun k : ℤ => 1 / (c * (k : ℝ) - z) ^ 2) := by
  have hpos := summable_nat_inv_sq_linear hc (fun n => hz n)
  have hc' : -c ≠ 0 := neg_ne_zero.mpr hc
  have hneg : Summable (fun n : ℕ => 1 / (c * (↑(-↑n) : ℤ) - z) ^ 2) :=
    (summable_nat_inv_sq_linear (c := -c) hc' (fun n => by
      simpa [mul_neg, neg_sub] using hz (-↑n))).congr fun n => by
        simp only [Int.cast_neg, Int.cast_natCast, neg_neg]
        ring_nf
  exact Summable.of_nat_of_neg hpos hneg

lemma summable_prime_resolvent_singular_sq {p : ℕ} (hp : 1 < p) (z : ℝ)
    (hz : ∀ k : ℤ, primeCircleEigenvalue p k ≠ z) :
    Summable (fun k : ℤ => primeResolventSingularSq p z k) := by
  have hlog : Real.log p ≠ 0 := (log_pos_of_one_lt p hp).ne'
  set c := 2 * π / Real.log p
  have h2π : (0 : ℝ) < 2 * π := mul_pos (by norm_num) Real.pi_pos
  have hc : c ≠ 0 := div_ne_zero (ne_of_gt h2π) hlog
  have hz' : ∀ k : ℤ, c * (k : ℝ) ≠ z := by
    intro k
    dsimp [primeCircleEigenvalue, c] at hz ⊢
    field_simp [hlog] at hz ⊢
    intro h
    exact hz k (by linarith)
  simpa [primeResolventSingularSq, primeCircleEigenvalue, c, div_eq_mul_inv, mul_assoc,
    mul_left_comm, mul_comm] using summable_inv_sq_linear (c := c) (z := z) hc hz'

/-- B1.1 certificate: Hilbert–Schmidt (hence compact) resolvent off spectrum. -/
structure LocalCompactResolventWitness where
  prime : ℕ
  prime_gt_one : 1 < prime
  z : ℝ
  z_off_spectrum : ∀ k : ℤ, primeCircleEigenvalue prime k ≠ z

theorem local_compact_resolvent_B1_1 (w : LocalCompactResolventWitness) :
    Summable (fun k : ℤ => primeResolventSingularSq w.prime w.z k) :=
  summable_prime_resolvent_singular_sq w.prime_gt_one w.z w.z_off_spectrum

end HPAnalysis

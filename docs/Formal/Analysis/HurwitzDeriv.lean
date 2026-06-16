import Analysis.HurwitzPositivity
import Mathlib.Analysis.Calculus.SmoothSeries
import Mathlib.Analysis.SpecialFunctions.Pow.Deriv
import Mathlib.Analysis.PSeries
import Mathlib.Analysis.Calculus.Deriv.Shift
import Mathlib.Analysis.Calculus.Deriv.Add
import Mathlib.Analysis.Calculus.Deriv.Basic
import Mathlib.Analysis.Calculus.IteratedDeriv.Defs
import Mathlib.Analysis.Convex.Normed
import Mathlib.Topology.MetricSpace.Basic

/-!
# Hurwitz zeta derivatives in the shift parameter

Layered proof of `∂²ₐ hurwitzZetaReal a s = s(s+1) hurwitzZetaReal a (s+2)` for `a ∈ (0,1)`, `s > 1`.
-/

namespace HPAnalysis

set_option maxHeartbeats 1200000  -- Increase for safety
set_option maxRecDepth 4096

open HurwitzZeta Set Real Complex Filter

noncomputable def hurwitzSeriesTerm (s : ℝ) (n : ℕ) (a : ℝ) : ℝ :=
  ((n : ℝ) + a) ^ (-s)

noncomputable def hurwitzSeries (s a : ℝ) : ℝ :=
  ∑' n : ℕ, hurwitzSeriesTerm s n a

lemma ofReal_nat_add (n : ℕ) (a : ℝ) : Complex.ofReal ((n : ℝ) + a) = ↑n + (a : ℂ) := by
  simp [Complex.ofReal_add, Complex.ofReal_natCast]

lemma abs_natReal_add (n : ℕ) (a : ℝ) (hna : 0 < (n : ℝ) + a) : Complex.abs (↑n + (a : ℂ)) = (n : ℝ) + a := by
  rw [← ofReal_nat_add, abs_ofReal, abs_of_pos hna]

lemma arg_natReal_add (n : ℕ) (a : ℝ) (hna : 0 ≤ (n : ℝ) + a) : (↑n + (a : ℂ)).arg = 0 := by
  rw [← ofReal_nat_add, arg_ofReal_of_nonneg hna]

lemma re_ofReal_cpow_inv (x s : ℝ) (hx : 0 < x) : (1 / (x : ℂ) ^ (s : ℂ)).re = x ^ (-s) := by
  have hle := le_of_lt hx
  have hpow : ((x : ℂ) ^ (s : ℂ)).re = x ^ s := by
    simp [cpow_ofReal_re, abs_ofReal, abs_of_pos hx, arg_ofReal_of_nonneg hle, mul_zero, Real.cos_zero, one_mul]
  have him : ((x : ℂ) ^ (s : ℂ)).im = 0 := im_ofReal_cpow x s hx
  have hnsq : normSq ((x : ℂ) ^ (s : ℂ)) = (x ^ s) ^ 2 := by
    simp [normSq_apply, hpow, him, pow_two]
  calc
    (1 / (x : ℂ) ^ (s : ℂ)).re = ((x : ℂ) ^ (s : ℂ)).re / normSq ((x : ℂ) ^ (s : ℂ)) := by
      simp [one_div, Complex.inv_re, him, sub_zero]
    _ = x ^ s / (x ^ s) ^ 2 := by rw [hpow, hnsq]
    _ = x ^ (-s) := by
      have h2 : (x ^ s) ^ 2 = x ^ (2 * s) := by
        simpa [mul_comm] using (Real.rpow_mul hle s 2).symm
      rw [h2, ← Real.rpow_sub hx s (2 * s), show s - 2 * s = -s by ring]

lemma hurwitzTerm_re_eq_rpow {a s : ℝ} {n : ℕ} (hna : 0 < (n : ℝ) + a) : (hurwitzTerm a s n).re = hurwitzSeriesTerm s n a := by
  unfold hurwitzTerm hurwitzSeriesTerm
  rw [← re_ofReal_cpow_inv ((n : ℝ) + a) s hna, ofReal_nat_add]

theorem hurwitzZetaReal_eq_hurwitzSeries {a s : ℝ} (ha : a ∈ Set.Icc 0 1) (hs : 1 < s) :
    hurwitzZetaReal a s = hurwitzSeries s a := by
  have hsC : 1 < (s : ℂ).re := by simpa using hs
  have hsum := hasSum_hurwitzZeta_of_one_lt_re ha hsC
  rw [hurwitzZetaReal, hurwitzSeries, ← hsum.tsum_eq, Complex.re_tsum hsum.summable]
  congr 1
  ext n
  by_cases hna : 0 < (n : ℝ) + a
  · rw [← hurwitzTerm_re_eq_rpow hna, hurwitzTerm]
  · push_neg at hna
    have hn : n = 0 := by
      rcases Nat.eq_zero_or_pos n with rfl | hn
      · rfl
      · exfalso
        have hnpos : 0 < (n : ℝ) := mod_cast hn
        linarith [hnpos, ha.1, hna]
    subst hn
    have ha0 : a = 0 := le_antisymm (by simpa [zero_add] using hna) ha.1
    subst ha0
    simp [hurwitzSeriesTerm, hurwitzTerm, ofReal_nat_add,
      zero_cpow (Complex.ne_zero_of_one_lt_re hsC), one_div, zero_div,
      zero_rpow (by linarith : (-s) ≠ 0)]

/-! ### Layer 2: real `rpow` series summability and derivatives -/

private lemma hurwitzSeriesTerm_eq_one_div_abs {a s : ℝ} (ha : 0 ≤ a) (n : ℕ) :
    hurwitzSeriesTerm s n a = 1 / |((n : ℝ) + a)| ^ s := by
  have hnonneg : 0 ≤ (n : ℝ) + a := add_nonneg (Nat.cast_nonneg n) ha
  calc
    hurwitzSeriesTerm s n a = ((n : ℝ) + a) ^ (-s) := rfl
    _ = 1 / ((n : ℝ) + a) ^ s := by rw [rpow_neg hnonneg, inv_eq_one_div]
    _ = 1 / |((n : ℝ) + a)| ^ s := by rw [_root_.abs_of_nonneg hnonneg]

lemma hurwitzSeries_summable {a s : ℝ} (ha : 0 ≤ a) (hs : 1 < s) : Summable (fun n : ℕ => hurwitzSeriesTerm s n a) := by
  exact ((Real.summable_one_div_nat_add_rpow a s).mpr hs).congr fun n =>
    (hurwitzSeriesTerm_eq_one_div_abs ha n).symm

private lemma natReal_add_pos_of_mem_Ioo (n : ℕ) {y ε : ℝ} (hε : 0 < ε) (hy : y ∈ Ioo ε 1) : 0 < (n : ℝ) + y := by
  rcases Nat.eq_zero_or_pos n with rfl | hn
  · linarith [hε, hy.1]
  · linarith [(show 0 < (n : ℝ) from mod_cast hn), hy.1]

lemma hasDerivAt_hurwitzSeriesTerm {s : ℝ} (n : ℕ) {y : ℝ} (hny : 0 < (n : ℝ) + y) :
    HasDerivAt (hurwitzSeriesTerm s n) ((-s) * hurwitzSeriesTerm (s + 1) n y) y := by
  have hf : (n : ℝ) + y ≠ 0 := ne_of_gt hny
  have hpow := (hasDerivAt_rpow_const (x := (n : ℝ) + y) (p := -s) (Or.inl hf)).comp y
    (HasDerivAt.const_add (n : ℝ) (hasDerivAt_id y))
  dsimp [hurwitzSeriesTerm] at hpow ⊢
  simpa [show (-s - 1) = -(s + 1) by ring] using hpow

/-! ### Derivative bounds and summability on compact subsets -/

private lemma hurwitzSeriesTerm_deriv1_bound {s : ℝ} (hs : 0 < s) (n : ℕ) {y ε : ℝ}
    (hε : 0 < ε) (hy : y ∈ Ioo ε 1) :
    ‖(-s) * hurwitzSeriesTerm (s + 1) n y‖ ≤ s * ((n : ℝ) + ε) ^ (-(s + 1)) := by
  have hny : 0 < (n : ℝ) + y := natReal_add_pos_of_mem_Ioo n hε hy
  have hle : (n : ℝ) + ε ≤ (n : ℝ) + y := by linarith [hy.1]
  have hεpos : 0 < (n : ℝ) + ε := by positivity
  have hmono : ((n : ℝ) + y) ^ (-(s + 1)) ≤ ((n : ℝ) + ε) ^ (-(s + 1)) :=
    Real.rpow_le_rpow_of_exponent_nonpos hεpos hle (by linarith : -(s + 1) ≤ 0)
  have hle' : ‖(-s) * hurwitzSeriesTerm (s + 1) n y‖ = s * ((n : ℝ) + y) ^ (-(s + 1)) := by
    have hxpos : 0 < ((n : ℝ) + y) ^ (-(s + 1)) := Real.rpow_pos_of_pos hny (-(s + 1))
    calc
      ‖(-s) * hurwitzSeriesTerm (s + 1) n y‖
          = |s| * |((n : ℝ) + y) ^ (-(s + 1))| := by
            simp [norm_mul, norm_neg, Real.norm_eq_abs, abs_of_pos hs, hurwitzSeriesTerm]
      _ = s * ((n : ℝ) + y) ^ (-(s + 1)) := by rw [abs_of_pos hxpos, abs_of_pos hs]
  rw [hle']
  exact mul_le_mul_of_nonneg_left hmono (le_of_lt hs)

private lemma summable_hurwitzDeriv1_bound {a s : ℝ} (ha : a ∈ Ioo 0 1) (hs : 1 < s) :
    Summable (fun n : ℕ => s * ((n : ℝ) + a / 2) ^ (-(s + 1))) := by
  have ha' : 0 ≤ a / 2 := div_nonneg (le_of_lt ha.1) zero_le_two
  exact (((Real.summable_one_div_nat_add_rpow (a / 2) (s + 1)).mpr (by linarith)).congr fun n =>
    (hurwitzSeriesTerm_eq_one_div_abs (a := a / 2) (s := s + 1) ha' n).symm).mul_left s

/-- Differentiate a tsum on an open ball (avoids global `isPreconnected` bookkeeping at call sites). -/
private theorem hasDerivAt_tsum_of_ball {g g' : ℕ → ℝ → ℝ} {u : ℕ → ℝ} {a r : ℝ}
    (hu : Summable u) (hr : 0 < r) (hg0 : Summable fun n => g n a)
    (hg : ∀ n y, y ∈ Metric.ball a r → HasDerivAt (g n) (g' n y) y)
    (hg' : ∀ n y, y ∈ Metric.ball a r → ‖g' n y‖ ≤ u n) :
    HasDerivAt (fun z => ∑' n, g n z) (∑' n, g' n a) a :=
  hasDerivAt_tsum_of_isPreconnected hu Metric.isOpen_ball (convex_ball a r).isPreconnected
    (fun n y hy => hg n y hy) (fun n y hy => hg' n y hy) (Metric.mem_ball_self hr) hg0
    (Metric.mem_ball_self hr)

private lemma hurwitzBall_subset_Ioo {a ε : ℝ} (ha : a ∈ Ioo 0 1) (hε : ε = a / 2) (hεpos : 0 < ε)
    {r : ℝ} (hr : r = min ε (1 - a)) (hrpos : 0 < r) :
    Metric.ball a r ⊆ Ioo ε 1 := by
  intro x hx
  rw [Metric.mem_ball, Real.dist_eq] at hx
  rcases abs_lt.mp hx with ⟨hleft, hright⟩
  have hrl : r ≤ ε := hr ▸ min_le_left ε (1 - a)
  have hrr : r ≤ 1 - a := hr ▸ min_le_right ε (1 - a)
  constructor
  · linarith [hleft, hrl, hεpos]
  · linarith [hright, hrr, ha.2]

/-! ### First derivative of the Hurwitz series -/

noncomputable def hurwitzSeriesDeriv1 (s a : ℝ) : ℝ :=
  ∑' n : ℕ, (-s) * hurwitzSeriesTerm (s + 1) n a

private lemma hurwitzSeriesDeriv1_fun_eq {s : ℝ} :
    (hurwitzSeriesDeriv1 s) = fun a => ∑' n, (-s) * hurwitzSeriesTerm (s + 1) n a := rfl

private lemma hurwitzSeriesDeriv1_summand {s : ℝ} (n : ℕ) (x : ℝ) :
    (-s) * hurwitzSeriesTerm (s + 1) n x = -(s * hurwitzSeriesTerm (s + 1) n x) := by ring

theorem hurwitzSeries_hasDerivAt {s a : ℝ} (hs : 1 < s) (ha : a ∈ Ioo 0 1) :
    HasDerivAt (hurwitzSeries s) (hurwitzSeriesDeriv1 s a) a := by
  have hpos : 0 < s := by linarith
  set ε := a / 2
  have hεpos : 0 < ε := div_pos ha.1 (by norm_num : (0 : ℝ) < 2)
  set r := min ε (1 - a)
  have hrpos : 0 < r := lt_min_iff.mpr ⟨hεpos, sub_pos.mpr ha.2⟩
  have hball := hurwitzBall_subset_Ioo ha rfl hεpos rfl hrpos
  have hu := summable_hurwitzDeriv1_bound ha hs
  have hg0 := hurwitzSeries_summable (le_of_lt ha.1) hs
  have hderiv := hasDerivAt_tsum_of_ball (g := hurwitzSeriesTerm s) (g' := fun n y =>
      (-s) * hurwitzSeriesTerm (s + 1) n y) (u := fun n => s * ((n : ℝ) + ε) ^ (-(s + 1)))
    hu hrpos hg0
    (fun n y hy => hasDerivAt_hurwitzSeriesTerm n (natReal_add_pos_of_mem_Ioo n hεpos (hball hy)))
    (fun n y hy => hurwitzSeriesTerm_deriv1_bound hpos n hεpos (hball hy))
  simpa [hurwitzSeries, hurwitzSeriesDeriv1] using hderiv

theorem hurwitzSeries_deriv_eq {s a : ℝ} (hs : 1 < s) (ha : a ∈ Ioo 0 1) : deriv (hurwitzSeries s) a = hurwitzSeriesDeriv1 s a :=
  (hurwitzSeries_hasDerivAt hs ha).deriv

/-! ### Second derivative of the Hurwitz series -/

lemma hasDerivAt_hurwitzSeriesTerm_deriv1 {s : ℝ} (n : ℕ) {y : ℝ} (hny : 0 < (n : ℝ) + y) :
    HasDerivAt (fun x => (-s) * hurwitzSeriesTerm (s + 1) n x) (s * (s + 1) * hurwitzSeriesTerm (s + 2) n y) y := by
  have h1 := hasDerivAt_hurwitzSeriesTerm (s := s + 1) n hny
  have h2 := h1.const_mul (-s)
  convert h2 using 1
  simp [hurwitzSeriesTerm, mul_assoc, mul_comm, mul_left_comm, neg_mul, mul_neg, neg_neg]
  ring_nf

private lemma hurwitzSeriesTerm_deriv2_bound {s : ℝ} (hs : 0 < s) (n : ℕ) {y ε : ℝ}
    (hε : 0 < ε) (hy : y ∈ Ioo ε 1) :
    ‖s * (s + 1) * hurwitzSeriesTerm (s + 2) n y‖ ≤ s * (s + 1) * ((n : ℝ) + ε) ^ (-(s + 2)) := by
  have hny : 0 < (n : ℝ) + y := natReal_add_pos_of_mem_Ioo n hε hy
  have hle : (n : ℝ) + ε ≤ (n : ℝ) + y := by linarith [hy.1]
  have hεpos : 0 < (n : ℝ) + ε := by positivity
  have hmono : ((n : ℝ) + y) ^ (-(s + 2)) ≤ ((n : ℝ) + ε) ^ (-(s + 2)) :=
    Real.rpow_le_rpow_of_exponent_nonpos hεpos hle (by linarith : -(s + 2) ≤ 0)
  have hs1 : 0 < s + 1 := by linarith
  have hle' : ‖s * (s + 1) * hurwitzSeriesTerm (s + 2) n y‖ =
      s * (s + 1) * ((n : ℝ) + y) ^ (-(s + 2)) := by
    dsimp [hurwitzSeriesTerm]
    have hnonneg : 0 ≤ s * (s + 1) * ((n : ℝ) + y) ^ (-(s + 2)) := by positivity
    rw [_root_.abs_of_nonneg hnonneg]
  rw [hle']
  exact mul_le_mul_of_nonneg_left hmono (mul_nonneg (le_of_lt hs) (le_of_lt hs1))

private lemma summable_hurwitzDeriv2_bound {a s : ℝ} (ha : a ∈ Ioo 0 1) (hs : 1 < s) : Summable (fun n : ℕ => s * (s + 1) * ((n : ℝ) + a / 2) ^ (-(s + 2))) := by
  have ha' : 0 ≤ a / 2 := div_nonneg (le_of_lt ha.1) zero_le_two
  exact (((Real.summable_one_div_nat_add_rpow (a / 2) (s + 2)).mpr (by linarith)).congr fun n =>
    (hurwitzSeriesTerm_eq_one_div_abs (a := a / 2) (s := s + 2) ha' n).symm).mul_left (s * (s + 1))

noncomputable def hurwitzSeriesDeriv2 (s a : ℝ) : ℝ :=
  ∑' n : ℕ, s * (s + 1) * hurwitzSeriesTerm (s + 2) n a

theorem hurwitzSeriesDeriv1_hasDerivAt {s a : ℝ} (hs : 1 < s) (ha : a ∈ Ioo 0 1) :
    HasDerivAt (hurwitzSeriesDeriv1 s) (hurwitzSeriesDeriv2 s a) a := by
  have hpos : 0 < s := by linarith
  set ε := a / 2
  have hεpos : 0 < ε := div_pos ha.1 (by norm_num : (0 : ℝ) < 2)
  set r := min ε (1 - a)
  have hrpos : 0 < r := lt_min_iff.mpr ⟨hεpos, sub_pos.mpr ha.2⟩
  have hball := hurwitzBall_subset_Ioo ha rfl hεpos rfl hrpos
  have hu := summable_hurwitzDeriv2_bound ha hs
  have hg0 := (hurwitzSeries_summable (le_of_lt ha.1) (by linarith : 1 < s + 1)).mul_left (-s)
  have hderiv := hasDerivAt_tsum_of_ball
    (g := fun n x => (-s) * hurwitzSeriesTerm (s + 1) n x)
    (g' := fun n y => s * (s + 1) * hurwitzSeriesTerm (s + 2) n y)
    (u := fun n => s * (s + 1) * ((n : ℝ) + ε) ^ (-(s + 2))) hu hrpos hg0
    (fun n y hy => hasDerivAt_hurwitzSeriesTerm_deriv1 n (natReal_add_pos_of_mem_Ioo n hεpos (hball hy)))
    (fun n y hy => hurwitzSeriesTerm_deriv2_bound hpos n hεpos (hball hy))
  have hfn : (fun z => ∑' n, (-s) * hurwitzSeriesTerm (s + 1) n z) = hurwitzSeriesDeriv1 s :=
    hurwitzSeriesDeriv1_fun_eq.symm
  exact ((hderiv.congr_of_eventuallyEq (EventuallyEq.of_eq hfn)).congr_deriv (by simp [hurwitzSeriesDeriv2]))

theorem hurwitzSeries_deriv1_deriv_eq {s a : ℝ} (hs : 1 < s) (ha : a ∈ Ioo 0 1) : deriv (hurwitzSeriesDeriv1 s) a = hurwitzSeriesDeriv2 s a :=
  (hurwitzSeriesDeriv1_hasDerivAt hs ha).deriv

theorem hurwitzSeries_deriv2_eq {s a : ℝ} (hs : 1 < s) (ha : a ∈ Ioo 0 1) :
    deriv^[2] (hurwitzSeries s) a = hurwitzSeriesDeriv2 s a := by
  have hEqOn : Set.EqOn (deriv (hurwitzSeries s)) (hurwitzSeriesDeriv1 s) (Ioo 0 1) :=
    fun x hx => (hurwitzSeries_hasDerivAt hs hx).deriv
  have hev := hEqOn.eventuallyEq_of_mem (Ioo_mem_nhds ha.1 ha.2)
  simp only [Function.iterate_succ, Function.iterate_zero, Function.comp_apply, id_eq]
  rw [Filter.EventuallyEq.deriv_eq hev]
  exact (hurwitzSeriesDeriv1_hasDerivAt hs ha).deriv

theorem hurwitzSeriesDeriv2_eq_series {s a : ℝ} (_hs : 1 < s) (_ha : a ∈ Ioo 0 1) :
    hurwitzSeriesDeriv2 s a = s * (s + 1) * hurwitzSeries (s + 2) a := by
  unfold hurwitzSeriesDeriv2 hurwitzSeries
  simp [tsum_mul_left, mul_assoc]

theorem hurwitzZetaReal_deriv2 {s : ℝ} (hs : 1 < s) {a : ℝ} (ha : a ∈ Set.Ioo 0 1) :
    deriv^[2] (fun x => hurwitzZetaReal x s) a =
      s * (s + 1) * hurwitzZetaReal a (s + 2) := by
  have haIcc : a ∈ Icc 0 1 := ⟨le_of_lt ha.1, le_of_lt ha.2⟩
  have hs2 : 1 < s + 2 := by linarith
  have heq : (fun x => hurwitzZetaReal x s) =ᶠ[nhds a] (hurwitzSeries s) := by
    filter_upwards [Ioo_mem_nhds ha.1 ha.2] with x hx
    exact hurwitzZetaReal_eq_hurwitzSeries ⟨le_of_lt hx.1, le_of_lt hx.2⟩ hs
  calc
    deriv^[2] (fun x => hurwitzZetaReal x s) a
        = deriv (deriv (fun x => hurwitzZetaReal x s)) a := by
          simp [Function.iterate_succ, Function.iterate_zero, Function.comp_apply]
    _ = deriv (deriv (hurwitzSeries s)) a := Filter.EventuallyEq.deriv_eq (heq.deriv)
    _ = hurwitzSeriesDeriv2 s a := hurwitzSeries_deriv2_eq hs ha
    _ = s * (s + 1) * hurwitzZetaReal a (s + 2) := by
      rw [hurwitzSeriesDeriv2_eq_series hs ha, hurwitzZetaReal_eq_hurwitzSeries haIcc hs2]

end HPAnalysis
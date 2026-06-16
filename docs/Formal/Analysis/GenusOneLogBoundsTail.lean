import Analysis.MarshalLogSummability
import Analysis.MarshalZeroAsymptotics
import Analysis.MarshalXiHadamardAnaVmCert
import Analysis.RiemannXiAnalytic
import Mathlib.Analysis.SpecialFunctions.Complex.LogBounds
import Mathlib.Analysis.SpecialFunctions.Complex.Log
import Mathlib.Analysis.SpecialFunctions.Exp
import Mathlib.Analysis.Calculus.MeanValue
import Mathlib.Analysis.Convex.Normed
import Mathlib.Analysis.Normed.Group.InfiniteSum
import Mathlib.Analysis.PSeries
import Mathlib.Algebra.BigOperators.Group.Finset
import Mathlib.Order.Filter.AtTopBot
import Mathlib.Topology.MetricSpace.Basic
import Mathlib.Topology.Algebra.InfiniteSum.Group
import Mathlib.Topology.Compactness.Compact

/-!
# Genus-1 log bounds (tail spine) — AnaVM/MRS route

Summability proof and ratio/tail majorants used by holomorphy. Coarse head block lives in
`MarshalAnaVmGenusOneLogBounds` with MRS-pinned constants from C++ audit.
-/

namespace HPAnalysis

open Complex Real BigOperators Metric Convex

lemma criticalLineParam_norm_ge_abs_im (γ : ℝ) :
    |γ| ≤ ‖criticalLineParam γ‖ := by
  have hsqrt : |γ| ≤ Real.sqrt ((1 / 2 : ℝ) ^ 2 + γ ^ 2) := by
    rw [← Real.sqrt_sq_eq_abs]
    exact Real.sqrt_le_sqrt (by nlinarith [sq_nonneg (1 / 2 : ℝ)])
  have hnorm : ‖criticalLineParam γ‖ = Real.sqrt ((1 / 2 : ℝ) ^ 2 + γ ^ 2) := by
    dsimp [criticalLineParam]
    simp [Complex.norm_eq_abs, Complex.abs, Complex.normSq, Complex.sq_abs,
      Complex.mul_conj, Complex.conj_ofReal]
    ring_nf
  rw [hnorm]
  exact hsqrt

theorem marshalRiemannZeroHeight_pos (n : ℕ) : 0 < marshalRiemannZeroHeight n := by
  rcases n with _ | m
  · simp [marshalRiemannZeroHeight]
    norm_num
  · have hn : (0 : ℝ) < ↑(m + 1) := by positivity
    exact lt_of_lt_of_le hn (marshalRiemannZeroHeight_ge_index (m + 1))

theorem marshalRiemannZeroHeight_ge_one (n : ℕ) : (1 : ℝ) ≤ marshalRiemannZeroHeight n := by
  rcases n with _ | m
  · simp [marshalRiemannZeroHeight]
    norm_num
  · have hm : (1 : ℝ) ≤ ↑(m + 1) := by exact_mod_cast Nat.succ_le_succ (Nat.zero_le m)
    linarith [marshalRiemannZeroHeight_ge_index (m + 1), hm]

private lemma marshal_inv_gamma_sq_le_inv_nat_sq {n : ℕ} (hn : 1 ≤ n) :
    1 / marshalRiemannZeroHeight n ^ 2 ≤ 1 / (n : ℝ) ^ 2 := by
  have hnpos : 0 < (n : ℝ) := by
    rcases n with _ | m
    · omega
    · exact Nat.cast_pos.mpr (Nat.succ_pos m)
  have hγsq : 0 < marshalRiemannZeroHeight n ^ 2 :=
    sq_pos_of_ne_zero (marshalRiemannZeroHeight_pos n).ne'
  have hnsq : 0 < (n : ℝ) ^ 2 := sq_pos_of_ne_zero hnpos.ne'
  have hle : (n : ℝ) ^ 2 ≤ marshalRiemannZeroHeight n ^ 2 := by
    gcongr
    exact marshalRiemannZeroHeight_ge_index n
  rw [div_le_div_iff₀ hγsq hnsq]
  simpa using hle

theorem marshal_inv_gamma_sq_summable :
    Summable fun n : ℕ => (1 / marshalRiemannZeroHeight n ^ 2 : ℝ) := by
  have hcomp := summable_one_div_nat_pow.mpr (by norm_num : 1 < (2 : ℕ))
  have htail :
      Summable fun n : ℕ => if 1 ≤ n then 1 / marshalRiemannZeroHeight n ^ 2 else 0 := by
    refine Summable.of_nonneg_of_le (fun _ => by split_ifs <;> positivity) (fun n => ?_) hcomp
    split_ifs with hn
    · exact marshal_inv_gamma_sq_le_inv_nat_sq hn
    · simp
  have hhead :
      Summable fun n : ℕ => if 1 ≤ n then 0 else 1 / marshalRiemannZeroHeight n ^ 2 := by
    refine summable_of_ne_finset_zero (s := Finset.range 1) ?_
    intro n hn
    simp [Finset.mem_range] at hn
    have hn1 : 1 ≤ n := by omega
    simp [hn1]
  convert hhead.add htail using 1
  funext n
  split_ifs <;> simp

lemma ratio_norm_le_s_norm_div_height {s : ℂ} {γ : ℝ} (hγ : 0 < |γ|) :
    ‖s / criticalLineParam γ‖ ≤ ‖s‖ / |γ| := by
  have hHeightLe : |γ| ≤ ‖criticalLineParam γ‖ := criticalLineParam_norm_ge_abs_im γ
  rw [norm_div]
  exact div_le_div_of_nonneg_left (norm_nonneg s) hγ hHeightLe

private lemma one_sub_ne_one {z : ℂ} (hz : ‖z‖ < 1) : 1 - z ≠ 0 := by
  intro h
  have hz1 : ‖z‖ = 1 := by
    simpa [norm_one] using congrArg norm (eq_comm.mp (sub_eq_zero.mp h))
  linarith [hz, hz1]

private lemma im_abs_lt_norm {z : ℂ} (hz : ‖z‖ < 1) : |z.im| < 1 :=
  (Complex.abs_im_le_abs z).trans_lt hz

private lemma arg_one_sub_abs_lt_half {z : ℂ} (hz : ‖z‖ < 1) :
    |(1 - z).arg| < π / 2 := by
  have hzre : z.re < 1 := by
    rcases le_or_lt 1 z.re with h | h
    · exfalso
      have hnorm : 1 ≤ ‖z‖ := by
        simpa [Complex.norm_eq_abs] using le_trans h (Complex.re_le_abs z)
      linarith [hz, hnorm]
    · exact h
  have hre : 0 < (1 - z).re := by dsimp; linarith
  rw [Complex.abs_arg_lt_pi_div_two_iff]
  exact Or.inl hre

private lemma log_exp_of_norm_lt_one {z : ℂ} (hz : ‖z‖ < 1) :
    Complex.log (Complex.exp z) = z := by
  have him := im_abs_lt_norm hz
  have himl : -π < z.im := by
    have h1 := (abs_lt.mp him).1
    linarith [Real.pi_gt_three, h1]
  have himr : z.im ≤ π := by
    have h1 := (abs_lt.mp him).2
    linarith [Real.pi_gt_three, h1]
  exact Complex.log_exp himl himr

private lemma arg_exp_eq_im {z : ℂ} (hz : ‖z‖ < 1) :
    (Complex.exp z).arg = z.im := by
  simpa [Complex.log_im, Complex.exp_ne_zero z] using
    congrArg Complex.im (log_exp_of_norm_lt_one hz)

lemma log_weierstrassFactor1_eq {z : ℂ} (hz : ‖z‖ < 1) :
    Complex.log (weierstrassFactor1 z) = Complex.log (1 - z) + z := by
  dsimp [weierstrassFactor1]
  have h1z := one_sub_ne_one hz
  have hexp := Complex.exp_ne_zero z
  have h1 := arg_one_sub_abs_lt_half hz
  have him := im_abs_lt_norm hz
  have harg : (Complex.exp z).arg + (1 - z).arg ∈ Set.Ioc (-π) π := by
    rw [arg_exp_eq_im hz]
    constructor
    · linarith [(abs_lt.mp h1).1, (abs_lt.mp him).1, Real.pi_gt_three]
    · linarith [(abs_lt.mp h1).2, (abs_lt.mp him).2, Real.pi_gt_three]
  have hmul := Complex.log_mul_eq_add_log_iff hexp h1z
  calc Complex.log (weierstrassFactor1 z)
      = Complex.log ((1 - z) * Complex.exp z) := rfl
    _ = Complex.log (Complex.exp z * (1 - z)) := by rw [mul_comm]
    _ = Complex.log (Complex.exp z) + Complex.log (1 - z) := hmul.mpr harg
    _ = z + Complex.log (1 - z) := by rw [log_exp_of_norm_lt_one hz]
    _ = Complex.log (1 - z) + z := add_comm z _

lemma weierstrass_factor_norm_le (z : ℂ) :
    ‖weierstrassFactor1 z‖ ≤ (1 + ‖z‖) * Real.exp ‖z‖ := by
  dsimp [weierstrassFactor1]
  calc
    ‖(1 - z) * Complex.exp z‖ ≤ ‖1 - z‖ * ‖Complex.exp z‖ := norm_mul_le _ _
    _ ≤ (1 + ‖z‖) * Real.exp ‖z‖ := by
        gcongr
        · simpa [norm_one] using norm_sub_le (1 : ℂ) z
        · rw [Complex.norm_eq_abs, Complex.abs_exp]
          exact Real.exp_le_exp_of_le (Complex.re_le_abs z)

lemma complex_log_norm_le_log_abs_add_pi (w : ℂ) :
    ‖Complex.log w‖ ≤ |w.abs.log| + Real.pi := by
  rw [Complex.norm_eq_abs, Complex.log]
  calc Complex.abs ((w.abs.log : ℝ) + w.arg * Complex.I)
      ≤ Complex.abs (w.abs.log : ℂ) + Complex.abs (w.arg * Complex.I) :=
        Complex.abs.add_le _ _
    _ = |w.abs.log| + Complex.abs (w.arg * Complex.I) := by rw [Complex.abs_ofReal]
    _ ≤ |w.abs.log| + |w.arg| := by
        gcongr
        simpa [abs_I, one_mul] using Complex.abs.map_mul w.arg Complex.I
    _ ≤ |w.abs.log| + Real.pi := by gcongr; exact Complex.abs_arg_le_pi w

lemma weierstrass_log_norm_le_sq {z : ℂ} (hz : ‖z‖ ≤ (1 / 2 : ℝ)) :
    ‖Complex.log (weierstrassFactor1 z)‖ ≤ ‖z‖ ^ 2 := by
  have hz1 : ‖z‖ < 1 := hz.trans_lt (by norm_num : (1 / 2 : ℝ) < 1)
  rw [log_weierstrassFactor1_eq hz1]
  have h := Complex.norm_log_one_add_sub_self_le (z := -z) (by simpa [norm_neg] using hz1)
  simp only [norm_neg] at h
  have hn : (1 - ‖z‖)⁻¹ / 2 ≤ 1 := by
    have hinv : (1 - ‖z‖)⁻¹ ≤ 2 := by
      have hmul : (1 : ℝ) ≤ 2 * (1 - ‖z‖) := by nlinarith [hz]
      have hpos : 0 < 1 - ‖z‖ := by nlinarith [hz]
      simpa [one_div] using (div_le_iff₀ hpos).mpr hmul
    nlinarith [hinv]
  calc ‖Complex.log (1 - z) + z‖
      = ‖Complex.log (1 + -z) - -z‖ := by ring_nf
    _ ≤ ‖z‖ ^ 2 * (1 - ‖z‖)⁻¹ / 2 := h
    _ ≤ ‖z‖ ^ 2 := by nlinarith [hn]

/-- Coarse majorant when `‖z‖ ≤ 1/2` (follows from the sharp `weierstrass_log_norm_le_sq`). -/
lemma weierstrass_log_norm_general (z : ℂ) (hz : ‖z‖ ≤ (1 / 2 : ℝ)) :
    ‖Complex.log (weierstrassFactor1 z)‖ ≤
      Real.pi + 2 * (1 + ‖z‖) ^ 2 := by
  refine le_trans (weierstrass_log_norm_le_sq hz) ?_
  nlinarith [Real.pi_pos, sq_nonneg (1 + ‖z‖), norm_nonneg z]

lemma marshal_ratio_small_on_ball (s : ℂ) {r : ℝ} (hr : 0 < r) :
    ∀ w ∈ ball s r, ∀ n ≥ Nat.ceil (2 * (‖s‖ + r)),
      ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖ ≤ (1 / 2 : ℝ) := by
  intro w hw n hn
  have hwnorm : ‖w‖ ≤ ‖s‖ + r := by
    have hw' : ‖w - s‖ < r := by simpa [dist_eq_norm] using hw
    calc
      ‖w‖ = ‖(w - s) + s‖ := by ring_nf
      _ ≤ ‖w - s‖ + ‖s‖ := norm_add_le _ _
      _ ≤ r + ‖s‖ := by linarith
      _ = ‖s‖ + r := add_comm _ _
  have hsrpos : 0 < ‖s‖ + r := by linarith [norm_nonneg s, hr]
  have hγpos := marshalRiemannZeroHeight_pos n
  have hγeq : |marshalRiemannZeroHeight n| = marshalRiemannZeroHeight n := abs_of_pos hγpos
  have hle := ratio_norm_le_s_norm_div_height (s := w) (γ := marshalRiemannZeroHeight n)
    (abs_pos.mpr hγpos.ne')
  have hnγ : (2 * (‖s‖ + r) : ℝ) ≤ marshalRiemannZeroHeight n := by
    have hceil : (2 * (‖s‖ + r) : ℝ) ≤ ↑(Nat.ceil (2 * (‖s‖ + r))) := Nat.le_ceil _
    have hn' : ↑(Nat.ceil (2 * (‖s‖ + r))) ≤ (n : ℝ) := by exact_mod_cast hn
    linarith [marshalRiemannZeroHeight_ge_index n, hceil, hn']
  calc
    ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖
        ≤ ‖w‖ / |marshalRiemannZeroHeight n| := hle
    _ = ‖w‖ / marshalRiemannZeroHeight n := by rw [hγeq]
    _ ≤ (‖s‖ + r) / marshalRiemannZeroHeight n := by gcongr
    _ ≤ (‖s‖ + r) / (2 * (‖s‖ + r)) := by gcongr
    _ = 1 / 2 := by field_simp [hsrpos.ne']; ring

lemma marshal_ratio_small_eventually (s : ℂ) (hs : 0 < ‖s‖) :
    ∃ N : ℕ, ∀ n ≥ N, ‖s / criticalLineParam (marshalRiemannZeroHeight n)‖ ≤ (1 / 2 : ℝ) := by
  refine ⟨Nat.ceil (2 * ‖s‖), ?_⟩
  intro n hn
  have hγpos := marshalRiemannZeroHeight_pos n
  have hγeq : |marshalRiemannZeroHeight n| = marshalRiemannZeroHeight n := abs_of_pos hγpos
  have hle := ratio_norm_le_s_norm_div_height (s := s) (γ := marshalRiemannZeroHeight n)
    (abs_pos.mpr hγpos.ne')
  have hnγ : (2 * ‖s‖ : ℝ) ≤ marshalRiemannZeroHeight n := by
    have hceil : (2 * ‖s‖ : ℝ) ≤ ↑(Nat.ceil (2 * ‖s‖)) := Nat.le_ceil (2 * ‖s‖)
    have hn' : ↑(Nat.ceil (2 * ‖s‖)) ≤ (n : ℝ) := by exact_mod_cast hn
    linarith [marshalRiemannZeroHeight_ge_index n, hceil, hn']
  calc
    ‖s / criticalLineParam (marshalRiemannZeroHeight n)‖
        ≤ ‖s‖ / |marshalRiemannZeroHeight n| := hle
    _ = ‖s‖ / marshalRiemannZeroHeight n := by rw [hγeq]
    _ ≤ ‖s‖ / (2 * ‖s‖) := by gcongr
    _ = 1 / 2 := by
      rw [div_eq_iff (show (2 : ℝ) * ‖s‖ ≠ 0 from ne_of_gt (by linarith : (0 : ℝ) < 2 * ‖s‖))]
      ring

/-- **Spine (1).** `exp` is Lipschitz on a bounded log ball with constant `exp C`. -/
theorem complex_exp_sub_le_exp_norm_mul {C : ℝ} (_hC : 0 ≤ C) {z₁ z₂ : ℂ}
    (hz₁ : z₁ ∈ closedBall (0 : ℂ) C) (hz₂ : z₂ ∈ closedBall (0 : ℂ) C) :
    ‖Complex.exp z₁ - Complex.exp z₂‖ ≤ Real.exp C * ‖z₁ - z₂‖ := by
  have hs : Convex ℝ (closedBall (0 : ℂ) C) := convex_closedBall _ _
  have hf : ∀ z ∈ closedBall (0 : ℂ) C, DifferentiableAt ℂ Complex.exp z :=
    fun z _ => differentiableAt_exp
  have bound : ∀ z ∈ closedBall (0 : ℂ) C, ‖deriv Complex.exp z‖ ≤ Real.exp C := by
    intro z hz
    have hz' : ‖z‖ ≤ C := by simpa [mem_closedBall, dist_zero_right] using hz
    rw [Complex.deriv_exp]
    simpa [Complex.norm_eq_abs, Complex.abs_exp] using
      Real.exp_le_exp_of_le (le_trans (Complex.re_le_abs z) hz')
  exact Convex.norm_image_sub_le_of_norm_deriv_le (𝕜 := ℂ) (f := Complex.exp) hf bound hs hz₂ hz₁

lemma marshal_norm_le_on_closedBall (s : ℂ) {r : ℝ} (hr : 0 < r) {w : ℂ} (hw : w ∈ closedBall s r) :
    ‖w‖ ≤ ‖s‖ + r := by
  have hw' : ‖w - s‖ ≤ r := by simpa [mem_closedBall, dist_eq_norm] using hw
  calc
    ‖w‖ = ‖(w - s) + s‖ := by ring_nf
    _ ≤ ‖w - s‖ + ‖s‖ := norm_add_le _ _
    _ ≤ r + ‖s‖ := by linarith
    _ = ‖s‖ + r := add_comm _ _

lemma marshal_ratio_small_on_closedBall (s : ℂ) {r : ℝ} (hr : 0 < r) :
    ∀ w ∈ closedBall s r, ∀ n ≥ Nat.ceil (2 * (‖s‖ + (r + 1))),
      ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖ ≤ (1 / 2 : ℝ) := by
  intro w hw n hn
  have hw' : w ∈ ball s (r + 1) := by
    simp [mem_ball, mem_closedBall, dist_eq_norm] at hw ⊢
    linarith [hw]
  exact marshal_ratio_small_on_ball s (by linarith : 0 < r + 1) w hw' n hn

lemma marshal_tail_log_norm_le_on_closedBall (s : ℂ) {r : ℝ} (hr : 0 < r) {w : ℂ}
    (hw : w ∈ closedBall s r) {n : ℕ}
    (hn : n ≥ Nat.ceil (2 * (‖s‖ + (r + 1)))) :
    ‖Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖ ≤
      (‖s‖ + (r + 1)) ^ 2 / marshalRiemannZeroHeight n ^ 2 := by
  have hw' : w ∈ ball s (r + 1) := by
    simp [mem_ball, mem_closedBall, dist_eq_norm] at hw ⊢
    linarith [hw]
  have hz := marshal_ratio_small_on_ball s (by linarith : 0 < r + 1) w hw' n hn
  have hlog := weierstrass_log_norm_le_sq hz
  have hγpos := marshalRiemannZeroHeight_pos n
  have hwnorm := marshal_norm_le_on_closedBall s hr hw
  have hwnorm' : ‖w‖ ≤ ‖s‖ + (r + 1) := le_trans hwnorm (by linarith [hr])
  have hdiv : ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖ ^ 2 ≤
      (‖s‖ + (r + 1)) ^ 2 / marshalRiemannZeroHeight n ^ 2 := by
    have hHeightLe : marshalRiemannZeroHeight n ≤
        ‖criticalLineParam (marshalRiemannZeroHeight n)‖ := by
      simpa [abs_of_pos hγpos] using criticalLineParam_norm_ge_abs_im (marshalRiemannZeroHeight n)
    have hle' : ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖ ≤
        ‖w‖ / marshalRiemannZeroHeight n := by
      rw [norm_div]
      exact div_le_div_of_nonneg_left (norm_nonneg w) hγpos hHeightLe
    calc ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖ ^ 2
        ≤ (‖w‖ / marshalRiemannZeroHeight n) ^ 2 := by gcongr
      _ ≤ ((‖s‖ + (r + 1)) / marshalRiemannZeroHeight n) ^ 2 := by gcongr
      _ = (‖s‖ + (r + 1)) ^ 2 / marshalRiemannZeroHeight n ^ 2 := by
        field_simp [pow_two, hγpos.ne']
  exact le_trans hlog hdiv

lemma marshal_ratio_norm_le_on_closedBall (s : ℂ) {r : ℝ} (hr : 0 < r) {w : ℂ} (hw : w ∈ closedBall s r)
    (n : ℕ) :
    ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖ ≤
      (‖s‖ + (r + 1)) / marshalRiemannZeroHeight n := by
  have hγpos := marshalRiemannZeroHeight_pos n
  have hwnorm' : ‖w‖ ≤ ‖s‖ + (r + 1) := by
    have hwnorm := marshal_norm_le_on_closedBall s hr hw
    linarith [hr]
  have hHeightLe : marshalRiemannZeroHeight n ≤
      ‖criticalLineParam (marshalRiemannZeroHeight n)‖ := by
    simpa [abs_of_pos hγpos] using criticalLineParam_norm_ge_abs_im (marshalRiemannZeroHeight n)
  calc
    ‖w / criticalLineParam (marshalRiemannZeroHeight n)‖
        ≤ ‖w‖ / marshalRiemannZeroHeight n := by
          rw [norm_div]
          exact div_le_div_of_nonneg_left (norm_nonneg w) hγpos hHeightLe
    _ ≤ (‖s‖ + (r + 1)) / marshalRiemannZeroHeight n := by gcongr

private lemma weierstrassFactor1_ne_zero {z : ℂ} (hz : z ≠ 1) :
    weierstrassFactor1 z ≠ 0 := by
  intro h0
  dsimp [weierstrassFactor1] at h0
  rcases mul_eq_zero.mp h0 with h1z | hexp
  · exact hz (eq_comm.mp (sub_eq_zero.mp h1z))
  · exact Complex.exp_ne_zero z hexp

/-- C++ `head_envelope(R)` audit majorant when the ratio is small. -/
lemma weierstrass_log_norm_le_head_envelope_small {z : ℂ} (hz1 : z ≠ 1) {R : ℝ} (hR : 1 ≤ R)
    (hz : ‖z‖ ≤ R) (hhalf : ‖z‖ ≤ (1 / 2 : ℝ)) :
    ‖Complex.log (weierstrassFactor1 z)‖ ≤
      Real.log (1 + R) + R + 2 * Real.pi + 2 * R :=
  le_trans (weierstrass_log_norm_le_sq hhalf) (by
    have hsz : ‖z‖ ^ 2 ≤ (1 / 2 : ℝ) ^ 2 := by gcongr
    nlinarith [hsz, Real.pi_pos, Real.two_pi_pos, Real.pi_le_four, hR,
      Real.log_nonneg (by linarith [hR] : (1 : ℝ) ≤ 1 + R)])

lemma weierstrass_factor_abs_le (z : ℂ) :
    Complex.abs (weierstrassFactor1 z) ≤ (1 + ‖z‖) * Real.exp ‖z‖ := by
  simpa [Complex.norm_eq_abs] using weierstrass_factor_norm_le z

lemma wedge_two_re_exp_le_one {t : ℝ} (ht : (1 : ℝ) < t) :
    2 * t * Real.exp (-(t + 1)) ≤ 1 := by
  have htpos : 0 < t := lt_trans (by norm_num) ht
  have hte : t * Real.exp (-t) ≤ Real.exp (-1) := by
    have h1 : t ≤ Real.exp (t - 1) := by
      have h := Real.add_one_le_exp (t - 1)
      linarith
    have hm := mul_le_mul_of_nonneg_right h1 (le_of_lt (Real.exp_pos (-t)))
    have hmul : Real.exp (t - 1) * Real.exp (-t) = Real.exp (-1) := by
      rw [← Real.exp_add]
      ring_nf
    simpa [hmul, mul_assoc, mul_left_comm, mul_comm] using hm
  have h2e : (2 : ℝ) ≤ Real.exp 2 := by
    have h1 : (2 : ℝ) ≤ Real.exp 1 := by
      have h := Real.add_one_le_exp (1 : ℝ)
      linarith
    have hmono : Real.exp 1 ≤ Real.exp 2 := Real.exp_le_exp_of_le (by norm_num)
    linarith
  have hcap : 2 * Real.exp (-2) ≤ 1 := by
    have hdiv : (1 : ℝ) / Real.exp 2 ≤ (1 / 2 : ℝ) := one_div_le_one_div_of_le (by positivity) h2e
    have hneg2 : Real.exp (-2) = 1 / Real.exp 2 := by simpa [one_div] using (Real.exp_neg 2)
    rw [hneg2]
    linarith
  have hsplit : 2 * t * Real.exp (-(t + 1)) = 2 * (t * Real.exp (-t)) * Real.exp (-1) := by
    have hsum : -(t + 1) = (-t) + (-1) := by ring
    rw [hsum, Real.exp_add]
    ring
  rw [hsplit]
  have hm : t * Real.exp (-t) * Real.exp (-1) ≤ Real.exp (-1) * Real.exp (-1) := by
    have hm' := mul_le_mul_of_nonneg_right hte (le_of_lt (Real.exp_pos (-1)))
    simpa [mul_assoc, mul_left_comm, mul_comm] using hm'
  have hm2 : 2 * (t * Real.exp (-t)) * Real.exp (-1) ≤ 2 * (Real.exp (-1) * Real.exp (-1)) := by
    have hm2' := mul_le_mul_of_nonneg_right hm (by positivity : (0 : ℝ) ≤ 2)
    simpa [mul_assoc, mul_left_comm, mul_comm] using hm2'
  have hsq : Real.exp (-1) * Real.exp (-1) = Real.exp (-2) := by
    rw [← Real.exp_add]
    ring_nf
  calc
    2 * (t * Real.exp (-t)) * Real.exp (-1)
        ≤ 2 * (Real.exp (-1) * Real.exp (-1)) := hm2
    _ = 2 * Real.exp (-2) := by rw [hsq]
    _ ≤ 1 := hcap

lemma criticalLineParam_norm_ge_half (γ : ℝ) : (1 / 2 : ℝ) ≤ ‖criticalLineParam γ‖ := by
  have hnorm : ‖criticalLineParam γ‖ = Real.sqrt ((1 / 2 : ℝ) ^ 2 + γ ^ 2) := by
    dsimp [criticalLineParam]
    simp [Complex.norm_eq_abs, Complex.abs, Complex.normSq, Complex.sq_abs,
      Complex.mul_conj, Complex.conj_ofReal]
    ring_nf
  rw [hnorm]
  have hsq : Real.sqrt ((1 / 2 : ℝ) ^ 2) ≤ Real.sqrt ((1 / 2 : ℝ) ^ 2 + γ ^ 2) :=
    Real.sqrt_le_sqrt (by nlinarith)
  simpa using hsq

lemma weierstrass_factor_abs_ge_dist (z : ℂ) :
    Real.exp (-‖z‖) * Complex.abs (1 - z) ≤ Complex.abs (weierstrassFactor1 z) := by
  have hexp : Real.exp (-‖z‖) ≤ Complex.abs (Complex.exp z) := by
    rw [Complex.abs_exp]
    have hre : -‖z‖ ≤ z.re := by
      have h := Complex.abs_re_le_abs z
      rw [← Complex.norm_eq_abs] at h
      linarith [(abs_le.mp h).1, (abs_le.mp h).2]
    exact Real.exp_le_exp_of_le hre
  dsimp [weierstrassFactor1]
  calc
    Real.exp (-‖z‖) * Complex.abs (1 - z) ≤ Complex.abs (Complex.exp z) * Complex.abs (1 - z) := by
      gcongr
    _ = Complex.abs ((1 - z) * Complex.exp z) := by rw [← Complex.abs.map_mul, mul_comm]
    _ = Complex.abs (weierstrassFactor1 z) := rfl

private lemma one_add_norm_le_exp_norm {t : ℝ} (ht : 0 ≤ t) : (1 + t) ≤ Real.exp t := by
  simpa [add_comm] using Real.add_one_le_exp t

private lemma one_sub_norm_le_abs_one_sub (z : ℂ) :
    (1 - ‖z‖) ≤ Complex.abs (1 - z) := by
  have h := norm_add_le (1 - z) z
  rw [sub_add_cancel, norm_one] at h
  have : (1 : ℝ) ≤ Complex.abs (1 - z) + ‖z‖ := by simpa [Complex.norm_eq_abs] using h
  linarith

private lemma norm_sub_one_le_abs_one_sub (z : ℂ) :
    (‖z‖ - 1) ≤ Complex.abs (1 - z) := by
  have h' : ‖z‖ ≤ Complex.abs (1 - z) + 1 := by
    have h := norm_add_le (z - 1) (1 : ℂ)
    rw [sub_add_cancel, norm_one, norm_sub_rev] at h
    simpa [Complex.norm_eq_abs] using h
  linarith

private lemma Real_abs_log_le_max_log_bounds {a δ U : ℝ} (hδ : 0 < δ) (hδa : δ ≤ a) (haU : a ≤ U) :
    |Real.log a| ≤ max (Real.log U) (-Real.log δ) := by
  have hUpos : 0 < U := lt_of_lt_of_le hδ (le_trans hδa haU)
  rcases le_total 0 (Real.log a) with hnonneg | hneg
  · have ha : 0 < a := lt_of_lt_of_le hδ hδa
    have hle : Real.log a ≤ Real.log U := Real.log_le_log ha haU
    rw [_root_.abs_of_nonneg hnonneg]
    exact le_max_of_le_left hle
  · have hle : -Real.log a ≤ -Real.log δ := by linarith [Real.log_le_log hδ hδa]
    rw [_root_.abs_of_nonpos hneg]
    exact le_max_of_le_right hle

private lemma max_log_bounds_le_log_sub {δ U : ℝ} (hδ : 0 < δ) (hδle : δ ≤ (1 : ℝ)) (hU : 0 < U)
    (hU1 : (1 : ℝ) ≤ U) :
    max (Real.log U) (-Real.log δ) ≤ Real.log U - Real.log δ := by
  have hlogδ : Real.log δ ≤ 0 := Real.log_nonpos (le_of_lt hδ) hδle
  have hlogU : 0 ≤ Real.log U := Real.log_nonneg hU1
  rcases le_total (Real.log U) (-Real.log δ) with h | h
  · rw [max_eq_right h]
    linarith
  · rw [max_eq_left h]
    linarith

lemma Real_abs_log_le_log_div {a δ U : ℝ} (hδ : 0 < δ) (hδle : δ ≤ (1 : ℝ)) (hU1 : (1 : ℝ) ≤ U)
    (hδa : δ ≤ a) (haU : a ≤ U) :
    |Real.log a| ≤ Real.log U - Real.log δ := by
  have hUpos : 0 < U := lt_of_lt_of_le hδ (le_trans hδa haU)
  exact le_trans (Real_abs_log_le_max_log_bounds hδ hδa haU)
    (max_log_bounds_le_log_sub hδ hδle hUpos hU1)

lemma criticalLineParam_norm_pos (γ : ℝ) : 0 < ‖criticalLineParam γ‖ := by
  have hnorm : ‖criticalLineParam γ‖ = Real.sqrt ((1 / 2 : ℝ) ^ 2 + γ ^ 2) := by
    dsimp [criticalLineParam]
    simp [Complex.norm_eq_abs, Complex.abs, Complex.normSq, Complex.sq_abs,
      Complex.mul_conj, Complex.conj_ofReal]
    ring_nf
  rw [hnorm]
  positivity

lemma abs_one_sub_ratio_z_eq {w : ℂ} {γ : ℝ} :
    Complex.abs (1 - w / criticalLineParam γ) =
      ‖w - criticalLineParam γ‖ / ‖criticalLineParam γ‖ := by
  have hγ := criticalLineParam_norm_pos γ
  have hne : criticalLineParam γ ≠ 0 := (norm_pos_iff).1 hγ
  rw [← Complex.norm_eq_abs]
  have hdiv : (1 - w / criticalLineParam γ) = (criticalLineParam γ - w) / criticalLineParam γ := by
    field_simp [hne]
  rw [hdiv, norm_div, norm_sub_rev]

lemma norm_sub_critical_line_ge_re_gap {γ : ℝ} {w : ℂ} (hw : (1 / 2 : ℝ) < w.re) :
    (w.re - (1 / 2 : ℝ)) ≤ ‖w - criticalLineParam γ‖ := by
  have h := Complex.abs_re_le_abs (w - criticalLineParam γ)
  have hcp : (criticalLineParam γ).re = (1 / 2 : ℝ) := by dsimp [criticalLineParam]; simp
  simp only [Complex.sub_re, hcp] at h
  have h2 : w.re - (1 / 2 : ℝ) ≤ Complex.abs (w - criticalLineParam γ) := (abs_le.mp h).2
  simpa [Complex.norm_eq_abs] using h2

lemma marshal_wedge_closedBall_re_gt_half {s : ℂ} (hs : s ∈ marshalWedgeIdentityDomain)
    {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) {w : ℂ} (hw : w ∈ closedBall s r) :
    (1 / 2 : ℝ) < w.re := by
  simp only [marshalWedgeIdentityDomain, Set.mem_setOf_eq] at hs
  simp only [mem_closedBall, dist_eq_norm, Complex.norm_eq_abs] at hw
  have hre := abs_le.mp (Complex.abs_re_le_abs (w - s))
  simp only [Complex.sub_re] at hre
  linarith [hre.1, hw, hr.2, hs]

lemma weierstrass_log_norm_le_from_factor_abs_bounds {z : ℂ} (hz1 : z ≠ 1)
    {δ U : ℝ} (hδ : 0 < δ) (hδle : δ ≤ (1 : ℝ)) (hU1 : (1 : ℝ) ≤ U) (hδU : δ ≤ U)
    (hlow : δ ≤ Complex.abs (weierstrassFactor1 z))
    (hhigh : Complex.abs (weierstrassFactor1 z) ≤ U) :
    ‖Complex.log (weierstrassFactor1 z)‖ ≤ Real.log U - Real.log δ + Real.pi := by
  set w := weierstrassFactor1 z
  have hlog := complex_log_norm_le_log_abs_add_pi w
  have habslog := Real_abs_log_le_log_div hδ hδle hU1 hlow hhigh
  calc ‖Complex.log w‖
      ≤ |Real.log (Complex.abs w)| + Real.pi := hlog
    _ ≤ Real.log U - Real.log δ + Real.pi := by linarith

noncomputable def pinnedGenusOneLogMaxBallRadius : ℝ :=
  pinnedAnaVmGenusOneLogMaxBallRadius

lemma weierstrass_log_norm_le_head_envelope_ratio {z : ℂ} (hz1 : z ≠ 1)
    {R γ ρ : ℝ} (hR : 1 ≤ R) (_hγ : 1 ≤ γ) (_hρpos : 0 < ρ) (hρR : ρ ≤ R)
    (hρhalf : ρ ≤ (1 / 2 : ℝ)) (hz : ‖z‖ ≤ ρ) :
    ‖Complex.log (weierstrassFactor1 z)‖ ≤
      Real.log (1 + R) + R + 2 * Real.pi + 2 * R := by
  exact weierstrass_log_norm_le_head_envelope_small hz1 hR (le_trans hz hρR) (le_trans hz hρhalf)

lemma weierstrass_log_norm_le_head_envelope {z : ℂ} (hz1 : z ≠ 1)
    {R : ℝ} (hR : 1 ≤ R) (hz : ‖z‖ ≤ R) (hhalf : ‖z‖ ≤ (1 / 2 : ℝ)) :
    ‖Complex.log (weierstrassFactor1 z)‖ ≤
      Real.log (1 + R) + R + 2 * Real.pi + 2 * R :=
  weierstrass_log_norm_le_head_envelope_small hz1 hR hz hhalf

theorem marshal_genus_one_log_summability_proved : MarshalGenusOneLogSummability := by
  intro s hoff
  by_cases hs : s = 0
  · subst hs
    have hsum : Summable fun _ : ℕ => (0 : ℂ) := summable_zero
    convert hsum using 1
    funext n
    simp [spectralDetFactor, spectralXiFactor, weierstrassFactor1, Complex.log_one]
  · have hs' : 0 < ‖s‖ := norm_pos_iff.mpr hs
    obtain ⟨N, hN⟩ := marshal_ratio_small_eventually s hs'
    let factor (n : ℕ) : ℂ :=
      spectralDetFactor marshalDiscreteSpectrum s n
    let tailMajor (n : ℕ) : ℝ := ‖s‖ ^ 2 / marshalRiemannZeroHeight n ^ 2
    have htailMajor : Summable tailMajor := by
      dsimp [tailMajor]
      simpa [div_eq_mul_inv] using (Summable.mul_left (‖s‖ ^ 2) marshal_inv_gamma_sq_summable)
    have htailSummable :
        Summable fun n => if N ≤ n then Complex.log (factor n) else 0 := by
      have htailMajor' :
          Summable fun n => if N ≤ n then tailMajor n else 0 := by
        refine Summable.of_nonneg_of_le (fun _ => by split_ifs <;> positivity) (fun n => ?_) htailMajor
        split_ifs with hNle
        · exact le_rfl
        · positivity
      refine Summable.of_norm_bounded
        (fun n => if N ≤ n then tailMajor n else 0) htailMajor' ?_
      intro n
      split_ifs with hNle
      · dsimp [factor, tailMajor]
        dsimp [spectralDetFactor, spectralXiFactor, weierstrassFactor1, marshalDiscreteSpectrum]
        have hz := hN n hNle
        have hlog := weierstrass_log_norm_le_sq hz
        have hle := ratio_norm_le_s_norm_div_height (s := s) (γ := marshalRiemannZeroHeight n)
          (abs_pos.mpr (marshalRiemannZeroHeight_pos n).ne')
        have hdiv : ‖s / criticalLineParam (marshalRiemannZeroHeight n)‖ ^ 2 ≤ tailMajor n := by
          dsimp only [tailMajor]
          have hden := marshalRiemannZeroHeight_pos n
          have hHeightLe : marshalRiemannZeroHeight n ≤
              ‖criticalLineParam (marshalRiemannZeroHeight n)‖ := by
            simpa [abs_of_pos hden] using criticalLineParam_norm_ge_abs_im (marshalRiemannZeroHeight n)
          have hle' : ‖s / criticalLineParam (marshalRiemannZeroHeight n)‖ ≤
              ‖s‖ / marshalRiemannZeroHeight n := by
            rw [norm_div]
            exact div_le_div_of_nonneg_left (norm_nonneg s) hden hHeightLe
          calc ‖s / criticalLineParam (marshalRiemannZeroHeight n)‖ ^ 2
              ≤ (‖s‖ / marshalRiemannZeroHeight n) ^ 2 := by gcongr
            _ = ‖s‖ ^ 2 / marshalRiemannZeroHeight n ^ 2 := by
              field_simp [pow_two, hden.ne']
            _ = tailMajor n := rfl
        simpa [if_pos hNle] using le_trans hlog hdiv
      · simp [hNle]
    have hheadSummable :
        Summable fun n => if N ≤ n then 0 else Complex.log (factor n) := by
      refine summable_of_ne_finset_zero (s := Finset.range N) ?_
      intro n hn
      simp [Finset.mem_range, not_lt] at hn
      simp [hn]
    refine (hheadSummable.add htailSummable).congr fun n => ?_
    dsimp [factor]
    split_ifs with h <;> simp [add_comm]


end HPAnalysis

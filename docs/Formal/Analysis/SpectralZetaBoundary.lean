import Analysis.SmoothnessA1
import Analysis.HurwitzDeriv
import Mathlib.Analysis.SpecialFunctions.Pow.Asymptotics
import Mathlib.Topology.Algebra.Order.Field
import Mathlib.Topology.Order.LeftRightNhds
import Mathlib.Topology.ContinuousOn
import Mathlib.Topology.Order.OrderClosed
import Mathlib.Topology.Algebra.InfiniteSum.Order
import Mathlib.Topology.Algebra.InfiniteSum.Basic
import Mathlib.Analysis.SpecialFunctions.Pow.Continuity

/-!
# Boundary coercivity of the pure-scaling spectral zeta

For `s > 1`, `spectralZeta` diverges to `+∞` as `θ ↘ 0` and `θ ↗ 2π` within `(0, 2π)`.
Endpoint continuity on `Icc` fails (Hurwitz series pole at shift `a = 0`); the interior
minimum argument on `Ioo` is the correct closure route.
-/

set_option maxHeartbeats 800000

namespace HPAnalysis

open scoped Topology
open Set Real Filter HurwitzZeta

private lemma one_lt_s_pos {s : ℝ} (hs : 1 < s) : 0 < s := lt_trans zero_lt_one hs

private lemma scaling_factor_pos {s logRatio : ℝ} (_hs : 1 < s) (hlog : logRatio ≠ 0) :
    0 < |logRatio| ^ s * twoPi ^ (-s) :=
  mul_pos (Real.rpow_pos_of_pos (abs_pos.mpr hlog) _) (Real.rpow_pos_of_pos twoPi_pos _)

private lemma twoPi_ne_zero : (twoPi : ℝ) ≠ 0 := twoPi_pos.ne'

lemma hurwitzSeriesTerm_zero_tendsto_atTop_at_zero {s : ℝ} (hs : 1 < s) :
    Tendsto (fun a : ℝ => hurwitzSeriesTerm s 0 a) (𝓝[>] 0) atTop := by
  have hs_pos : 0 < s := lt_trans zero_lt_one hs
  have hrecip : Tendsto (fun a : ℝ => a⁻¹) (𝓝[>] 0) atTop := tendsto_inv_zero_atTop
  refine ((tendsto_rpow_atTop hs_pos).comp hrecip).congr' ?_
  filter_upwards [self_mem_nhdsWithin] with a ha
  simp [hurwitzSeriesTerm, Function.comp_apply, zero_add, Nat.cast_zero,
    inv_rpow (le_of_lt ha), rpow_neg (le_of_lt ha), inv_inv]

lemma hurwitzZetaReal_ge_term_zero {a s : ℝ} (ha : a ∈ Ioo 0 1) (hs : 1 < s) :
    hurwitzSeriesTerm s 0 a ≤ hurwitzZetaReal a s := by
  have haIcc : a ∈ Icc 0 1 := ⟨le_of_lt ha.1, le_of_lt ha.2⟩
  rw [hurwitzZetaReal_eq_hurwitzSeries haIcc hs, hurwitzSeries]
  have hnn : ∀ n, 0 ≤ hurwitzSeriesTerm s n a := fun n =>
    Real.rpow_nonneg (add_nonneg (Nat.cast_nonneg n) (le_of_lt ha.1)) _
  exact le_tsum (hurwitzSeries_summable (le_of_lt ha.1) hs) 0 (fun j _ => hnn j)

private lemma theta_div_twoPi_tendsto_nhdsLT_one :
    Tendsto (fun θ : ℝ => θ / twoPi) (𝓝[<] twoPi) (𝓝[<] 1) := by
  have hid : Tendsto (fun θ : ℝ => θ) (𝓝[<] twoPi) (𝓝 twoPi) := continuousWithinAt_id.tendsto
  have hdiv : Tendsto (fun θ : ℝ => θ / twoPi) (𝓝[<] twoPi) (𝓝 1) := by
    simpa [div_self, twoPi_ne_zero] using hid.div tendsto_const_nhds twoPi_ne_zero
  refine tendsto_nhdsWithin_of_tendsto_nhds_of_eventually_within _ hdiv ?_
  filter_upwards [self_mem_nhdsWithin] with θ hθ
  exact (div_lt_one twoPi_pos).mpr hθ

private lemma one_sub_theta_div_twoPi_tendsto_nhdsGT_zero :
    Tendsto (fun θ : ℝ => 1 - θ / twoPi) (𝓝[<] twoPi) (𝓝[>] 0) := by
  have hdiv : Tendsto (fun θ : ℝ => θ / twoPi) (𝓝[<] twoPi) (𝓝 1) :=
    theta_div_twoPi_tendsto_nhdsLT_one.mono_right inf_le_left
  have hone : Tendsto (fun _ : ℝ => (1 : ℝ)) (𝓝[<] twoPi) (𝓝 1) := tendsto_const_nhds
  have hsub : Tendsto (fun θ => 1 - θ / twoPi) (𝓝[<] twoPi) (𝓝 0) := by
    simpa [sub_self] using hone.sub hdiv
  refine tendsto_nhdsWithin_of_tendsto_nhds_of_eventually_within _ hsub ?_
  filter_upwards [self_mem_nhdsWithin] with θ hθ
  simpa [sub_pos, div_lt_one twoPi_pos] using hθ

private lemma theta_div_twoPi_tendsto_nhdsGT_zero :
    Tendsto (fun θ : ℝ => θ / twoPi) (𝓝[>] 0) (𝓝[>] 0) := by
  have hid : Tendsto (fun θ : ℝ => θ) (𝓝[>] 0) (𝓝 0) := continuousWithinAt_id.tendsto
  have hdiv : Tendsto (fun θ : ℝ => θ / twoPi) (𝓝[>] 0) (𝓝 0) := by
    simpa using hid.div tendsto_const_nhds twoPi_ne_zero
  refine tendsto_nhdsWithin_of_tendsto_nhds_of_eventually_within _ hdiv ?_
  filter_upwards [self_mem_nhdsWithin] with θ hθ
  exact div_pos hθ twoPi_pos

lemma hurwitzZetaReal_tendsto_atTop_at_zero {s : ℝ} (hs : 1 < s) :
    Tendsto (fun a : ℝ => hurwitzZetaReal a s) (𝓝[>] 0) atTop := by
  have hmono : ∀ᶠ a in 𝓝[>] 0, hurwitzSeriesTerm s 0 a ≤ hurwitzZetaReal a s := by
    filter_upwards [Ioo_mem_nhdsWithin_Ioi' (by norm_num : (0 : ℝ) < 1)] with a ha
    exact hurwitzZetaReal_ge_term_zero ha hs
  exact tendsto_atTop_mono' (𝓝[>] 0) hmono (hurwitzSeriesTerm_zero_tendsto_atTop_at_zero hs)

private lemma spectralZeta_ge_first_term {s logRatio θ : ℝ} (hs : 1 < s) (hlog : logRatio ≠ 0)
    (hθ : θ ∈ Ioo 0 twoPi) :
    |logRatio| ^ s * twoPi ^ (-s) * hurwitzZetaReal (θ / twoPi) s ≤
      spectralZeta s θ logRatio := by
  rcases hθ with ⟨hθ0, hθ1⟩
  have hθIcc : θ ∈ Icc 0 twoPi := ⟨le_of_lt hθ0, le_of_lt hθ1⟩
  have hscale := scaling_factor_pos hs hlog
  have h1 := (hurwitzZetaReal_pos (one_sub_theta_shift_mem_Icc θ hθIcc) hs).le
  rw [spectralZeta, hurwitzShift]
  exact mul_le_mul_of_nonneg_left (le_add_of_nonneg_right h1) (le_of_lt hscale)

private lemma spectralZeta_ge_second_term {s logRatio θ : ℝ} (hs : 1 < s) (hlog : logRatio ≠ 0)
    (hθ : θ ∈ Ioo 0 twoPi) :
    |logRatio| ^ s * twoPi ^ (-s) * hurwitzZetaReal (1 - θ / twoPi) s ≤
      spectralZeta s θ logRatio := by
  rcases hθ with ⟨hθ0, hθ1⟩
  have hθIcc : θ ∈ Icc 0 twoPi := ⟨le_of_lt hθ0, le_of_lt hθ1⟩
  have hscale := scaling_factor_pos hs hlog
  have h0 := (hurwitzZetaReal_pos (theta_shift_mem_Icc θ hθIcc) hs).le
  rw [spectralZeta, hurwitzShift]
  exact mul_le_mul_of_nonneg_left (le_add_of_nonneg_left h0) (le_of_lt hscale)

lemma spectralZeta_tendsto_atTop_at_zero {s logRatio : ℝ} (hs : 1 < s) (hlog : logRatio ≠ 0) :
    Tendsto (fun θ => spectralZeta s θ logRatio) (𝓝[>] 0) atTop := by
  have hzeta := hurwitzZetaReal_tendsto_atTop_at_zero hs
  have hshift : Tendsto (fun θ => hurwitzZetaReal (θ / twoPi) s) (𝓝[>] 0) atTop :=
    hzeta.comp theta_div_twoPi_tendsto_nhdsGT_zero
  have hC := hshift.const_mul_atTop (scaling_factor_pos hs hlog)
  have hEv : ∀ᶠ θ in 𝓝[>] 0, θ ∈ Ioo 0 twoPi :=
    Ioo_mem_nhdsWithin_Ioi' (by linarith [twoPi_pos])
  have hmono : ∀ᶠ θ in 𝓝[>] 0, |logRatio| ^ s * twoPi ^ (-s) *
      hurwitzZetaReal (θ / twoPi) s ≤ spectralZeta s θ logRatio :=
    hEv.mono fun θ hθ => spectralZeta_ge_first_term hs hlog hθ
  exact tendsto_atTop_mono' (𝓝[>] 0) hmono hC

lemma spectralZeta_tendsto_atTop_at_twoPi {s logRatio : ℝ} (hs : 1 < s) (hlog : logRatio ≠ 0) :
    Tendsto (fun θ => spectralZeta s θ logRatio) (𝓝[<] twoPi) atTop := by
  have hzeta := hurwitzZetaReal_tendsto_atTop_at_zero hs
  have hshift : Tendsto (fun θ => hurwitzZetaReal (1 - θ / twoPi) s) (𝓝[<] twoPi) atTop :=
    hzeta.comp one_sub_theta_div_twoPi_tendsto_nhdsGT_zero
  have hC := hshift.const_mul_atTop (scaling_factor_pos hs hlog)
  have hEv : ∀ᶠ θ in 𝓝[<] twoPi, θ ∈ Ioo 0 twoPi :=
    Ioo_mem_nhdsWithin_Iio' (by linarith [twoPi_pos])
  have hmono : ∀ᶠ θ in 𝓝[<] twoPi, |logRatio| ^ s * twoPi ^ (-s) *
      hurwitzZetaReal (1 - θ / twoPi) s ≤ spectralZeta s θ logRatio :=
    hEv.mono fun θ hθ => spectralZeta_ge_second_term hs hlog hθ
  exact tendsto_atTop_mono' (𝓝[<] twoPi) hmono hC

end HPAnalysis

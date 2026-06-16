import Mathlib.Analysis.Convex.Function
import Mathlib.Topology.Order.Compact
import Mathlib.Topology.Order.LeftRightNhds
import Mathlib.Topology.Instances.Real
import Mathlib.Order.Filter.Extr

/-!
# A4 — unique minimizer on a compact or coercive interval
-/

namespace HPAnalysis

open scoped Topology
open Set Filter

private lemma four_pos : (0 : ℝ) < 4 := by norm_num

/-- **A4 (closed interval).** Strict convexity + continuity on `Icc a b` ⇒ unique minimizer. -/
theorem unique_minimum_of_strictConvexOn_Icc {f : ℝ → ℝ} {a b : ℝ} (hab : a < b)
    (hf : ContinuousOn f (Icc a b))
    (hconv : StrictConvexOn ℝ (Icc a b) f) :
    ∃! xMin, xMin ∈ Icc a b ∧ IsMinOn f (Icc a b) xMin := by
  obtain ⟨xMin, hxMin, hmin⟩ :=
    IsCompact.exists_isMinOn (s := Icc a b) isCompact_Icc (nonempty_Icc.mpr (le_of_lt hab)) hf
  refine ⟨xMin, ⟨hxMin, hmin⟩, ?_⟩
  intro y hy
  rcases hy with ⟨hyIcc, hyMin⟩
  exact hconv.eq_of_isMinOn hyMin hmin hyIcc hxMin

/-- Existence of a minimizer on `(a, b)` when `f` is continuous and coercive at both ends. -/
theorem exists_minimum_of_coercive_Ioo {f : ℝ → ℝ} {a b : ℝ} (hab : a < b)
    (hf : ContinuousOn f (Ioo a b))
    (hfa : Tendsto f (𝓝[>] a) atTop)
    (hfb : Tendsto f (𝓝[<] b) atTop) :
    ∃ xMin, xMin ∈ Ioo a b ∧ IsMinOn f (Ioo a b) xMin := by
  set c := (a + b) / 2
  have hc : c ∈ Ioo a b := by
    dsimp [c]
    constructor <;> linarith [hab]
  have hfa_ev : {x | f c + 1 ≤ f x} ∈ 𝓝[>] a :=
    hfa.eventually_ge_atTop (f c + 1)
  obtain ⟨ua, hua, hsa⟩ := (mem_nhdsWithin_Ioi_iff_exists_Ioo_subset).1 hfa_ev
  have hfb_ev : {x | f c + 1 ≤ f x} ∈ 𝓝[<] b :=
    hfb.eventually_ge_atTop (f c + 1)
  obtain ⟨lb, hlb, hsb⟩ := (mem_nhdsWithin_Iio_iff_exists_Ioo_subset).1 hfb_ev
  set ε := min (ua - a) (min (b - lb) ((b - a) / 4))
  have hεa : 0 < ua - a := sub_pos.mpr hua
  have hεb : 0 < b - lb := sub_pos.mpr hlb
  have hεquarter : 0 < (b - a) / 4 := div_pos (sub_pos.mpr hab) four_pos
  have hε : 0 < ε := lt_min hεa (lt_min hεb hεquarter)
  set K := Icc (a + ε) (b - ε)
  have hK_sub : K ⊆ Ioo a b := by
    intro x hx
    exact ⟨lt_of_lt_of_le (lt_add_of_pos_right a hε) hx.1,
      lt_of_le_of_lt hx.2 (sub_lt_self b hε)⟩
  have hε_le_quarter : ε ≤ (b - a) / 4 := by
    dsimp [ε]
    exact le_trans (min_le_right _ _) (min_le_right _ _)
  have hε_le_half : ε ≤ (b - a) / 2 := by linarith [hε_le_quarter, hab]
  have hcK : c ∈ K := by
    dsimp [K, c]
    constructor <;> linarith [hε_le_half, hab]
  obtain ⟨xMin, hxMin, hminK⟩ :=
    isCompact_Icc.exists_isMinOn (K.nonempty_of_mem hcK) (hf.mono hK_sub)
  refine ⟨xMin, hK_sub hxMin, ?_⟩
  rw [isMinOn_iff]
  intro y hy
  rcases hy with ⟨hya, hyb⟩
  by_cases hyK : y ∈ K
  · exact hminK hyK
  · rcases em (y < a + ε) with hy_low | hle_low
    · have hε_le_ua : ε ≤ ua - a := by dsimp [ε]; exact min_le_left _ _
      have hbound : a + ε ≤ ua := by linarith [hε_le_ua]
      have hy_left : y ∈ Ioo a ua := ⟨hya, hy_low.trans_le hbound⟩
      have hfc : f c + 1 ≤ f y := hsa hy_left
      exact le_trans (hminK hcK) (le_trans (le_of_lt (lt_add_one (f c))) hfc)
    · have hy_high : b - ε < y := by
        by_contra hle
        push_neg at hle
        dsimp [K] at hyK ⊢
        exact hyK ⟨not_lt.mp hle_low, hle⟩
      have hε_le_lb : ε ≤ b - lb := by
        dsimp [ε]
        exact le_trans (min_le_right _ _) (min_le_left _ _)
      have hlb_le : lb ≤ b - ε := by linarith [hε_le_lb]
      have hy_right : y ∈ Ioo lb b := ⟨lt_of_le_of_lt hlb_le hy_high, hyb⟩
      have hfc : f c + 1 ≤ f y := hsb hy_right
      exact le_trans (hminK hcK) (le_trans (le_of_lt (lt_add_one (f c))) hfc)

/-- **A4 (open interval, coercive).** Strict convexity on `(a, b)` + boundary blow-up ⇒ unique minimizer. -/
theorem unique_minimum_of_strictConvexOn_Ioo {f : ℝ → ℝ} {a b : ℝ} (hab : a < b)
    (hf : ContinuousOn f (Ioo a b))
    (hconv : StrictConvexOn ℝ (Ioo a b) f)
    (hfa : Tendsto f (𝓝[>] a) atTop)
    (hfb : Tendsto f (𝓝[<] b) atTop) :
    ∃! xMin, xMin ∈ Ioo a b ∧ IsMinOn f (Ioo a b) xMin := by
  obtain ⟨xMin, hxMin, hmin⟩ := exists_minimum_of_coercive_Ioo hab hf hfa hfb
  refine ⟨xMin, ⟨hxMin, hmin⟩, ?_⟩
  intro y hy
  rcases hy with ⟨hyIoo, hyMin⟩
  exact hconv.eq_of_isMinOn hyMin hmin hyIoo hxMin

end HPAnalysis

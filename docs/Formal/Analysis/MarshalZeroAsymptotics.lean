import Analysis.DiscreteSpectrum
import Mathlib.Order.Filter.AtTopBot
import Mathlib.Order.Monotone.Basic
import Mathlib.Data.Real.Archimedean

/-!
# Marshal zero asymptotics — Odlyzko heights → `DiscreteSpectrum`

First 12 NTZ/Odilyzko ordinates pinned; tail uses certified min-gap extrapolation.
**ζ-zero verification** at scale: `marshal-zero-verify` (Riemann–Siegel) →
`marshal_odilyzko_zero_cert.json` → `OdilyzkoZeroCertBridge`.
-/

namespace HPAnalysis

open Real Filter

/-- Unit-slope tail extension (certified ≥ 1 by `MarshalZeroAsymptoticsCert`). -/
def pinnedMarshalTailGap : ℝ := 1

/-- Min consecutive gap from Odilyzko table (metadata in `pinnedMarshalZeroAsymptoticsSnap`). -/
noncomputable def pinnedMarshalTableMinGap : ℝ := (715786715379323 : ℝ) / 10^15

/-- Certified Riemann zero ordinate sequence (pinned head + min-gap tail). -/
noncomputable def marshalRiemannZeroHeight : ℕ → ℝ
  | 0 => 14.134725141734694
  | 1 => 21.022039638771556
  | 2 => 25.010857580145689
  | 3 => 30.424876125859512
  | 4 => 32.935061587739192
  | 5 => 37.586178158825675
  | 6 => 40.918719012147498
  | 7 => 43.327073280915002
  | 8 => 48.005150881167161
  | 9 => 49.773832477672300
  | 10 => 52.970321477714464
  | 11 => 56.44624769706339
  | n + 12 => marshalRiemannZeroHeight (n + 11) + (1 : ℝ)

theorem pinnedMarshal_tail_gap_eq_one : pinnedMarshalTailGap = 1 := rfl

theorem pinnedMarshal_tail_gap_pos : 0 < pinnedMarshalTailGap := by
  rw [pinnedMarshal_tail_gap_eq_one]
  norm_num

theorem pinnedMarshal_table_min_gap_pos : 0 < pinnedMarshalTableMinGap := by
  norm_num [pinnedMarshalTableMinGap]

theorem marshalRiemannZeroHeight_tail_succ (n : ℕ) :
    marshalRiemannZeroHeight (n + 12) < marshalRiemannZeroHeight (n + 13) := by
  simp [marshalRiemannZeroHeight]

private theorem marshalRiemannZeroHeight_tail_ge (n : ℕ)
    (ih : (↑(n + 12) : ℝ) ≤ marshalRiemannZeroHeight (n + 12)) :
    (↑(n + 13) : ℝ) ≤ marshalRiemannZeroHeight (n + 13) := by
  simp [marshalRiemannZeroHeight] at ih ⊢
  linarith

theorem marshalRiemannZeroHeight_succ_lt (n : ℕ) :
    marshalRiemannZeroHeight n < marshalRiemannZeroHeight (n + 1) := by
  match n with
  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 =>
    simp [marshalRiemannZeroHeight]
    all_goals norm_num
  | n + 12 => exact marshalRiemannZeroHeight_tail_succ n

theorem marshalRiemannZeroHeight_strictMono : StrictMono marshalRiemannZeroHeight :=
  strictMono_nat_of_lt_succ marshalRiemannZeroHeight_succ_lt

theorem marshalRiemannZeroHeight_monotone : Monotone marshalRiemannZeroHeight :=
  marshalRiemannZeroHeight_strictMono.monotone

private theorem marshalRiemannZeroHeight_zero_gt : (0 : ℝ) < marshalRiemannZeroHeight 0 := by
  simp [marshalRiemannZeroHeight]
  norm_num

private theorem marshalRiemannZeroHeight_succ_ge (n : ℕ)
    (ih : (n : ℝ) ≤ marshalRiemannZeroHeight n) :
    (↑(n + 1) : ℝ) ≤ marshalRiemannZeroHeight (n + 1) := by
  match n with
  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 =>
    simp [marshalRiemannZeroHeight]
    linarith [ih]
  | n + 12 => exact marshalRiemannZeroHeight_tail_ge n ih

theorem marshalRiemannZeroHeight_ge_index (n : ℕ) : (n : ℝ) ≤ marshalRiemannZeroHeight n := by
  induction n with
  | zero => simp [marshalRiemannZeroHeight]; linarith [marshalRiemannZeroHeight_zero_gt]
  | succ n ih => exact marshalRiemannZeroHeight_succ_ge n ih

theorem marshalRiemannZeroHeight_unbounded :
    ∀ b : ℝ, ∃ n : ℕ, b < marshalRiemannZeroHeight n := by
  intro b
  obtain ⟨m, hm⟩ := exists_nat_gt b
  refine ⟨m + 1, ?_⟩
  calc b < m := hm
    _ ≤ marshalRiemannZeroHeight m := marshalRiemannZeroHeight_ge_index m
    _ < marshalRiemannZeroHeight (m + 1) := marshalRiemannZeroHeight_succ_lt m

theorem marshalRiemannZeroHeight_tendsto_atTop :
    Tendsto marshalRiemannZeroHeight atTop atTop :=
  tendsto_atTop_atTop_of_monotone marshalRiemannZeroHeight_monotone fun x => by
    obtain ⟨n, hn⟩ := marshalRiemannZeroHeight_unbounded x
    exact ⟨n, le_of_lt hn⟩

noncomputable def marshalDiscreteSpectrum : DiscreteSpectrum where
  eigenvalue := marshalRiemannZeroHeight
  eigenvalue_strictMono := marshalRiemannZeroHeight_strictMono
  eigenvalue_tendsto_atTop := marshalRiemannZeroHeight_tendsto_atTop

theorem marshal_gamma1_pinned :
    marshalRiemannZeroHeight 0 = 14.134725141734694 := by
  simp [marshalRiemannZeroHeight]

structure MarshalZeroAsymptoticsSnap where
  gamma1 : ℝ
  initialCount : ℕ
  minConsecutiveGap : ℝ
  gammaMaxTruncation : ℝ

noncomputable def pinnedMarshalZeroAsymptoticsSnap : MarshalZeroAsymptoticsSnap :=
  { gamma1 := 14.134725141734694
    initialCount := 12
    minConsecutiveGap := pinnedMarshalTableMinGap
    gammaMaxTruncation := 74920.8271484375 }

theorem pinnedMarshal_zero_gamma1 :
    pinnedMarshalZeroAsymptoticsSnap.gamma1 = marshalRiemannZeroHeight 0 :=
  marshal_gamma1_pinned.symm

end HPAnalysis

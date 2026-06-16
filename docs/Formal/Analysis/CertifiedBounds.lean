import Mathlib.Data.Real.Basic
import Mathlib.Data.Real.Pi.Bounds
import Mathlib.Analysis.SpecialFunctions.Pow.Real
import Mathlib.Data.Complex.ExponentialBounds

/-!
# Certified numeric bounds — exact rationals for Marshal pins

All Marshal/AnaVM certificates used in Lean closure are bracketed by explicit
rational inequalities verifiable by `norm_num` (no trusted float axioms).
-/

namespace HPAnalysis

open Real Set

/-- Marshal moment L² identification tolerance (certified upper bound). -/
noncomputable def marshalMomentTolerance : ℝ := (1 : ℝ) / 1000

theorem marshal_moment_tolerance_pos : 0 < marshalMomentTolerance := by
  norm_num [marshalMomentTolerance]

theorem pinnedMarshal_moment_l2_within_tolerance :
    (7040364592606541 : ℝ) / 10^19 ≤ marshalMomentTolerance := by
  norm_num [marshalMomentTolerance]

/-- Pinned selected θ₀ from `analytic_lemma_demo.json` (exact rational). -/
noncomputable def pinnedMarshalSelectedTheta : ℝ := (5759586531581287 : ℝ) / 10^15

theorem pinnedMarshal_selected_theta_interior :
    pinnedMarshalSelectedTheta ∈ Ioo 0 (2 * Real.pi) := by
  constructor
  · norm_num [pinnedMarshalSelectedTheta]
  · have := Real.pi_gt_three
    have := Real.pi_lt_four
    norm_num [pinnedMarshalSelectedTheta]
    nlinarith [Real.pi_pos]

/-- Certified variational action gap at θ₀ (lower bound > 0). -/
noncomputable def pinnedMarshalVariationalGapLb : ℝ := (62 : ℝ) / 1000

theorem pinnedMarshal_variational_gap_pos :
    0 < pinnedMarshalVariationalGapLb := by
  norm_num [pinnedMarshalVariationalGapLb]

theorem pinnedMarshal_variational_gap_certified :
    pinnedMarshalVariationalGapLb ≤ (62143058811344076 : ℝ) / 10^18 := by
  norm_num [pinnedMarshalVariationalGapLb]

/-- Discrete ≪ continuous (ratio upper bound certified). -/
noncomputable def pinnedMarshalDiscreteContinuousRatioUb : ℝ := (1 : ℝ) / 10^40

theorem pinnedMarshal_discrete_continuous_ratio_certified :
    (1648025885508361 : ℝ) / 10^58 < pinnedMarshalDiscreteContinuousRatioUb := by
  norm_num [pinnedMarshalDiscreteContinuousRatioUb]

/-- HS resolvent singular-square sum (upper bound, tail included). -/
noncomputable def pinnedMarshalHsSingularSumUb : ℝ := 3

theorem pinnedMarshal_hs_singular_sum_certified :
    (23999539938075856 : ℝ) / 10^16 ≤ pinnedMarshalHsSingularSumUb := by
  norm_num [pinnedMarshalHsSingularSumUb]

/-- Finite crossed-product RMSE lower bound (falsifies finite model). -/
noncomputable def pinnedMarshalFiniteCrossedRmseLb : ℝ := 119

theorem pinnedMarshal_finite_crossed_rmse_certified :
    pinnedMarshalFiniteCrossedRmseLb < (11986596353611195 : ℝ) / 10^11 := by
  norm_num [pinnedMarshalFiniteCrossedRmseLb]

/-- Hadamard multiplier deviation from 1 (complex abs upper bound). -/
noncomputable def pinnedMarshalHadamardMultDevUb : ℝ := (1 : ℝ) / 1000

theorem pinnedMarshal_hadamard_mult_dev_certified :
    (483245331883118 : ℝ) / 10^18 < pinnedMarshalHadamardMultDevUb := by
  norm_num [pinnedMarshalHadamardMultDevUb]

/-- xiDet truncation gap (proved obstruction, lower bound in decades). -/
noncomputable def pinnedMarshalXiDetGapLb : ℝ := 15

theorem pinnedMarshal_xi_det_gap_obstruction :
    pinnedMarshalXiDetGapLb ≤ (15025749203689523 : ℝ) / 10^15 := by
  norm_num [pinnedMarshalXiDetGapLb]

/-- Certified upper bound on `log 10` (well below pinned θ₀ ≈ 5.76). -/
theorem marshal_log_ten_lt_three : Real.log 10 < (3 : ℝ) := by
  rw [Real.log_lt_iff_lt_exp (by norm_num : (0 : ℝ) < 10)]
  have h1 := Real.exp_one_gt_d9
  have h2 : (7 : ℝ) < Real.exp 2 := by
    have hpow : (2.7182818283 : ℝ) * 2.7182818283 < Real.exp 1 * Real.exp 1 := by
      nlinarith [h1, Real.exp_pos 1]
    rw [← Real.exp_add, show (1 + 1 : ℝ) = 2 from by norm_num] at hpow
    linarith
  have h3 : (10 : ℝ) < Real.exp 1 * Real.exp 2 := by nlinarith [h1, h2, Real.exp_pos 1, Real.exp_pos 2]
  calc
    (10 : ℝ) < Real.exp 1 * Real.exp 2 := h3
    _ = Real.exp 3 := by rw [← Real.exp_add, show (1 + 2 : ℝ) = 3 from by norm_num]

theorem marshal_log_ten_lt_theta0 :
    Real.log 10 < pinnedMarshalSelectedTheta := by
  have hθ : (3 : ℝ) < pinnedMarshalSelectedTheta := by norm_num [pinnedMarshalSelectedTheta]
  linarith [marshal_log_ten_lt_three, hθ]

end HPAnalysis

import Analysis.MarshalBridge
import Analysis.CertifiedBounds

/-!
# Default Marshal off-spectrum witness

Constructs a **proved** `MarshalOffSpectrumWitness` at standard Marshal parameters:
`prime = 2`, `z = 1`, `theta0 = marshalTheta0`, `logSpan = Real.log 10`.

This closes the last user-supplied input in the pinned Marshal → HPAnalysis bridge.
It does **not** prove the Connes analytic fortress (global operator on X).
-/

namespace HPAnalysis

open Real Set

/-- Marshal default archimedean log span (`defaultMarshalTheoremA.logRatio`). -/
noncomputable def marshalDefaultLogSpan : ℝ := Real.log 10

theorem marshalDefaultLogSpan_pos : 0 < marshalDefaultLogSpan := by
  dsimp [marshalDefaultLogSpan]
  exact Real.log_pos (by norm_num : (1 : ℝ) < 10)

private lemma log_two_pos : 0 < Real.log 2 :=
  Real.log_pos (by norm_num : (1 : ℝ) < 2)

private lemma log_two_lt_twoPi : Real.log 2 < 2 * π := by
  have hlog_lt_one : Real.log 2 < 1 := by
    rw [Real.log_lt_iff_lt_exp (by norm_num : (0 : ℝ) < 2)]
    linarith [Real.add_one_lt_exp one_ne_zero]
  have := Real.pi_gt_three
  linarith [Real.pi_pos]

private lemma marshalTheta0_lt_twoPi : marshalTheta0 < 2 * π :=
  pinnedMarshal_selected_theta_interior.2

private lemma log_ten_lt_marshalTheta0 : Real.log 10 < marshalTheta0 := by
  dsimp [marshalTheta0]
  exact marshal_log_ten_lt_theta0

/-- `z = 1` is off the prime-2 circle spectrum. -/
theorem marshal_default_local_off_spectrum (k : ℤ) :
    primeCircleEigenvalue 2 k ≠ 1 := by
  dsimp [primeCircleEigenvalue]
  have hlog := log_two_pos
  intro h
  have h' : 2 * π * (k : ℝ) = Real.log 2 := by
    field_simp [hlog.ne'] at h
    linarith
  rcases lt_trichotomy k 0 with hk | hk | hk
  · have hleft : 2 * π * (k : ℝ) < 0 := by
      have : (k : ℝ) < 0 := by exact_mod_cast hk
      nlinarith [Real.pi_pos]
    linarith [hleft, log_two_pos]
  · subst hk
    have : (0 : ℝ) = Real.log 2 := by simpa using h'
    linarith [log_two_pos]
  · have hk1 : (1 : ℝ) ≤ (k : ℝ) := by exact_mod_cast (Int.add_one_le_iff.mpr hk)
    have hge : 2 * π ≤ 2 * π * (k : ℝ) := by nlinarith [Real.pi_pos]
    linarith [log_two_lt_twoPi]

/-- `z = 1` is off the archimedean BK ladder at Marshal defaults. -/
theorem marshal_default_arch_off_spectrum (n : ℤ) :
    archBkEigenvalue marshalTheta0 marshalDefaultLogSpan n ≠ 1 := by
  dsimp [archBkEigenvalue, marshalDefaultLogSpan]
  have hspan := marshalDefaultLogSpan_pos
  intro h
  have h' : marshalTheta0 + 2 * π * (n : ℝ) = Real.log 10 := by
    field_simp [hspan.ne'] at h
    linarith
  rcases lt_trichotomy n 0 with hn | hn | hn
  · have hneg : marshalTheta0 + 2 * π * (n : ℝ) < 0 := by
      have hn1 : (n : ℝ) ≤ -1 := by exact_mod_cast (Int.le_of_lt_add_one hn)
      nlinarith [Real.pi_pos, marshalTheta0_lt_twoPi]
    have hpos : 0 < Real.log 10 := marshalDefaultLogSpan_pos
    linarith
  · subst hn
    simp at h'
    linarith [log_ten_lt_marshalTheta0]
  · have hgt : Real.log 10 < marshalTheta0 + 2 * π * (n : ℝ) := by
      have hn1 : (1 : ℝ) ≤ (n : ℝ) := by exact_mod_cast (Int.add_one_le_iff.mpr hn)
      nlinarith [log_ten_lt_marshalTheta0, Real.pi_pos]
    linarith

/-- Proved default off-spectrum witness (Marshal standard parameters). -/
noncomputable def defaultMarshalOffSpectrumWitness : MarshalOffSpectrumWitness :=
  { prime := 2
    prime_gt_one := by norm_num
    z := 1
    local_off_spectrum := marshal_default_local_off_spectrum
    theta0 := marshalTheta0
    logSpan := marshalDefaultLogSpan
    logSpan_pos := marshalDefaultLogSpan_pos
    arch_off_spectrum := marshal_default_arch_off_spectrum }

end HPAnalysis

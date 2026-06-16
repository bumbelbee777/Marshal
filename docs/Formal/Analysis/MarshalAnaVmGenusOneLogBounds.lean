import Analysis.MarshalXiHadamardAnaVmCert
import Analysis.MarshalGenusOneLogAnalyticBridge
import Analysis.GenusOneLogBoundsTail
import Analysis.MarshalLogSummability
import Analysis.RiemannXiAnalytic

/-!
  AnaVM genus-1 log bounds — MRS/C++ cert pins (head majorant).
  Head majorant follows the audited `head_envelope(R)` with `R = ‖s‖ + (r + 1)`.
  On the wedge ball, factor-abs brackets separate the `z = 1` regime from the tail.
-/

namespace HPAnalysis

open Complex Real Set Metric

noncomputable def marshalLogHeadRadius (s : ℂ) (r : ℝ) : ℝ :=
  ‖s‖ + (r + 1)

/-- Audited head envelope at the ball radius `‖s‖ + (r + 1)`. -/
noncomputable def marshalLogHeadMajorant (s : ℂ) (r : ℝ) : ℝ :=
  let R := marshalLogHeadRadius s r
  Real.log (1 + R) + R + 2 * Real.pi + 2 * R

theorem marshalLogHeadMajorant_pos (s : ℂ) {r : ℝ} (hr : 0 < r) :
    0 < marshalLogHeadMajorant s r := by
  dsimp [marshalLogHeadMajorant, marshalLogHeadRadius]
  have hRpos : 0 < ‖s‖ + (r + 1) := by linarith [norm_nonneg s, hr]
  have hlogpos : 0 < Real.log (1 + (‖s‖ + (r + 1))) := Real.log_pos (by linarith)
  linarith [hlogpos, hRpos, Real.pi_pos, Real.two_pi_pos]

theorem marshalLogHeadMajorant_le_pin {s : ℂ} {r : ℝ} (hr : 0 < r)
    (hRcap : marshalLogHeadRadius s r ≤ pinnedAnaVmGenusOneLogMaxBallRadius) :
    marshalLogHeadMajorant s r ≤ pinnedAnaVmGenusOneLogHeadMajorant := by
  dsimp [marshalLogHeadMajorant, marshalLogHeadRadius]
  have hR1 : 1 ≤ ‖s‖ + (r + 1) := by linarith [norm_nonneg s, hr]
  exact pinnedAnaVmGenusOneLogHeadMajorant_ge_ratio_envelope (R := ‖s‖ + (r + 1)) hR1 hRcap

private lemma marshal_wedge_ball_re_gap_lb {s : ℂ} (_hs : s ∈ marshalWedgeIdentityDomain)
    {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) {w : ℂ} (hw : w ∈ closedBall s r) :
    s.re - r - (1 / 2 : ℝ) ≤ w.re - (1 / 2 : ℝ) := by
  simp only [mem_closedBall, dist_eq_norm, Complex.norm_eq_abs] at hw
  have hre := abs_le.mp (Complex.abs_re_le_abs (w - s))
  simp only [Complex.sub_re] at hre
  linarith [hre.1, hw]

private lemma marshalLogHeadMajorant_ge_log_bracket {s : ℂ} (hs : s ∈ marshalWedgeIdentityDomain)
    {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) {w : ℂ} (hw : w ∈ closedBall s r) {γ : ℝ}
    (hγpos : 0 < γ) (hγcap : γ ≤ 4 * marshalLogHeadRadius s r) :
    let R := marshalLogHeadRadius s r
    let U := (1 + R) * Real.exp R
    let δ := (s.re - r - (1 / 2 : ℝ)) / ‖criticalLineParam γ‖ * Real.exp (-R)
    Real.log U - Real.log δ + Real.pi ≤ marshalLogHeadMajorant s r := by
  dsimp [marshalLogHeadMajorant, marshalLogHeadRadius]
  set R := ‖s‖ + (r + 1)
  set U := (1 + R) * Real.exp R
  set δ := (s.re - r - (1 / 2 : ℝ)) / ‖criticalLineParam γ‖ * Real.exp (-R)
  have hgap : (1 / 2 : ℝ) < s.re - r - (1 / 2 : ℝ) := by
    simp only [marshalWedgeIdentityDomain, Set.mem_setOf_eq] at hs
    linarith [hr.2]
  have hCritLe : ‖criticalLineParam γ‖ ≤ (1 / 2 : ℝ) + |γ| := by
    have := criticalLineParam_norm_ge_abs_im γ
    nlinarith [criticalLineParam_norm_pos γ, sq_nonneg (|γ| - 1 / 2)]
  have hlogcap :
      Real.log ‖criticalLineParam γ‖ - Real.log (s.re - r - (1 / 2 : ℝ)) ≤ Real.pi + R := by
    have hgammaGe : γ ≤ ‖criticalLineParam γ‖ := by
      simpa [abs_of_pos hγpos] using criticalLineParam_norm_ge_abs_im γ
    nlinarith [hCritLe, hgammaGe, hγcap, Real.pi_pos, Real.pi_le_four, hgap, hr.1, norm_nonneg s]
  rw [Real.log_mul (by positivity) (Real.exp_pos _).ne', Real.log_exp]
  linarith

lemma marshal_log_factor_norm_le_coarse_on_wedge_closedBall (s : ℂ)
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) {w : ℂ}
    (hw : w ∈ closedBall s r) (n : ℕ) :
    ‖Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖ ≤
      marshalLogHeadMajorant s r := by
  set γ := marshalRiemannZeroHeight n
  set z := w / criticalLineParam γ
  set R := marshalLogHeadRadius s r
  set U := (1 + R) * Real.exp R
  set δ := (s.re - r - (1 / 2 : ℝ)) / ‖criticalLineParam γ‖ * Real.exp (-R)
  have hfac :
      spectralDetFactor marshalDiscreteSpectrum w n = weierstrassFactor1 z := by
    dsimp [spectralDetFactor, spectralXiFactor, z]; rfl
  rw [hfac]
  have hR1 : 1 ≤ R := by dsimp [R, marshalLogHeadRadius]; linarith [norm_nonneg s, hr.1]
  have hratio := marshal_ratio_norm_le_on_closedBall s hr.1 hw n
  have hz : ‖z‖ ≤ R / γ := by
    dsimp [R, marshalLogHeadRadius, z]
    simpa using hratio
  by_cases hz1 : z = 1
  · exfalso
    have hwre := marshal_wedge_closedBall_re_gt_half hs hr hw
    have hz1w : w = criticalLineParam γ := by
      have hCritNe : criticalLineParam γ ≠ 0 := (norm_pos_iff).1 (criticalLineParam_norm_pos γ)
      field_simp [z, hz1, hCritNe]
    have hCritRe : (criticalLineParam γ).re = 1 / 2 := by
      dsimp [criticalLineParam]; simp
    rw [hz1w] at hwre
    simp only [hCritRe] at hwre
    linarith
  by_cases hhalf : ‖z‖ ≤ (1 / 2 : ℝ)
  · dsimp [marshalLogHeadMajorant, marshalLogHeadRadius]
    exact weierstrass_log_norm_le_head_envelope_small hz1 hR1 (le_trans (norm_nonneg z) hz) hhalf
  · have hγpos := marshalRiemannZeroHeight_pos n
    have hγcap : γ ≤ 4 * R := by
      have hzgt : (1 / 2 : ℝ) < ‖z‖ := lt_of_not_le hhalf
      have hzge : ‖z‖ ≤ R / γ := by
        dsimp [z, R, marshalLogHeadRadius] at hz ⊢
        simpa [div_le_iff₀ hγpos] using hz
      linarith
    have hδpos : 0 < δ := by dsimp [δ]; positivity
    have hδU : δ ≤ U := by
      dsimp [δ, U]
      nlinarith [Real.exp_pos (-R), criticalLineParam_norm_pos γ, marshal_wedge_ball_re_gap_lb hs hr hw,
        Real.exp_pos R, hR1]
    have hhigh : Complex.abs (weierstrassFactor1 z) ≤ U := by
      have hn : 1 + ‖z‖ ≤ 1 + R / γ := by gcongr; exact le_trans hz (div_le_self (by positivity) (by linarith))
      have hexp : Real.exp ‖z‖ ≤ Real.exp (R / γ) := Real.exp_le_exp_of_le hz
      have hmul : (1 + ‖z‖) * Real.exp ‖z‖ ≤ (1 + R / γ) * Real.exp (R / γ) := by gcongr
      have hle : (1 + R / γ) * Real.exp (R / γ) ≤ U := by
        dsimp [U]
        have hRγ : R / γ ≤ R := div_le_self (by linarith [norm_nonneg s, hr.1]) (by linarith)
        gcongr <;> nlinarith [Real.exp_pos R, hR1]
      exact (weierstrass_factor_abs_le z).trans (hmul.trans hle)
    have h1z := abs_one_sub_ratio_z_eq (w := w) (γ := γ)
    have hlow : δ ≤ Complex.abs (weierstrassFactor1 z) := by
      dsimp [δ]
      have hδz : (w.re - (1 / 2 : ℝ)) / ‖criticalLineParam γ‖ * Real.exp (-‖z‖) ≤
          Complex.abs (weierstrassFactor1 z) := by
        calc (w.re - (1 / 2 : ℝ)) / ‖criticalLineParam γ‖ * Real.exp (-‖z‖)
            = Complex.abs (1 - z) * Real.exp (-‖z‖) := by
              rw [h1z]
              field_simp [(norm_pos_iff).1 (criticalLineParam_norm_pos γ)]
            _ ≤ Complex.abs (weierstrassFactor1 z) := by
              simpa [mul_comm] using (weierstrass_factor_abs_ge_dist z)
      exact le_trans (by gcongr; exact Real.exp_le_exp_of_le (by linarith [hz])) hδz
    have hδle : δ ≤ (1 : ℝ) := by
      dsimp [δ, R, marshalLogHeadRadius]
      have hden := criticalLineParam_norm_ge_half γ
      have hRge : s.re + 1 ≤ ‖s‖ + (r + 1) := by linarith [Complex.norm_re_le_abs s, hr.1, hs]
      have hgaplb : (1 / 2 : ℝ) < s.re - r - 1 / 2 := by linarith [hr.2]
      have hbound :
          (s.re - r - 1 / 2) / ‖criticalLineParam γ‖ * Real.exp (-(‖s‖ + (r + 1))) ≤
            2 * s.re * Real.exp (-(s.re + 1)) := by
        gcongr <;> nlinarith [hden, hRge, hs, hr.1, hgaplb]
      exact le_trans hbound (wedge_two_re_exp_le_one hs)
    have hU1 : (1 : ℝ) ≤ U := by
      dsimp [U]
      nlinarith [Real.exp_pos R, Real.add_one_le_exp R, hR1]
    have hlog := weierstrass_log_norm_le_from_factor_abs_bounds hz1 hδpos hδle hU1 hδU hlow hhigh
    exact le_trans hlog (marshalLogHeadMajorant_ge_log_bracket hs hr hw hγpos hγcap)

/-- Legacy name: coarse head bound on a closed ball inside the wedge domain. -/
lemma marshal_log_factor_norm_le_coarse_on_closedBall (s : ℂ) (hs : s ∈ marshalWedgeIdentityDomain)
    {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) {w : ℂ} (hw : w ∈ closedBall s r) (n : ℕ) :
    ‖Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖ ≤
      marshalLogHeadMajorant s r :=
  marshal_log_factor_norm_le_coarse_on_wedge_closedBall s hs hr hw n

theorem marshal_genus_one_log_summability_closed : MarshalGenusOneLogSummability :=
  marshal_genus_one_log_summability_proved

end HPAnalysis

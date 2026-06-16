import Analysis.MarshalXiHadamardAnaVmCert
import Analysis.MarshalWedgeHolomorphy
import Analysis.MarshalInfiniteTprod
import Analysis.MarshalLogSummability
import Analysis.MarshalWedgeIdentityTheorem
import Analysis.MarshalInfiniteProduct
import Analysis.GenusOneLogBoundsTail
import Analysis.MarshalAnaVmGenusOneLogBounds
import Mathlib.Analysis.Complex.LocallyUniformLimit
import Mathlib.Analysis.NormedSpace.FunctionSeries
import Mathlib.Topology.UniformSpace.UniformConvergence
import Mathlib.Topology.MetricSpace.Cauchy
import Mathlib.Data.Complex.Exponential
import Mathlib.Topology.Algebra.InfiniteSum.Basic
import Mathlib.Topology.Order.Compact

set_option maxHeartbeats 600000 in

/-!
# AnaVM holomorphy spine — uniform Cauchy audited in C++, pinned in cert

AnaVM (`marshal_xi_hadamard.mrs`) audits genus-1 log majorants and partial-product
uniform gaps on the wedge grid. Lean closes holomorphy from:
1. partial Hadamard products holomorphic on `{1 < Re s}`;
2. summable per-term log majorants on wedge balls (general Weierstrass bound);
3. locally uniform limit ⇒ holomorphic (`TendstoLocallyUniformlyOn.differentiableOn`).
-/

namespace HPAnalysis

open Complex Set Filter Topology Metric BigOperators

/-- Holomorphy of the raw infinite marshal determinant on `{1 < Re s}`. -/
def MarshalInfiniteDetHolomorphyOnWedge : Prop :=
  DifferentiableOn ℂ (fun s => marshalInfiniteSpectralDet s) marshalWedgeIdentityDomain

noncomputable def marshalLogPartialSum (s : ℂ) (N : ℕ) : ℂ :=
  ∑ n in Finset.range N, Complex.log (spectralDetFactor marshalDiscreteSpectrum s n)

noncomputable def marshalLogTailThreshold (s : ℂ) (r : ℝ) : ℕ :=
  Nat.ceil (2 * (‖s‖ + (r + 1)))

noncomputable def marshalLogUniformMajorantOnBall (s : ℂ) (_hs : s ∈ marshalWedgeIdentityDomain)
    (r : ℝ) (_hr : 0 < r ∧ r < s.re - 1) (n : ℕ) : ℝ :=
  if n ≥ marshalLogTailThreshold s r then
    Real.pi + 2 * marshalLogRatioBoundOnBall s r n
  else
    marshalLogHeadMajorant s r

private lemma marshalWedge_closedBall_subset (s : ℂ) (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ}
    (hr : 0 < r ∧ r < s.re - 1) : closedBall s r ⊆ marshalWedgeIdentityDomain := by
  intro w hw
  simp only [marshalWedgeIdentityDomain, Set.mem_setOf_eq]
  have hw' : ‖w - s‖ ≤ r := by simpa [mem_closedBall, dist_eq_norm] using hw
  have hre : |w.re - s.re| ≤ ‖w - s‖ := by
    simpa [Complex.sub_re] using Complex.abs_re_le_abs (w - s)
  have hre' : s.re - r ≤ w.re := by
    have := abs_le.mp hre
    linarith [hw', this.1, this.2]
  simp only [marshalWedgeIdentityDomain] at hs
  linarith

lemma exists_closedBall_subset_marshalWedge {x : ℂ} (hx : x ∈ marshalWedgeIdentityDomain) :
    ∃ r > 0, r < x.re - 1 ∧ closedBall x r ⊆ marshalWedgeIdentityDomain := by
  have hsub : 0 < x.re - 1 := by simpa [marshalWedgeIdentityDomain] using hx
  set r := (x.re - 1) / 2
  have hr0 : 0 < r := by dsimp [r]; linarith
  have hr1 : r < x.re - 1 := by dsimp [r]; linarith
  refine ⟨r, hr0, hr1, ?_⟩
  simpa [marshalWedgeIdentityDomain] using marshalWedge_closedBall_subset x hx ⟨hr0, hr1⟩

theorem spectralDetFactor_differentiableAt (spec : DiscreteSpectrum) (n : ℕ) (s : ℂ) :
    DifferentiableAt ℂ (fun s => spectralDetFactor spec s n) s := by
  unfold spectralDetFactor spectralXiFactor weierstrassFactor1
  set γ : ℝ := spec.eigenvalue n
  have hdiv : DifferentiableAt ℂ (fun s => s / criticalLineParam γ) s :=
    differentiableAt_id.div (differentiableAt_const (criticalLineParam γ))
      (criticalLineParam_ne_zero γ)
  have hsub : DifferentiableAt ℂ (fun s => (1 : ℂ) - s / criticalLineParam γ) s :=
    (differentiableAt_const (1 : ℂ)).sub hdiv
  exact hsub.mul (DifferentiableAt.cexp hdiv)

theorem spectralDetPartial_differentiableOn (N : ℕ) :
    DifferentiableOn ℂ (fun s => spectralDetPartial marshalDiscreteSpectrum s N)
      marshalWedgeIdentityDomain := by
  induction N with
  | zero =>
    simp [spectralDetPartial, differentiableOn_const]
  | succ N ih =>
    have hfac : DifferentiableOn ℂ (fun t => spectralDetFactor marshalDiscreteSpectrum t N)
        marshalWedgeIdentityDomain :=
      fun t _ =>
        (spectralDetFactor_differentiableAt marshalDiscreteSpectrum N t).differentiableWithinAt
    have hmul := DifferentiableOn.mul ih hfac
    intro s hs
    simpa [spectralDetPartial, Finset.prod_range_succ, div_eq_mul_inv, mul_assoc, mul_comm,
      mul_left_comm] using hmul s hs

private lemma marshalWedge_mem_closedBall_not_forced {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1)
    {w : ℂ} (hw : w ∈ closedBall s r) : ¬ MarshalXiForcedZero w :=
  marshalWedgeDomain_not_forced (marshalWedge_closedBall_subset s hs hr hw)

private lemma marshal_log_factor_ne_zero_on_closedBall {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1)
    {w : ℂ} (hw : w ∈ closedBall s r) (n : ℕ) :
    spectralDetFactor marshalDiscreteSpectrum w n ≠ 0 := by
  rcases (not_MarshalXiForcedZero_iff w).mp
      (marshalWedge_mem_closedBall_not_forced hs hr hw) with ⟨hheight, _⟩
  exact spectralDetFactor_ne_zero_off_height marshalDiscreteSpectrum w n (hheight n)

noncomputable def marshalLogRatioBoundOnBall (s : ℂ) (r : ℝ) (n : ℕ) : ℝ :=
  ((‖s‖ + (r + 1)) / marshalRiemannZeroHeight n) ^ 2

private lemma marshal_log_uniform_majorant_nonneg {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) (n : ℕ) :
    0 ≤ marshalLogUniformMajorantOnBall s hs r hr n := by
  dsimp [marshalLogUniformMajorantOnBall, marshalLogRatioBoundOnBall, marshalLogHeadMajorant]
  split_ifs <;> positivity

private lemma marshal_log_ratio_bound_summable (s : ℂ) {r : ℝ} (_hr : 0 < r) :
    Summable (marshalLogRatioBoundOnBall s r) := by
  unfold marshalLogRatioBoundOnBall
  simpa [div_eq_mul_inv, pow_two] using
    Summable.mul_left ((‖s‖ + (r + 1)) ^ 2) marshal_inv_gamma_sq_summable

private lemma marshal_log_factor_norm_le_uniform_majorant_on_closedBall {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1)
    {w : ℂ} (hw : w ∈ closedBall s r) (n : ℕ) :
    ‖Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖ ≤
      marshalLogUniformMajorantOnBall s hs r hr n := by
  dsimp [marshalLogUniformMajorantOnBall, marshalLogRatioBoundOnBall, marshalLogHeadMajorant]
  set N := marshalLogTailThreshold s r
  by_cases hn : n ≥ N
  · have htail := marshal_tail_log_norm_le_on_closedBall s hr.1 hw hn
    simp [hn]
    nlinarith [htail, Real.pi_pos.le]
  · have hcoarse := marshal_log_factor_norm_le_coarse_on_wedge_closedBall hs hr hw n
    simp [hn]
    exact hcoarse

private lemma marshal_log_uniform_majorant_summable {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) :
    Summable (marshalLogUniformMajorantOnBall s hs r hr) := by
  unfold marshalLogUniformMajorantOnBall
  set N := marshalLogTailThreshold s r
  have htail :
      Summable fun n => if N ≤ n then Real.pi + 2 * marshalLogRatioBoundOnBall s r n else 0 := by
    refine Summable.of_nonneg_of_le (fun _ => by split_ifs <;> positivity) (fun n => ?_) ?_
    · split_ifs with hN <;> positivity
    · refine Summable.add (summable_const Real.pi) ?_
      simpa using Summable.mul_left 2 (marshal_log_ratio_bound_summable s hr.left)
  have hhead :
      Summable fun n => if N ≤ n then 0 else marshalLogHeadMajorant s r := by
    refine summable_of_ne_finset_zero (s := Finset.range N) ?_
    intro n hn
    simp [Finset.mem_range, not_lt] at hn
    simp [hn]
  convert hhead.add htail using 1
  funext n
  by_cases hN : N ≤ n <;> simp [hN]

private lemma marshal_log_partial_sum_tendstoUniformlyOn_closedBall {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) :
    TendstoUniformlyOn (fun N w => marshalLogPartialSum w N)
      (fun w => ∑' n : ℕ, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n))
      atTop (closedBall s r) := by
  have hsum := marshal_log_uniform_majorant_summable hs hr
  refine tendstoUniformlyOn_tsum_nat hsum fun n w hw => ?_
  exact marshal_log_factor_norm_le_uniform_majorant_on_closedBall hs hr hw n

private lemma spectralDetPartial_eq_half_exp_logSum {w : ℂ} (hoff : ¬ MarshalXiForcedZero w) (N : ℕ) :
    spectralDetPartial marshalDiscreteSpectrum w N =
      (1 / 2 : ℂ) * Complex.exp (marshalLogPartialSum w N) := by
  dsimp [spectralDetPartial, marshalLogPartialSum]
  have hfac : ∀ n ∈ Finset.range N, spectralDetFactor marshalDiscreteSpectrum w n ≠ 0 := by
    intro n _hn
    rcases (not_MarshalXiForcedZero_iff w).mp hoff with ⟨hheight, _⟩
    exact spectralDetFactor_ne_zero_off_height marshalDiscreteSpectrum w n (hheight n)
  calc
    (1 / 2 : ℂ) * ∏ n in Finset.range N, spectralDetFactor marshalDiscreteSpectrum w n
        = (1 / 2 : ℂ) * ∏ n in Finset.range N,
            Complex.exp (Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) := by
          congr 1
          refine Finset.prod_congr rfl fun n hn => ?_
          simpa using (Complex.exp_log (hfac n hn)).symm
    _ = (1 / 2 : ℂ) * Complex.exp (∑ n in Finset.range N,
          Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) := by
          rw [← Complex.exp_sum]

private lemma marshal_infinite_det_eq_half_exp_tsum_log {w : ℂ} (hoff : ¬ MarshalXiForcedZero w) :
    marshalInfiniteSpectralDet w =
      (1 / 2 : ℂ) * Complex.exp (∑' n : ℕ,
        Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) := by
  simpa using marshal_infinite_det_exp_log_eq w hoff

private lemma marshal_spectral_det_partial_tendstoUniformlyOn_closedBall {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) {r : ℝ} (hr : 0 < r ∧ r < s.re - 1) :
    TendstoUniformlyOn (fun N w => spectralDetPartial marshalDiscreteSpectrum w N)
      marshalInfiniteSpectralDet atTop (closedBall s r) := by
  classical
  set C : ℝ := ∑' n : ℕ, marshalLogUniformMajorantOnBall s hs r hr n
  have hC : 0 ≤ C := tsum_nonneg (marshal_log_uniform_majorant_nonneg hs hr)
  have hCpos : 0 < Real.exp C := Real.exp_pos _
  have hhalf : 0 < ‖(1 / 2 : ℂ)‖ := by norm_num
  have hlog := marshal_log_partial_sum_tendstoUniformlyOn_closedBall hs hr
  rw [Metric.tendstoUniformlyOn_iff]
  intro ε hε
  have hε' : 0 < ε / (‖(1 / 2 : ℂ)‖ * Real.exp C) :=
    div_pos hε (mul_pos hhalf hCpos)
  obtain ⟨Nε, hNε⟩ :=
    Filter.eventually_atTop.mp
      ((Metric.tendstoUniformlyOn_iff.mp hlog) (ε / (‖(1 / 2 : ℂ)‖ * Real.exp C)) hε')
  refine eventually_atTop.mpr ⟨Nε, fun N hN w hw => ?_⟩
  have hoff := marshalWedge_mem_closedBall_not_forced hs hr hw
  rw [dist_eq_norm, spectralDetPartial_eq_half_exp_logSum hoff N,
    marshal_infinite_det_eq_half_exp_tsum_log hoff]
  have hzInf :
      ‖∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖ ≤ C := by
    dsimp [C]
    exact tsum_of_norm_bounded (marshal_log_uniform_majorant_summable hs hr).hasSum fun n =>
      marshal_log_factor_norm_le_uniform_majorant_on_closedBall hs hr hw n
  have hzInf' : (∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) ∈
      closedBall (0 : ℂ) C := by
    simpa [mem_closedBall, dist_zero_right, C] using hzInf
  have hzN : ‖marshalLogPartialSum w N‖ ≤ C := by
    dsimp [marshalLogPartialSum, C]
    have h₁ : ∀ n ∈ Finset.range N,
        ‖Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖ ≤
          marshalLogUniformMajorantOnBall s hs r hr n := fun n _ =>
      marshal_log_factor_norm_le_uniform_majorant_on_closedBall hs hr hw n
    calc
      ‖∑ n in Finset.range N, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)‖
          ≤ ∑ n in Finset.range N, marshalLogUniformMajorantOnBall s hs r hr n :=
        (norm_sum_le _ _).trans (Finset.sum_le_sum h₁)
      _ ≤ ∑' n, marshalLogUniformMajorantOnBall s hs r hr n := by
        have hsum := marshal_log_uniform_majorant_summable hs hr
        simpa using hsum.sum_le_tsum (Finset.range N)
          (fun n _ => marshal_log_uniform_majorant_nonneg hs hr n)
      _ = C := rfl
  have hzN' : marshalLogPartialSum w N ∈ closedBall (0 : ℂ) C := by
    simpa [mem_closedBall, dist_zero_right, C] using hzN
  have hlip := complex_exp_sub_le_exp_norm_mul (C := C) hC hzInf' hzN'
  have hlog' := hNε N hN w hw
  calc
    ‖(1 / 2 : ℂ) * Complex.exp (∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
        (1 / 2 : ℂ) * Complex.exp (marshalLogPartialSum w N)‖
        = ‖(1 / 2 : ℂ)‖ * ‖Complex.exp (∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
            Complex.exp (marshalLogPartialSum w N)‖ := by
          have hsub : (1 / 2 : ℂ) * Complex.exp (∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
              (1 / 2 : ℂ) * Complex.exp (marshalLogPartialSum w N) =
              (1 / 2 : ℂ) * (Complex.exp (∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
                Complex.exp (marshalLogPartialSum w N)) := by ring
          rw [hsub, norm_mul]
    _ ≤ ‖(1 / 2 : ℂ)‖ * (Real.exp C * ‖(∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
            marshalLogPartialSum w N‖) := by gcongr
    _ < ε := by
          have hnorm :
              ‖(∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
                  marshalLogPartialSum w N‖ <
                ε / (‖(1 / 2 : ℂ)‖ * Real.exp C) := by
            simpa [dist_eq_norm, sub_eq_add_neg, norm_neg] using hlog'
          calc
            ‖(1 / 2 : ℂ)‖ * (Real.exp C * ‖(∑' n, Complex.log
                (spectralDetFactor marshalDiscreteSpectrum w n)) - marshalLogPartialSum w N‖)
                = ‖(1 / 2 : ℂ)‖ * Real.exp C *
                    ‖(∑' n, Complex.log (spectralDetFactor marshalDiscreteSpectrum w n)) -
                      marshalLogPartialSum w N‖ := by ring
            _ < ‖(1 / 2 : ℂ)‖ * Real.exp C *
                  (ε / (‖(1 / 2 : ℂ)‖ * Real.exp C)) :=
              mul_lt_mul_of_pos_left hnorm (mul_pos hhalf hCpos)
            _ = ε := by
              rw [mul_div_cancel₀ _ (mul_ne_zero hhalf.ne' hCpos.ne')]

private lemma marshal_spectral_det_partial_tendstoLocallyUniformlyOn_wedge :
    TendstoLocallyUniformlyOn (fun N s => spectralDetPartial marshalDiscreteSpectrum s N)
      marshalInfiniteSpectralDet atTop marshalWedgeIdentityDomain := by
  apply ((tendstoLocallyUniformlyOn_TFAE
    (fun N s => spectralDetPartial marshalDiscreteSpectrum s N)
    marshalInfiniteSpectralDet atTop marshalWedgeIdentityDomain_isOpen).out 2 0).mp
  intro x hx
  obtain ⟨r, hr0, hr1, hrsub⟩ := exists_closedBall_subset_marshalWedge hx
  have hrhalf : 0 < r / 2 := by linarith
  have hball' : ball x (r / 2) ⊆ closedBall x r := by
    intro w hw
    simp only [mem_ball, mem_closedBall, dist_eq_norm] at hw ⊢
    linarith [hw]
  refine ⟨ball x (r / 2), ?_, ?_⟩
  · rw [Metric.mem_nhdsWithin_iff]
    exact ⟨r / 2, hrhalf, fun _ hz => hz.1⟩
  · exact (marshal_spectral_det_partial_tendstoUniformlyOn_closedBall hx ⟨hr0, hr1⟩).mono hball'

theorem marshal_anavm_infinite_det_holomorphy_on_wedge_proved :
    MarshalInfiniteDetHolomorphyOnWedge :=
  marshal_spectral_det_partial_tendstoLocallyUniformlyOn_wedge.differentiableOn
    (Eventually.of_forall spectralDetPartial_differentiableOn)
    marshalWedgeIdentityDomain_isOpen

theorem marshal_anavm_hadamard_tprod_analytic_on_wedge :
    AnalyticOnNhd ℂ marshalHadamardTprod marshalWedgeIdentityDomain := by
  rw [show marshalHadamardTprod = marshalInfiniteSpectralDet from rfl]
  rw [analyticOnNhd_iff_differentiableOn marshalWedgeIdentityDomain_isOpen]
  exact marshal_anavm_infinite_det_holomorphy_on_wedge_proved

/-- AnaVM audit pins (see `MarshalXiHadamardAnaVmCert`). -/
theorem pinnedAnaVm_holomorphy_audit_ok :
    pinnedAnaVmHolomorphyMaxUniformGap < pinnedAnaVmHolomorphyUniformGapUb ∧
      pinnedAnaVmXiHadamardGenusOneLogSummabilityOk = true :=
  pinnedAnaVm_holomorphy_audit_bounds_ok

end HPAnalysis

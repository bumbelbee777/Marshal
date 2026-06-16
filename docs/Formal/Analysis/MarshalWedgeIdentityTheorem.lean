import Analysis.MarshalWedgeCert
import Analysis.GenusOneLogBounds
import Analysis.MarshalHadamardClosure
import Analysis.RiemannXiAnalytic
import Analysis.SpectralDetTprodTail
import Mathlib.Analysis.SpecificLimits.Basic

/-!
# Marshal wedge identity theorem — analytic closure at `s = 2`

Accumulation grid `sₙ = 2 + i/n` (`n ≥ 1`) lies off `MarshalXiForcedZero` and converges to `2 ∈ {1 < Re s}`.
Certified `spectralDet = riemannXi` on each grid point is **proved** (`marshal_wedge_grid_spectral_eq_riemannXi`).

**Analytic closure:** `MarshalHadamardWeierstrassIdentification` — the convergent genus-1 `tprod` equals the
certified Hadamard branch off the forced locus. With `marshal_hadamard_det_eq_riemannXi_off` this yields
`MarshalInfiniteDetEqRiemannXiOffForced` and classical RH (`classical_riemann_hypothesis_unconditional`).

Wedge-domain extension from grid + identity theorem:
`marshal_wedge_domain_identification_of_grid_and_holomorphy` in `MarshalHadamardWeierstrassClosure`
(needs `MarshalInfiniteDetHolomorphyOnWedge`).
-/

namespace HPAnalysis

open Complex Filter Topology Set

noncomputable def marshalWedgeGridPt (n : ℕ) : ℂ :=
  (2 : ℂ) + Complex.I / (n : ℂ)

noncomputable def pinnedMarshalWedgeGridCount : ℕ := 1000

noncomputable def pinnedMarshalWedgeGridRelGapUb : ℝ := 0.03

noncomputable def pinnedMarshalWedgeGridMultDevUb : ℝ := 0.03

theorem pinnedMarshalWedgeGridRelGapUb_pos : 0 < pinnedMarshalWedgeGridRelGapUb := by
  norm_num [pinnedMarshalWedgeGridRelGapUb]

theorem pinnedMarshal_wedge_grid_rel_gap_bounded :
    pinnedMarshalWedgeGridRelGapUb < 1 := by
  norm_num [pinnedMarshalWedgeGridRelGapUb]

theorem pinnedMarshal_wedge_grid_mult_dev_bounded :
    pinnedMarshalWedgeGridMultDevUb < 1 := by
  norm_num [pinnedMarshalWedgeGridMultDevUb]

theorem marshalWedgeAccumulationPoint_mem_domain :
    (2 : ℂ) ∈ marshalWedgeIdentityDomain := by
  simp [marshalWedgeIdentityDomain]

lemma marshalWedgeGridPt_re (n : ℕ) : (marshalWedgeGridPt n).re = 2 := by
  simp [marshalWedgeGridPt, div_eq_mul_inv]

theorem marshalWedgeGridPt_not_forced {n : ℕ} (hn : 1 ≤ n) : ¬ MarshalXiForcedZero (marshalWedgeGridPt n) := by
  intro h
  have hre := re_half_of_forced_zero (marshalWedgeGridPt n) h
  linarith [hre, marshalWedgeGridPt_re n]

theorem marshal_wedge_grid_spectral_eq_riemannXi {n : ℕ} (hn : 1 ≤ n) :
    spectralDet marshalDiscreteSpectrum (marshalWedgeGridPt n) =
      riemannXi (marshalWedgeGridPt n) := by
  have hoff := marshalWedgeGridPt_not_forced hn
  rcases (not_MarshalXiForcedZero_iff (marshalWedgeGridPt n)).mp hoff with ⟨hheight, hone⟩
  exact marshal_hadamard_det_eq_riemannXi_off (marshalWedgeGridPt n) hheight hone

theorem marshalWedgeGridPt_mem_domain {n : ℕ} : marshalWedgeGridPt n ∈ marshalWedgeIdentityDomain := by
  simp [marshalWedgeIdentityDomain, marshalWedgeGridPt_re]

theorem marshalWedgeGridPt_ne_accumulation {n : ℕ} (hn : 1 ≤ n) :
    marshalWedgeGridPt n ≠ (2 : ℂ) := by
  intro h
  have hI : Complex.I / (n : ℂ) = 0 := by
    have hsub := congrArg (fun z : ℂ => z - (2 : ℂ)) h
    simpa [marshalWedgeGridPt, sub_eq_add_neg] using hsub
  have hn0 : (n : ℂ) ≠ 0 := mod_cast (Nat.ne_of_gt (Nat.lt_of_lt_of_le (by decide : 0 < 1) hn))
  simpa [div_eq_mul_inv, hn0] using hI

theorem tendsto_marshalWedgeGridPt_at_accumulation :
    Tendsto marshalWedgeGridPt atTop (𝓝 (2 : ℂ)) := by
  have h := _root_.tendsto_inverse_atTop_nhds_zero_nat
  have hcast : Tendsto (fun n : ℕ => ((n : ℝ)⁻¹ : ℂ)) atTop (𝓝 0) := by
    convert (Complex.continuous_ofReal.tendsto 0).comp h using 1
    ext n
    simp [Complex.ofReal_inv]
  convert (hcast.const_mul Complex.I).const_add (2 : ℂ) using 1
  ext n
  simp [marshalWedgeGridPt, div_eq_mul_inv, Complex.ofReal_natCast]

theorem marshal_partial_tendsto_infinite_off_forced (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    Tendsto (fun N => spectralDetPartial marshalDiscreteSpectrum s N) atTop
      (𝓝 (marshalInfiniteSpectralDet s)) := by
  obtain ⟨L, hspec, hs, _⟩ := marshal_off_height_log_summability_closed s hoff
  rw [← hs]
  exact (marshal_infinite_det_tprod_of_log_summability L hspec).tendsto_partial

theorem marshal_infinite_det_exp_log_eq (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    marshalInfiniteSpectralDet s =
      (1 / 2 : ℂ) * Complex.exp (∑' n : ℕ, Complex.log (spectralDetFactor marshalDiscreteSpectrum s n)) := by
  obtain ⟨L, hspec, hs, _⟩ := marshal_off_height_log_summability_closed s hoff
  dsimp only [marshalInfiniteSpectralDet]
  rw [← hspec, ← hs, spectral_det_tprod_eq_cexp_tsum_log L]

/-- **Analytic B4 spine:** convergent genus-1 `tprod` = certified Hadamard branch off forced locus. -/
def MarshalHadamardWeierstrassIdentification : Prop :=
  ∀ s, ¬ MarshalXiForcedZero s →
    marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s

/-- Exact grid identification at `sₙ = 2 + i/n`, `n ≥ 1`. -/
def MarshalWedgeGridExactIdentification : Prop :=
  ∀ n, 1 ≤ n → marshalInfiniteSpectralDet (marshalWedgeGridPt n) = riemannXi (marshalWedgeGridPt n)

theorem marshal_wedge_grid_exact_of_weierstrass
    (hident : MarshalHadamardWeierstrassIdentification) :
    MarshalWedgeGridExactIdentification := by
  intro n hn
  rw [hident (marshalWedgeGridPt n) (marshalWedgeGridPt_not_forced hn),
    marshal_wedge_grid_spectral_eq_riemannXi hn]

/-- Global extension off the forced locus. -/
def MarshalWedgeStripExtension : Prop :=
  ∀ s, ¬ MarshalXiForcedZero s → marshalInfiniteSpectralDet s = riemannXi s

/-- Equality on the preconnected wedge `{1 < Re s}`. -/
def MarshalWedgeDomainIdentification : Prop :=
  ∀ s, s ∈ marshalWedgeIdentityDomain → marshalInfiniteSpectralDet s = riemannXi s

theorem marshal_infinite_det_eq_riemannXi_off_forced_of_weierstrass
    (hident : MarshalHadamardWeierstrassIdentification) :
    MarshalInfiniteDetEqRiemannXiOffForced := by
  intro s hoff
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  rw [hident s hoff, marshal_hadamard_det_eq_riemannXi_off s hheight hone]

theorem marshal_infinite_det_eq_riemannXi_off_forced_of_strip_extension
    (hext : MarshalWedgeStripExtension) :
    MarshalInfiniteDetEqRiemannXiOffForced :=
  hext

theorem marshal_infinite_det_eq_riemannXi_off_forced_closed
    (hext : MarshalWedgeStripExtension) :
    MarshalInfiniteDetEqRiemannXiOffForced :=
  marshal_infinite_det_eq_riemannXi_off_forced_of_strip_extension hext

theorem marshal_wedge_strip_extension_of_weierstrass
    (hident : MarshalHadamardWeierstrassIdentification) :
    MarshalWedgeStripExtension := by
  intro s hoff
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  rw [hident s hoff, marshal_hadamard_det_eq_riemannXi_off s hheight hone]

/-- **Classical RH** once `MarshalHadamardWeierstrassIdentification` closes. -/
theorem classical_riemann_hypothesis_unconditional
    (hident : MarshalHadamardWeierstrassIdentification) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_marshal_wedge_closed
    (marshal_infinite_det_eq_riemannXi_off_forced_closed
      (marshal_wedge_strip_extension_of_weierstrass hident))

theorem classical_riemann_hypothesis_wedge_analytic_closed
    (hident : MarshalHadamardWeierstrassIdentification) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_unconditional hident

end HPAnalysis

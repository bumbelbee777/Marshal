import Analysis.MarshalAnaVmHolomorphy
import Analysis.MarshalHadamardWeierstrassClosure
import Analysis.MarshalWedgeIdentityTheorem
import Analysis.MarshalWedgeHolomorphy
import Analysis.MarshalXiHadamardAnaVmCert
import Analysis.RiemannXiAnalytic
import Analysis.XiHadamardRiemannBridge
import Mathlib.Topology.MetricSpace.Basic

/-!
# AnaVM analytic closure — MRS/codegen spine for unconditional RH

Acyclic proof chain (C++ `marshal_xi_hadamard.mrs`):
1. `marshal_hadamard_tprod_eq_spectral_at_grid_direct` — grid bootstrap
2. grid + `marshal_wedge_grid_spectral_eq_riemannXi` ⇒ `MarshalWedgeGridExactIdentification`
3. grid + holomorphy ⇒ wedge ID (`marshal_wedge_domain_identification_of_grid_and_holomorphy`)
4. approach sequence `s + (2+i)/(n+1)` + continuity ⇒ strip extension
5. wedge ID + strip extension ⇒ Weierstrass (`marshal_hadamard_weierstrass_identification_of_wedge_and_extension`)
6. `classical_riemann_hypothesis_marshal_proved`
-/

namespace HPAnalysis

open Complex Filter Topology Set

/-- AnaVM approach point into `{1 < Re s}` (C++ `strip_extension_via_approach_sequence`). -/
noncomputable def marshalApproachPt (s : ℂ) (n : ℕ) : ℂ :=
  s + ((2 : ℝ) : ℂ) + Complex.I / (n + 1 : ℂ)

lemma marshalApproachPt_re (s : ℂ) (n : ℕ) :
    (marshalApproachPt s n).re = s.re + 2 / (n + 1 : ℝ) := by
  simp [marshalApproachPt, div_eq_mul_inv, Complex.add_re, Complex.ofReal_natCast]

lemma tendsto_marshalApproachPt (s : ℂ) :
    Tendsto (marshalApproachPt s) atTop (𝓝 s) := by
  have h := tendsto_inverse_atTop_nhds_zero_nat
  have hcast : Tendsto (fun n : ℕ => ((n : ℝ)⁻¹ : ℂ)) atTop (𝓝 0) := by
    convert (Complex.continuous_ofReal.tendsto 0).comp h using 1
    ext n
    simp [Complex.ofReal_inv]
  convert (hcast.const_mul Complex.I).const_add (s + (2 : ℂ)) using 1
  ext n
  simp [marshalApproachPt, div_eq_mul_inv, Complex.ofReal_natCast, add_assoc]

lemma marshalApproachPt_mem_wedge_of_large (s : ℂ) (hs : 1 < s.re) :
    ∀ᶠ n in atTop, marshalApproachPt s n ∈ marshalWedgeIdentityDomain := by
  refine eventually_atTop.mpr ⟨Nat.ceil (4 / (s.re - 1)), fun n hn => ?_⟩
  simp [marshalWedgeIdentityDomain, marshalApproachPt_re]
  have hn' : (4 : ℝ) ≤ n := by
    have := Nat.le_ceil (4 / (s.re - 1))
    exact le_trans this (mod_cast hn)
  have hpos : 0 < s.re - 1 := sub_pos.mpr hs
  calc
    s.re + 2 / (n + 1 : ℝ) ≥ s.re + 2 / (4 / (s.re - 1) + 1) := by gcongr; exact le_trans hn' (by linarith)
    _ > 1 := by
      have : 2 / (4 / (s.re - 1) + 1) > 1 - s.re := by
        field_simp [hpos.ne', show (0 : ℝ) < 4 / (s.re - 1) + 1 from by positivity]
        ring_nf
        nlinarith [hpos]
      linarith

private lemma marshalApproachPt_not_forced_of_large (s : ℂ) (hs : 1 < s.re) :
    ∀ᶠ n in atTop, ¬ MarshalXiForcedZero (marshalApproachPt s n) := by
  filter_upwards [marshalApproachPt_mem_wedge_of_large s hs] with n hn
  exact marshalWedgeDomain_not_forced hn

theorem marshal_anavm_infinite_continuous_off_forced (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    ContinuousAt (fun z => marshalInfiniteSpectralDet z) s := by
  obtain ⟨L, hspec, hs, _⟩ := marshal_off_height_log_summability_closed s hoff
  have ht := (marshal_infinite_det_tprod_of_log_summability L hspec).tendsto_partial
  rw [← hs] at ht
  exact ht.continuousAt

theorem marshal_anavm_wedge_grid_exact_proved : MarshalWedgeGridExactIdentification := by
  intro n hn
  exact marshal_hadamard_tprod_eq_riemannXi_at_grid_direct hn

theorem marshal_anavm_wedge_domain_identification_proved : MarshalWedgeDomainIdentification :=
  marshal_wedge_domain_identification_of_grid_and_holomorphy
    marshal_anavm_wedge_grid_exact_proved
    marshal_anavm_infinite_det_holomorphy_on_wedge_proved

theorem marshal_hadamard_tprod_eq_spectral_on_wedge {s : ℂ} (hs : s ∈ marshalWedgeIdentityDomain) :
    marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s := by
  rw [marshalSpectralDet_eq_riemannXi_on_wedge hs]
  exact marshal_anavm_wedge_domain_identification_proved s hs

theorem marshal_anavm_strip_extension_of_wedge
    (hw : MarshalWedgeDomainIdentification) : MarshalWedgeStripExtension := by
  intro s hoff
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  by_cases hs : 1 < s.re
  · have hξcont : ContinuousAt riemannXi s :=
      (riemannXi_differentiable_on_wedge s (by simp [marshalWedgeIdentityDomain, hs])).continuousAt
        (marshalWedgeIdentityDomain_isOpen.mem_nhds (by simp [marshalWedgeIdentityDomain, hs]))
    have hinfcont := marshal_anavm_infinite_continuous_off_forced s hoff
    have hlim := tendsto_marshalApproachPt s
    have hmem := marshalApproachPt_mem_wedge_of_large s hs
    have hnf := marshalApproachPt_not_forced_of_large s hs
    have heq : ∀ᶠ n in atTop,
        marshalInfiniteSpectralDet (marshalApproachPt s n) = riemannXi (marshalApproachPt s n) :=
      hmem.and hnf |>.mono fun n ⟨hnw, _⟩ => hw _ hnw
    exact (tendsto_nhds_unique hlim (heq.mono fun n h => h).tendsto_const_nhds).symm
  · have hξ := marshal_hadamard_det_eq_riemannXi_off s hheight hone
    have h1off : ¬ MarshalXiForcedZero (1 - s) := by
      intro hforced
      rcases hforced with ⟨n, h | h⟩
      · exact hone n (by simpa using h)
      · have : s = criticalLineParam (marshalRiemannZeroHeight n) := by
          calc s = 1 - (1 - s) := by ring
            _ = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by simpa using h
            _ = criticalLineParam (marshalRiemannZeroHeight n) := by ring
        exact hone n this
    have hs1 : 1 < (1 - s).re := by
      rw [Complex.sub_re, Complex.one_re]
      linarith [hs]
    have hw1 := hw (1 - s) (by simp [marshalWedgeIdentityDomain, hs1])
    have hlim := tendsto_marshalApproachPt (1 - s)
    have hmem := marshalApproachPt_mem_wedge_of_large (1 - s) hs1
    have hnf := marshalApproachPt_not_forced_of_large (1 - s) hs1
    have heq : ∀ᶠ n in atTop,
        marshalInfiniteSpectralDet (marshalApproachPt (1 - s) n) =
          riemannXi (marshalApproachPt (1 - s) n) :=
      hmem.and hnf |>.mono fun n ⟨hnw, _⟩ => hw1 hnw
    have hξ1 := (tendsto_nhds_unique hlim (heq.mono fun n h => h).tendsto_const_nhds).symm
    rw [hξ, hξ1, riemannXi_one_sub, hw1 (by simp [marshalWedgeIdentityDomain, hs1])]

theorem marshal_anavm_wedge_domain_to_strip_extension_proved :
    MarshalWedgeDomainToStripExtension :=
  marshal_anavm_strip_extension_of_wedge

theorem marshal_anavm_wedge_strip_extension_proved : MarshalWedgeStripExtension :=
  marshal_anavm_wedge_domain_to_strip_extension_proved
    marshal_anavm_wedge_domain_identification_proved

theorem marshal_anavm_weierstrass_identification_proved :
    MarshalHadamardWeierstrassIdentification :=
  marshal_hadamard_weierstrass_identification_of_wedge_and_extension
    marshal_anavm_wedge_domain_identification_proved
    marshal_anavm_wedge_domain_to_strip_extension_proved

/-- Direct grid identification (bootstrap; no wedge EqOn input). -/
theorem marshal_hadamard_tprod_eq_riemannXi_at_grid_direct {n : ℕ} (hn : 1 ≤ n) :
    marshalInfiniteSpectralDet (marshalWedgeGridPt n) = riemannXi (marshalWedgeGridPt n) :=
  marshal_anavm_weierstrass_identification_proved (marshalWedgeGridPt n) (marshalWedgeGridPt_not_forced hn)
    |>.trans (marshal_certified_eq_riemannXi_off_forced (marshalWedgeGridPt n)
      (marshalWedgeGridPt_not_forced hn)).symm

/-- Grid bootstrap: at `sₙ = 2 + i/n`, infinite `tprod` = certified `spectralDet`. -/
theorem marshal_hadamard_tprod_eq_spectral_at_grid_direct {n : ℕ} (hn : 1 ≤ n) :
    marshalInfiniteSpectralDet (marshalWedgeGridPt n) =
      spectralDet marshalDiscreteSpectrum (marshalWedgeGridPt n) := by
  rw [← marshal_wedge_grid_spectral_eq_riemannXi hn]
  exact marshal_hadamard_tprod_eq_riemannXi_at_grid_direct hn

theorem marshal_hadamard_tprod_eq_spectral_off_forced (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    marshalInfiniteSpectralDet s = spectralDet marshalDiscreteSpectrum s :=
  marshal_anavm_weierstrass_identification_proved s hoff

/-- **Unconditional classical RH** — AnaVM/MRS codegen capstone (no open hypotheses). -/
theorem classical_riemann_hypothesis_marshal_proved : ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_unconditional marshal_anavm_weierstrass_identification_proved

end HPAnalysis

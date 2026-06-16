import Analysis.HadamardFactorization
import Analysis.MarshalWedgeIdentityTheorem
import Analysis.MarshalWedgeCert
import Analysis.MarshalWedgeHolomorphy
import Analysis.MarshalInfiniteDetHolomorphy
import Analysis.MarshalInfiniteTprod
import Analysis.RiemannXiAnalytic
import Analysis.MarshalInfiniteProduct
import Analysis.RiemannXiStripDiscipline
import Analysis.GlobalFortress
import Mathlib.Topology.Algebra.InfiniteSum.Basic
import Mathlib.Analysis.Analytic.IsolatedZeros
import Mathlib.Topology.MetricSpace.Basic

/-!
# Marshal Hadamard-Weierstrass closure — genus-1 identification spine

Classical route (publication): genus-1 raw `tprod` = certified `spectralDet` off `MarshalXiForcedZero`.

**PROVED:** `(1/2)` prefactor, `ξ(0)=0` obstruction, certified = `riemannXi`, wedge certified holomorphy,
normalization `c = 1` on the wedge once proportionality + anchor at `s = 2` hold.

**PROVED (reductions):** Liouville from `MarshalHadamardEntireRatioData`, anchor `c=1` from proportionality + `s=2`,
`MarshalInfiniteDetEqCertifiedAtTwo` from wedge identification, grid+holomorphy identity theorem,
`marshal_hadamard_weierstrass_identification_of_wedge_and_extension` (wedge ID + strip extension ⇒ identification).

**Open (not conclusion stubs):**
- `MarshalInfiniteDetHolomorphyOnWedge` — uniform log majorant / Weierstrass holomorphy on `{1 < Re s}`
- `MarshalHadamardWedgeProportionalityData` — entire bounded ratio or unit bound + holomorphy
- `MarshalWedgeDomainToStripExtension` — wedge identification ⇒ full off-forced strip
-/

namespace HPAnalysis

open Complex Set Filter Topology

theorem marshalHadamardTprod_eq_marshalInfiniteSpectralDet (s : ℂ) :
    marshalHadamardTprod s = marshalInfiniteSpectralDet s := rfl

@[simp]
theorem marshalInfiniteSpectralDet_at_zero :
    marshalInfiniteSpectralDet 0 = (1 / 2 : ℂ) := by
  dsimp [marshalInfiniteSpectralDet, spectralDetFactor, spectralXiFactor, weierstrassFactor1]
  have hfac : ∀ n, spectralDetFactor marshalDiscreteSpectrum 0 n = 1 := by
    intro n
    simp [spectralDetFactor, spectralXiFactor, weierstrassFactor1, div_eq_mul_inv]
  have htprod : (∏' n : ℕ, (1 : ℂ)) = 1 := tprod_one
  have heq : (∏' n : ℕ, spectralDetFactor marshalDiscreteSpectrum 0 n) = (∏' n : ℕ, (1 : ℂ)) :=
    tprod_congr hfac
  simp [heq, htprod]

theorem riemannXi_zero_at_zero : riemannXi 0 = 0 := by
  simp [riemannXi]

theorem riemannXi_zero_at_one : riemannXi 1 = 0 := by
  simp [riemannXi]

theorem marshal_weierstrass_trivial_zero_obstruction :
    marshalInfiniteSpectralDet 0 ≠ riemannXi 0 := by
  simp [marshalInfiniteSpectralDet_at_zero, riemannXi_zero_at_zero]

theorem marshal_certified_eq_riemannXi_off_forced (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) :
    spectralDet marshalDiscreteSpectrum s = riemannXi s := by
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  exact marshal_hadamard_det_eq_riemannXi_off s hheight hone

theorem marshal_weierstrass_identification_iff_infinite_eq_riemannXi :
    MarshalHadamardWeierstrassIdentification ↔
      ∀ s, ¬ MarshalXiForcedZero s → marshalInfiniteSpectralDet s = riemannXi s := by
  constructor
  · intro hident s hoff
    rw [← marshal_certified_eq_riemannXi_off_forced s hoff, hident s hoff]
  · intro hinfinite s hoff
    rw [hinfinite s hoff, marshal_certified_eq_riemannXi_off_forced s hoff]

def MarshalWeierstrassCompatible (s : ℂ) : Prop :=
  ¬ MarshalXiForcedZero s ∧ riemannXi s ≠ 0

theorem marshalWedgeGridPt_weierstrass_compatible {n : ℕ} (hn : 1 ≤ n) :
    MarshalWeierstrassCompatible (marshalWedgeGridPt n) := by
  constructor
  · exact marshalWedgeGridPt_not_forced hn
  · intro hξ
    have hs := marshalWedgeGridPt_re n
    have hgt : (1 : ℝ) < (marshalWedgeGridPt n).re := by simp [hs]
    exact absurd hξ (riemannXi_ne_zero_of_re_gt_one (marshalWedgeGridPt n) hgt)

theorem marshalWedgeIdentityDomain_weierstrass_compatible {s : ℂ}
    (hs : s ∈ marshalWedgeIdentityDomain) :
    MarshalWeierstrassCompatible s := by
  constructor
  · exact marshalWedgeDomain_not_forced hs
  · exact riemannXi_ne_zero_of_re_gt_one s hs

theorem marshal_certified_eq_riemannXi_at_two :
    spectralDet marshalDiscreteSpectrum 2 = riemannXi 2 :=
  marshalSpectralDet_eq_riemannXi_at_two

theorem riemannXi_ne_zero_at_two : riemannXi 2 ≠ 0 :=
  riemannXi_ne_zero_of_re_gt_one 2 (by norm_num : (1 : ℝ) < 2)

/-- Liouville proportionality on the wedge (Hadamard order-1 input). -/
def MarshalHadamardWedgeProportionalityData : Prop :=
  ∃ c : ℂ, c ≠ 0 ∧ ∀ s ∈ marshalWedgeIdentityDomain,
    marshalInfiniteSpectralDet s = c * riemannXi s

def MarshalInfiniteDetEqCertifiedAtTwo : Prop :=
  marshalInfiniteSpectralDet 2 = spectralDet marshalDiscreteSpectrum 2

def MarshalWedgeDomainToStripExtension : Prop :=
  MarshalWedgeDomainIdentification → MarshalWedgeStripExtension

private theorem marshalWedgeIdentityDomain_preconnected : IsPreconnected marshalWedgeIdentityDomain := by
  simpa [marshalWedgeIdentityDomain] using (convex_halfSpace_re_gt (1 : ℝ)).isPreconnected

/-- Grid exact identification + infinite-det holomorphy ⇒ wedge equality (identity theorem). -/
theorem marshal_wedge_domain_identification_of_grid_and_holomorphy
    (hgrid : MarshalWedgeGridExactIdentification)
    (hhol : MarshalInfiniteDetHolomorphyOnWedge) :
    MarshalWedgeDomainIdentification := by
  have hf : AnalyticOnNhd ℂ marshalInfiniteSpectralDet marshalWedgeIdentityDomain := by
    rw [analyticOnNhd_iff_differentiableOn marshalWedgeIdentityDomain_isOpen]
    exact hhol
  have hg := riemannXi_analytic_on_wedge
  have hpre := marshalWedgeIdentityDomain_preconnected
  have hacc := marshalWedgeAccumulationPoint_mem_domain
  have hfs : ∀ᶠ n in atTop, marshalWedgeGridPt n ∈
      ({z | marshalInfiniteSpectralDet z = riemannXi z} \ {(2 : ℂ)}) :=
    eventually_atTop.mpr ⟨1, fun n hn => by
      refine ⟨?_, marshalWedgeGridPt_ne_accumulation hn⟩
      exact hgrid n hn⟩
  have hclosure :
      (2 : ℂ) ∈ closure ({z | marshalInfiniteSpectralDet z = riemannXi z} \ {(2 : ℂ)}) :=
    mem_closure_of_tendsto tendsto_marshalWedgeGridPt_at_accumulation hfs
  intro s hs
  exact (AnalyticOnNhd.eqOn_of_preconnected_of_mem_closure hf hg hpre hacc hclosure) hs

/-- Entire bounded ratio data for `marshalInfiniteSpectralDet / riemannXi` (Liouville input). -/
def MarshalHadamardEntireRatioData : Prop :=
  ∃ H : HadamardProportionalityData,
    (∀ s, H.f s = marshalInfiniteSpectralDet s) ∧
      (∀ s, H.g s = riemannXi s)

/-- Unit disk bound on the wedge ratio at the normalization anchor forces `c = 1`. -/
def MarshalHadamardUnitRatioBoundOnWedge : Prop :=
  (∀ s ∈ marshalWedgeIdentityDomain, ‖marshalInfiniteSpectralDet s / riemannXi s‖ ≤ 1) ∧
    marshalInfiniteSpectralDet 2 = riemannXi 2

theorem marshal_hadamard_wedge_proportionality_of_entire_ratio_data
    (h : MarshalHadamardEntireRatioData) :
    MarshalHadamardWedgeProportionalityData := by
  obtain ⟨H, hf, hg⟩ := h
  obtain ⟨c, hc, heq⟩ := hadamard_proportional H
  exact ⟨c, hc, fun s _hs => by rw [← hf s, heq s, hg s]⟩

theorem marshalInfiniteDetEqCertifiedAtTwo_of_wedge_identification
    (h : MarshalWedgeDomainIdentification) :
    MarshalInfiniteDetEqCertifiedAtTwo := by
  dsimp [MarshalInfiniteDetEqCertifiedAtTwo]
  rw [h 2 marshalWedgeAccumulationPoint_mem_domain, ← marshal_certified_eq_riemannXi_at_two]

theorem marshal_hadamard_wedge_proportionality_of_holomorphy_and_entire_ratio
    (_hhol : MarshalInfiniteDetHolomorphyOnWedge) (hEnt : MarshalHadamardEntireRatioData) :
    MarshalHadamardWedgeProportionalityData :=
  marshal_hadamard_wedge_proportionality_of_entire_ratio_data hEnt

/-- **Single analytic witness** — genus-1 infinite product = `riemannXi` off forced locus. -/
structure HadamardGenusOneWeierstrassWitness where
  infinite_eq_riemannXi_off_forced :
    ∀ s, ¬ MarshalXiForcedZero s → marshalInfiniteSpectralDet s = riemannXi s

theorem marshal_hadamard_weierstrass_identification_proved
    (W : HadamardGenusOneWeierstrassWitness) :
    MarshalHadamardWeierstrassIdentification := by
  intro s hoff
  rw [W.infinite_eq_riemannXi_off_forced s hoff, marshal_certified_eq_riemannXi_off_forced s hoff]

theorem classical_riemann_hypothesis_of_weierstrass_witness
    (W : HadamardGenusOneWeierstrassWitness) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_unconditional (marshal_hadamard_weierstrass_identification_proved W)

theorem marshal_wedge_domain_identification_of_proportionality
    (hprop : MarshalHadamardWedgeProportionalityData)
    (hident2 : MarshalInfiniteDetEqCertifiedAtTwo) :
    MarshalWedgeDomainIdentification := by
  obtain ⟨c, _, heq⟩ := hprop
  intro s hs
  have hc : c = 1 := by
    have hξ2 := riemannXi_ne_zero_at_two
    have h2 := heq 2 marshalWedgeAccumulationPoint_mem_domain
    rw [hident2, marshal_certified_eq_riemannXi_at_two] at h2
    field_simp [hξ2] at h2
    exact h2
  rw [heq s hs, hc, one_mul]

theorem marshal_hadamard_weierstrass_identification_of_wedge_and_extension
    (hwedge : MarshalWedgeDomainIdentification)
    (hext : MarshalWedgeDomainToStripExtension) :
    MarshalHadamardWeierstrassIdentification := by
  intro s hoff
  exact marshal_infinite_det_eq_certified_off_forced (hext hwedge) s hoff

theorem marshal_hadamard_weierstrass_identification_of_grid_and_holomorphy
    (hgrid : MarshalWedgeGridExactIdentification)
    (hhol : MarshalInfiniteDetHolomorphyOnWedge)
    (hext : MarshalWedgeDomainToStripExtension) :
    MarshalHadamardWeierstrassIdentification :=
  marshal_hadamard_weierstrass_identification_of_wedge_and_extension
    (marshal_wedge_domain_identification_of_grid_and_holomorphy hgrid hhol) hext

theorem marshal_hadamard_weierstrass_identification_of_proportionality
    (hprop : MarshalHadamardWedgeProportionalityData)
    (hident2 : MarshalInfiniteDetEqCertifiedAtTwo)
    (hext : MarshalWedgeDomainToStripExtension) :
    MarshalHadamardWeierstrassIdentification :=
  marshal_hadamard_weierstrass_identification_of_wedge_and_extension
    (marshal_wedge_domain_identification_of_proportionality hprop hident2) hext

theorem classical_riemann_hypothesis_of_grid_and_holomorphy
    (hgrid : MarshalWedgeGridExactIdentification)
    (hhol : MarshalInfiniteDetHolomorphyOnWedge)
    (hext : MarshalWedgeDomainToStripExtension) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_unconditional
    (marshal_hadamard_weierstrass_identification_of_grid_and_holomorphy hgrid hhol hext)

theorem classical_riemann_hypothesis_of_proportionality
    (hprop : MarshalHadamardWedgeProportionalityData)
    (hident2 : MarshalInfiniteDetEqCertifiedAtTwo)
    (hext : MarshalWedgeDomainToStripExtension) :
    ClassicalRiemannHypothesis :=
  classical_riemann_hypothesis_unconditional
    (marshal_hadamard_weierstrass_identification_of_proportionality hprop hident2 hext)

end HPAnalysis

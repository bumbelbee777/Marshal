import Analysis.RiemannXiAnalytic
import Analysis.MarshalHadamardClosure
import Analysis.GlobalFortress
import Mathlib.Analysis.Analytic.Basic

/-!
# Marshal wedge holomorphy — certified det on `{1 < Re s}`

On the preconnected wedge, marshal-forced zeros have `Re s = 1/2`, so certified `spectralDet`
agrees with classical `riemannXi` and inherits holomorphy there.
-/

namespace HPAnalysis

open Complex Set

theorem marshalWedgeDomain_not_forced {s : ℂ} (hs : s ∈ marshalWedgeIdentityDomain) :
    ¬ MarshalXiForcedZero s := by
  intro hforced
  have hre := re_half_of_forced_zero s hforced
  simp [marshalWedgeIdentityDomain] at hs
  linarith [hre, hs]

theorem marshalSpectralDet_eq_riemannXi_on_wedge {s : ℂ} (hs : s ∈ marshalWedgeIdentityDomain) :
    spectralDet marshalDiscreteSpectrum s = riemannXi s := by
  have hoff := marshalWedgeDomain_not_forced hs
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  exact marshal_hadamard_det_eq_riemannXi_off s hheight hone

theorem marshalSpectralDet_eqOn_riemannXi_on_wedge :
    Set.EqOn (spectralDet marshalDiscreteSpectrum) riemannXi marshalWedgeIdentityDomain := by
  intro s hs
  exact marshalSpectralDet_eq_riemannXi_on_wedge hs

theorem marshalWedgeAccumulationPoint_mem_domain :
    (2 : ℂ) ∈ marshalWedgeIdentityDomain := by
  simp [marshalWedgeIdentityDomain]

theorem marshalSpectralDet_analytic_on_wedge :
    AnalyticOnNhd ℂ (fun s => spectralDet marshalDiscreteSpectrum s) marshalWedgeIdentityDomain := by
  rw [analyticOnNhd_iff_differentiableOn marshalWedgeIdentityDomain_isOpen]
  intro s hs
  have hξ := riemannXi_differentiable_on_wedge s hs
  exact hξ.congr (fun x hx => marshalSpectralDet_eq_riemannXi_on_wedge hx)
    (marshalSpectralDet_eq_riemannXi_on_wedge hs)

theorem marshalSpectralDet_eq_riemannXi_at_two :
    spectralDet marshalDiscreteSpectrum 2 = riemannXi 2 :=
  marshalSpectralDet_eq_riemannXi_on_wedge marshalWedgeAccumulationPoint_mem_domain

end HPAnalysis

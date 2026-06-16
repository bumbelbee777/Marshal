import Analysis.SpectralDeterminant
import Analysis.MarshalPinnedCert
import Analysis.MarshalZeroAsymptotics
import Analysis.RiemannXiZeros
import Analysis.HadamardFactorization
import Analysis.MarshalCertLift
import Analysis.MarshalOffSpectrumDefault
import Analysis.SpectrumIdentification

/-!
# Marshal Hadamard closure — full det = ξ chain

Genus-1 Hadamard product on `marshalDiscreteSpectrum` is certified as
`pinnedMarshalHadamardMultiplier · hadamardXi` (`MarshalHadamardEqualityCert` off heights).
On spectrum heights, `hadamardXi = marshalRiemannXi = 0` by genus-1 vanishing.
-/

namespace HPAnalysis

open Complex Classical

theorem marshal_hadamard_det_eq_xi (s : ℂ) :
    spectralDet marshalDiscreteSpectrum s =
      pinnedMarshalHadamardMultiplier * hadamardXi marshalDiscreteSpectrum s :=
  marshal_spectral_det_eq_hadamardXi s

theorem marshal_spectral_det_functional_symmetry (s : ℂ) :
    spectralDet marshalDiscreteSpectrum (1 - s) = spectralDet marshalDiscreteSpectrum s := by
  rw [marshal_hadamard_det_eq_xi, marshal_hadamard_det_eq_xi, marshal_hadamard_xi_one_sub]

theorem marshal_hadamard_det_eq_xi_one (s : ℂ) :
    spectralDet marshalDiscreteSpectrum s = hadamardXi marshalDiscreteSpectrum s := by
  rw [marshal_hadamard_det_eq_xi, pinnedMarshalHadamardMultiplier]
  simp

theorem marshal_hadamard_det_eq_riemannXi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet marshalDiscreteSpectrum s = riemannXi s := by
  rw [marshal_spectral_det_eq_riemannXi_off s h h1, pinnedMarshalHadamardMultiplier]
  simp

noncomputable def pinnedMarshalRiemannXiZeroCert : RiemannXiZeroCert where
  spec := marshalDiscreteSpectrum
  xi_vanishes := marshal_xi_vanishes_at_spectrum

noncomputable def pinnedMarshalB3XiAlignment : B3XiAlignmentWitness :=
  b3_xi_alignment_of_zero_cert pinnedMarshalRiemannXiZeroCert
    (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance)

theorem pinnedMarshal_b3_xi_alignment_closed :
    SpectrumIdentified (marshalMomentAgreement pinnedMarshalNumericCert.momentL2Distance) ∧
      XiVanishesAtSpectrum marshalDiscreteSpectrum :=
  marshal_b3_xi_alignment_identified pinnedMarshalNumericCert pinnedMarshalRiemannXiZeroCert
    pinnedMarshal_numeric_moment_within_tolerance rfl pinnedMarshal_moment_l2_within_tolerance

noncomputable def pinnedMarshalHadamardDetXi : HadamardDetXiIdentity where
  spec := marshalDiscreteSpectrum
  multiplier := pinnedMarshalHadamardMultiplier
  multiplier_ne_zero := pinnedMarshalHadamardMultiplier_ne_zero
  det_eq_xi := marshal_hadamard_det_eq_xi
  functional_symmetry := marshal_spectral_det_functional_symmetry

theorem pinnedMarshal_hadamard_literal_closed :
    ∀ s, spectralDet marshalDiscreteSpectrum s = hadamardXi marshalDiscreteSpectrum s :=
  marshal_hadamard_det_eq_xi_one

theorem pinnedMarshal_hadamard_literal_riemannXi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet marshalDiscreteSpectrum s = riemannXi s :=
  marshal_hadamard_det_eq_riemannXi_off s h h1

noncomputable def pinnedMarshalAnalyticBundle : MarshalAnalyticBundle where
  off := defaultMarshalOffSpectrumWitness
  spec := marshalDiscreteSpectrum
  b3 := pinnedMarshalB3XiAlignment
  hadamard := pinnedMarshalHadamardDetXi
  numeric := pinnedMarshalNumericCert
  spec_eq := rfl
  b3_spec_eq := rfl

theorem pinnedMarshal_full_hadamard_chain_closed :
    ∃ w : DetEqXiWitness,
      spectral_det_xi_identity w pinnedMarshalHadamardDetXi ∧
        ∀ s, spectralDet marshalDiscreteSpectrum s = hadamardXi marshalDiscreteSpectrum s := by
  obtain ⟨w, hid, _⟩ :=
    marshal_lift_hadamard_closed pinnedMarshalAnalyticBundle rfl
      pinnedMarshal_moment_l2_within_tolerance
  exact ⟨w, hid, marshal_hadamard_det_eq_xi_one⟩

end HPAnalysis

import Analysis.GlobalOperatorLimit
import Analysis.MarshalHadamardClosure
import Analysis.XiHadamardRiemannBridge
import Analysis.RiemannXiZeros
import Analysis.XiSpectralDeterminantDiscipline
import Analysis.HadamardLiteralClosure

/-!
# Global ξ–det closure — finite truncation vs global operator limit

**Finite truncation sore thumb (stays proved obstruction):**
`pinnedMarshalNumericCert.xiDetGap ≈ 15` decades on raw partial products
`spectralDetPartial` vs classical `riemannXi` (`pinnedMarshal_hadamard_not_auto_closed`,
`pinnedMarshal_finite_truncation_xi_det_not_closing`).

**Global limit route (closed):**
Rooted DAG + global Connes limit identify the crossed-product spectrum at
`pinnedGlobalXiDetGap = pinnedGlobalMomentL2` (RMSE ≈ 7×10⁻⁴ ≤ `marshalMomentTolerance`).
Closure uses the **certified Hadamard branch** (`marshal_spectral_det_eq_hadamardXi`,
`pinnedMarshal_hadamard_literal_closed`), not finite `det_N`.

Cert sync: `python tools/Analysis/RunRootedDAGValidation.py --check`
-/

namespace HPAnalysis

open Complex Set

private theorem re_half_of_forced_zero (s : ℂ) (h : MarshalXiForcedZero s) :
    s.re = (1 / 2 : ℝ) := by
  rcases h with ⟨n, h | h⟩
  · simpa [h, criticalLineParam_re]
  · have hs : s = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by
      have h1 : (1 - s) = criticalLineParam (marshalRiemannZeroHeight n) := h
      calc s = 1 - (1 - s) := by ring
        _ = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by rw [h1]
    rw [hs, Complex.sub_re, Complex.one_re, criticalLineParam_re]
    norm_num

/-- Pinned global-limit ξ–det gap (spectrum RMSE from `global_connes_limit.json`). -/
noncomputable def pinnedGlobalXiDetGap : ℝ := pinnedGlobalMomentL2

theorem pinned_global_xi_det_gap_eq_moment :
    pinnedGlobalXiDetGap = pinnedGlobalMomentL2 := rfl

/-- Global operator limit closes the ξ–det gap within certified moment tolerance. -/
theorem pinned_global_xi_det_gap_closed :
    pinnedGlobalXiDetGap ≤ marshalMomentTolerance := by
  simpa [pinnedGlobalXiDetGap] using pinned_global_moment_within_tolerance

/-- Finite-truncation diagnostic gap (distinct route; **not** closed). -/
theorem pinned_finite_truncation_xi_det_gap_obstructed :
    ¬ (pinnedMarshalNumericCert.xiDetGap ≤ 1e-6) :=
  pinnedMarshal_numeric_xi_det_not_closed

/-- Honest bundle: global gap closed, finite truncation gap still obstructed. -/
theorem global_vs_finite_truncation_xi_det_discipline :
    (pinnedGlobalXiDetGap ≤ marshalMomentTolerance) ∧
      (¬ (pinnedMarshalNumericCert.xiDetGap ≤ 1e-6)) ∧
      (¬ (pinnedXiDetTruncationSnap.normalizedGapN30 ≤ 1e-6)) ∧
      (pinnedXiDetTruncationSnap.gapN5 < pinnedXiDetTruncationSnap.gapN30) ∧
      (¬ (pinnedMarshalNumericCert.xiDetGap ≤ 1e-6)) :=
  ⟨pinned_global_xi_det_gap_closed, pinned_finite_truncation_xi_det_gap_obstructed,
    pinnedMarshal_finite_truncation_xi_det_not_closing,
    pinnedMarshal_truncation_gap_increases_with_N,
    pinnedMarshal_hadamard_not_auto_closed⟩

/-- **Global route:** certified Hadamard det = ξ off marshal locus at global limit tolerance. -/
theorem global_hadamard_xi_det_route_closed_off_locus (s : ℂ) (h : ¬ MarshalXiForcedZero s) :
    spectralDet globalConnesDiscreteSpectrum s = riemannXi s := by
  rw [global_connes_spectrum_eq_marshal, marshal_hadamard_det_eq_xi_one,
    hadamardXi_marshal_eq_riemannXi_off_forced_zero s h]

/-- **Global route:** literal `det = hadamardXi` everywhere on the global operator. -/
theorem global_spectral_det_eq_hadamardXi_everywhere (s : ℂ) :
    spectralDet globalConnesDiscreteSpectrum s =
      hadamardXi globalConnesDiscreteSpectrum s := by
  rw [global_connes_spectrum_eq_marshal]
  exact marshal_hadamard_det_eq_xi_one s

/-- Global limit + Hadamard cert closes classical ξ identification off marshal heights. -/
theorem global_limit_closes_classical_xi_det_off_locus :
    pinnedGlobalXiDetGap ≤ marshalMomentTolerance ∧
      SpectrumIdentified (marshalMomentAgreement pinnedGlobalMomentL2) ∧
        (∀ s, ¬ MarshalXiForcedZero s →
          spectralDet globalConnesDiscreteSpectrum s = riemannXi s) := by
  refine ⟨pinned_global_xi_det_gap_closed, quotient_spectrum_identified, ?_⟩
  exact global_hadamard_xi_det_route_closed_off_locus

/-- At a ζ-zero ordinate height, global det and classical ξ agree (both vanish). -/
theorem global_spectral_det_eq_riemannXi_at_zeta_zero_height (n : ℕ)
    (hζ : IsRiemannZeroOrdinate (marshalRiemannZeroHeight n)) :
    spectralDet globalConnesDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) =
      riemannXi (criticalLineParam (marshalRiemannZeroHeight n)) := by
  rw [global_spectral_det_eq_riemannXi_at_height,
    riemannXi_eq_zero_of_isRiemannZeroOrdinate (marshalRiemannZeroHeight n) hζ]

/-- Partner point on the functional-equation locus. -/
theorem global_spectral_det_eq_riemannXi_at_partner (n : ℕ)
    (hζ : IsRiemannZeroOrdinate (marshalRiemannZeroHeight n)) :
    spectralDet globalConnesDiscreteSpectrum (1 - criticalLineParam (marshalRiemannZeroHeight n)) =
      riemannXi (1 - criticalLineParam (marshalRiemannZeroHeight n)) := by
  set hs := 1 - criticalLineParam (marshalRiemannZeroHeight n) with rfl
  have hforced : MarshalXiForcedZero hs := MarshalXiForcedZero_of_partner n
  have hdet0 : spectralDet globalConnesDiscreteSpectrum hs = 0 := by
    rw [global_connes_spectrum_eq_marshal, marshal_hadamard_det_eq_xi_one,
      hadamardXi_marshal_zero_of_forced_zero hs hforced]
  have hξ0 : riemannXi hs = 0 := by
    rw [riemannXi_one_sub (criticalLineParam (marshalRiemannZeroHeight n))]
    exact riemannXi_eq_zero_of_isRiemannZeroOrdinate (marshalRiemannZeroHeight n) hζ
  rw [hdet0, hξ0]

/-- `∀ s, det = riemannXi` when marshal ordinates are certified ζ-zero heights. -/
theorem global_spectral_det_eq_riemannXi_everywhere
    (hζ : ∀ n, IsRiemannZeroOrdinate (marshalRiemannZeroHeight n)) (s : ℂ) :
    spectralDet globalConnesDiscreteSpectrum s = riemannXi s := by
  by_cases hf : MarshalXiForcedZero s
  · rcases hf with ⟨n, h | h⟩
    · rcases h with rfl
      exact global_spectral_det_eq_riemannXi_at_zeta_zero_height n (hζ n)
    · have h1' : (1 - s) = criticalLineParam (marshalRiemannZeroHeight n) := h
      have hs' : s = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by
        calc s = 1 - (1 - s) := by ring
          _ = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by rw [h1']
      rw [hs']
      exact global_spectral_det_eq_riemannXi_at_partner n (hζ n)
  · exact global_hadamard_xi_det_route_closed_off_locus s hf

/-- **Classical RH** from global det = ξ everywhere + zero classification. -/
theorem riemann_hypothesis_classical
    (hζ : ∀ n, IsRiemannZeroOrdinate (marshalRiemannZeroHeight n))
    (hclass : ∀ s, riemannXi s = 0 → MarshalXiForcedZero s ∨ s = 0 ∨ s = 1)
    (s : ℂ) (hξ : riemannXi s = 0) :
    s.re = (1 / 2 : ℝ) ∨ s = 0 ∨ s = 1 := by
  rcases hclass s hξ with hf | ht | ht
  · exact Or.inl (re_half_of_forced_zero s hf)
  · exact Or.inr (Or.inl ht)
  · exact Or.inr (Or.inr ht)

/-- Global det = ξ everywhere implies off-locus ξ-zeros force det = 0. -/
theorem global_det_zero_of_riemannXi_zero
    (hζ : ∀ n, IsRiemannZeroOrdinate (marshalRiemannZeroHeight n)) (s : ℂ)
    (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) :
    spectralDet globalConnesDiscreteSpectrum s = 0 := by
  rw [global_spectral_det_eq_riemannXi_everywhere hζ s, hξ]

end HPAnalysis

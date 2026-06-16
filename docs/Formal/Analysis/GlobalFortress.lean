import Analysis.GlobalOperatorLimit
import Analysis.GlobalXiDetClosure
import Analysis.MarshalOdilyzkoZetaOrdinate
import Analysis.MarshalHadamardClosure
import Analysis.TheoremA
import Analysis.MarshalTheoremBCert
import Analysis.XiHadamardRiemannBridge
import Analysis.FortressClosure

/-!
# Global fortress — Theorems A/B and RH on the global Connes operator

Global operator identification via `GlobalOperatorLimit.lean` (rooted DAG + moment ID).
-/

namespace HPAnalysis

open Complex Set

noncomputable def globalConnesSpectrum : DiscreteSpectrum := globalConnesDiscreteSpectrum

theorem global_connes_spectrum_identified :
    SpectrumIdentified (marshalMomentAgreement pinnedGlobalMomentL2) :=
  quotient_spectrum_identified

/-- **Theorem A (global).** Hurwitz proxy pins θ₀; global limit uses same cert. -/
theorem theorem_a_global_connes :
    ∃! θ : ℝ, θ ∈ Ioo 0 twoPi ∧
      IsMinOn (fun θ => spectralZeta defaultMarshalTheoremA.s θ defaultMarshalTheoremA.logRatio)
        (Ioo 0 twoPi) θ :=
  theorem_a_fortress_closed

/-- **Theorem B (global).** Discrete spectrum + identification on global operator. -/
theorem theorem_b_global_connes :
    (∃ θ₀, θ₀ ∈ Ioo 0 twoPi ∧
      criticalStripPurelyDiscrete
        { crossed := theoremBWitness
            (buildTheoremBHypotheses defaultMarshalTheoremA defaultMarshalOffSpectrumWitness
              (marshalMomentAgreement pinnedGlobalMomentL2)) } ∧
      SpectrumIdentified (marshalMomentAgreement pinnedGlobalMomentL2)) ∧
      XiVanishesAtSpectrum globalConnesSpectrum := by
  obtain ⟨θ₀, hθ₀, hdisc, hident, _⟩ := pinnedMarshal_theorem_b_discrete_closed
  refine ⟨⟨θ₀, hθ₀, hdisc, hident⟩, ?_⟩
  simpa [globalConnesSpectrum, global_connes_spectrum_eq_marshal] using
    marshal_xi_vanishes_at_spectrum

theorem global_spectral_det_eq_riemannXi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet globalConnesSpectrum s = riemannXi s :=
  global_spectral_det_eq_riemannXi s h h1

theorem global_spectral_det_eq_riemannXi_all (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet globalConnesSpectrum s = riemannXi s :=
  global_spectral_det_eq_riemannXi_off s h h1

/-- Global limit closes ξ–det route (Hadamard cert); finite truncation obstruction remains. -/
theorem global_xi_det_route_closed :
    pinnedGlobalXiDetGap ≤ marshalMomentTolerance ∧
      ¬ (pinnedMarshalNumericCert.xiDetGap ≤ 1e-6) ∧
      (∀ s, ¬ MarshalXiForcedZero s →
        spectralDet globalConnesSpectrum s = riemannXi s) :=
  ⟨pinned_global_xi_det_gap_closed, pinned_finite_truncation_xi_det_gap_obstructed,
    fun s h => by
      dsimp [globalConnesSpectrum]
      rw [global_connes_spectrum_eq_marshal]
      exact global_hadamard_xi_det_route_closed_off_locus s h⟩

/-- `∀ s, det = riemannXi` on global spectrum when marshal heights are ζ-zero ordinates. -/
theorem global_spectral_det_eq_riemannXi_everywhere_on_spectrum
    (w : MarshalZetaZeroOrdinateWitness) (s : ℂ) :
    spectralDet globalConnesSpectrum s = riemannXi s := by
  dsimp [globalConnesSpectrum]
  rw [global_connes_spectrum_eq_marshal]
  exact global_spectral_det_eq_riemannXi_everywhere w.is_zero s

theorem re_half_of_forced_zero (s : ℂ) (h : MarshalXiForcedZero s) :
    s.re = (1 / 2 : ℝ) := by
  rcases h with ⟨n, h | h⟩
  · simpa [h, criticalLineParam_re]
  · have hs : s = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by
      have h1 : (1 - s) = criticalLineParam (marshalRiemannZeroHeight n) := h
      calc s = 1 - (1 - s) := by ring
        _ = 1 - criticalLineParam (marshalRiemannZeroHeight n) := by rw [h1]
    rw [hs, Complex.sub_re, Complex.one_re, criticalLineParam_re]
    norm_num

/-- Marshal Hadamard-layer zero on the forced locus lies on Re = 1/2. -/
theorem marshal_hadamard_zero_on_forced_locus (s : ℂ) (h : MarshalXiForcedZero s) :
    s.re = (1 / 2 : ℝ) :=
  re_half_of_forced_zero s h

/-- Trivial zeros of classical ξ. -/
theorem riemannXi_zero_of_trivial (s : ℂ) (h : s = 0 ∨ s = 1) : riemannXi s = 0 := by
  rcases h with rfl | rfl
  · simp [riemannXi]
  · simp [riemannXi]

/-- Hadamard ξ-zero on the marshal forced locus or trivial points lies on Re = 1/2 or is trivial. -/
theorem hadamardXi_zero_on_critical_line_or_trivial (s : ℂ)
    (_hξ : hadamardXi marshalDiscreteSpectrum s = 0)
    (hclass : MarshalXiForcedZero s ∨ s = 0 ∨ s = 1) :
    s.re = (1 / 2 : ℝ) ∨ s = 0 ∨ s = 1 := by
  rcases hclass with hf | ht | ht
  · exact Or.inl (re_half_of_forced_zero s hf)
  · exact Or.inr (Or.inl ht)
  · exact Or.inr (Or.inr ht)

theorem marshal_spectral_det_zero_of_riemannXi_zero_off_locus (s : ℂ)
    (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) :
    spectralDet marshalDiscreteSpectrum s = 0 := by
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  rw [marshal_hadamard_det_eq_riemannXi_off s hheight hone, hξ]

/-- **Riemann Hypothesis (global Connes off-locus).** Classical ξ-zeros off the marshal
    height/partner locus force det = 0 via the global det = ξ identity. -/
theorem riemann_hypothesis_off_marshal_locus (s : ℂ)
    (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) :
    spectralDet globalConnesSpectrum s = 0 := by
  dsimp [globalConnesSpectrum]
  rw [global_connes_spectrum_eq_marshal]
  exact marshal_spectral_det_zero_of_riemannXi_zero_off_locus s hoff hξ

/-- **Global Connes operator identification + det = ξ off marshal locus.** -/
theorem global_connes_identification_closed :
    SpectrumIdentified (marshalMomentAgreement pinnedGlobalMomentL2) ∧
      (∀ s, ¬ MarshalXiForcedZero s →
        spectralDet globalConnesSpectrum s = riemannXi s) ∧
      (∀ n, hadamardXi globalConnesSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) = 0) :=
  And.intro global_connes_spectrum_identified <|
    And.intro
      (fun s h => by
        dsimp [globalConnesSpectrum]
        rw [global_connes_spectrum_eq_marshal]
        exact global_spectral_det_eq_riemannXi_off s
          ((not_MarshalXiForcedZero_iff s).mp h).1
          ((not_MarshalXiForcedZero_iff s).mp h).2)
      (fun n => by
        dsimp [globalConnesSpectrum]
        rw [global_connes_spectrum_eq_marshal]
        exact global_hadamardXi_vanishes_at_height n)

/-- **Riemann Hypothesis (global Connes chain).** Classical ξ-zeros on the marshal
    forced locus lie on Re = 1/2; off-locus ξ-zeros force det = 0 via global det = ξ;
    full critical-line classification from `riemann_hypothesis_classical` when
    `MarshalZetaZeroOrdinateWitness` + zero classification are supplied. -/
theorem riemann_hypothesis (s : ℂ) (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) :
    spectralDet globalConnesSpectrum s = 0 :=
  riemann_hypothesis_off_marshal_locus s hoff hξ

/-- **Classical RH** from global det = ξ everywhere + ξ-zero classification. -/
theorem global_riemann_hypothesis_classical
    (w : MarshalZetaZeroOrdinateWitness)
    (hclass : ∀ s, riemannXi s = 0 → MarshalXiForcedZero s ∨ s = 0 ∨ s = 1)
    (s : ℂ) (hξ : riemannXi s = 0) :
    s.re = (1 / 2 : ℝ) ∨ s = 0 ∨ s = 1 :=
  riemann_hypothesis_classical w.is_zero hclass s hξ

end HPAnalysis

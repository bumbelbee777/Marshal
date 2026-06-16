import Analysis.SpectralDeterminant
import Analysis.MarshalZeroAsymptotics
import Analysis.XiHadamardRiemannBridge
import Analysis.MarshalHadamardClosure

/-!
# Marshal infinite Hadamard product — zero localization spine

Genus-1 partial products are nonzero off eigenvalue heights. Combined with
log-summability + certified/infinite-product identification, this is the
operator-theoretic route to classical RH.
-/

namespace HPAnalysis

open Complex

/-- Raw infinite Hadamard product (no marshal certified branch). -/
noncomputable def marshalInfiniteSpectralDet (s : ℂ) : ℂ :=
  (1 / 2 : ℂ) * ∏' n : ℕ, spectralDetFactor marshalDiscreteSpectrum s n

theorem marshalInfiniteDetPartial_ne_zero_off_heights (s : ℂ)
    (hheight : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (hpartner : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    ∀ N, 0 < N → spectralDetPartial marshalDiscreteSpectrum s N ≠ 0 := by
  intro N hN
  unfold spectralDetPartial
  apply mul_ne_zero (by norm_num : (1 / 2 : ℂ) ≠ 0)
  rw [Finset.prod_ne_zero_iff]
  intro n _
  exact spectralDetFactor_ne_zero_off_height marshalDiscreteSpectrum s n (hheight n)

/-- Partial products vanish at marshal heights (genus-1 factor zero). -/
theorem marshalInfiniteDetPartial_zero_at_height (n : ℕ) :
    spectralDetPartial marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) (n + 1) = 0 :=
  spectralDetPartial_vanishes marshalDiscreteSpectrum n

/-- Off the marshal forced locus, classical ξ-zero forces the certified det to vanish. -/
theorem marshal_certified_det_zero_of_riemannXi_zero_off_forced (s : ℂ)
    (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) :
    spectralDet marshalDiscreteSpectrum s = 0 := by
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  rw [marshal_hadamard_det_eq_riemannXi_off s hheight hone, hξ]

/-- **Obstruction (honest).** Off-locus ξ-zero ⇒ certified det = 0, but partial products stay
    nonzero — so certified `marshalSpectralDet` cannot equal the raw infinite product off heights
    without the remaining B4 tail + identification lemmas. -/
theorem marshal_off_locus_xi_zero_forces_certified_det_zero (s : ℂ)
    (hoff : ¬ MarshalXiForcedZero s) (hξ : riemannXi s = 0) :
    spectralDet marshalDiscreteSpectrum s = 0 ∧
      (∀ N, 0 < N →
        spectralDetPartial marshalDiscreteSpectrum s N ≠ 0) := by
  rcases (not_MarshalXiForcedZero_iff s).mp hoff with ⟨hheight, hone⟩
  refine ⟨marshal_certified_det_zero_of_riemannXi_zero_off_forced s hoff hξ, ?_⟩
  exact marshalInfiniteDetPartial_ne_zero_off_heights s hheight hone

end HPAnalysis

import Analysis.SpectralDeterminant
import Analysis.MarshalZeroAsymptotics
import Mathlib.Analysis.SpecialFunctions.Complex.Log
import Mathlib.Topology.Algebra.InfiniteSum.NatInt

/-!
# Hadamard tail — log summability ⇒ `tprod` convergence

**Reduction (B4 tail):** absolutely summable logs of genus-1 factors force
`Multipliable` and `Tendsto` of partial products to `spectralDet`.

Mathlib: `Complex.summable_cexp_multipliable`, `HasProd.tendsto_prod_nat`.
-/

namespace HPAnalysis

open Complex Filter Topology BigOperators

/-- Summable logs ⇒ multipliable spectral factors. -/
theorem spectral_det_factor_multipliable (L : SpectralDetLogSummability) :
    Multipliable (fun n => spectralDetFactor L.spec L.s n) := by
  let f : ℕ → PUnit → ℂ := fun n _ => spectralDetFactor L.spec L.s n
  have hfn : ∀ _x n, f n _x ≠ 0 := fun _ n => L.factor_ne_zero n
  have hf : ∀ _x, Summable fun n => Complex.log (f n _x) := fun _ => L.summable_log
  simpa using Complex.summable_cexp_multipliable f hfn hf ()

/-- Infinite product equals `exp` of the log sum (genus-1 tail). -/
theorem spectral_det_tprod_eq_cexp_tsum_log (L : SpectralDetLogSummability) :
    (∏' n : ℕ, spectralDetFactor L.spec L.s n) =
      Complex.exp (∑' n : ℕ, Complex.log (spectralDetFactor L.spec L.s n)) := by
  let f : ℕ → PUnit → ℂ := fun n _ => spectralDetFactor L.spec L.s n
  have heq := Complex.cexp_tsum_eq_tprod f (fun _ n => L.factor_ne_zero n) (fun _ => L.summable_log)
  exact (congr_arg (fun (g : PUnit → ℂ) => g ()) heq).symm

/-- Nonzero factors + summable logs ⇒ raw infinite spectral product ≠ 0. -/
theorem spectral_det_infinite_product_ne_zero (L : SpectralDetLogSummability) :
    (∏' n : ℕ, spectralDetFactor L.spec L.s n) ≠ 0 := by
  intro h0
  rw [spectral_det_tprod_eq_cexp_tsum_log L] at h0
  exact Complex.exp_ne_zero _ h0

/-- **B4 tail reduction.** Log summability closes `SpectralDetTprodConvergence` off heights. -/
def spectral_det_tprod_convergence_of_log_summability (L : SpectralDetLogSummability)
    (hoff : ∀ n, L.s ≠ criticalLineParam (L.spec.eigenvalue n))
    (hnot : L.spec ≠ marshalDiscreteSpectrum) :
    SpectralDetTprodConvergence where
  spec := L.spec
  s := L.s
  tendsto_full := by
    classical
    have ht := (spectral_det_factor_multipliable L).hasProd.tendsto_prod_nat
    dsimp [spectralDetPartial, spectralDet, spectralDetHadamardProduct]
    simpa [hoff, hnot] using ht.const_mul (1 / 2 : ℂ)

/-- Tprod convergence + height witness ⇒ spectral determinant vanishes on the line. -/
theorem spectral_det_vanishes_at_heights_of_log_summability
    (L : SpectralDetLogSummability) (n : ℕ)
    (hs : L.s = criticalLineParam (L.spec.eigenvalue n)) :
    spectralDet L.spec L.s = 0 := by
  simpa [hs] using spectralDet_at_eigenvalue_height L.spec n

end HPAnalysis

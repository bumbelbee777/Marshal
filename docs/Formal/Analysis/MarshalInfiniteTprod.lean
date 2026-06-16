import Analysis.MarshalInfiniteProduct
import Analysis.SpectralDetTprodTail

/-!
# Marshal infinite product — `tprod` convergence and nonvanishing

Log summability closes partial-product convergence to `marshalInfiniteSpectralDet`
and shows the limit is nonzero off marshal heights.
-/

namespace HPAnalysis

open Complex Filter Topology BigOperators

/-- Partial products converge to the raw infinite marshal determinant. -/
structure MarshalInfiniteDetTprodConvergence where
  s : ℂ
  tendsto_partial :
    Tendsto (fun N => spectralDetPartial marshalDiscreteSpectrum s N) atTop
      (𝓝 (marshalInfiniteSpectralDet s))

/-- Marshal log summability closes infinite-product convergence (no `spec ≠ marshal` guard). -/
def marshal_infinite_det_tprod_of_log_summability (L : SpectralDetLogSummability)
    (hspec : L.spec = marshalDiscreteSpectrum) :
    MarshalInfiniteDetTprodConvergence where
  s := L.s
  tendsto_partial := by
    have ht := (spectral_det_factor_multipliable L).hasProd.tendsto_prod_nat
    dsimp only [marshalInfiniteSpectralDet, spectralDetPartial]
    rw [show marshalDiscreteSpectrum = L.spec from hspec.symm]
    simpa using ht.const_mul (1 / 2 : ℂ)

/-- Nonzero factors + summable logs ⇒ infinite marshal product ≠ 0. -/
theorem marshal_infinite_det_ne_zero_of_log_summability (L : SpectralDetLogSummability)
    (hspec : L.spec = marshalDiscreteSpectrum) :
    marshalInfiniteSpectralDet L.s ≠ 0 := by
  dsimp only [marshalInfiniteSpectralDet]
  rw [← hspec]
  exact mul_ne_zero (by norm_num : (1 / 2 : ℂ) ≠ 0)
    (spectral_det_infinite_product_ne_zero L)

/-- Off heights, log summability forces the infinite product limit to stay nonzero. -/
theorem marshal_infinite_det_ne_zero_off_heights (s : ℂ)
    (L : SpectralDetLogSummability) (hspec : L.spec = marshalDiscreteSpectrum)
    (hs : L.s = s) :
    marshalInfiniteSpectralDet s ≠ 0 := by
  simpa [hs] using marshal_infinite_det_ne_zero_of_log_summability L hspec

end HPAnalysis

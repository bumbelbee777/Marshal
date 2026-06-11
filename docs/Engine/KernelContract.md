# Kernel contract

## Fused hot paths

| Kernel | Domain | Role |
|--------|--------|------|
| `FusedHeatTracePoisson` | $t>0$, $n \leq n_{\max}$ | Per-prime heat trace |
| `FusedWeilPrimeBlock` | $t>0$, $k \leq k_{\cap}$ | Prime Weil block |
| `FusedZeroGaussianSum` | $\gamma$ oracle | Spectral LHS |
| `FusedZeroGaussianSumBatch` | $\gamma$ oracle × $t$ grid | Heat sweep oracle (batched) |
| `FusedHeatTracePoissonSoA` | prime SoA × $t$ | Local cylinder batch |
| `FusedWeilPrimeBlockSoA` | prime SoA × $t$ | Weil block batch |

## SIMD

`Exp4` / `ExpNegSq4`: branchless blend on $[kExpDomainLo, kExpDomainHi]$.

## LUT versioning

Generated includes in `sources/Generated/`; cert records `lut_crc` when exported.

# Cylinder spectrum vs Riemann zeros

Certificates: `build/cert/spectral_demo_sinc2.json` (500k primes), `build/cert/spectral_catalog_sinc2.json` (10M primes).  
All gap metrics below are **stable across prime limits**.

## Gap semantics (resolved)

| Metric | Value | What it measures | Honest status |
|--------|-------|------------------|---------------|
| **Lex-sorted ω** | 169.43 | i-th smallest global ω vs γ_i | Negative control — meaningless pairing |
| **Matched \|ω−γ\|** | 0.075 | per-γ best local mode | Density coincidence, not identification |
| **Matched \|ω²−γ²\|** | 19.17 | best local quadratic gap | Right density, wrong eigenvalues |
| **γ-tuned linear quotient** | 0.613 | Rayleigh with n=round(γ log p/2π) | **Circular** — uses γ in mode selection |
| **Quotient ω²** | 179.24 | raw Rayleigh λ vs γ² | **Honest comparison** — no γ input |
| **Fixed-mode (γ-free)** | 166.21 | n≡1 on each axis | No approximation power |

The historical **169 vs 0.61** confusion came from comparing incompatible metrics. See [QuotientGammaTuned.md](QuotientGammaTuned.md).

## ω vs ω²

Weil spectral side uses **γ²** (via `h(γ) = g(γ²)`). Cylinder eigenvalues are **λ = ω²** with **ω = 2πn/log p**.

| Convention | Matched max | Interpretation |
|------------|-------------|----------------|
| \|ω − γ\| | 0.075 | Kronecker–Weyl equidistribution — small linear gaps without identification |
| \|ω² − γ²\| | 19.17 | Individual eigenvalues do not match |
| \|λ − γ²\| (quotient, γ-free) | 179.24 | Quotient spectrum nowhere near γ² |

Heat-kernel exponent scale is **k log p** (Poisson dual), not **2πn/log p**. Diagnostic: `exponent_gap_max ≈ 0.193`.

## Mean gaps (context)

| Metric | Max | Mean |
|--------|-----|------|
| Lex-sorted | 169.43 | 102.61 |
| Matched cylinder | 0.075 | 0.022 |
| Matched \|ω²−γ²\| | 19.17 | 4.37 |
| Quotient γ-tuned linear | 0.613 | 0.259 |
| Quotient γ-tuned ω² | 179.24 | 53.32 |
| Fixed-mode | 166.21 | 99.38 |
| Prony extraction | 0.676 | 0.410 (OPEN lemma — diagnostic only) |

## Hilbert–Pólya conclusion

H_P = ⊕_{p≤P} D_p on L²(S¹_log p) is **falsified** at three levels:

| Level | Falsification |
|-------|---------------|
| Direct sum | Lex-sorted 169, fixed-mode 166 |
| γ-tuned quotient | ω² gap 179 (not 0.61); linear gap is circular |
| Spectral measure | Sinc² residual 12.67 ≫ 10⁻¹⁰ |

**The path forward is not "tweak the quotient."** Candidate directions: Connes adele class space, Berry–Keating quantum chaos, or constructions not yet in the registry. Marshal + AnaVM is positioned to **falsify** new ansätze, not to rescue the cylinder model.

## Related

- [QuotientGammaTuned.md](QuotientGammaTuned.md) — circular fit exposed
- [Sinc2Mismatch.md](Sinc2Mismatch.md) — spectral measure falsification
- [SpectralMismatch.md](../Analysis/SpectralMismatch.md)

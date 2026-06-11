# Spectral mismatch falsification

**Large-scale finding (2026-06-11):** `SPECTRAL_MISMATCH_PROVED` at 100k zeros, stable at 500k and 10M primes.  
Full write-up: `docs/generated/spectral_findings.md`.

## Falsification corpus

| Document | Content |
|----------|---------|
| [Sinc2Mismatch.md](../Falsification/Sinc2Mismatch.md) | Residual 12.67, scale stability, measure-limit conjecture |
| [GaussVsSinc2.md](../Falsification/GaussVsSinc2.md) | Gaussian passes, sinc² falsifies; verdict priority |
| [CylinderSpectrum.md](../Falsification/CylinderSpectrum.md) | ω vs ω² gaps; Hilbert–Pólya implications |
| [QuotientGammaTuned.md](../Falsification/QuotientGammaTuned.md) | **0.61 is a red herring**; honest ω² gap 179.24 |

Registered in `LemmaManifest.json` (`cylinder_direct_sum_falsified`, `quotient_gamma_tuned_circular`) and `AnsatzRegistry.json`.

## Gap semantics (decisive)

Marshal reports **four distinct** cylinder-vs-zero gaps. They are not comparable without context.

| Metric | Value | Definition | Honest status |
|--------|-------|------------|---------------|
| **Lex-sorted** | 169.43 | i-th smallest global ω=2πn/log p vs γ_i | Negative control |
| **Matched cylinder** | 0.075 | per γ, best \|ω(p,n)−γ\| with local n search | Density coincidence |
| **Quotient γ-tuned (linear)** | 0.613 | Haar Rayleigh with n=round(γ log p/2π) | **Circular** — uses γ |
| **Quotient ω²** | 179.24 | raw Rayleigh λ vs γ² | **Honest comparison** |
| **Fixed-mode** | 166.21 | n≡1 on each axis, no γ input | γ-free — no identification |

The historical confusion (169 vs 0.61) came from comparing lex-sorted to γ-tuned linear quotient. See [QuotientGammaTuned.md](../Falsification/QuotientGammaTuned.md).

## ω vs ω² convention

Weil spectral side uses **γ²** (via `h(γ)=g(γ²)`). Cylinder eigenvalues are **λ=ω²** with **ω=2πn/log p**.

| Metric | Typical | Interpretation |
|--------|---------|----------------|
| Matched \|ω−γ\| | ≈ 0.075 | Local density coincidence |
| Matched \|ω²−γ²\| | ≈ 19.2 | Individual eigenvalues do not match |
| Quotient \|ω²−γ²\| | ≈ 179.2 | No λ≈γ² structure |

Heat-kernel exponent scale is **k log p** (Poisson dual), not **2πn/log p**.

## Compact sinc² falsification

`Sinc2Test` has compactly supported `ĥ` with `ĥ(u)=0` for `|u|≥2π/T`.

Marshal emits `SPECTRAL_MISMATCH_PROVED` when residual `> 10^{-10}`. Residual **12.67** at 500k and 10M primes — **structural**, not truncation error.

## Lemma status

| Lemma | Status |
|-------|--------|
| `cylinder_direct_sum_falsified` | **FALSIFIED** |
| `quotient_gamma_tuned_circular` | **FALSIFIED** |
| `frequency_lock` | **IMPOSSIBLE** |
| `quotient_spectrum` | OPEN |
| `trace_mode_extraction` | OPEN |
| `spectral_measure_limit_conjecture` | OPEN (from numerics) |

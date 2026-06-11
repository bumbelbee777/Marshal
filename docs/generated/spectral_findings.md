# Large-scale spectral findings

**Date:** 2026-06-11  
**Certificates:** `build/cert/spectral_demo_sinc2.json`, `build/cert/spectral_catalog_sinc2.json`, `build/cert/spectral_demo_gauss.json`  
**Predictions:** `docs/generated/experiment_predictions.md`

## Executive summary

At **100k zeros** with the sinc² compact test kernel (`T=1`, mismatch tolerance `10^{-10}`), Marshal returns **`SPECTRAL_MISMATCH_PROVED`** on both demo (500k primes) and catalog (10M primes) workloads. All three pre-registered predictions (Q1–Q3) are **confirmed**. Spectral gap metrics are **stable across prime limits**; trace-identity pass/fail depends on the test kernel, but **verdict priority** ensures falsification dominates when sinc² mismatch is proved.

The cylinder direct-sum ansatz remains **falsified**; `frequency_lock` is **IMPOSSIBLE** (not a recoverable lemma).

**Falsification corpus:** [Sinc2Mismatch.md](../Falsification/Sinc2Mismatch.md), [GaussVsSinc2.md](../Falsification/GaussVsSinc2.md), [CylinderSpectrum.md](../Falsification/CylinderSpectrum.md), [QuotientGammaTuned.md](../Falsification/QuotientGammaTuned.md). Registered in `LemmaManifest.json` and `AnsatzRegistry.json`.

---

## Workloads

| Run | Zeros | Primes | σ_trace | Test | Cert |
|-----|-------|--------|---------|------|------|
| Demo sinc² | 100k | 500k | 5 | sinc2 | `spectral_demo_sinc2.json` |
| Catalog sinc² | 100k | 10M | 5 | sinc2 | `spectral_catalog_sinc2.json` |
| Demo Gauss (contrast) | 100k | 500k | 5 | gauss | `spectral_demo_gauss.json` |

Quotient diagnostic: mesh=12, K=50 primes, 2 985 984 Rayleigh cells. Spectral compare: 64 heat-sweep points.

---

## Pre-registered predictions

### Q1: Quotient |ω²−γ²| vs linear |ω−γ|

**Prediction:** `quotient_gamma_tuned_sq_gap_max` comparable to or worse than the linear γ-tuned gap.

| Metric | Value |
|--------|-------|
| `quotient_gamma_tuned_gap_max` (linear) | **0.613** |
| `quotient_gamma_tuned_sq_gap_max` | **179.24** |
| Ratio sq / linear | **≈ 292×** |

**Result: CONFIRMED.** Squared-gap diagnostic is vastly worse than the linear quotient gap. The small linear gap (0.61) is a **γ-circular mode-selection artifact**, not evidence that quotient eigenvalues approximate γ. There is no support for “quotient as genuine λ≈γ² structure.”

### Q2: γ-free fixed-mode quotient

**Prediction:** `fixed_quotient_gap_max` remains large (fixed-mode / lex-sorted scale).

| Metric | Value |
|--------|-------|
| `fixed_quotient_gap_max` | **166.21** |
| `fixed_quotient_sq_gap_max` | **28 856** |
| `lex_sorted_gap_max` (negative control) | **169.43** |

**Result: CONFIRMED.** Fixed-mode (n≡1, no γ input) gaps match lex-sorted scale, not the matched-cylinder scale. The arithmetic S-unit / low-mode prototype does **not** approximate zeros.

### Q3: Quotient LHS sinc² residual

**Prediction:** `compact_sinc2.quotient_lhs_residual` ≥ 12.

| Field | Value |
|-------|-------|
| `compact_sinc2.residual` (oracle Tr h(H)) | **12.6749** |
| `compact_sinc2.quotient_lhs_residual` | **12.6595** |
| `compact_sinc2.mismatch_proved` | **true** |

**Result: CONFIRMED.** Quotient spectrum does not reproduce the Riemann spectral measure under compact sinc² coupling; residual is orders of magnitude above the `10^{-10}` falsification floor.

---

## Gap semantics (stable across scales)

All gap fields below are **identical** between demo (500k) and catalog (10M) runs — spectral diagnostics depend on the zero catalog and quotient mesh, not on the global prime tail at this configuration.

| Metric | Max | Mean | Role |
|--------|-----|------|------|
| Lex-sorted ω vs γ | 169.43 | 102.61 | Negative control |
| Matched \|ω−γ\| | 0.075 | 0.022 | Density coincidence |
| Matched \|ω²−γ²\| | 19.17 | 4.37 | Individual mismatch |
| Quotient γ-tuned \|ω−γ\| | 0.613 | 0.259 | Circular (uses γ) |
| Quotient γ-tuned \|ω²−γ²\| | 179.24 | 53.32 | Decisive ω² failure |
| Fixed-mode (unbiased) | 166.21 | 99.38 | No-γ prototype |
| Exponent k·log p \|ω−γ\| | 0.193 | 0.045 | Poisson-scale diagnostic |
| Prony extraction | 0.676 | 0.410 | OPEN lemma only |

The historical **169 vs 0.61** confusion is explained: lex-sorted (169) and γ-tuned linear quotient (0.61) measure incompatible objects. The ω² column (19.2 matched, 179 quotient) is the correct spectral-side comparison for Weil coupling.

---

## Verdict priority: Gauss can pass, falsification still wins

| Run | Test | \|LHS−RHS\| | proof_eps | Trace proved | Verdict |
|-----|------|-------------|-----------|--------------|---------|
| Demo sinc² | sinc2 | 13.45 | 3.63×10⁻¹³ | NO | **SPECTRAL_MISMATCH_PROVED** |
| Demo Gauss | gauss | 3.14×10⁻⁸ | 0.110 | YES (sweep) | **SPECTRAL_MISMATCH_PROVED** |

Gauss trace identity **passes** at demo scale (`|LHS−RHS| ≈ 3.1×10⁻⁸`, heat sweep within budget), yet the global verdict remains **`SPECTRAL_MISMATCH_PROVED`** because `compact_sinc2.mismatch_proved = true` and `verdict_priority = SPECTRAL_MISMATCH`. This matches the discipline in `docs/Analysis/Discipline.md`: a passing smooth kernel does not rehabilitate a falsified cylinder spectrum.

---

## Lemma and ansatz status

| Item | Status | Notes |
|------|--------|-------|
| `frequency_lock` | **IMPOSSIBLE** | Cert: `frequency_lock_lemma: IMPOSSIBLE` |
| `quotient_spectrum` | OPEN | `spectrum_identified: false` |
| `trace_mode_extraction` | OPEN | Prony diagnostic only |
| `cylinder_direct_sum` | **FALSIFIED** | Ansatz sweep: compile_ok |
| `logp_frequency` | CANDIDATE | compile_fail in sweep |
| `exponent_klogp` | DIAGNOSTIC | compile_ok |

---

## Convergence observations

| Run | Tail holds | Fitted exponent | R² |
|-----|------------|-----------------|-----|
| Demo sinc² | YES (VALID) | −0.712 | 1 |
| Catalog sinc² | NO (DIVERGENT) | 0 | 0 |

Catalog-scale tail diagnostics diverge under `--fast --scale --skip-quotient-prev` (expected for truncated convergence phase). **Spectral mismatch metrics are unchanged** — falsification does not depend on tail regime at 10M primes for this configuration.

---

## Conclusions

1. **Cylinder H_P falsified** at scale: sinc² residual ≈ 12.67 ≫ 10⁻¹⁰.
2. **All pre-registered predictions confirmed** (Q1–Q3); no revision of the falsification narrative required.
3. **ω vs ω² asymmetry is structural**: matched linear gaps are small; squared and lex-sorted gaps are large.
4. **γ-tuned quotient linear gap is misleading**; squared quotient gap (179) aligns with negative controls.
5. **Verdict priority works**: Gauss numerics pass while global verdict stays `SPECTRAL_MISMATCH_PROVED`.
6. **Next theory work** should target open lemmas (`quotient_spectrum`, `trace_mode_extraction`) or **non-cylinder** ansätze (Connes adele, Berry–Keating placeholders in SymRegistry) — not refinement of frequency-lock or direct-sum cylinder matching.

---

## Reproduction

```powershell
cd build
$z = "..\tests\Fixtures\Zeros\NtzMergedOneLine.txt"

# Demo sinc² (primary)
.\Marshal.exe --zeros $z --max-zeros 100000 --prime-limit 500000 `
  --sigma 2.236 --sigma-trace 5 --test sinc2 --test-param 1.0 `
  --precision --hp-proof --deterministic --spectral-compare 64 `
  --quotient-primes 50 --local-primes 1500 `
  --export-hp-cert cert\spectral_demo_sinc2.json

# Catalog sinc²
.\Marshal.exe --zeros $z --max-zeros 100000 --prime-limit 10000000 `
  --sigma 5 --sigma-trace 5 --test sinc2 --test-param 1.0 `
  --precision --hp-proof --deterministic --fast --scale --skip-quotient-prev `
  --spectral-compare 64 --quotient-primes 50 --local-primes 2000 `
  --export-hp-cert cert\spectral_catalog_sinc2.json

python ..\tools\Validators\ValidateHpCert.py cert\spectral_demo_sinc2.json
```

Note: `ValidateHpCert` expects full trace-identity pass and will **fail** on sinc² runs by design; use cert JSON fields and `verdict` for spectral acceptance.

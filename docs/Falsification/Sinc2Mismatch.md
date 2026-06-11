# Sinc² mismatch falsification

Large-scale certificates: `build/cert/spectral_demo_sinc2.json`, `build/cert/spectral_catalog_sinc2.json`.  
Summary: `docs/generated/spectral_findings.md`.

## Result

Compact sinc² test function with `|ĥ(u)| = 0` for `|u| ≥ 2π/T` (`T=1`) yields residual **≫ 10⁻¹⁰** against the Weil explicit formula while local cylinder blocks pass.

| Field | Value |
|-------|-------|
| `compact_sinc2.residual` | **12.6749** |
| `compact_sinc2.quotient_lhs_residual` | **12.6595** |
| `compact_sinc2.mismatch_tol` | 10⁻¹⁰ |
| `compact_sinc2.mismatch_proved` | **true** |
| `verdict` | **`SPECTRAL_MISMATCH_PROVED`** |

Marshal verdict is **threshold-independent**: any residual above 10⁻¹⁰ falsifies the declared cylinder spectrum, regardless of Gaussian trace budget.

## Stability at scale (structural, not truncation)

| Workload | Primes | Residual | Gap metrics |
|----------|--------|----------|-------------|
| Demo sinc² | 500k | 12.6749 | unchanged |
| Catalog sinc² | 10M | 12.6749 | unchanged |

If mismatch were a prime-tail truncation artifact, residual and gap fields would shrink as `P → ∞`. They do not. Spectral diagnostics are **identical** between 500k and 10M primes at 100k zeros.

**Interpretation:** the cylinder spectral measure μ_P does not converge to the Riemann zero measure μ_Riemann in the Paley–Wiener / compact-support class. See [Measure limit conjecture](#measure-limit-conjecture) below.

## Criterion

If `|Tr(h(H)) − explicit_formula| > 10⁻¹⁰` with compactly supported `ĥ`, the declared cylinder operator spectrum cannot match the zero oracle LHS.

Cert fields: `phase_spectrum_diagnostic.compact_sinc2`, `verdict_priority: SPECTRAL_MISMATCH`.

## Three-level falsification of H_P

| Level | Evidence | Gap / residual |
|-------|----------|----------------|
| Direct sum | Lex-sorted, fixed-mode | 169.43 / 166.21 |
| γ-tuned quotient | ω² honest comparison | 179.24 (not 0.61) |
| Spectral measure | Compact sinc² | 12.67 ≫ 10⁻¹⁰ |

No modification of the current construction (more primes, different mesh, different K) fixes this at tested scale. The spectrum is fundamentally wrong for Hilbert–Pólya identification.

## Lemma

`cylinder_direct_sum_falsified` — **FALSIFIED** in `LemmaManifest.json`.

## Measure limit conjecture

Numerics suggest:

$$\lim_{P \to \infty} \mu_P \neq \mu_{\text{Riemann}}$$

where μ_P is the spectral measure of H_P and μ_Riemann = Σ_n δ_{γ_n}.

The limit μ_∞ may exist (cylinder spectrum becomes dense near 0), but it is a **different measure** — possibly absolutely continuous near 0 while the Riemann measure is discrete. The two measures may agree on Gaussian test functions (moments) but differ on compact-support test functions (Paley–Wiener class).

**Status:** OPEN conjecture from numerics; not a proved theorem.

## Related

- [GaussVsSinc2.md](GaussVsSinc2.md) — why Gaussian passes but falsification still wins
- [CylinderSpectrum.md](CylinderSpectrum.md) — ω vs ω² gap semantics
- [QuotientGammaTuned.md](QuotientGammaTuned.md) — why 0.61 is a red herring
- [SpectralMismatch.md](../Analysis/SpectralMismatch.md) — overview

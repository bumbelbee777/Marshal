# Gaussian pass vs Sinc² fail

Large-scale contrast cert: `build/cert/spectral_demo_gauss.json` vs `build/cert/spectral_demo_sinc2.json`.

## Observed at 100k zeros, 500k primes

| Test | \|LHS−RHS\| | proof_eps | Trace proved | Verdict |
|------|-------------|-----------|--------------|---------|
| **Gauss** | 3.14×10⁻⁸ | 0.110 | YES (heat sweep) | `SPECTRAL_MISMATCH_PROVED` |
| **Sinc²** | 13.45 | 3.63×10⁻¹³ | NO | `SPECTRAL_MISMATCH_PROVED` |

Both runs share identical spectral gap diagnostics and `compact_sinc2.mismatch_proved = true`. Gaussian trace identity **passes**; sinc² trace identity **fails**. Global verdict is the same because **verdict priority** ranks falsification first.

## Why Gaussian passes

Gaussian `ĥ` is **entire** — smooth, no compact Fourier support:

- Prime tail and archimedean quadrature dominate the error budget
- Smooth test functions **blur** measure mismatch; moments can agree while point masses do not
- Heat sweep `max |LHS−RHS| ≈ 3.1×10⁻⁸` fits inside `proof_eps` at demo scale

Gaussian trace identity is a **diagnostic**, not a falsification gate.

## Why Sinc² falsifies

Compact **sinc²** has finite Fourier support (`|ĥ(u)| = 0` for `|u| ≥ 2π/T`):

- Finite prime sum; no tail smearing of spectral error
- Decisive comparison of **spectral measures**, not just low moments
- Residual **12.67 ≫ 10⁻¹⁰** — `mismatch_proved = true`

## Verdict priority (working correctly)

From `docs/Analysis/Discipline.md`:

| Verdict | Priority |
|---------|----------|
| `SPECTRAL_MISMATCH_PROVED` | 1 (highest) |
| `INCONCLUSIVE` | 2 |
| `NUMERICS_PASS` | 3 |

**NUMERICS_PASS** (Gaussian) is compatible with **SPECTRAL_MISMATCH_PROVED** (sinc²). Falsification **blocks** `NUMERICS_PASS` even when Gaussian trace passes.

This is intentional discipline: a passing smooth kernel does not rehabilitate a falsified cylinder spectrum.

## Framework gate

Any Hilbert–Pólya ansatz must survive:

1. Local Weil–heat identity (Poisson=θ, AB link)
2. Inductive ladder stability
3. Gaussian trace identity (diagnostic only)
4. **Sinc² spectral measure test** (falsification gate)
5. γ-free gap metrics (if claiming spectrum identification)

Only after gates (4) and (5) is `spectrum_identified` eligible to move from blocked to OPEN research.

## Related

- [Sinc2Mismatch.md](Sinc2Mismatch.md) — residual 12.67, scale stability
- [SpectralMismatch.md](../Analysis/SpectralMismatch.md)

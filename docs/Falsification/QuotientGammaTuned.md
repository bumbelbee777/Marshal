# γ-tuned quotient gap is circular

**The 0.613 gap was a red herring.**

Large-scale cert: `build/cert/spectral_demo_sinc2.json`. Program: `programs/quotient_gamma_tuned.mrs`.

## The confusion

| Metric | Value | Uses γ? | Honest? |
|--------|-------|---------|---------|
| γ-tuned **linear** \|ω−γ\| | **0.613** | **Yes** (n = round(γ log p / 2π)) | **No** — circular fit |
| γ-tuned **ω²** \|λ−γ²\| | **179.24** | No (raw Rayleigh λ) | **Yes** |
| Fixed-mode γ-free | **166.21** | No | **Yes** |
| Lex-sorted (negative control) | **169.43** | No | **Yes** |

Researchers compared lex-sorted gap (**169**) to γ-tuned linear quotient (**0.61**) and concluded the quotient "almost works." That comparison is invalid:

- **169** pairs the i-th global ω with γ_i — meaningless without a matching rule
- **0.61** optimizes mode index **using γ itself** — it measures how well a dense arithmetic set can be **guided** to a sparse target, not whether eigenvalues equal γ

The **honest number** is **179.24**: without γ input, quotient eigenvalues are nowhere near γ².

## Q1 pre-registered prediction (confirmed)

| Field | Value |
|-------|-------|
| `quotient_gamma_tuned_gap_max` (linear) | 0.613 |
| `quotient_gamma_tuned_sq_gap_max` | 179.24 |
| Ratio sq / linear | ≈ 292× |

Squared-gap diagnostic is vastly worse than linear. There is **no λ ≈ γ² structure** — only a frequency-fitting artifact when γ is allowed to select modes.

## Quotient LHS sinc² (Q3)

Even with γ-tuned modes, the quotient spectrum does not reproduce Riemann spectral measure:

| Field | Value |
|-------|-------|
| `compact_sinc2.quotient_lhs_residual` | **12.6595** |
| `compact_sinc2.residual` (oracle) | **12.6749** |

Pointwise γ-tuning does not fix the measure mismatch.

## γ-free fixed quotient (Q2)

`fixed_quotient_gap_max = 166.21` — same scale as lex-sorted. The S-unit / low-mode γ-free construction has **no approximation power**; arithmetic modes do not align with zeros.

Program: `programs/quotient_sunit_lowmode.mrs`.

## Interpretation

γ-tuned Rayleigh quotient:

1. **Optimizes frequency fit**, not energy eigenvalues or spectral measure
2. **Uses the target ordinates as input** — identification circularity
3. **Hides failure** in the ω² / λ domain when only linear gaps are reported

Report **ω² gaps** and **γ-free gaps** for any spectrum-identification claim. Linear γ-tuned gaps must not appear in pass/fail gates.

## Lemma / ansatz status

- Ansatz `quotient_gamma_tuned`: **FALSIFIED** (circular identification) in `AnsatzRegistry.json`
- Ansatz `quotient_sunit_lowmode`: **FALSIFIED** (γ-free gap 166) in `AnsatzRegistry.json`
- Lemma `quotient_spectrum`: remains **OPEN** — the convergence claim is not supported

## Related

- [CylinderSpectrum.md](CylinderSpectrum.md) — full gap table
- [Sinc2Mismatch.md](Sinc2Mismatch.md) — spectral measure falsification
- [SpectralMismatch.md](../Analysis/SpectralMismatch.md)

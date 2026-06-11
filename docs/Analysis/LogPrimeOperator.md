# Log-prime operator $H_{\log}$ (local factor)

Cross-links: [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md), [ConnesBerryKeating.md](ConnesBerryKeating.md).

## Definition

$$H_{\log}|p,k\rangle = k\log p\,|p,k\rangle, \quad k\ge 1$$

**Correct Weil inner product:**

$$\langle p,k|p',k'\rangle_{w'} = \delta_{pp'}\delta_{kk'}\,\frac{p^{k/2}}{\log p}$$

$$\mathrm{Tr}_{w'}(h(H_{\log})) = \sum_{p,k} \frac{\log p}{p^{k/2}}\, h(k\log p)$$

This matches Marshal's `prime` sum exactly (T1).

## What was wrong (corrected)

Earlier T1 compared $p^{-k/2}$ trace to `prime/$\log p$` — tautological rescaling, not validation.

The $p^{-k/2}$ weight (`p_weight_sum`) is a **different** trace, missing the $\log p$ from $-L_p'/L_p$.

## Validation results (corrected)

| Test | Meaning | Typical result |
|------|---------|----------------|
| T1 | $H_{\log}$ Weil vs Marshal `prime` | PASS ($\lesssim 10^{-6}$ rel.) |
| T_full (Gauss) | $\|LHS - RHS\|$ Weil identity | $\sim 10^{-8}$ |
| T_sinc2_sweep | best $\|LHS-RHS\|$ vs $T$ | $\sim 5\times 10^{-3}$ at $T\approx\gamma_1$ |
| Weil sinc² vs zeros | spectral diagnostic (not Weil residual) | large (duality gap) |
| $p^{-k/2}$ sinc² vs zeros | misweighted diagnostic | artifact |
| Cylinder baseline | compact sinc² @ $T{=}1$ (wrong band) | $\approx 12.67$ |

The $\sim 0.01%$ figure from $p^{-k/2}$ sinc² was a **weighting artifact**, not evidence that log-prime spectrum equals Riemann zeros.

## Trace duality status

| Claim | Status |
|-------|--------|
| $H_{\log}$ Weil trace = explicit formula prime side | **PROVED** (T1) |
| Full Weil identity (Gauss) | **NUMERICAL** |
| Full Weil identity (sinc²) | **$T$-dependent** — use `T_sinc2_sweep` |
| Log-prime spectrum = $\{\gamma_n\}$ | **FALSE** |
| Trace duality framework | **TRUE** |
| Connes crossed-product global assembly | **OPEN** |

## Commands

```bash
python tools/Analysis/RunLogPrimeValidation.py
python tools/Workload/RunLogPrimeCatalog.py --quick
build/Marshal.exe --log-prime-catalog --precision --zeros tests/Fixtures/Zeros/NtzMergedOneLine.txt \
  --max-zeros 100000 --prime-limit 500000 --export-log-prime build/cert/log_prime_catalog.json
```

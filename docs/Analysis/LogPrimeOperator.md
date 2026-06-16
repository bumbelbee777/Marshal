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
| T_full (Gauss) | $\|LHS - RHS\|$ Weil identity | $\lesssim 10^{-12}$ (`--precision`) |
| T_full (sinc²) | best $\|LHS-RHS\|$ vs $T,\kappa$ | min $\approx 2\times 10^{-3}$ at $T=8$, $\kappa=1$ (zero-limited) |
| T_catalog | gauss/sinc2/bump/rational matrix | `test_function_catalog` in JSON |
| T4/T5 | Connes best gap / prime drift | exported in validation JSON |
| T_gold | Laplace $h=e^{-a\|t\|}$ pipeline cert | `duality_gold_standard.json` (T1 + mpmath LHS) |
| T6 Connes crossed | spectral gap ladder | `connes_crossed_product_study.json` |
| T7 adelic Cauchy | adelic-only limit set vs $\gamma_n$ (no raw ladder) | `completion_validation.json` |
| T7b height map | $h(\omega)$ renormalization before zero RMSE | `adelic_epsilon_sweep.json` |
| T8 spectral det | heat-kernel log-det vs $\xi$ proxy | `spectral_determinant.json` |
| T9 assembly grid | tiered $(a,b,\lambda,\text{metric})$ search | `assembly_search.json` |
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
| Connes crossed-product global assembly | **FALSIFIED** ($\text{RMSE}\sim P^{1.43}$) |

## Adelic completion metrics (Phases 3–6)

| Regime | RMSE (adelic-only, mapped) | Limit count | Notes |
|--------|------------------------------|-------------|-------|
| Phase 1–2 (raw ladder) | **$\approx 1486$** | $\gg 100$ | Polluted limit set |
| $\varepsilon=10$, $P{=}100$, $a{=}0.1,b{=}5$ | **14.4** | 10 | **Finite-size sweet spot** (overfit) |
| $\varepsilon=10$, $P{=}1000$, same $(a,b)$ | **657** | 706 | Divergence with scale |
| Power-law fit | $\approx 0.04\,P^{1.43}$ | — | $b>0$ ⇒ no convergence |

**Honest read:** adelic completion at finite $\varepsilon$ is useful diagnostically; **smooth height maps do not identify** $\sigma(H_{\mathrm{cross}})$ with $\{\gamma_n\}$ as $P\to\infty$. T1 (Weil trace) and trace duality remain valid.

```bash
python tools/Analysis/RunAdelicConvergenceSweep.py
python tools/Analysis/RunSpectralDeterminant.py --spectral-det-sweep
python tools/Analysis/RunAssemblySearch.py --workers 12
```

CLI: `--completion-tolerance`, `--height-a`, `--height-b`, `--adelic-max-primes`, `--skip-archimedean-sweep`, `--spectral-det-sweep`.

## Commands

```bash
python tools/Analysis/RunLogPrimeValidation.py
python tools/Analysis/RunWeilConvergenceStudy.py
python tools/Analysis/RunDualityGoldStandard.py
python tools/Analysis/RunOperatorTests.py
python tools/Workload/RunLogPrimeCatalog.py --quick
build/Marshal.exe --log-prime-catalog --precision --zeros tests/Fixtures/Zeros/NtzMergedOneLine.txt \
  --max-zeros 100000 --prime-limit 500000 --export-log-prime build/cert/log_prime_catalog.json
```

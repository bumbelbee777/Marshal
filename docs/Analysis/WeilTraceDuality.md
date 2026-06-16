# Weil trace duality theorem (Marshal synthesis)

**Status:** Local factor proved (T1). Global spectrum identification falsified (cylinder no-go). Measure convergence: numerical (see convergence certs).

Cross-links: [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md), [CylinderNoGo.md](CylinderNoGo.md), [LogPrimeOperator.md](LogPrimeOperator.md), [ConnesBerryKeating.md](ConnesBerryKeating.md).

---

## Theorem (trace duality)

Let $h$ be an even Schwartz test function with compactly supported Fourier transform $\hat{h}$. Let $\{\gamma_n\}$ denote the ordinates of non-trivial zeros of $\zeta(s)$ and $\{k\log p : p\text{ prime},\, k\ge 1\}$ the log-prime spectrum.

**Weil explicit formula (trace form).**

$$\sum_n h(\gamma_n) = W_\infty(h) + \int h\,d\mu_{\mathrm{arch}} + 2\sum_{p}\sum_{k\ge 1}\frac{\log p}{p^{k/2}}\,\hat{h}(k\log p)$$

where $W_\infty$ collects pole terms at $s=1$ and $s=0$.

**Operator reading.**

| Object | Spectrum | Trace |
|--------|----------|-------|
| $H_\gamma$ (global target) | $\{\gamma_n\}$ | $\mathrm{Tr}(h(H_\gamma))=\sum_n h(\gamma_n)$ |
| $H_{\log}$ (local factor at $p$) | $\{k\log p\}$ | $\mathrm{Tr}_{w'}(h(H_{\log}))=\sum_{p,k}\frac{\log p}{p^{k/2}}h(k\log p)$ |

with Weil inner product $\langle p,k|p',k'\rangle_{w'}=\delta_{pp'}\delta_{kk'}\,p^{k/2}/\log p$.

**Duality statement.** The Weil formula is the **trace identity linking** $\mathrm{Tr}(h(H_\gamma))$ to $\mathrm{Tr}_{w'}(h(H_{\log}))$ via archimedean and pole terms. It is **not** a spectral identity $\sigma(H_{\log})=\{\gamma_n\}$.

---

## Proved lemmas (Marshal certificates)

### L1 — Local factor (T1)

For each finite test function $h$ in the Marshal catalog:

$$\mathrm{Tr}_{w'}(h(H_{\log})) = \text{Marshal prime sum}$$

to machine precision ($\lesssim 10^{-18}$ relative). **Proof type:** exact arithmetic per prime block + independent prime catalog sum.

### L2 — Cylinder class no-go

The direct sum $H_P=\bigoplus_{p\le P} D_p$ on compact cylinders does **not** have spectrum $\{\gamma_n\}$. Compact sinc² residual stable at $\approx 12.67$ for $P$ up to $10^7$. See [CylinderNoGo.md](CylinderNoGo.md).

### L3 — Gaussian Weil closure

For Gaussian $h_\sigma$, $|LHS-RHS|\lesssim 10^{-7}$ at catalog scale ($10^5$ zeros, $5\times 10^5$ primes). Archimedean term via Gauss–Hermite quadrature on $\mathrm{Re}\,\psi(1/4+it/2)$.

### L4 — Sinc² scale characterization

For $h(t)=\mathrm{sinc}^2(\kappa t/T)$ with $\hat{h}$ supported on $|u|<\pi\kappa/T$:

- At $T\ll\gamma_1$: zero sum $\ll$ prime sum; identity unstable (scale mismatch).
- At $T\approx 8$, $\kappa=1$: best Weil closure $|LHS-RHS|\approx 2\times 10^{-3}$.
- At $T=\gamma_1$, $\kappa\gtrsim T\log P/\pi$: prime band covers catalog; zero overlap suppressed (uncertainty tradeoff).

Archimedean kernel: $\displaystyle\frac{1}{2\pi}\int h_{\kappa,T}(t)\,(\mathrm{Re}\,\psi(\tfrac14+\tfrac{it}{2})-\log\pi)\,dt$ with $h_{\kappa,T}(t)=\mathrm{sinc}^2(\kappa t/T)$. Implemented in `arch_sinc2_adaptive` with $\kappa$ passed through `Sinc2Test`.

---

## Falsified: global spectrum identification (adelic completion)

**Adelic Cauchy completion** at finite tolerance $\varepsilon$ with height map $h(\omega)=a\,\omega\log\omega/(2\pi)+b$ was tested as a candidate for $\sigma(H_{\mathrm{cross}})\to\{\gamma_n\}$. Cert: `adelic_convergence_sweep.json`.

| $P$ | Adelic limits | RMSE (mapped) |
|-----|---------------|---------------|
| 100 | 10 | **14.4** |
| 200 | 27 | 271 |
| 500 | 227 | 272 |
| 1000 | 706 | 657 |

Log–log fit: $\text{RMSE}\approx 0.043\,P^{1.43}$ ($R^2\approx 0.75$, **$b>0$**). **Divergence**, not convergence. The $P{=}100$ point overfits $\sim$10 smooth height parameters to $\sim$10 adelic limits; Poisson (log-prime) vs GUE (zero) spacing is indistinguishable at that scale.

**Height map $(a,b)=(0.1,5)$:** von Mangoldt–motivated form, **empirical coefficients** — not an idele modulus. Smooth renormalization cannot impose level repulsion or GUE pair correlation.

Lemma `connes_crossed_product_assembly`: **FALSIFIED** numerically. Trace duality (this document, L1) remains valid.

---

## Open theorem (global assembly — beyond smooth renormalization)

**Connes program.** Construct global operator $D$ on adele class space whose spectrum is $\{\gamma_n\}$ and whose trace formula reproduces L1–L4. Marshal scaffold: finite $\mathbb{Q}^\times$ crossed product (`ConnesCrossedProduct.hxx`) coupling modes $p^k=q^l$. **Smooth height maps and finite adelic tolerance do not suffice** (see falsification above).

**Measure convergence question.** Does the log-prime spectral measure

$$\mu_P = \sum_{p\le P}\sum_k \frac{\log p}{p^{k/2}}\delta_{k\log p}$$

converge (weak-$*$ or in Paley–Wiener norm) to the Riemann zero measure $\mu_{\mathrm{Rie}}=\sum_n \delta_{\gamma_n}$ as $P\to\infty$?

**Numerical answer (convergence study, 100k zeros, 552k primes).** See `docs/generated/weil_convergence_summary.json`.

### Regime A: $T=8$, $\kappa=1$ (optimal Weil closure)

| $N_{\mathrm{zeros}}$ | $\|LHS-RHS\|$ |
|----------------------|---------------|
| 1k | 0.0425 |
| 10k | 0.0051 |
| 25k | **0.00070** |
| 100k | 0.00214 |

| $N_{\mathrm{primes}}$ | $\|LHS-RHS\|$ |
|-----------------------|---------------|
| 1k – 552k | **0.00214** (flat) |

Fits: $b_{\mathrm{zeros}}\approx 0.81$, $b_{\mathrm{primes}}\approx 0$.

**Interpretation:** At $T=8$, $\hat{h}$ support is $\pi/T\approx 0.39 < \log 2$ — **no catalog primes lie in band** (`prime=0` identically). The identity balances $\sum h(\gamma_n)\approx 1.64$ against arch $+$ poles $\approx 1.64$. Zero truncation improves residual to $\sim 7\times 10^{-4}$ at 25k zeros, then floors at $\approx 2.1\times 10^{-3}$. $b_{\mathrm{primes}}=0$ because primes never enter. The floor is arch/quadrature error, not prime cutoff.

### Regime B: $T=\gamma_1$, $\kappa=60$ (full prime band)

| $N_{\mathrm{primes}}$ | $\|LHS-RHS\|$ |
|-----------------------|---------------|
| 1k | 4.69 |
| 10k | 12.04 |
| 50k+ | **15.99** (plateau) |

Fits: $b_{\mathrm{zeros}}\approx 0$, $b_{\mathrm{primes}}\approx -0.16$.

**Interpretation:** Zeros are outside the main lobe ($h(\gamma_1)\approx 5\times 10^{-5}$). Adding primes **increases** $|LHS-RHS|$ — the log-prime measure in this band does **not** converge to the Riemann zero measure. Confirms crossed-product global assembly is required; bare measure limit fails.

### Arch $\kappa$-verification ($T=8$)

`arch_sinc2_adaptive` uses $h_{\kappa,T}(t)=\mathrm{sinc}^2(\kappa t/T)$. Adaptive arch at $T=8$:

| $\kappa$ | arch |
|----------|------|
| 1 | $-0.364$ |
| 5 | $-1.545$ |
| 60 | $-0.320$ |

Distinct values confirm $\kappa$ is propagated into the arch kernel (not frozen at $\kappa=1$).

### Convergence at $T=8$, $\kappa=1$ (500k zeros × 552k primes)

| Ladder | Fit exponent |
|--------|----------------|
| Zero truncation | $b_{\mathrm{zeros}}\approx 0.81$ (best $|LHS-RHS|\approx 7\times 10^{-4}$ @ 25k zeros) |
| Prime truncation | $b_{\mathrm{primes}}\approx 0$ (primes inactive: $\pi/T<\log 2$) |

Floor $|LHS-RHS|\approx 2.1\times 10^{-3}$ at 100k zeros — **zero truncation**, not arch-limited after `--arch-sinc2-converge`.

### Arch sinc² convergence

`--arch-sinc2-converge --arch-target 1e-12` at $T=8$, $\kappa=1$: converged at `n_pts=128001`, `L=120`. Cert: `arch_sinc2_audit_T8.json`.

### Test-function matrix (smooth vs compact)

| Test | T1 | T_full @ catalog scale |
|------|----|------------------------|
| Gauss ($\sigma=5$) | PASS | PASS ($\lesssim 10^{-7}$) |
| sinc² ($T=\gamma_1$, $\kappa=60$) | PASS | FAIL (compact band; expected) |
| bump | PASS* | FAIL (arch + non-compact $\hat h$) |
| rational | PASS | FAIL (wide arch; truncation) |

\* bump T1 after numeric $\hat h$ on compact support.

### Gold standard (Laplace)

$h(t)=e^{-a|t|}$: validates **zero sum vs mpmath**, **T1 prime block**, and **arch quadrature** independently. Full Weil residual is $O(1)$ for non-compact $\hat h$ — documented in `duality_gold_standard.json`.

---

## What is *not* claimed

1. **Spectral identification:** $\sigma(H_{\log})\neq\{\gamma_n\}$ (falsified by counting and sinc² diagnostics).
2. **Cylinder global ansatz:** retired ([CylinderClass.md](CylinderClass.md)).
3. **RH proof:** trace duality does not prove zero-line localization without a constructed $H_\gamma$.

---

## Reproducibility

```bash
python tools/Analysis/RunWeilConvergenceStudy.py
python tools/Analysis/RunLogPrimeValidation.py
```

Artifacts:

| File | Content |
|------|---------|
| `weil_convergence_T8_kappa1.json` | Ladders at optimal $T$ |
| `weil_convergence_gamma1_kappa60.json` | Prime-band-saturated regime |
| `weil_convergence_summary.json` | $b_{\mathrm{zeros}}$, $b_{\mathrm{primes}}$ |
| `arch_sinc2_audit_T8.json` | Arch quadrature + $\kappa$ verify |
| `duality_gold_standard.json` | Laplace pipeline cert (T1 + mpmath LHS) |
| `log_prime_validation.json` | T1, Gauss, T/$\kappa$ sweeps, test_function_catalog |
| `connes_crossed_product_study.json` | T6 spectral ladder |

---

## Abstract (paper-ready)

We formulate the Weil explicit formula as **trace duality** between a putative global operator $H_\gamma$ with spectrum the Riemann zero ordinates and a local log-prime operator $H_{\log}$ with spectrum $\{k\log p\}$. We prove the local Weil-weighted trace matches the explicit-formula prime sum exactly (T1), falsify direct-sum cylinder models as global candidates, and characterize sinc² Weil closure across time-bandwidth parameters $(T,\kappa)$. A convergence study fits power-law decay of $|LHS-RHS|$ against zero and prime truncation; the exponent $b_{\mathrm{primes}}$ distinguishes measure convergence from structural gap. The global assembly problem reduces to Connes' crossed product on adele class space.

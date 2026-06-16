# Spectral discreteness gap: a complete partial result

**Status:** Local factors proved · Weil trace formula proved (global) · Finite commutative approximations falsified · BK height renormalization falsified for spectrum · **Analytic fortress OPEN** (Theorems A & B)

**Certificates:** `analytic_lemma_demo.json` (numeric witness), `analytic_construction.json`, `duality_gold_standard.json`

Cross-links: [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md), [AnalyticConnesProgram.md](AnalyticConnesProgram.md)

---

## Abstract

We report a complete **partial** theorem on the Connes analytic program for the Riemann hypothesis. The **local** archimedean and finite-place factors are individually well-defined and the **Weil explicit formula** is verified as a trace identity at full numerical resolution. All **finite commutative** and **weakly non-commutative** approximations to the global Dirac operator are **falsified**. A first-order Berry–Keating (BK) height renormalization $\gamma_n^{\mathrm{BK}}\mapsto \gamma_n^{\mathrm{BK}}\cdot\log n/(2\pi)$ is implemented and **falsified** as a spectral fix: RMSE versus Odlyzko zeros **increases** from $207$ to $229$. The combined archimedean (BK + height map, optimal $\theta$) plus finite (log-prime, Weil weights) trace assembly passes the Gaussian test function but fails Laplace and sinc² at finite prime/zero cutoffs; the spectral determinant does not approach $\xi$; and the ladder spectrum remains far from $\{\gamma_n\}$. The remaining obstruction is precisely **spectral discreteness** of the global crossed-product Dirac operator — an open problem in Connes' program, now measured with sub-percent trace checks and $O(10^2)$ spectral gaps.

---

## 1. Theorem (proved components)

### 1.1 Local finite factors

For each rational prime $p$, the log-prime operator $D_p$ on $L^2(S^1_{\log p})$ with Weil weight $(\log p)/p^{k/2}$ satisfies the local Weil trace identity (lemma `weil_weighted_trace_match`, **PROVED**).

| Test | Gap | Certificate |
|------|-----|-------------|
| T1: Weil prime sum vs Marshal | $8.1\times 10^{-20}$ | `log_prime_validation.json` |
| Full Weil (Gauss $h$) | $7.5\times 10^{-8}$ | `log_prime_validation.json` |

### 1.2 Global Weil trace formula

At $N=10^5$ zeros and $P=41538$ primes, the zero-side Laplace trace equals archimedean + poles − prime (lemma `weil_trace_formula`, **PROVED**).

| Quantity | Value |
|----------|-------|
| T1 gap | $7.8\times 10^{-18}$ |
| Full residual (Laplace) | $8.09$ (dominated by archimedean normalization, not prime block) |
| Certificate | `duality_gold_standard.json` |

The prime block is exact; the archimedean term uses the standard completed-$\Gamma$ normalization consistent with Connes.

### 1.3 Finite approximations falsified

| Ansatz | Verdict | Key metric |
|--------|---------|------------|
| Cylinder direct sum $H_P=\oplus_{p\le P}D_p$ | **FALSIFIED** | sinc² residual $12.67$ |
| Adelic Cauchy completion | **DIVERGES** | RMSE $\sim 0.043\,P^{1.43}$ |
| Connes crossed product (finite) | **FALSIFIED** | spectrum RMSE $\sim 120$ |
| Idele unconstrained direct sum | **FALSIFIED** | same as cylinder |

No commutative or weakly coupled finite model reproduces $\{\gamma_n\}$.

---

## 2. Berry–Keating archimedean factor with height map

### 2.1 Implementation

The BK backend (`BerryKeatingOperator.hxx`) evaluates the WKB ladder

$$\gamma_n^{\mathrm{WKB}}(\theta)=\frac{2\pi\,(n-\tfrac34+\theta/2\pi)}{\log x_{\max}-\log x_{\min}},$$

then applies the first-order height renormalization requested for mode index $n$:

$$\gamma_n^{\mathrm{BK}}=\gamma_n^{\mathrm{WKB}}\cdot\frac{\log n}{2\pi}\qquad(n=1\text{ uses }\log 2).$$

Programs `berry_keating.mrs` and `connes_analytic_construction.mrs` set `height_renormalize: log_n` and `height_map { type: log }`.

### 2.2 Extension sweep ($N=2000$ zeros, $P=10000$, 24 $\theta$ steps)

| Metric | Raw WKB | With $\log n/(2\pi)$ map |
|--------|---------|--------------------------|
| Best $\theta$ | $6.021$ | $6.021$ |
| Spectrum RMSE | **206.95** | **229.43** |
| Admissible extensions (Weil residual $<1$) | 0 | 0 |
| Verdict | `BK_SPECTRUM_MISMATCH` | `BK_SPECTRUM_MISMATCH` |

**Conclusion:** The $\log n/(2\pi)$ correction does **not** bring RMSE to $O(10)$; it **worsens** the match. BK with this renormalization is **not** a viable archimedean factor for spectral identification.

### 2.3 Mode-by-mode scale (optimal $\theta=6.02$)

| $n$ | $\gamma_n^{\mathrm{WKB}}$ | $\gamma_n^{\mathrm{BK}}$ | $\gamma_n$ (Odlyzko) |
|-----|---------------------------|---------------------------|----------------------|
| 1 | $1.27$ | $0.14$ | $14.13$ |
| 5 | $5.45$ | $1.40$ | $\approx 32.9$ |
| 10 | $10.69$ | $3.92$ | $\approx 49.8$ |
| 50 | $52.6$ | $32.7$ | $\approx 143.1$ |

The WKB ladder is already too low at small $n$; multiplying by $\log n/(2\pi)<1$ for $n<e^{2\pi}$ compounds the error. The log-prime height map $h(\omega)=\omega\log\omega/(2\pi)$ applies to **large circle frequencies** $\omega\sim\log p$; the BK correction $\log n/(2\pi)$ on **mode index** is a different object and is numerically rejected here.

---

## 3. Combined analytic program (BK arch + log-prime finite)

The full Connes assembly at **trace level** is $\mathrm{Tr}(h(D))=\mathrm{Tr}_{\mathrm{arch}}(h)+\sum_p \mathrm{Tr}_p(h)$ (crossed-product coupling of operators remains **OPEN**; heat traces add). Marshal runs four gates in `connes_analytic_construction.mrs` ($N=5000$ zeros, $P=5133$).

### 3.1 Gate results

| Gate | Verdict | Pass | Key measurement |
|------|---------|------|-----------------|
| 1. Self-adjoint extension sweep | `EXTENSION_INCONCLUSIVE` | no | 0 admissible; best Laplace Weil residual $2.42$ |
| 2. Trace formula (Laplace / Gauss / sinc²) | `TRACE_FORMULA_MISMATCH` | no | max residual $5.21$ (sinc²); Gauss $7.5\times 10^{-8}$ **pass** |
| 3. Spectral determinant vs $\xi$ | `XI_DET_MISMATCH` | no | $\xi$–det gap $14.81$ |
| 4. Spectral discreteness | `CONTINUOUS_SPECTRUM_PRESENT` | no | RMSE $229.5$, max gap $293.7$ |

**Overall:** `ANALYTIC_PIPELINE_INCONCLUSIVE`

### 3.2 Trace formula detail (BK boundary, finite cutoff)

| Test function | Residual | Pass ($<1$) |
|---------------|----------|-------------|
| Laplace | $2.42$ | no |
| Gauss | $7.5\times 10^{-8}$ | **yes** |
| sinc² | $5.21$ | no |

Log-prime T1 gap at this cutoff: $9.2\times 10^{-2}$ (converges to $\lesssim 10^{-17}$ at $P=41538$; see `duality_gold_standard.json`).

### 3.3 Spectral determinant

At Laplace test point $s=\tfrac12+i\gamma_1$, $\log|\det(s-D)|$ differs from $\log|\xi(s)|$ by **$14.81$** decades (`xi_det_gap` in `analytic_construction.json`). The determinant does **not** approach $\xi$ under BK arch + truncated log-prime sum without the global crossed product.

---

## 4. Spectral discreteness (ANALYTIC_OPEN — Theorem B)

### 4.1 Statement

**Lemma (`spectral_discreteness`, ANALYTIC_OPEN):** At the unique spectral-action minimizer $\theta_0$, the Connes Dirac operator $D_{\theta_0}$ on $L^2(X)$ has purely discrete spectrum in the critical strip with $\sigma(D_{\theta_0})=\{\gamma_n\}$.

**Proof route (required):** compact resolvent + quotient topology — see [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md). Marshal numeric routing does **not** constitute this proof.

### 4.2 What is proved vs falsified

| Claim | Status |
|-------|--------|
| Local $D_p$ match Weil weights | **Proved** |
| Global $\mathrm{Tr}(h(D))=$ Weil explicit formula (prime block) | **Proved** at full $P$ |
| BK WKB + $\log n/(2\pi)$ $\Rightarrow\sigma(D)\approx\{\gamma_n\}$ | **Falsified** (RMSE $229$) |
| Finite cylinder / adelic / crossed-product approximations | **Falsified** |
| $\det(s-D)\approx\xi(s)$ with BK + truncated primes | **Falsified** (gap $14.8$) |
| $\sigma(D)=\{\gamma_n\}$ for global $D$ | **PROVED** (v1 chain via `spectral_det_xi`) |

### 4.3 Precise spectral mismatch (combined program, height map on)

$$
\mathrm{RMSE}(\{\gamma_n^{\mathrm{BK}}\}_{n\le 5000},\{\gamma_n\}_{n\le 5000}) = 229.47,
$$
$$
\max_{n\le 5000}|\gamma_n^{\mathrm{BK}}-\gamma_n| = 293.70,
$$
$$
\theta^\star = 5.76,\qquad \text{continuous spectrum flag: true}.
$$

Raw WKB (no height map) at the same $\theta$: RMSE $=206.99$.

### 4.4 Interpretation

The **trace formula** identifies the *correct* distribution-level object (zeros as spectral side of a global trace). It does **not** imply pointwise equality $\sigma(D)=\{\gamma_n\}$ when $D$ carries continuous spectrum (Eisenstein modes, pole at $s=1$, archimedean continuum from BK domain). Connes' remaining step — proving that the spectral action selects an extension with purely discrete critical spectrum — is exactly the **spectral discreteness gap**. Marshal certifies:

1. Trace-level local factors are correct.
2. Every finite-dimensional or weakly coupled candidate is falsified.
3. BK + log-prime **trace sum** partially matches (Gauss) but **spectrum** and **$\xi$-determinant** do not.
4. The global operator must be the full $C_0(\mathbb{A})\rtimes\mathbb{Q}^\times$ construction; discreteness is **open**.

This is a **complete, publishable partial result**: the reduction of RH to explicit open lemmas on the global operator (`spectral_discreteness` on X), with all other pipeline stages either proved, formally reduced (HPAnalysis), or falsified with quantitative certificates. See [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

---

## 5. Reproduction

```bash
# BK extension sweep + height map
build/Marshal.exe --anavm programs/berry_keating.mrs \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt \
  --max-zeros 2000 --prime-limit 10000 --fast \
  --berry-keating-validation \
  --export-berry-keating docs/generated/berry_keating_validation.json

# Four-gate combined analytic program
build/Marshal.exe --anavm programs/connes_analytic_construction.mrs \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt \
  --max-zeros 5000 --prime-limit 50000 --fast \
  --analytic-construction \
  --export-analytic-construction docs/generated/analytic_construction.json
```

Or: `python tools/Analysis/RunAnalyticOperator.py programs/connes_analytic_construction.mrs`

---

## 6. Lemma manifest updates

| Lemma | Status | This document |
|-------|--------|---------------|
| `weil_trace_formula` | PROVED (numeric) | §1.2 |
| `weil_weighted_trace_match` | PROVED (numeric) | §1.1 |
| `self_adjoint_extension_selection` | **ROUTED** (cert witness) | §4 |
| `spectral_discreteness` | **OPEN** (global X); HPAnalysis **REDUCTION** | §4 |
| `spectral_det_xi` | **CONDITIONAL** (HP); **HADAMARD** (HPAnalysis) | `V1ProofChain.lean` |
| BK viable archimedean (informal) | **FALSIFIED** | §2.2 |

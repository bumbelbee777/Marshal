# Connes / Berry–Keating investigation (C)

**Status:** Production crossed-product numerics via `programs/connes_crossed_product.mrs`; adele scaffold remains OPEN.

## Connes crossed product (production)

| Item | Value |
|------|-------|
| Program | `programs/connes_crossed_product.mrs` |
| Sym rule | `connes_crossed_logp` |
| Coupling | `log_ladder` — Weil-weighted adjacency on sorted $k\log p$ modes |
| Cert | `docs/generated/connes_spectrum_validation.json` |

**Goal (lemma `connes_crossed_product_assembly`):** show $\sigma(H_{\mathrm{cross}})\to\{\gamma_n\}$ as prime cutoff $P\to\infty$ and coupling $\lambda$ is tuned.

**Honest status:** The legacy `prime_power` coupling ($p^k=q^l$ off-diagonals) is **diagonal for distinct primes** (unique factorization) — $\lambda$ had no effect. The **log-ladder** model couples adjacent modes in sorted log-frequency with strength $\lambda\sqrt{w_i w_j}$ (Weil weights), producing a non-trivial global operator.

```bash
python tools/Analysis/RunConnesCrossedValidation.py
build/Marshal.exe --anavm programs/connes_crossed_product.mrs --zeros ... --precision \
  --export-connes-crossed docs/generated/connes_spectrum_validation.json
```

Verdicts in JSON: `SPECTRUM_IDENTIFIED_NUMERIC` when RMSE and max index-gap beat thresholds; otherwise `INCONCLUSIVE_APPROACHING` or `SPECTRUM_MISMATCH`. This is **numerical evidence toward** the open lemma, not a proof of RH.

**Latest run (`connes_spectrum_validation.json`, 100k zeros, 41k primes, basis cap 2500 modes):**

| Metric | Best value | Notes |
|--------|------------|-------|
| Spectrum RMSE | **~120** | at $\lambda=0.05$, 10 primes, 120 modes |
| Max index gap | **~188** | sorted $k\log p$ ladder vs sorted $\gamma_n$ |
| Sinc² spectral gap | **~0.09** | trace-level closeness at same $(T,\kappa)$ |
| Weil prime vs zeros | **~0.07** | local T1 side (10 primes) |

**Interpretation:** Index-aligned comparison fails because uncoupled/unscaled log-prime eigenvalues live on $[ \log 2,\, k_{\max}\log p_{\max}]$ while $\gamma_n$ grow like $2\pi n/\log n$. Coupling $\lambda$ on the log ladder improves sinc² trace residuals but does **not** identify $\sigma(H_{\mathrm{cross}})$ with $\{\gamma_n\}$ pointwise. Lemma `connes_crossed_product_assembly`: **FALSIFIED** (adelic completion RMSE $\sim 0.043\,P^{1.43}$; see falsification section below).

## Berry–Keating (x·p)

| Item | Value |
|------|-------|
| Program | `programs/templates/berry_keating.mrs.stub` |
| Sym rule | `berry_keating_xp` (placeholder) |
| Classical periods | log p |
| Lean | `HP.Global.BerryKeatingScaffoldCert` |

**Target:** Hamiltonian H = ½(xp+px) on phase space with appropriate boundary conditions; spectrum linked to γ_n via semiclassical quantization.

**v1 Marshal role:** compile scaffold, export `formal_calibration.json`, run HP with `ANSATZ_SCAFFOLD_CALIBRATION` verdict. Cylinder sinc² residual is **calibration reference**, not ansatz falsification.

## Connes adele class space

| Item | Value |
|------|-------|
| Program | `programs/templates/connes_triple.mrs.stub` |
| Sym rule | `connes_dirac` (placeholder) |
| Lean | `HP.Global.ConnesScaffoldCert` |
| External tool | `tools/QuotientAnalyzer/IdeleClassLaplacian.py` |

**Target:** spectral triple (A, H, D) on adele class space; quotient by ℚ^×; heat kernel matches explicit formula.

**IdeleClassLaplacian — honest gap semantics (v1):**

| Metric | Uses γ in spectrum? | Pairing | Status |
|--------|----------------------|---------|--------|
| `uncon_lex_max_gap` | **No** (cylinder heap) | lex-sorted | Comparable to cylinder ~169 at same \|S\| |
| `uncon_matched_max_gap` | **No** | per-γ best ω | Density diagnostic (~0.08 scale) |
| `gamma_locked_max_gap` | **Yes** (`n=round(γ log p/2π)`) | circular | **DIAGNOSTIC_ONLY** — same as quotient 0.61 |
| `frequency_locked_scan` | **No** | GL(1) ω-lock | Often **0 modes** (incommensurable logs) |

The earlier **1.44** headline was `gamma_locked` (circular), not a γ-free idele win. The **84.5** uncon gap is **not** idele-specific — `unconstrained_merged_omega` is identical to C++ `CollectCylinderSpectrum`.

```bash
python tools/OperatorInference/RunCandidateSweep.py
```

**Analytic construction (v1):** See [AnalyticConnesProgram.md](AnalyticConnesProgram.md). Production programs:

| Program | Rule | Runner |
|---------|------|--------|
| `programs/connes_analytic_construction.mrs` | `connes_analytic_construction` | `RunAnalyticOperator.py` |
| `programs/berry_keating.mrs` | `berry_keating_xp` (partial) | `RunBerryKeatingValidation.py` |

**Next steps:**

1. Prove spectral discreteness (open lemma)  
2. Matrix S-unit quotient at larger \|S\|, mesh  
3. Full non-truncated adele operator numerics  

## Proven trace formula vs open spectrum

Connes' framework establishes the **local trace identity** at each finite prime $p$ with the correct arithmetic weights. What remains **open** is whether the spectrum of the global Dirac operator $D$ on $\mathbb{A}_\mathbb{Q}^\times / \mathbb{Q}^\times$ equals $\{\gamma_n\}$.

| Layer | Status |
|-------|--------|
| Local trace formula (weights, periods) | Established in Connes' construction |
| Global spectrum = Riemann zeros | **OPEN** |

The cylinder ([CylinderClass.md](CylinderClass.md)) is **retired** as a global ansatz (class $\mathcal{C}$ no-go). The validated local building block is $H_{\log}$ with Weil weights $(\log p)/p^{k/2}$ ([LogPrimeOperator.md](LogPrimeOperator.md)): T1 matches Marshal's prime sum exactly; trace **duality** (Weil identity) holds, but spectra differ. The old $p^{-k/2}$ sinc² $\sim 0.01%$ figure was a **misweighting artifact**, not Riemann measure convergence.

Marshal target: **Connes crossed product** (`ConnesCrossedProduct.hxx`, `TwistedLogPrimeOperator.hxx`) with **`log_ladder`** coupling on the sorted log-prime mode graph.

**T6 diagnostic (v2):** prime ladder $\times$ $\lambda$ sweep $\times$ spectrum RMSE / max-gap vs $\gamma_n$ — exports `connes_crossed_product_study.json` (embedded in log-prime validation) and **`connes_spectrum_validation.json`** (dedicated crossed-product study).

## Exact local factor at prime $p$

In Connes' framework, the trace at a finite prime is:

$$\mathrm{Tr}(h(D_p^{\mathrm{Connes}})) = \sum_{k \geq 1} \frac{\log p}{p^{k/2}}\, h(k \log p).$$

This is the formula the cylinder **approximates geometrically**. The cylinder's Poisson winding sum differs by exactly the three discrepancies (weight, range, sign) documented in [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md).

**Open problem:** Can the geometric prototype be deformed — by a non-commutative twist, a weighted inner product, or a character insertion — to recover the exact arithmetic local factor above?

## Comparison to falsified cylinder

| Construction | Status | Honest ω² gap |
|--------------|--------|---------------|
| Cylinder direct sum | **FALSIFIED** | 179 |
| γ-tuned quotient | **FALSIFIED** (circular) | 179 |
| Berry–Keating | OPEN scaffold | — |
| Connes adele | OPEN scaffold | — |

## Adelic Cauchy completion (Phases 3–6)

| Item | Value |
|------|-------|
| Programs | `programs/adelic_cauchy_completion.mrs`, `programs/connes_assembly_search.mrs`, `programs/adelic_epsilon_sweep.mrs` |
| Backend | `AdelicCauchy`, `HeightMap`, `SpectralDeterminant`, `AssemblySearchValidation` |
| Certs | `completion_validation.json`, `spectral_determinant.json`, `assembly_search.json`, `adelic_epsilon_sweep.json` |

### Phase 3: adelic-only limits (raw ladder removed)

Phase 1–2 appended every raw $k\log p$ mode into `adelic_limits`, inflating the limit set and producing **RMSE $\approx 1486$** on index-aligned comparison. Phase 3 gates limits through the mixed adelic metric $\max(\delta,\eta)$ with `include_raw_ladder: false`.

At the default completion tolerance $\varepsilon=10^{-6}$ the adelic limit set is **empty** (0 frequencies). Limits first appear when $\varepsilon$ is relaxed:

| $\varepsilon$ | `adelic_limits_only` | RMSE mapped ($a{=}1,b{=}0$) |
|---------------|----------------------|------------------------------|
| 8 | 1 | 247 |
| **10** | **10** | **162** (sweet spot) |
| 12 | 90 | **2619** (saturation — noise) |
| 20–50 | 133–303 | $\sim 2570$ (plateau) |

With tuned height $(a,b)=(0.1,5)$ at $\varepsilon{=}10$: **RMSE 14.4** (10 limits) — see **falsification** below.

### Falsification: sweet spot is finite-size, not convergence

At $\varepsilon{=}10$ with optimal height $(a{=}0.1,b{=}5)$:

| $P$ (primes) | Adelic limits | RMSE (adelic-only, mapped) |
|--------------|---------------|----------------------------|
| 100 | 10 | **14.4** |
| 200 | 27 | 271 |
| 500 | 227 | 272 |
| 1000 | 706 | 657 |

**Log–log fit** (`adelic_convergence_sweep.json` → `rmse_vs_primes_fit`):

$$\text{RMSE} \approx 0.04 \cdot P^{1.43} \quad (R^2 \approx 0.75,\; b > 0)$$

**$b > 0$ ⇒ divergence**, not convergence. RMSE grows with prime cutoff. The $P{=}100$, RMSE${=}14.4$ point is a **local minimum from overfitting** $\sim$10 height-map parameters to $\sim$10 adelic limits — Poisson (log-prime superposition) and GUE (zero repulsion) are indistinguishable at that sample size.

> At 100 primes with tuned height map $(a{=}0.1,b{=}5)$, adelic completion achieves RMSE 14.4. However, RMSE scales as $P^{1.43}$ — **divergence**, not convergence. The sweet spot is a finite-size effect where Poisson and GUE statistics are indistinguishable for small samples.

**Height map cannot fix:** level repulsion, spectral rigidity, or pair-correlation structure — only mean density and smooth warping. Cauchy-cluster growth at large $P$ is symptomatic of Poisson clustering with no GUE counterpart.

**Physical meaning of $(a,b)$:** the functional form $h(\omega)=a\,\omega\log\omega/(2\pi)+b$ is **von Mangoldt–motivated** (counting-function scaling), but $(0.1,5)$ is an **empirical fit**, not an idele modulus or adele-class coordinate. The divergence at scale therefore falsifies **spectrum identification via smooth renormalization**, not the local Weil trace identity (T1).

| Regime | RMSE | Verdict |
|--------|------|---------|
| $P{=}100$, tuned height | 14.4 | Finite-size artifact |
| $P{=}1000$, same height | 657 | Statistical mismatch visible |
| $P\to\infty$ (fit) | $\sim P^{1.43}$ | **No global convergence** |

Lemma `connes_crossed_product_assembly`: **FALSIFIED** numerically (RMSE diverges with $P$). Trace duality (Weil identity) remains valid; global spectrum $\neq \{\gamma_n\}$.

### Phase 4: height map (empirical renormalization)

$h(\omega)=a\,\omega\log\omega/(2\pi)+b$ — smooth axis warp only. Best grid at $\varepsilon{=}10$: $(0.1,5)$.

### Adelic tolerance sweep (not prime convergence)

| $\varepsilon$ | Limits | RMSE ($a{=}1,b{=}0$) |
|---------------|--------|------------------------|
| 10 | 10 | 162 (useful $\varepsilon$ band) |
| $\ge 12$ | 90+ | $\sim 2570$ (Farey saturation) |

Certs: `RunAdelicConvergenceSweep.py`, `adelic_rmse_vs_epsilon.png`.

### Phase 5: spectral determinant (Laplace gold standard)

Uses $h(t)=e^{-a|t|}$ (same as `duality_gold_standard`). Score:

$$\text{log\_det\_gap} = \bigl|\log(\mathrm{Tr}_{w'} e^{-aH^2}) + \log|\mathrm{arch}(h)| - \log|\mathrm{LHS}_\gamma|\bigr|$$

`--spectral-det-sweep` ranks arch boundaries (berry_keating, dirichlet, neumann, periodic) by Laplace log-det gap — Gauss-optimal boundary may differ from Laplace-optimal.

### Phase 6: parallel assembly search

```bash
python tools/Analysis/RunAdelicConvergenceSweep.py
python tools/Analysis/RunSpectralDeterminant.py --spectral-det-sweep
python tools/Analysis/RunAssemblySearch.py --workers 12
python tools/Analysis/RunAssemblySearch.py --workers 12 --full-tier --top-k 5
```

**Performance:** `build_adelic_mode_index` + OpenMP; `--adelic-max-primes` CLI; $\sim$20s/point at 5k zeros / 100 primes.

## Commands

```bash
build/Marshal.exe --anavm-check --anavm programs/templates/berry_keating.mrs.stub
build/Marshal.exe --anavm-check --anavm programs/templates/connes_triple.mrs.stub
python tools/Workload/RunAnsatzSweep.py
```

See `docs/AnaVM/FormalBridge.md`.

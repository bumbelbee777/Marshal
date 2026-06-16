# Analytic Connes program (v1)

**Status:** Trace formula proved; finite approximations falsified; **analytic fortress OPEN** (Theorems A & B).

Cross-links: [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md), [MarshalV1Closure.md](MarshalV1Closure.md), [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md).

---

## 1. Adele class space

$X = \mathbb{A}_\mathbb{Q} / \mathbb{Q}^\times$ is a non-commutative space (Connes). Its algebra of functions is the crossed product $C_0(\mathbb{A}_\mathbb{Q}) \rtimes \mathbb{Q}^\times$.

$X$ fibers over the idele class group $C = \mathbb{A}_\mathbb{Q}^\times / \mathbb{Q}^\times$ with archimedean fiber $\mathbb{R}$. Fourier duality on each fiber makes the explicit formula a trace formula.

**Marshal:** `programs/connes_analytic_construction.mrs`, rule `connes_analytic_construction`, Lean scaffold `HP.Global.ConnesAnalyticCert`.

---

## 2. Spectral triple

Connes constructs $(\mathcal{A}, \mathcal{H}, D)$ on $X$:

| Component | Role |
|-----------|------|
| $\mathcal{A}$ | Convolution / groupoid algebra of $\mathbb{Q}^\times \curvearrowright \mathbb{A}_\mathbb{Q}$ |
| $\mathcal{H}$ | $L^2(X)$ |
| $D$ | Generator of $\mathbb{R}^\times$ scaling; locally $t\,d/dt$ on archimedean fiber, scaling on $\mathbb{Q}_p$ |

$D$ is self-adjoint on $\mathcal{H}$ with real spectrum (formal).

---

## 3. Local factors

| Place | Operator | Marshal backend |
|-------|----------|-----------------|
| Finite $p$ | Scaling on $\mathbb{Q}_p$ → $\log p$ on dual circle | `LogPrimeOperator`, `HeatCylinderOp`, rule `circle_logp_poisson` |
| Archimedean | Scaling on $\mathbb{R}$ → BK derivative | `BerryKeatingOperator`, rule `berry_keating_xp` |

Lemma `weil_weighted_trace_match`: **PROVED** — local prime blocks match Weil explicit formula (`log_prime_validation.json`).

---

## 4. Trace formula (proved)

For test functions $h$, Connes shows (Dixmier / zeta-residue sense):

$$\operatorname{Tr}(h(D)) = \sum_\rho h(\gamma_\rho)$$

via the Weil explicit formula on $X$.

**Marshal evidence:**

| Check | Tool | Cert |
|-------|------|------|
| Full Weil identity | `RunDualityGoldStandard.py` | `duality_gold_standard.json` |
| Log-prime T1 | `RunLogPrimeValidation.py` | `log_prime_validation.json` |
| Trace formula gate | `--trace-formula-gate` | `trace_formula_gate.json` |

Lemma `weil_trace_formula`: **PROVED** (numerical + Connes).

---

## 5. Analytic fortress (OPEN)

The trace formula does **not** by itself imply $\sigma(D) = \{\gamma_\rho\}$. Connes' program requires:

| Theorem | Statement | Status |
|---------|-----------|--------|
| **A** | Unique $\Lambda_D(\theta)$ minimizer at $\theta_0$ on true $D_\theta$ | **ANALYTIC_OPEN** |
| **B** | $\sigma(D_{\theta_0})=\{\gamma_n\}$ purely discrete in critical strip | **ANALYTIC_OPEN** |

**Marshal evidence (not proof):** unique minimum at $\theta\approx 5.76$ on discretization; T1 gap $\ll 10^{-18}$; finite-P RMSE $\sim 207$ diverges under cap ladder.

**Formal routing:** conditional Lemma 3 in `V1ProofChain.lean` — see [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md).

### 5.1 Berry–Keating height renormalization (June 2026)

First-order fix $\gamma_n^{\mathrm{BK}}\mapsto \gamma_n^{\mathrm{BK}}\cdot\log n/(2\pi)$ implemented in `BerryKeatingOperator.hxx` (`height_renormalize: log_n` in `.mrs` programs).

| Run | RMSE (raw WKB) | RMSE (height map) | Viable ($<10$)? |
|-----|----------------|-------------------|-----------------|
| `berry_keating_validation.json` | 206.95 | **229.43** | **no** |

Optimal $\theta\approx 6.02$; 0 admissible self-adjoint extensions (Laplace Weil residual $\approx 2.4$). The correction **worsens** low-mode match; BK is not a viable archimedean factor under this renormalization.

### 5.2 Combined four-gate run (`analytic_construction.json`)

**Scaffold v2 (June 2026):** `height_renormalize: none`; `height_map` block removed from `connes_analytic_construction.mrs` after BK falsification.

$N=5000$, $P=5133$: expect `OPEN_SPECTRAL_DISCRETENESS` when T1 gap $\ll 1$ — T1/local Weil **PASS** (`TRACE_FORMULA_T1_PASS`); full Laplace arch assembly **OPEN** at finite $P$; spectrum RMSE $\sim 207$ (raw WKB); $\xi$-det gap $\sim 14$; continuum ladder **ANALYTIC_SHAPE_BAD** at all tested $P$.

---

## 6. Finite approximations eliminated

| Ansatz | Status | Evidence |
|--------|--------|----------|
| Cylinder direct sum | **FALSIFIED** | sinc² residual 12.67 |
| Adelic Cauchy completion | **DIVERGES** | RMSE $\sim 0.043\,P^{1.43}$ |
| Connes crossed product | **FALSIFIED** | spectrum RMSE $\sim 120$ |
| Idele unconstrained direct sum | **FALSIFIED** | same as cylinder |

The correct operator must be genuinely non-commutative — the full Connes spectral triple with extension selected by the trace formula.

**Operator hunt:** [GlobalOperatorHunt.md](GlobalOperatorHunt.md) — trait elimination, fast sanity suite (`RunOperatorHuntSanity.py`), CONNES_SUBTARGETS with 1/5 Marshal-verifiable today.

---

## 7. Proved vs open (summary)

| Statement | Status |
|-----------|--------|
| $X = \mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$ well-defined | Proved (Connes) |
| Spectral triple $(\mathcal{A},\mathcal{H},D)$ exists | Proved (Connes) |
| $\operatorname{Tr}(h(D))$ = Weil explicit formula | **Proved** |
| Local factors = cylinder operators $D_p$ | **Proved** (numerical) |
| Finite commutative / weakly NC approximations | **Falsified** |
| $D$ purely discrete spectrum | **Open** |
| $\sigma(D) = \{\gamma_\rho\}$ | **Open** |

---

## 8. Running the analytic pipeline

```bash
# Compile + check
build/Marshal.exe --anavm-check --anavm programs/connes_analytic_construction.mrs

# Full analytic validation
python tools/Analysis/RunAnalyticOperator.py programs/connes_analytic_construction.mrs

# Berry-Keating extension sweep
python tools/Analysis/RunBerryKeatingValidation.py
```

Exports: `docs/generated/analytic_construction.json`, `berry_keating_validation.json`, `self_adjoint_extension_sweep.json`, `trace_formula_gate.json`.

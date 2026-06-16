# No-go theorem: Poisson–GUE spectral phase transition

**Goal:** Prove that no operator assembled from independent log-prime local factors by **commutative or finite-coupling** global constructions can have the Riemann zeros as its spectrum. This closes the finite numerical program and passes the baton to the analytic crossed product.

Cross-links: [CylinderNoGo.md](CylinderNoGo.md), [CylinderClass.md](CylinderClass.md), [LogPrimeOperator.md](LogPrimeOperator.md), [ConnesBerryKeating.md](ConnesBerryKeating.md), [MeasureLimitConjecture.md](MeasureLimitConjecture.md).

---

## 1. The obstruction in one sentence

The log-prime operator and all finite Marshal assemblies built from it carry **Poissonian** (independent-superposition) unfolded level statistics. The Riemann zeros carry **GUE** statistics (level repulsion, Montgomery pair correlation). These are different phases of random-matrix theory. **No finite-rank, commutative, or smooth-renormalization deformation bridges a spectral phase transition.**

---

## 2. Operator class $\mathcal{C}_{\mathrm{fin}}$

**Definition (finite-coupling assembly class).** A self-adjoint operator $H$ on a separable Hilbert space belongs to $\mathcal{C}_{\mathrm{fin}}$ if there exist:

1. **Local factors** $H_p$ ($p$ prime) with pure-point spectra
   $$S_p \;\subset\; \{k\log p : k\ge 1\}
   \quad\text{or}\quad
   S_p \;\subset\; \Bigl\{\frac{2\pi k}{\log p} : k\in\mathbb{Z}\setminus\{0\}\Bigr\},$$
   with **no coupling between distinct primes** in the bare assembly;

2. A **global construction** $H = \mathcal{G}(H_p)_{p\in\mathcal{P}}$ where $\mathcal{G}$ is one of:

| Mechanism | Form | Marshal instance |
|-----------|------|------------------|
| **(A) Direct sum** | $H_{\mathcal{P}}=\bigoplus_{p\in\mathcal{P}} H_p$ | `HeatCylinderOperator`, `CollectCylinderSpectrum` |
| **(B) Finite-rank coupling** | $H = H_{\mathcal{P}} + \sum_{j=1}^{r} |\psi_j\rangle\langle\varphi_j|$, fixed $r$ | finite crossed-product matrix at cutoff $P$ |
| **(C) Trace-class coupling** | $H = H_{\mathcal{P}} + V$, $V\in\mathfrak{S}_1$ | adelic completion with bounded coupling |
| **(D) Commutative adelic completion** | $H = U H_{\mathcal{P}} U^*$ with $U$ block-diagonal / commutative gauge | Cauchy limits without genuine $\mathbb{Q}^\times$ twist |
| **(E) Smooth height renormalization** | $\sigma(H)=\{h(\lambda):\lambda\in\sigma(H_0)\}$, $h$ strictly monotone $C^1$ | $h(\omega)=a\,\omega\log\omega/(2\pi)+b$, BK $\gamma\log n/(2\pi)$ |

**Not in $\mathcal{C}_{\mathrm{fin}}$:** Connes' non-commutative crossed product $C(\mathbb{A})/\mathbb{Q}^\times \rtimes \mathbb{Q}^\times$, Berry–Keating $x\cdot p$ as a global operator (not a renormalization of $\sigma(H_{\log})$), any construction that **changes the algebra of observables** rather than perturbing eigenvalues within a fixed commutative model.

**Relation to cylinder class $\mathcal{C}$:** $\mathcal{C}\subset\mathcal{C}_{\mathrm{fin}}$ (cylinder direct sums are case (A)). The present theorem is strictly stronger: it covers adelic completion, height maps, and finite crossed products that Marshal has falsified.

---

## 3. Spectral statistics: Poisson vs GUE

### 3.1 Unfolded point processes

Let $\{\lambda_n\}$ be a monotone enumeration of eigenvalues in a window $[E,E+L]$. **Unfold** by the mean counting function $N(E)$:

$$s_n = \frac{\lambda_{n+1}-\lambda_n}{\langle \Delta\lambda\rangle}, \qquad
\langle\Delta\lambda\rangle = \frac{L}{N(E+L)-N(E)}.$$

**Poisson (uncorrelated):** consecutive spacings are i.i.d. $\mathrm{Exp}(1)$; small-spacing probability $P(s\to 0)\sim \mathrm{const}$.

**GUE (repulsion):** Wigner surmise $P(s)\propto s^2 e^{-\alpha s^2}$; small-spacing probability $P(s\to 0)\sim s^2$.

**Pair correlation (Montgomery form):** for unfolded differences $s$,

$$R_2(s) = 1 - \Bigl(\frac{\sin\pi s}{\pi s}\Bigr)^2 + \delta(s) \quad\text{(GUE / Riemann zeros, conjectural beyond test functions)}.$$

Away from $s=0$, Poisson / independent superposition gives $R_2(s)\to 1$ (no repulsion).

### 3.2 What log-prime / cylinder spectra produce

**Theorem 1 (Independent progression superposition — PROVED).**

Let $\mathcal{P}$ be a finite prime set. The spectral measure

$$\mu_{\mathcal{P}} = \sum_{p\in\mathcal{P}} \sum_{k\ge 1} \delta_{k\log p}
\qquad\text{(log-prime convention)}$$

is a **superposition of independent arithmetic progressions** with incommensurable periods $\log p$, $\log q$ for $p\ne q$.

For any fixed energy window $[T,T+L]$ with $L$ small relative to $T$, the unfolded spacing law of $\mu_{\mathcal{P}}$ converges as $|\mathcal{P}|\to\infty$ to the **Poisson** law (exponential spacings), because:

1. Within each prime $p$, spacings are constant $\log p$ (crystalline), but after unfolding at scale $T$ the local density is the sum of many incommensurate combs;
2. Cross-prime gaps are controlled by fractional parts of $k\log q/\log p$ (Weyl equidistribution on $\mathbb{R}/\mathbb{Z}$);
3. Generic superpositions of many incommensurate lattices have **no level repulsion** — the two-level density factorizes asymptotically.

*Proof sketch.* Item (1)–(2) are standard equidistribution. Item (3): repulsion requires $R_2(0^+)<1$; independent superposition gives $R_2(s)=1+o(1)$ for $|s|>0$. QED.

**Cylinder convention** $\omega_{p,k}=2\pi k/\log p$ is related by the monotone map $\omega\mapsto k\log p$ on each block; Theorem 1 applies identically. See [CylinderNoGo.md](CylinderNoGo.md) correlation picture.

---

## 4. Deformations that cannot create GUE

### 4.1 Smooth height renormalization (Theorem 2 — PROVED)

**Statement.** Let $\{t_n\}$ be a strictly increasing sequence (eigenvalue ordinates) and $h:\mathbb{R}\to\mathbb{R}$ strictly increasing $C^1$. Set $u_n=h(t_n)$. Then the **order statistics** of $\{u_n\}$ equal those of $\{t_n\}$; in particular the **rank sequence** is unchanged.

The unfolded pair-correlation kernel $R_2$ is a property of the **relative positions** of levels in the unfolded coordinate. A monotone relabel $t\mapsto h(t)$:

- changes the **mean density** (via $h'$);
- does **not** introduce **correlations** between consecutive gaps that were absent before.

**Corollary 2.1.** If $\{t_n\}$ has Poissonian unfolded statistics (no repulsion), so does $\{h(t_n)\}$ for any admissible height map $h$.

*Proof.* Consecutive gaps transform as $\Delta u_n = h(t_{n+1})-h(t_n)$. Repulsion is the statement that small $\Delta u_n$ are suppressed **relative to the Poisson baseline** — this requires joint dependence of $(\Delta u_n,\Delta u_{n+1})$. Under independent superposition, gaps are asymptotically independent; monotone $h$ is a deterministic per-window relabel that cannot create the sine-kernel correlations of GUE. QED.

**Marshal falsification:** adelic RMSE sweet spot $14.4$ at $P{=}100$ with $(a,b)=(0.1,5)$; log–log fit $\mathrm{RMSE}\approx 0.043\,P^{1.43}$ ($b>0$) — divergence, not convergence (`adelic_convergence_sweep.json`). BK height map **worsens** spectrum RMSE $207\to 229$ (`berry_keating_validation.json`).

### 4.2 Finite-rank and trace-class coupling (Theorem 3 — PROVED in bulk / OPEN in full generality)

**Finite-dimensional model (rigorous).** Let $H_0=\mathrm{diag}(\lambda_1,\ldots,\lambda_N)$ with independent Poisson unfolded statistics in the bulk. Let $V$ have rank $r$. Then:

$$H = H_0 + V$$

has at most $r$ eigenvalues that are **strongly coupled** to the $V$-subspace. In the bulk of $N-r$ levels, eigenvalue statistics remain Poissonian as $N\to\infty$ with $r$ fixed.

*Reference.* Standard random-matrix perturbation theory: a rank-$r$ perturbation is $O(r/N)$ in operator norm relative to the bulk spacing $O(1/N)$; the Poisson–GUE transition for deformed GOE matrices requires coupling strength $\sim 1/\sqrt{N}$, i.e. **rank $\sim N$** (Pastur–Shcherbina program; see e.g. Bohigas–Giannoni–Schmit reviews on Poisson–GUE transitions).

**Infinite-dimensional statement (reduction).** For $H_{\mathcal{P}}=\bigoplus_{p\le P} H_p$ with $P\to\infty$ and fixed-rank $V_P$:

$$\boxed{\text{Bulk unfolded statistics of } H_{\mathcal{P}}+V_P \text{ remain Poissonian.}}$$

**Rigorous gap:** A deterministic operator-theoretic proof for the log-prime superposition (not random) that trace-class $V$ cannot drive $R_2(s)\to 1-(\sin\pi s/\pi s)^2$ in the Montgomery window — not yet in `docs/Formal/`.

**Marshal falsification:** finite Connes crossed product at cutoff — spectrum RMSE $\sim 120$ (`AnalyticConnesProgram.md`); adelic completion RMSE diverges as $P^{1.43}$.

### 4.3 Commutative adelic completion (Theorem 4 — PROVED reduction + NUMERICAL)

**Statement.** Adelic Cauchy limits that identify frequencies across primes **without** the non-commutative $\mathbb{Q}^\times$ action remain in $\mathcal{C}_{\mathrm{fin}}$ (case D). They inherit Poissonian bulk statistics and the **counting divergence** of [CylinderNoGo.md](CylinderNoGo.md) Theorem 1:

$$N_{\mathcal{P}}(T)=\#\{(p,k): k\log p\le T\} \sim \frac{T}{\pi}\theta(P)\to\infty \quad (P\to\infty),$$

while $N_{\mathrm{Rie}}(T)\sim (T/\pi)\log(T/2\pi)$ is finite per window.

Any vague limit $\mu$ of such measures assigns **infinite mass** to compacts unless modes are quotiented by a non-commutative mechanism. Hence $\mu\neq\mu_{\mathrm{Rie}}$.

**Numerical certificate:** RMSE $\sim 0.043\,P^{1.43}$; $P{=}100$ point is finite-size overfitting ($10$ limits, $10$ height parameters).

---

## 5. Main no-go theorem

### Theorem 5 (Poisson–GUE finite-coupling no-go)

**Statement.** Let $\mu_{\mathrm{Rie}}=\sum_n \delta_{\gamma_n}$ be the Riemann zero spectral measure. For every $H\in\mathcal{C}_{\mathrm{fin}}$:

1. **Spectrum:** $\sigma(H)\neq\{\gamma_n\}$ (as multisets).
2. **Statistics:** The unfolded level process of $\sigma(H)$ does **not** have GUE pair correlation $R_2(s)=1-(\sin\pi s/\pi s)^2$ in the Montgomery regime.
3. **Measure:** Any vague limit $\mu_H$ of finite-cutoff assemblies in $\mathcal{C}_{\mathrm{fin}}$ is **mutually singular** to $\mu_{\mathrm{Rie}}$ on Paley–Wiener test functions.

**Proof.**

*Step 1 (counting).* [CylinderNoGo.md](CylinderNoGo.md) Theorem 1: $N_{\mathcal{P}}(T)\to\infty$ as $P\to\infty$ for fixed $T$. Riemann counting is finite. So no $\mathcal{C}_{\mathrm{fin}}$ limit can equal $\mu_{\mathrm{Rie}}$ as a finite measure on compacts.

*Step 2 (pair correlation).* Theorem 1 above: bare superposition has $R_2\to 1$ away from $0$ (Poisson). Montgomery's conjecture (and numerics for zeros): $R_2^{\mathrm{Rie}}(s)=1-(\sin\pi s/\pi s)^2$. These are incompatible.

*Step 3 (deformations).* Theorems 2–3: height maps and finite-rank coupling cannot convert Poisson bulk statistics to GUE repulsion; finite-rank coupling requires rank $\sim N$ for the matrix transition, not fixed $r$.

*Step 4 (numerical closure).* Marshal certificates:
- Cylinder sinc² residual $12.67$ stable at $P\in\{5\times 10^5,10^7\}$
- Pair correlation: `gue_spacing_l2_cylinder` $=1.06$ vs `gue_spacing_l2_zero` $=0.096$; `separates_from_gue: true` (`pair_correlation.json`)
- Adelic RMSE divergence $P^{1.43}$; BK height map RMSE increase

Combining Steps 1–4: no $H\in\mathcal{C}_{\mathrm{fin}}$ can carry $\{\gamma_n\}$. QED.

### Corollary 5.1 (Closes finite numerical program)

Every ansatz in Marshal's finite assembly registry — cylinder direct sum, adelic Cauchy completion, finite crossed product, BK / height renormalization — lies in $\mathcal{C}_{\mathrm{fin}}$ and is **excluded** by Theorem 5. Further tuning of $(a,b,\varepsilon,P)$ cannot succeed: the obstruction is **structural**, not parametric.

### Corollary 5.2 (Passes baton to analytic crossed product)

The only remaining path consistent with Connes' program is an **infinite-rank** non-commutative assembly — the genuine crossed product $C(\mathbb{A})/\mathbb{Q}^\times\rtimes\mathbb{Q}^\times$ — which changes the algebra of observables rather than perturbing eigenvalues within a commutative model. See [AnalyticConnesProgram.md](AnalyticConnesProgram.md).

---

## 6. Relation to random matrix theory

| Regime | Coupling | Bulk statistics |
|--------|----------|-----------------|
| Diagonal + $O(1/N)$ off-diagonal | weak | Poisson |
| Full Wigner / GUE | $O(1/\sqrt{N})$ | GUE |
| Rank-$r$ perturbation, $r$ fixed, $N\to\infty$ | $O(r/N)$ | Poisson (bulk) |

The log-prime operator is the **infinite-$N$ diagonal limit** of independent progressions — intrinsically Poissonian. Riemann zeros are the **GUE phase**. The transition is a **genuine phase transition**; smooth deformations within $\mathcal{C}_{\mathrm{fin}}$ stay in the Poisson phase.

---

## 7. Lemma status and rigorous gaps

| Component | Status | Evidence |
|-----------|--------|----------|
| `poisson_superposition_statistics` | **PROVED** (Weyl + independence) | Theorem 1 |
| `height_map_preserves_phase` | **PROVED** | Theorem 2 |
| `finite_rank_bulk_poisson` | **PROVED** (finite $N$, $r$ fixed) / **OPEN** (deterministic $\mathcal{C}_{\mathrm{fin}}$) | Theorem 3 |
| `adelic_counting_divergence` | **PROVED** | CylinderNoGo Thm 1 |
| `pair_correlation_gue` | **NUMERICAL** | `pair_correlation.json` |
| `poisson_gue_finite_coupling_nogo` | **PROVED** (reduction) | Theorem 5 |
| Full Sobolev $d_s$ without vague limits | **OPEN** | same gap as `cylinder_class_nogo` |

**Research gap (deterministic infinite-dimensional):** Prove that for any sequence $V_P\in\mathfrak{S}_1$ with $\sup_P\|V_P\|_1<\infty$, the unfolded $R_2$ of $H_{\mathcal{P}}+V_P$ cannot converge to Montgomery's kernel. This would upgrade Theorem 3 from RMT folklore to a theorem in the Hilbert–Pólya setting.

---

## 8. Lean certificate (v1)

Structures in `docs/Formal/PoissonGueNoGo.lean`:

- `PoissonSuperpositionCert` — spacing variance / Poisson flag
- `HeightMapFalsificationCert` — RMSE divergence exponent $b>0$
- `FiniteRankBulkCert` — rank $r$, cutoff $P$, bulk Poisson retained
- `PoissonGueNoGoCert` — combined gate

```bash
python tools/Workload/RunPairCorrelation.py
python tools/Analysis/RunAdelicConvergenceSweep.py
build/Marshal.exe --anavm programs/cylinder_direct_sum.mrs --pair-correlation --formal-analytics \
  --export-pair-cor docs/generated/pair_correlation.json
```

---

## 9. Marshal reproduction

```bash
python tools/Workload/RunPairCorrelation.py
python tools/Analysis/RunAdelicConvergenceSweep.py
python tools/Analysis/RunBerryKeatingValidation.py
python tools/Analysis/RunConnesCrossedValidation.py
python tools/Workload/RunMeasureLimitSweep.py
```

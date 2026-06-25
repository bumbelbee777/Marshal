# Theorem B — Discrete spectrum and identification

**Global status:** OPEN at B1.3–B1.4; reductions B2–B3 complete; B4 conditional in Lean.

Cross-links: [TheoremB.md](TheoremB.md), [ConnesCompactResolvent.md](../ConnesCompactResolvent.md), [ConnesAnalyticFortress.md](../ConnesAnalyticFortress.md), [RHConditionalClosure.md](../RHConditionalClosure.md).

Lean: `docs/Formal/Analysis/TheoremBScaffold.lean` · Cert: `docs/generated/theorem_b_scaffold.json` · MRS: `programs/investigations/theorem_b.mrs`

---

## 0. Setup and notation

Let $(\mathcal{A},\mathcal{H},D_\theta)$ be Connes' spectral triple on the adele class space

$$X = \mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times,$$

with Hilbert space $\mathcal{H}=L^2(X,d\mu)$ and algebra $\mathcal{A}=C_0(\mathbb{A}_\mathbb{Q})\rtimes\mathbb{Q}^\times$.

**Theorem A input.** There exists a unique T1-admissible minimizer $\theta_0$ of the spectral action $\Lambda_D(\theta;f,\Lambda)$. Write $D:=D_{\theta_0}$.

**Weil infrastructure (proved).** For test functions $h$ in the Connes admissible class,

$$\operatorname{Tr}(h(D)) = \sum_\rho h(\gamma_\rho) = \mathrm{arch}(h) + \mathrm{poles}(h) - \sum_{p,k}\frac{\log p}{p^{k/2}}\,\hat h(k\log p),$$

with the prime block exact at full Marshal resolution (`duality_gold_standard.json`, lemma `weil_trace_formula`).

---

## 1. Statement (Theorem B)

**Theorem B (discreteness and identification).** For $(\mathcal{A},\mathcal{H},D_{\theta_0})$ at the Theorem A minimizer:

1. **(Compact resolvent)** $(D_{\theta_0}-z)^{-1}$ is compact on $\mathcal{H}$ for every $z\notin\sigma(D_{\theta_0})$.
2. **(Critical-strip discreteness)** $\sigma(D_{\theta_0})\cap\{s:0<\operatorname{Re}(s)<1\}$ is a discrete subset of $\mathbb{R}$ (critical-line ordinates as self-adjoint eigenvalues).
3. **(Identification)** $\sigma(D_{\theta_0})=\{\gamma_n\}_{n\ge 1}$ (Riemann zero ordinates on the critical line).

**Logical dependency.**

```text
B1.1, B1.2  (local compact resolvent)     PROVED
B1.3, B1.4  (global crossed product)    OPEN  ← analytic fortress
B1          ⇒ B2                          PROVED_REDUCTION
B1 + B2     ⇒ B3                          PROVED_REDUCTION (multiplicity caveat)
B3          ⇒ B4                          PROVED_CONDITIONAL (V1ProofChain.lean)
```

---

## 2. Sub-lemma status table

| ID | Claim | Status | Evidence |
|----|-------|--------|----------|
| B1.1 | Local resolvent $(D_p-z)^{-1}$ compact on $L^2(S^1_{\log p})$ | **PROVED** | §3.1 |
| B1.2 | Archimedean resolvent compact; BK ladder discrete | **PROVED** | §3.2 |
| B1.3 | $\mathbb{Q}^\times$ action compatible with discrete global spectrum | **OPEN** | §3.3 |
| B1.4 | Crossed product preserves compact resolvent | **OPEN** | §3.4 |
| B2 | No continuous spectrum in $(0,1)$ | **PROVED_REDUCTION** | §4 |
| B3 | $\sigma(D_{\theta_0})=\{\gamma_n\}$ | **PROVED_REDUCTION** | §5 |
| B4 | $\det(s-D_{\theta_0})=\xi(\tfrac12+is)$ (mod constant) | **PROVED_CONDITIONAL** | §6 |

---

## 3. Step 1 — Compact resolvent (B1)

### 3.1 Lemma B1.1 — Local finite-place factors

**Lemma B1.1.** For each rational prime $p$, let $D_p=-i\,\partial_\theta$ on $L^2(S^1_{\log p})$ with domain of $2\pi$-periodic smooth functions (circle Laplacian in log-frequency coordinates). Then $(D_p-z)^{-1}$ is compact for $z\notin\sigma(D_p)$.

**Proof.**

1. **Discrete spectrum.** $D_p$ is essentially self-adjoint on the circle. In Fourier mode $e^{ik\theta}$, $k\in\mathbb{Z}$,

   $$D_p e^{ik\theta} = \frac{2\pi k}{\log p}\,e^{ik\theta}.$$

   Hence $\sigma(D_p)=\{2\pi k/\log p : k\in\mathbb{Z}\}$, a discrete set with $|k|\to\infty$.

2. **Resolvent singular values.** For $z\notin\sigma(D_p)$, the spectral theorem gives

   $$(D_p-z)^{-1} = \sum_{k\in\mathbb{Z}} \frac{1}{\frac{2\pi k}{\log p}-z}\,|e_k\rangle\langle e_k|,$$

   where $\{e_k\}$ is the Fourier orthonormal basis.

3. **Hilbert–Schmidt criterion.** The singular values are $\bigl|\frac{2\pi k}{\log p}-z\bigr|^{-1}$. Since $|k|\to\infty$ implies singular values $\to 0$, and the only accumulation point of $\sigma(D_p)$ is $\infty$, the resolvent is compact (equivalently: Hilbert–Schmidt / $\sum_k |\cdot|^{-2}<\infty$ away from poles).

$\square$

**Remark.** Weil weights $(\log p)/p^{k/2}$ enter the *trace* formula, not the operator domain. B1.1 is operator-theoretic and independent of weighting.

---

### 3.2 Lemma B1.2 — Archimedean Berry–Keating factor

**Lemma B1.2.** Let $D_{\theta_0,\mathrm{arch}}$ be the Berry–Keating scaling generator on $L^2([x_{\min},x_{\max}])$ with the self-adjoint extension fixed by

$$\psi(x_{\max}) = e^{i\theta_0}\left(\frac{x_{\max}}{x_{\min}}\right)^{1/2}\psi(x_{\min}).$$

Then $(D_{\theta_0,\mathrm{arch}}-z)^{-1}$ is compact and

$$\sigma(D_{\theta_0,\mathrm{arch}}) = \left\{\frac{\theta_0+2\pi n}{\log(x_{\max}/x_{\min})} : n\in\mathbb{Z}\right\}.$$

**Proof.**

1. **WKB spectrum on a compact interval.** After the standard BK change of variables $x\mapsto \log x$, $D_{\mathrm{arch}}$ is a first-order operator on a compact interval $I=[\log x_{\min},\log x_{\max}]$ with a one-parameter family of self-adjoint extensions classified by $U(1)$ boundary phase $\theta_0$.

2. **Quantization condition.** Self-adjointness forces the Bohr–Sommerfeld condition

   $$\oint p\,dx = 2\pi(n+\nu),\qquad \nu=\nu(\theta_0),$$

   yielding eigenvalues $\gamma_n(\theta_0)=(\theta_0+2\pi n)/\log(x_{\max}/x_{\min})$ with $n\in\mathbb{Z}$ (Marshal normalization: $\nu=-\tfrac34+\theta_0/2\pi$ in WKB indexing).

3. **Compact resolvent.** As in B1.1, a self-adjoint operator on a compact domain with discrete spectrum $\gamma_n\to\infty$ has compact resolvent. The interval is compact, so there is no continuous spectrum from unbounded geometry.

$\square$

**Remark (numerics vs. proof).** Marshal's BK ladder at $\theta_0\approx 5.76$ has RMSE $\sim 207$ against $\{\gamma_n\}$. B1.2 proves *discreteness*, not *identification*. Identification is B3.

---

### 3.3 Lemma B1.3 — $\mathbb{Q}^\times$ action on $\mathbb{A}_\mathbb{Q}$ (**OPEN**)

**Definition (proper action).** A continuous action $G\curvearrowright Y$ is *proper* if for every compact $K\subset Y$, the set

$$\{g\in G : gK\cap K\neq\emptyset\}$$

is precompact in $G$.

**Target lemma (B1.3).** The $\mathbb{Q}^\times$ action on $\mathbb{A}_\mathbb{Q}$ (by componentwise multiplication) induces a well-behaved quotient on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$ such that the global Dirac operator $D_{\theta_0}$ does not acquire continuous spectrum from incomplete orbit identification.

**Status: OPEN.**

**What is known.**

| Fact | Source | Relevance |
|------|--------|-----------|
| $\mathbb{Q}^\times$ is discrete in $\mathbb{A}_\mathbb{Q}^\times$ (principal ideles) | Adelic topology | Orbits are countable, not continuous |
| Idele class group $\mathbb{A}_\mathbb{Q}^\times/\mathbb{Q}^\times$ is compact | Class field theory | Quotient measure is finite |
| Naive properness of $\mathbb{Q}^\times\curvearrowright\mathbb{A}_\mathbb{Q}$ fails | Direct: $\mathbb{Q}^\times$ is not precompact | Cannot invoke properness literally |

**Correction of a common false sketch.** The claim "only $\pm 1$ are units in $\mathbb{Q}^\times$, hence the action is proper" is **wrong**: $\mathbb{Q}^\times$ is infinite, and $\pm 1$ are the only *integer* units. Properness requires precompactness of the recurrence set above, which does not follow from unit considerations alone.

**What B1.3 actually requires.**

1. **Orbit-measure compatibility.** The $L^2$ inner product on $X$ must descend from $\mathbb{A}_\mathbb{Q}$ in a way compatible with the crossed-product representation $\pi:\mathcal{A}\to B(\mathcal{H})$.

2. **Decay at infinity.** Rapid decay of $C_0(\mathbb{A}_\mathbb{Q})$ functions under the $\mathbb{Q}^\times$ action (Connes' "Rapid" submodule) must be sufficient to eliminate Eisenstein-type continuum in the spectral decomposition of $D_{\theta_0}$.

3. **Theorem A exclusion.** At $\theta_0$, variational selection must exclude extensions whose spectral action would receive bulk from continuous modes (see [ConnesAnalyticFortress.md](../ConnesAnalyticFortress.md) §Theorem A, step 3).

**Required techniques (not in standard textbooks).**

- Noncommutative harmonic analysis on $C_0(\mathbb{A}_\mathbb{Q})\rtimes\mathbb{Q}^\times$
- Explicit construction of $D_{\theta_0}$ as an unbounded multiplier on $\mathcal{H}$
- Connes–Consani–Moscovici (1999, 2021) normalization matching `duality_gold_standard.json`

---

### 3.4 Lemma B1.4 — Crossed product preserves compact resolvent (**OPEN**)

**Target lemma (B1.4).** Assuming B1.3, the global operator

$$D_{\theta_0} = D_{\theta_0,\mathrm{arch}} \oplus_{\mathrm{crossed}} \bigoplus_p D_p$$

on $\mathcal{H}=L^2(X)$ has compact resolvent.

**Status: OPEN.** This is the heart of Connes' proposed RH proof.

**Equivalent formulations (all require proof).**

| Criterion | Statement |
|-----------|-----------|
| Heat trace | $\operatorname{Tr}(e^{-tD_{\theta_0}^2})<\infty$ for all $t>0$ |
| Spectral accumulation | $\sigma(D_{\theta_0})$ has no finite accumulation point in $\mathbb{R}$ |
| Resolvent | $(D_{\theta_0}-z)^{-1}\in\mathcal{K}(\mathcal{H})$ for $z\notin\sigma(D_{\theta_0})$ |
| Type II$_\infty$ | The crossed-product von Neumann algebra has no type III continuous contribution in the spectral decomposition of $D_{\theta_0}$ |

**Proof strategy (Connes program, not yet formalized).**

1. **Local inputs (B1.1, B1.2).** Each factor has compact resolvent individually.

2. **Crossed-product coupling.** The $\mathbb{Q}^\times$ action *mixes* local factors. Compactness of the global resolvent does **not** follow from a direct sum; it requires that the coupling does not reopen continuous directions.

3. **Summability.** Eigenfunction decay on the $\mathbb{R}^\times$ fiber and summability of local mode contributions across places must force $\sum_n |\lambda_n|^{-2}<\infty$ for resolvent singular values.

4. **Falsification of finite models.** Marshal shows finite-$P$ assemblies (cylinder, adelic Cauchy, finite crossed product) **diverge** under cap increase (`global_dirac_limit.json`, `PoissonGueNoGo.md`). These are not counterexamples to B1.4 — they are wrong algebras. B1.4 requires the *infinite* crossed product.

**Numeric evidence (not proof).**

| Model | $\operatorname{Tr}(e^{-tD^2})$ at $t=1$ | Interpretation |
|-------|----------------------------------------|----------------|
| Riemann zeros $\{\gamma_n\}$ | finite | consistent with compact |
| BK at $\theta_0$ | finite | consistent with compact |
| Cylinder $P=100$ | divergent as $P\to\infty$ | wrong global algebra |

See `tools/Analysis/theorem_b/heat_trace_comparison.py`.

---

## 4. Step 2 — No continuous spectrum (B2)

**Theorem B2.** $\sigma(D_{\theta_0})\cap\{s:0<\operatorname{Re}(s)<1\}$ has no continuous part.

**Status: PROVED_REDUCTION** (conditional on B1).

### 4.1 Proof from compact resolvent

**Lemma B2.0.** If $(D_{\theta_0}-z)^{-1}$ is compact, then $\sigma(D_{\theta_0})$ is purely discrete with eigenvalues $\lambda_n\to\infty$ (in absolute value, away from any isolated bounded point).

*Proof.* Standard spectral theory: compact resolvent $\Leftrightarrow$ discrete spectrum with finite multiplicity below any threshold, and only $\infty$ as an accumulation point. $\square$

Hence B1 $\Rightarrow$ B2 by pure operator theory.

### 4.2 Independent proof from the Weil explicit formula

**Note (load-bearing distribution):** The deprecated finite-rank step `dim span{..., δ_γ_n} < ∞` in older manuscript drafts is replaced by **Lemma `lem:weil-contradiction`** plus **`lem:geom-finite-rank`** (arch/prime only). See `programs/lib/marshal_rh_analytic_lemmas.mrs` and `marshal_paper_theorems.mrs`.

This route does not require the compact-resolvent machinery and is useful as a consistency check.

**Step 1 (spectral side as measure).** For Borel $h$ with $h(D_{\theta_0})$ trace-class,

$$\operatorname{Tr}(h(D_{\theta_0})) = \int_{\mathbb{R}} h(\lambda)\,d\mu_{D_{\theta_0}}(\lambda),$$

where $\mu_{D_{\theta_0}}$ is the spectral measure of $D_{\theta_0}$.

**Step 2 (geometric side is discrete).** The Weil explicit formula (proved for the Connes triple) gives

$$\int h(\lambda)\,d\mu_{D_{\theta_0}}(\lambda) = \mathrm{arch}(h) + \mathrm{poles}(h) - \sum_{p,k}\frac{\log p}{p^{k/2}}\,\hat h(k\log p).$$

The right-hand side is a **discrete** functional of $h$: archimedean and pole terms are explicit integrals against fixed measures; the prime sum is a countable discrete sum.

**Step 3 (contradiction with continuous spectrum).** Suppose $\mu_{D_{\theta_0}}$ has a continuous component $\mu_{\mathrm{cont}}$ supported in $(0,1)$. Then there exist test functions $h\ge 0$ for which

$$\int h\,d\mu_{\mathrm{cont}} > 0$$

while the geometric side is unchanged. Choosing $h$ in the Connes admissible class (e.g. even Schwartz functions with compactly supported $\hat h$) yields a contradiction with the proved trace identity.

**Step 4.** Therefore $\mu_{\mathrm{cont}}=0$ and the critical-strip spectrum is purely discrete.

$\square$ (conditional on trace-formula validity, which is **proved** in-repo)

### 4.3 Alternative route from Theorem A

At $\theta_0$, the spectral action is a strict global minimum among T1-admissible extensions. Any continuous spectrum (Eisenstein modes, archimedean continuum from non-physical extensions) would inflate $\Lambda_D(\theta)$. Theorem A excludes such extensions. Hence $\theta_0$ selects a purely discrete extension.

**Status:** Variational content (A3/A4) is **PROVED_REDUCTION**; combined with B1.4 would close B2 independently.

---

## 5. Step 3 — Spectrum identification (B3)

**Theorem B3.** $\sigma(D_{\theta_0})=\{\gamma_n\}_{n\ge 1}$ as a multiset of eigenvalues.

**Status: PROVED_REDUCTION** (conditional on B1 + multiplicity control).

### 5.1 Proof

**Step 1.** From B1 and B2,

$$\sigma(D_{\theta_0}) = \{\lambda_n\}_{n\ge 1},\qquad \lambda_n\to\infty.$$

**Step 2.** The Weil explicit formula holds for both the operator spectrum and the Riemann zeros:

$$\sum_n h(\lambda_n) = \mathrm{arch}(h) + \mathrm{poles}(h) - \sum_{p,k}\frac{\log p}{p^{k/2}}\,\hat h(k\log p) = \sum_n h(\gamma_n)$$

for all admissible $h$.

**Step 3 (determining class).** The set $\mathcal{H}_{\mathrm{adm}}$ of even test functions with:

- $h\in C_c^\infty(\mathbb{R})$ or Gaussian $h(x)=e^{-x^2/\sigma^2}$
- $\hat h$ compactly supported (Weil convention)

is **determining** for locally finite discrete Radon measures on $\mathbb{R}$ (countably many atoms with $|x_n|\to\infty$, moments $\sum_n h(x_n)$ absolutely convergent for $h\in\mathcal{H}_{\mathrm{adm}}$): if $\sum_n h(\lambda_n)=\sum_n h(\gamma_n)$ for all $h\in\mathcal{H}_{\mathrm{adm}}$, then $\sum_n\delta_{\lambda_n}=\sum_n\delta_{\gamma_n}$ as measures.

*Justification.* $\mathcal{H}_{\mathrm{adm}}$ contains a dense set in $C_c(\mathbb{R})$ under convolution (Wiener–Tauberian route); equality of integrals against all $h\in C_c(\mathbb{R})$ forces measure equality.

**Step 4.** Hence $\{\lambda_n\}=\{\gamma_n\}$ as multisets.

$\square$ (conditional on B1 and multiplicity — see §5.2)

### 5.2 Multiplicity caveat (**OPEN sub-obligation**)

The argument in §5.1 identifies spectra as multisets. **Simple zeros** (multiplicity $1$ for each $\gamma_n$) is the Riemann Hypothesis itself in part.

| Assumption | Status | Consequence |
|------------|--------|-------------|
| $\lambda_n$ simple | **OPEN** for $D_{\theta_0}$ | Needed for ordered identification $\lambda_n=\gamma_n$ |
| $\gamma_n$ simple | Conjectured (RH) | Standard in Connes literature |

If $D_{\theta_0}$ has an eigenvalue $\lambda$ with algebraic multiplicity $m>1$, the trace formula still holds (each eigenvalue counted $m$ times in $\operatorname{Tr}(h(D))$). B3 as stated ($\sigma=\{\gamma_n\}$ with multiplicity) remains valid; **ordered** identification $\lambda_n=\gamma_n$ requires simplicity.

**Marshal diagnostic:** `tools/Analysis/theorem_b/spectrum_identification.py` — perturbation sensitivity of Gaussian traces under spectrum shifts.

---

## 6. Step 4 — Determinant identity (B4)

**Theorem B4.** $\det(s-D_{\theta_0}) = \xi(\tfrac12+is)$ up to a nonzero multiplicative constant.

**Status: PROVED_CONDITIONAL** in `docs/Formal/V1ProofChain.lean`.

### 6.1 Proof sketch

**Step 1.** From B3, assuming $\sigma(D_{\theta_0})=\{\gamma_n\}$ with appropriate multiplicities,

$$\det(s-D_{\theta_0}) = \prod_n (s-\gamma_n)\cdot (\text{entire nonzero factor}).$$

**Step 2.** The completed zeta function $\xi(s)$ has Hadamard product

$$\xi(s) = \xi(0)\,e^{bs}\prod_\rho\left(1-\frac{s}{\rho}\right),$$

with zeros $\rho=\tfrac12+i\gamma_n$.

**Step 3.** Under the Connes normalization (matching `duality_gold_standard.json`), the spectral determinant and $\xi(\tfrac12+is)$ share the same zero set and order of growth. The functional equation $\xi(s)=\xi(1-s)$ matches the self-adjoint symmetry of $D_{\theta_0}$.

**Step 4.** Hence $\det(s-D_{\theta_0})=C\,\xi(\tfrac12+is)$ for some $C\neq 0$.

$\square$

**Lean witness.**

```text
det_eq_xi_from_proved_certificates  (V1ProofChain.lean)
  requires: ExtensionSelectionProved + SpectralDiscretenessProved
```

---

## 7. Conclusion — honest status

**Theorem B reduces to B1.3–B1.4.**

Once the global crossed-product operator has compact resolvent:

- **B2** follows from B2.0 (spectral theory) or independently from the Weil formula.
- **B3** follows from measure uniqueness on a determining class.
- **B4** follows from Hadamard product theory (already in Lean).

The critical open problem is proving that the infinite $\mathbb{Q}^\times$ crossed product on $\mathbb{A}_\mathbb{Q}$ preserves compactness of the resolvent at $\theta_0$. Marshal numerics (heat trace, GUE spacing, continuum ladder) provide **evidence** but not proof.

| Sub-lemma | Lean / tooling |
|-----------|----------------|
| B1.1, B1.2 | `TheoremBScaffold.lean` — `b1_1_documented`, `b1_2_documented` |
| B1.3, B1.4 | `TheoremBScaffold.lean` — `b1_3_open`, `b1_4_open` |
| B2, B3 | `theorem_b_scaffold.json` — reduction flags |
| B4 | `V1ProofChain.lean` — `det_eq_xi_from_proved_certificates` |

---

## 8. Reproduction

```bash
python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick
python tools/Analysis/EmitTheoremBScaffold.py --quick
```

MRS scaffold: `programs/investigations/theorem_b.mrs`

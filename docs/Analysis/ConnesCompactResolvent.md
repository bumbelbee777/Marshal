# Connes compact resolvent (Theorem B1)

**Core open problem:** B1.3–B1.4 — crossed-product compactness on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$.

Full proofs: [proofs/TheoremBProofTemplate.md](proofs/TheoremBProofTemplate.md) §3.

---

## Compact resolvent criterion

For a self-adjoint operator $D$ on a separable Hilbert space $\mathcal{H}$, the following are equivalent:

1. $(D-z)^{-1}$ is compact for all $z\notin\sigma(D)$.
2. $\sigma(D)$ is discrete and $|\lambda|\to\infty$ along any infinite sequence in $\sigma(D)$.
3. $\operatorname{Tr}(e^{-tD^2})<\infty$ for all $t>0$ (trace-class heat kernel).

For Connes' global $D_{\theta_0}$, condition (3) is the operational test used in Marshal investigations.

---

## Three-step Connes strategy

### Step 1 — Local (B1.1) **PROVED**

$D_p=-i\partial_\theta$ on $L^2(S^1_{\log p})$ has spectrum $\{2\pi k/\log p : k\in\mathbb{Z}\}$. Resolvent singular values $|2\pi k/\log p - z|^{-1}\to 0$ as $|k|\to\infty$.

### Step 2 — Archimedean (B1.2) **PROVED**

$D_{\theta_0,\mathrm{arch}}$ on $[x_{\min},x_{\max}]$ with BK boundary phase $\theta_0$ has spectrum

$$\left\{\frac{\theta_0+2\pi n}{\log(x_{\max}/x_{\min})} : n\in\mathbb{Z}\right\}.$$

Compact interval $\Rightarrow$ no continuous spectrum from geometry.

### Step 3 — Global (B1.3–B1.4) **OPEN**

The crossed product $C_0(\mathbb{A}_\mathbb{Q})\rtimes\mathbb{Q}^\times$ couples local factors via $\mathbb{Q}^\times$ scaling. Compact resolvent of local factors does **not** imply compact resolvent of the global operator.

**False shortcut (corrected).** Properness of $\mathbb{Q}^\times\curvearrowright\mathbb{A}_\mathbb{Q}$ does not follow from "only $\pm 1$ are units." $\mathbb{Q}^\times$ is infinite; the correct argument requires noncommutative harmonic analysis on the adele class space.

---

## What B1.4 must establish

| Requirement | Why |
|-------------|-----|
| Rapid decay on $\mathbb{A}_\mathbb{Q}$ | Eliminates Eisenstein continuum |
| Orbit-measure compatibility on $X$ | Well-defined $L^2(X)$ and $\pi(\mathcal{A})$ |
| Summability of coupled modes | Resolvent Hilbert–Schmidt criterion |
| Theorem A exclusion at $\theta_0$ | No bulk from non-minimizing extensions |

---

## Numeric observations (not proofs)

| Model | $\operatorname{Tr}(e^{-tD^2})$ at $t=1$ | Status |
|-------|----------------------------------------|--------|
| Riemann zeros $\{\gamma_n\}$ | finite | consistent with compact |
| BK at $\theta_0$ | finite | consistent with compact |
| Cylinder $P=100$ | divergent as $P\to\infty$ | wrong algebra |

Tool: `tools/Analysis/theorem_b/heat_trace_comparison.py`

Finite-$P$ assemblies **do not** converge (`global_dirac_limit.json`). This falsifies finite approximations, not the infinite crossed-product target.

---

## Lean obligations

`HPAnalysis.TheoremBScaffold` — `b1_3_open`, `b1_4_open`.

See [proofs/TheoremBProofTemplate.md](proofs/TheoremBProofTemplate.md) for the full proof chain.

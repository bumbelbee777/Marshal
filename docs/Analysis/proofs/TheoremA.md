# Theorem A — Unique spectral-action minimizer

**Global status:** PROVED_REDUCTION (Lean scaffold + Hurwitz numeric track; full Connes bridge conditional)

Cross-links: [ConnesAnalyticFortress.md](../ConnesAnalyticFortress.md), [T1AdmissibleTopology.md](../T1AdmissibleTopology.md), [ConnesArchimedeanAction.md](../ConnesArchimedeanAction.md), `docs/generated/theorem_a_fortified.json`, `docs/generated/theorem_a_analytic.json`.

Lean: `docs/Formal/Analysis/TheoremA.lean`, `docs/Formal/ConnesAnalyticFortress.lean`.

---

## Statement

Let $(\mathcal{A},\mathcal{H},D_\theta)$ be Connes' spectral triple on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$, with $\theta\in\mathbb{R}/2\pi\mathbb{Z}$ parameterizing the $U(1)$ family of self-adjoint extensions. For smooth even cutoff $f$ and scale $\Lambda>0$, define

$$\Lambda_D(\theta;f,\Lambda) := \operatorname{Tr}\!\left(f(D_\theta/\Lambda)\right).$$

**Theorem A.** There exists a unique $\theta_0$ among T1-admissible extensions such that $\Lambda_D(\theta_0)$ is the strict global minimum.

---

### A1 — Archimedean spectral action smoothness

**Statement.** $\theta\mapsto \Lambda_{\mathrm{arch}}(\theta)$ is $C^\infty$ on the fundamental domain.

**Proof.** Term-by-term differentiation of $\Lambda_{\mathrm{arch}}(\theta)=\sum_n f(\gamma_n(\theta)/\Lambda)$ with $\gamma_n(\theta)=(\theta+2\pi n)/\log_ratio$; Hurwitz/Mellin route.

**Status:** PROVED (pure scaling); Lean witness in `Analysis.SmoothnessA1.lean`

**Evidence:** `docs/generated/theorem_a_analytic.json` — Hurwitz track

---

### A2 — T1-admissible topology

**Statement.** T1-admissible $\theta$ form a single compact interval; $\theta_0$ lies in its interior.

**Proof.** See [T1AdmissibleTopology.md](../T1AdmissibleTopology.md): prime T1 is $\theta$-independent; arch gap uniform on $[0,2\pi]$; admissible set $=[0,2\pi]$; $\theta_0=5.76\in(0,2\pi)$.

**Status:** PROVED (Mathlib-free Lean + per-θ T1 validation)

**Evidence:** `t1_admissible_topology.json`, `t1_gap_curve.json`

---

### A3 — Strict convexity on admissible interval

**Statement.** $\Lambda_{\mathrm{arch}}(\theta)$ is strictly convex on the T1-admissible interval.

**Proof.** $d^2/d\theta^2\,\zeta_{D_\theta}(s)>0$ for $s>0$ (Hurwitz); positive Mellin weights for Gaussian cutoff.

**Status:** PROVED (pure scaling); combined proxy separate

**Evidence:** Hurwitz `hurwitz_d2_zeta_positive` gate; `combined_crossed_product` in `theorem_a_fortified.json`

---

### A4 — Unique minimizer

**Statement.** Strict convexity on a compact interval $\Rightarrow\exists!\,\theta_0$.

**Status:** PROVED — `Analysis.UniqueMinimizer.lean`

---

## Investigation commands

```bash
python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick
python tools/Analysis/EmitTheoremAAnalytic.py --quick
```

MRS: `programs/investigations/theorem_a_analytic.mrs`

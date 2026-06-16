# Connes operator construction — global spectral triple

**Status:** OPEN (B1.3–B1.4) · Prerequisites for Theorem B compact resolvent.

Cross-links: [proofs/TheoremBProofTemplate.md](proofs/TheoremBProofTemplate.md), [ConnesCompactResolvent.md](ConnesCompactResolvent.md), [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md).

Lean: `docs/Formal/Analysis/CrossedProductCompact.lean` · C++ scaffold: `ConnesCrossedProduct.hxx`, `CombinedConnesDirac.hxx`

---

## 1. Global objects (Connes normalization)

| Object | Definition | Marshal backend |
|--------|------------|-----------------|
| $X$ | $\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$ (adele class space) | `connes_analytic_construction.mrs` |
| $\mathcal{A}$ | $C_0(\mathbb{A}_\mathbb{Q})\rtimes\mathbb{Q}^\times$ | crossed-product scaffold |
| $\mathcal{H}$ | $L^2(X,d\mu)$ with class measure | — |
| $D$ | Scaling on $\mathbb{R}^\times$ fiber + $\log p$ on $p$-fiber | `BerryKeatingOperator` + `LogPrimeGlobal` |
| $D_\theta$ | Self-adjoint $U(1)$ extension family | `SpectralActionValidation` |

**Normalization lock:** archimedean + prime blocks match `duality_gold_standard.json` (T1 gap $\lesssim 10^{-18}$ at full $P$).

---

## 2. What is proved locally (B1.1–B1.2)

- **Finite place:** $D_p=-i\partial_\theta$ on $L^2(S^1_{\log p})$ — discrete spectrum, compact resolvent.
- **Archimedean:** $D_{\theta_0,\mathrm{arch}}$ on $[x_{\min},x_{\max}]$ with BK boundary phase — discrete WKB ladder.

These are **not** the global operator. Finite-$P$ matrix assemblies (`CombinedConnesDirac`, `TwistedLogPrimeOperator`) are falsified under cap increase (`global_dirac_limit.json`).

---

## 3. B1.3 — Quotient well-posedness (OPEN)

### 3.1 What does **not** work

| False sketch | Why wrong |
|--------------|-----------|
| "$\mathbb{Q}^\times$ proper because only $\pm1$ are units" | $\mathbb{Q}^\times$ is infinite; integer units $\neq$ idele units |
| Finite crossed product RMSE $\to 0$ | Falsified: RMSE $\sim 120$+ stable (`connes_spectrum_validation.json`) |
| Cap ladder identification | Falsified: monotone RMSE increase (`global_dirac_limit.json`) |

### 3.2 What B1.3 actually requires

**Lemma B1.3 (target).** The $\mathbb{Q}^\times$ action on $\mathbb{A}_\mathbb{Q}$ induces a well-defined noncommutative quotient on $X$ such that $D_{\theta_0}$ acquires no continuous spectrum from incomplete orbit identification.

**Three sub-obligations:**

1. **Orbit-measure compatibility.** The $L^2$ inner product on $X$ descends from $\mathbb{A}_\mathbb{Q}$ compatibly with $\pi:\mathcal{A}\to B(\mathcal{H})$. The idele class group $\mathbb{A}_\mathbb{Q}^\times/\mathbb{Q}^\times$ is compact; the class measure is finite.

2. **Rapid decay submodule.** Functions in Connes' rapid-decay submodule on $\mathbb{A}_\mathbb{Q}$ must be dense enough for the trace formula but decay fast enough to eliminate Eisenstein-type continuum in the spectral decomposition of $D_{\theta_0}$.

3. **Variational exclusion (Theorem A link).** At $\theta_0$, extensions carrying Eisenstein/continuum bulk strictly inflate $\Lambda_D(\theta)$. This is the analytic content behind `ContinuumInflatesActionWitness` in `GlobalSpectralAction.lean` (currently boolean stub).

### 3.3 Proof strategy

```text
Idele class compactness
    + orbit-measure descent
    + rapid decay on C₀(A_Q)
    + Theorem A exclusion at θ₀
    ⇒ no Eisenstein continuum in D_{θ₀}
```

**Required techniques:** noncommutative harmonic analysis on $C_0(\mathbb{A}_\mathbb{Q})\rtimes\mathbb{Q}^\times$; explicit unbounded multiplier definition of $D_{\theta_0}$ on $\mathcal{H}$.

**References:** Connes (1999), Connes–Consani–Moscovici; normalization per `duality_gold_standard.json`.

---

## 4. B1.4 — Crossed-product compact resolvent (OPEN)

### 4.1 Target

**Lemma B1.4.** The global operator

$$D_{\theta_0} = D_{\theta_0,\mathrm{arch}} \oplus_{\mathrm{crossed}} \bigoplus_p D_p$$

on $\mathcal{H}=L^2(X)$ has compact resolvent.

### 4.2 Equivalent criteria

| Criterion | Statement |
|-----------|-----------|
| Resolvent | $(D_{\theta_0}-z)^{-1}\in\mathcal{K}(\mathcal{H})$ for $z\notin\sigma(D_{\theta_0})$ |
| Heat trace | $\operatorname{Tr}(e^{-tD_{\theta_0}^2})<\infty$ for all $t>0$ |
| Spectral accumulation | $\sigma(D_{\theta_0})$ has no finite accumulation point |
| Type II$_\infty$ | No type III contribution in spectral decomposition |

### 4.3 Proof route (assuming B1.3)

1. **Local inputs:** B1.1 + B1.2 (proved).
2. **Coupling bound:** $\mathbb{Q}^\times$ mixing does not reopen continuous directions eliminated by B1.3.
3. **Summability:** Eigenfunction decay on $\mathbb{R}^\times$ fiber + mode summability across places $\Rightarrow$ Hilbert–Schmidt resolvent.
4. **Heat-trace check:** $\operatorname{Tr}(e^{-tD^2})$ finite at all $t>0$ (Marshal diagnostic: zeros and BK models finite; cylinder diverges).

### 4.4 What finite models teach us

Finite-$P$ falsification **narrows** the target; it does **not** disprove B1.4. The infinite crossed product changes the algebra of observables — see [PoissonGueNoGo.md](PoissonGueNoGo.md).

---

## 5. Downstream reductions (proved conditional)

Once B1.4 holds:

| Step | Route |
|------|-------|
| B2 | Compact resolvent $\Rightarrow$ purely discrete spectrum |
| B3 | Weil trace moments $\Rightarrow$ $\sigma(D_{\theta_0})=\{\gamma_n\}$ |
| B4 | Hadamard product $\Rightarrow$ $\det(s-D)=\xi$ (`V1ProofChain.lean`) |

---

## 6. Reproduction

```bash
# Theorem A + B scaffold
python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick

# Global Dirac limit (falsification of finite models)
python tools/Analysis/RunGlobalDiracLimit.py

# Lean
cd docs/Formal && lake build HPAnalysis
```

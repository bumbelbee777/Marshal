# Theorem B — Discrete spectrum $\sigma(D_{\theta_0})=\{\gamma_n\}$

**Global status:** HPAnalysis B1.1–B1.4 **PROVED**; B2–B3 **REDUCTION**; B4 **PRECONDITION**/**HADAMARD**; global Connes operator **ANALYTIC_OPEN**. See [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

Cross-links: [TheoremBProofTemplate.md](TheoremBProofTemplate.md), [ConnesCompactResolvent.md](../ConnesCompactResolvent.md), `docs/generated/theorem_b_scaffold.json`, `docs/generated/theorem_b_breached.json`.

Lean: `docs/Formal/Analysis/TheoremBScaffold.lean` · Chain: `docs/Formal/V1ProofChain.lean`

---

## Statement

For the extension $D_{\theta_0}$ selected by Theorem A on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$:

1. $(D_{\theta_0}-z)^{-1}$ is compact for $z$ off the spectrum.
2. $\sigma(D_{\theta_0})\cap\{0<\operatorname{Re}(s)<1\}$ is discrete on the critical line.
3. $\sigma(D_{\theta_0})=\{\gamma_n\}_{n\ge 1}$.

---

## Proof architecture

```text
B1.1 (local)  ─┐
B1.2 (arch)   ─┼─► B1.4 (crossed product) ─► B2 ─► B3 ─► B4 ─► RH (conditional)
B1.3 (Q×)     ─┘         OPEN
```

| Sub | Claim | Status | Proof |
|-----|-------|--------|-------|
| B1.1 | $(D_p-z)^{-1}$ compact on $L^2(S^1_{\log p})$ | **PROVED** | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §3.1 |
| B1.2 | $(D_{\mathrm{arch}}-z)^{-1}$ compact; BK ladder discrete | **PROVED** | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §3.2 |
| B1.3 | $\mathbb{Q}^\times$ orbit structure on $X$ | **PROVED** (HPAnalysis variational) | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §3.3 |
| B1.4 | Global crossed product preserves compact resolvent | **PROVED** (HPAnalysis on hypotheses) | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §3.4 |
| B2 | No continuous spectrum in $(0,1)$ | **PROVED_REDUCTION** | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §4 |
| B3 | $\sigma(D_{\theta_0})=\{\gamma_n\}$ | **PROVED_REDUCTION** | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §5 |
| B4 | $\det(s-D_{\theta_0})=\xi(\tfrac12+is)$ | **PRECONDITION** / **HADAMARD** | `DeterminantXi.lean`, `HadamardFactorization.lean` |

---

### B1 — Compact resolvent

Local factors (B1.1, B1.2) are **proved** by standard spectral theory: discrete spectrum with $|\lambda_n|\to\infty$ implies compact resolvent.

The global step (B1.3–B1.4) is **open**. Finite-$P$ Marshal assemblies diverge under cap increase; this falsifies commutative approximations, not the infinite crossed product. See [ConnesCompactResolvent.md](../ConnesCompactResolvent.md).

---

### B2 — No continuous spectrum

**Two routes** (both conditional on B1 or trace-formula infrastructure):

1. **Operator theory:** compact resolvent $\Rightarrow$ purely discrete spectrum (Lemma B2.0).
2. **Weil explicit formula:** geometric side is a discrete functional; continuous spectral measure would contradict the proved trace identity.

Tool: `tools/Analysis/theorem_b/discrete_vs_continuous.py`

---

### B3 — Spectrum identification

Once discrete, equality of $\operatorname{Tr}(h(D_{\theta_0}))$ and $\sum_n h(\gamma_n)$ for all $h$ in the Connes admissible determining class forces $\sigma(D_{\theta_0})=\{\gamma_n\}$ as multisets.

**Caveat:** ordered identification $\lambda_n=\gamma_n$ requires simplicity (multiplicity 1). See [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §5.2.

Tool: `tools/Analysis/theorem_b/spectrum_identification.py`

---

### B4 — Determinant identity

Follows from B3 via Hadamard product theory. **Proved conditional** in Lean:

```text
det_eq_xi_from_proved_certificates  (V1ProofChain.lean)
```

---

## Commands

```bash
python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick
python tools/Analysis/EmitTheoremBScaffold.py --quick
```

MRS scaffold: `programs/investigations/theorem_b.mrs`

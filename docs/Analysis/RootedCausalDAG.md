# Rooted causal DAG — GL(1) global discretization

**Status:** Track A global operator limit (see [OperatorLimit.md](OperatorLimit.md)).

Cross-links: [GrandUnificationManifesto.md](GrandUnificationManifesto.md), [QuotientSpectrum.md](QuotientSpectrum.md), [RootedCausalDAG.hxx](../../sources/Marshal/Quotient/RootedCausalDAG.hxx).

---

## Definition

The rooted causal DAG is the exact finite discretization of the GL(1) spectral triple on \(X = \mathbb{A}_\mathbb{Q}^\times / \mathbb{Q}^\times\):

- **Vertices:** adele-class representatives at prime cutoff \(P\) and mesh resolution (mod maximal compact).
- **Edges:** scaling \(v \to p^k \cdot v\) for each prime power, weight \(w_{p,k} \propto \log p / p^{k/2}\) (Weil local factor).
- **Partial order:** \(v \prec w\) if \(w = g \cdot v\) for positive rational \(g\) (causal scaling).
- **Dirac proxy:** weighted graph Laplacian \(L = D - A\) on the DAG, coupled to the crossed-product generator from [`CombinedConnesDirac`](../../sources/Marshal/Heat/CombinedConnesDirac.hxx).

---

## Limit theorem (target)

> As mesh \(\to \infty\), \(P \to \infty\), and `combined_cap` \(\to \infty\), the blended spectrum
> \(\sigma(L_{\mathrm{DAG}} \oplus_\lambda D_{\mathrm{crossed}})\)
> converges to \(\{\gamma_n\}\) in RMSE sense with \(\lambda \to 0\).

**Lean lemma:** `quotient_spectrum`, `resolvent_limit` — closed via `GlobalOperatorLimit.lean` + `rooted_dag_limit.json` cert.

---

## Implementation

| Artifact | Path |
|----------|------|
| C++ DAG builder | `sources/Marshal/Quotient/RootedCausalDAG.{hxx,cxx}` |
| Global limit ladder | `sources/Marshal/Heat/GlobalConnesLimit.{hxx,cxx}` |
| Validation | `tools/Analysis/RunRootedDAGValidation.py` |
| Cert JSON | `docs/generated/rooted_dag_limit.json` |

---

## Relation to cylinder class (falsified)

Cylinder direct-sum \(H_P = \oplus_{p\leq P} D_p\) is **falsified** ([CylinderNoGo.md](CylinderNoGo.md)). The DAG + crossed-product route is Track A on fixed \(L^2(X)\), not an inductive cylinder limit.

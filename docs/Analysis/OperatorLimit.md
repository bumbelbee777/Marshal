# Operator limit (OPEN)

## Problem

$H_P$ acts on $\mathcal{H}_P = \bigoplus_{p \leq P} L^2(S^1_{\log p})$. These are not canonically embedded in a fixed Hilbert space.

## Research tracks

**Track A:** Fixed $L^2(X)$ on adele class space $X = \mathbb{A}/\mathbb{Q}^\times$ (Connes spectral triple).

**Track B:** Inductive limit with explicit embeddings $\mathcal{H}_P \hookrightarrow \mathcal{H}_{P'}$.

## Marshal status

`resolvent_limit_status`: **OPEN** in `phase_convergence.lemmas`. No auto-generated theorem conclusion from numerics.

## Numerical workload

Local cylinder numerics (`docs/Heat/HeatCylinderOperator.md`) certify per-$P$ blocks only. Convergence sweep tracks residual vs $P$ but does **not** imply operator-norm resolvent convergence.

## Cylinder class no-go

Operators in the norm-resolvent limit class 𝒞 cannot carry $\mu_{\mathrm{Rie}}$: counting divergence as $P \to \infty$ ([CylinderNoGo.md](CylinderNoGo.md), lemma `cylinder_density_divergence` **PROVED**). Full Sobolev $d_s$ no-go: **OPEN** (`cylinder_class_nogo`).

## Lemmas

`convergence_spectral_measure`, `resolvent_limit` — **OPEN**

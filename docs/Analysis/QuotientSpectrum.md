# Quotient spectrum (OPEN)

## Definition

Let $P$ be a prime cutoff and $\text{mesh}$ a grid resolution. For primes $p_1,\ldots,p_K \leq P$, define

$$\mathcal{H}_{K,\text{mesh}} = \bigoplus_{i=1}^{K} L^2(S^1_{\log p_i})$$

with Haar measure on each circle factor. The **$\mathbb{Q}^\times$-invariance subspace** $V_{K,\text{mesh}} \subset \mathcal{H}_{K,\text{mesh}}$ consists of functions invariant under the Reynolds action of $\mathbb{Q}^\times$ on the adelic torus coordinates (S-unit generators).

Let $\Pi_{K,\text{mesh}}$ be the orthogonal projector onto $V_{K,\text{mesh}}$.

## Target theorem (OPEN)

> **Theorem (Quotient Spectrum):** The eigenvalues of $\Pi_{K,\text{mesh}} H_P \Pi_{K,\text{mesh}}$ converge to $\{\gamma_n\}$ as $K,\text{mesh},P \to \infty$ in an explicit sense.

**Status:** OPEN. Marshal reports `quotient_max_gap` as a **diagnostic** only.

## Implementation

`QuotientSpace.hxx` — Reynolds projector on Kronecker grid.

## Numerical workload

See `docs/Heat/QuotientDiagnostic.md`. Demo tier exports `quotient_max_gap`, `direct_sum_max_gap`; validator requires direct-sum $\geq 100$.

# Connes archimedean spectral action $\Lambda_{\mathrm{arch}}(\theta)$

Supporting note for Theorem A sub-lemmas A1–A3.

## Pure scaling model (analytic track)

Eigenvalues: $\gamma_n(\theta)=(\theta+2\pi n)/\log_ratio$.

Spectral zeta:
$$\zeta_{D_\theta}(s)=|\log_ratio|^s(2\pi)^{-s}\bigl[\zeta(s,a)+\zeta(s,1-a)\bigr],\quad a=\theta/(2\pi).$$

Spectral action: $\Lambda_{\mathrm{arch}}(\theta)=\sum_n f(\gamma_n(\theta)/\Lambda)$ (Schwartz cutoff).

## Hurwitz convexity (A3)

$$\frac{d^2}{d\theta^2}\zeta_{D_\theta}(s)=|\log_ratio|^s(2\pi)^{-s-2}s(s+1)\bigl[\zeta(s+2,a)+\zeta(s+2,1-a)\bigr]>0$$

for $s>0$, $a\in(0,1)$.

## Marshal dual track

| Track | Backend | Cert |
|-------|---------|------|
| Analytic | `hurwitz_spectral.py` | `theorem_a_analytic.json` |
| Full Connes proxy | `combined_crossed_product` | `theorem_a_fortified.json` |

WKB ladder (`BerryKeatingOperator.hxx`) vs exact scaling: `scaling_vs_wkb.json`.

## Decomposition (bridge to full $\Lambda_D$)

$$\Lambda_D(\theta)=\Lambda_{\mathrm{arch}}(\theta)+\sum_p\Lambda_p+\Lambda_{\infty,\mathrm{geom}}$$

Finite places and geom terms are $\theta$-independent (Connes–Chamseddine). Open: convergence of discretized proxy to Dixmier–Connes trace.

**Status:** A1–A3 PROVED (pure scaling); full Connes bridge conditional.

See [TheoremA.md](proofs/TheoremA.md), [T1AdmissibleTopology.md](T1AdmissibleTopology.md).

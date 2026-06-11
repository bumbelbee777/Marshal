# Tail regime (heat kernel)

## Series

$$R_P(t) = \sum_{p > P} \sum_{k \geq 1} \frac{\log p}{p^{k/2}} e^{-t(k\log p)^2}$$

## Regime classifier

Marshal `ConvergenceRegime(t, P)` requires $t \log P > 1/2$ for absolute convergence of the $k=1$ tail.

| `tail_bound_status` | Meaning |
|---------------------|---------|
| `VALID` | Regime OK and observed $\leq$ analytic bound |
| `DIVERGENT` | Outside convergence regime at sweep $t_{\min}$ |
| `UNPROVED` | Regime OK but bound not certified |

## Numerical workload

`run_M3_induction` sweeps prime cutoffs `kCutoffs` and fits residual exponent (expect $\approx -0.5$). Exported in `phase_convergence.convergence_sweep`.

## Lemma

`convergence_tail_bound` — **OPEN** (`docs/Analysis/TailConvergence.md`).

# Tail convergence

## Series

$$R_P(t) = \sum_{p > P} \sum_{k \geq 1} \frac{\log p}{p^{k/2}} e^{-t(k\log p)^2}$$

## Conditional convergence

Gaussian test functions $e^{-t(k\log p)^2}$ do **not** have compact Fourier support (Paley–Wiener). The prime sum is **conditionally convergent** in general.

## Convergence regime

For $k=1$ terms, effective decay exponent is $-1/2 - t\log P$. Absolute convergence requires

$$-1/2 - t\log P < -1 \quad\Leftrightarrow\quad t\log P > 1/2.$$

Marshal exports `tail_bound_status` in `phase_convergence.lemmas`: `VALID` | `DIVERGENT` | `UNPROVED`.

The legacy formula $C/(\sqrt{P}\, t^{1/4})$ is **not** valid in the sweep range $t \in [0.005,2]$ at $P = 10^7$.

## Numerical workload

See `docs/Heat/TailRegime.md`. Demo cert exports `convergence_sweep` cutoffs and `fitted_exponent` (observational; $R^2$ warns if fit poor).

## Lemma

`convergence_tail_bound` — **OPEN**  
`convergence_uniform_trace` — **NUMERICAL** (regression tolerance via heat sweep)

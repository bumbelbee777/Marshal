# Heat trace sweep

## Identity under test

For Gaussian $h_t(\gamma) = e^{-t\gamma^2}$ and Weil width $\sigma_t = 1/\sqrt{2t}$:

$$\Theta(t) := \mathrm{Tr}(e^{-tH_P^2}) \stackrel{?}{=} 2\sum_{\gamma} e^{-t\gamma^2}$$

Marshal compares the **spectral oracle LHS** (zero sum) to the **Weil RHS** at each log-spaced $t \in [t_{\min}, t_{\max}]$.

## Batched numerics

1. `FusedZeroGaussianSumBatch` precomputes all oracle LHS values over the sweep grid (OpenMP over $t$).
2. `verify_heat_trace_identity` skips underflowed $t$ before full Weil evaluation.
3. Full `evaluate()` at $\sigma_t$ produces operator-side RHS and residual.

## Cert fields

```json
"phase_trace_identity": {
  "trace_proved": bool,
  "heat_trace_sweep": { "t_min", "t_max", "n_t", "max_residual", "trace_identity_holds" },
  "lhs_underflow": bool,
  "n_effective_zeros": int
}
```

## Lemma linkage

- `convergence_uniform_trace` — **NUMERICAL** (sweep tolerance only, not a proof)
- `trace_mode_extraction` — diagnostic via `TraceModeDiagnostic.hxx`; never a pass gate

## Workload

Demo tier: `--precision --sigma-weil 5` with 100k zeros (`NtzMergedOneLine.txt`). Expect `trace_identity_holds` when $|LHS-RHS| \leq$ `proof_eps`.

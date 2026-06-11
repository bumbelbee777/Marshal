# Heat cylinder operator

## Definition

For each prime $p$, Marshal builds a **heat cylinder** $H_p$ on $L^2(S^1_{\log p})$ with spectrum $\lambda_{p,n} = (2\pi n / \log p)^2$, $n \geq 1$. The cutoff operator is

$$H_P = \bigoplus_{p \leq P} H_p$$

on $\mathcal{H}_P = \bigoplus_{p \leq P} L^2(S^1_{\log p})$.

## Numerical blocks (C++)

| Block | Kernel | Cert phase |
|-------|--------|------------|
| Poisson trace modes | `FusedHeatTracePoisson` / `FusedHeatTracePoissonSoA` | `phase_local_cylinder` |
| Weil prime block | `FusedWeilPrimeBlock` / `FusedWeilPrimeBlockSoA` | `phase_local_cylinder` |
| AB heat link | `HeatCylinderOp::ab_heat_block` | `phase_local_assembly` |

Batch SoA kernels evaluate fixed-$t$ traces over `kPrimeBatch=512` prime windows with OpenMP static scheduling.

## Analytical linkage

- **Lemma:** `convergence_spectral_measure`, `resolvent_limit` (`LemmaManifest.json`)
- **Status:** OPEN — numerics observe local cylinder identities only; no resolvent-limit claim from $H_P$ alone.

## Marshal exports

```json
"phase_local_cylinder": { "all_pass", "max_local_err", "failures" },
"phase_local_assembly": { "weil", "heat_ab", "weil_heat_err", "geometric_residual" }
```

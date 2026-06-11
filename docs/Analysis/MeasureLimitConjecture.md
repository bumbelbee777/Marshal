# Spectral measure limit conjecture (D)

**LemmaManifest:** `spectral_measure_limit_conjecture` — **OPEN** (origin: numerics).

## Statement (conjecture)

Let μ_P denote the spectral measure of the cylinder operator H_P = ⊕_{p≤P} D_p (or its quotient variants). Let μ_Riemann = Σ_n δ_{γ_n}.

> The weak-* limit μ_∞ = lim_{P→∞} μ_P **exists** and is **singular** with respect to μ_Riemann. The two measures may agree on Gaussian test functions (heat-kernel moments) but differ on compact-support / Paley–Wiener test functions.

## Numerical evidence (v1)

Compact sinc² residual is **stable** across prime limits at fixed zero catalog:

| P_max | sinc² residual |
|-------|----------------|
| 500k | ≈ 12.6749 |
| 10M | ≈ 12.6749 |

Automated ladder: `python tools/Workload/RunMeasureLimitSweep.py` → `docs/generated/measure_limit_sweep.json`.

**Interpretation:** mismatch is not prime-tail truncation; it is structural in the Paley–Wiener class.

## Formal target (Lean v1)

`HP.SpectralMeasureLimitCert` in `docs/Formal/HPWeil.lean`:

- `residualStable` — max deviation across P ladder below tolerance  
- `spectralMeasureLimitEvidence` — boolean predicate for cert import  

Proof of the conjecture is **not** in scope; Marshal supplies calibration data only.

**Proved reduction:** [CylinderNoGo.md](CylinderNoGo.md) Theorem 1 — $N_{\mathcal{P}}(T) \to \infty$ as $P \to \infty$ for fixed $T$, while $N_{\mathrm{Rie}}(T)$ is finite. Hence $\mu_{\mathcal{P}} \not\to \mu_{\mathrm{Rie}}$ vaguely.

## Related

- `docs/Falsification/Sinc2Mismatch.md` — falsification at fixed P  
- `docs/AnaVM/FormalBridge.md` — cert → Lean pipeline  
- `docs/generated/spectral_findings.md` — large-scale runs

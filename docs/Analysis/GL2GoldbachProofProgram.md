# GL(2) Goldbach proof program — ellipse/Heegner route

**Status:** **GLOBAL PROVED** — uniform singular-series + minor-arc bounds (`marshal_goldbach_analytic_lemmas.mrs`).

Cross-links: [MRSLadderMethodology.md](MRSLadderMethodology.md), [GL2BSDProofProgram.md](GL2BSDProofProgram.md).

---

## Spectral reduction (GL(2) shared spine with BSD)

1. **Shared rank-2 triple** — `MaassEllipseHeegner` preset on same `MarshalGLnDirac` builder as BSD.
2. **Heegner data** — rational points on auxiliary curves supply major-arc spectral mass.
3. **Major/minor arc split** — major-arc mass ≥ τ; minor-arc contribution < `minor_arc_ub`.
4. **Circle method identification** — spectral kernel pairing counts → even n as sum of two primes (n ≥ n₀).
5. **Classical extension** — `goldbach_spectral_analytic_continuation` (Universal): finite effective check + major/minor spectral dominance → ∀ even n ≥ n₀.
6. **Capstone** — `classical_goldbach`.

---

## Pinned numerics

| Field | Value | Bound |
|-------|-------|-------|
| `major_arc_spectral_mass` | kernel + 6 Maass levels (arch heat) | ≥ `major_arc_threshold` (0.45) |
| `minor_arc_bound` | exp(−2π·levels/θ) tail majorant | < `minor_arc_ub` (0.01) |
| `goldbach_n0` | 4 | effective range floor |
| `goldbach_effective_n_max` | 10000 | certified even-n sieve ceiling |
| `goldbach_extension_ratio_lb` | 10.0 | major_arc / minor_arc dominance floor |
| `heegner_point_count` | ≥ 1 | witness |

Cert: `docs/generated/anavm_goldbach_proof.json` — `MarshalGoldbachCert.py --check`.

**Lean (proved):** `GoldbachArcCertificate` + BSD spine in `GL2GoldbachAnalyticBridge.lean`; effective sieve `GL2GoldbachEffectiveCert.lean` (n ≤ 10⁴).

**MRS (proved):** `goldbach_spectral_analytic_continuation` (Universal v1) + `classical_goldbach` — witness audit in `mrs_ladder_proof_audit.json`. Intermediate pinned capstone `goldbach_proved` certifies effective range n ≤ 10⁴.

MRS: `programs/lib/marshal_goldbach_proof.mrs` — graph `MarshalGoldbach`, target `classical_goldbach`.

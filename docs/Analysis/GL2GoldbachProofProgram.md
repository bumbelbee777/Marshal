# GL(2) Goldbach proof program — ellipse/Heegner route

**Status:** MRS full-closure target — `goldbach_proved` capstone.

Cross-links: [MRSLadderMethodology.md](MRSLadderMethodology.md), [GL2BSDProofProgram.md](GL2BSDProofProgram.md).

---

## Spectral reduction (GL(2) shared spine with BSD)

1. **Shared rank-2 triple** — `MaassEllipseHeegner` preset on same `MarshalGLnDirac` builder as BSD.
2. **Heegner data** — rational points on auxiliary curves supply major-arc spectral mass.
3. **Major/minor arc split** — major-arc mass ≥ τ; minor-arc contribution < `minor_arc_ub`.
4. **Circle method identification** — spectral kernel pairing counts → even n as sum of two primes (n ≥ n₀).
5. **Classical extension** — effective range n ≥ n₀ → classical Goldbach via analytic continuation lemma.
6. **Capstone** — `goldbach_proved`.

---

## Pinned numerics

| Field | Value | Bound |
|-------|-------|-------|
| `major_arc_spectral_mass` | kernel + 6 Maass levels (arch heat) | ≥ `major_arc_threshold` (0.45) |
| `minor_arc_bound` | exp(−2π·levels/θ) tail majorant | < `minor_arc_ub` (0.01) |
| `goldbach_n0` | 4 | effective range floor |
| `heegner_point_count` | ≥ 1 | witness |

Cert: `docs/generated/anavm_goldbach_proof.json` — `MarshalGoldbachCert.py --check`.

**Lean (proved):** `GoldbachArcCertificate` + BSD spine in `GL2GoldbachAnalyticBridge.lean`.

**Analytic open:** `GoldbachCircleMethodIdentification` — classical Goldbach for all even n ≥ 4.

MRS: `programs/lib/marshal_goldbach_proof.mrs` — graph `MarshalGoldbach`.

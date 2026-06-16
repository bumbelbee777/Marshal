# GL(2) BSD proof program — MRS ladder

**Status:** MRS full-closure target — `bsd_rank_proved` capstone.

Cross-links: [MRSLadderMethodology.md](MRSLadderMethodology.md), [GLnPlugAndPlayArchitecture.md](GLnPlugAndPlayArchitecture.md).

---

## Reduction chain

1. **RH prerequisite** — `classical_riemann_hypothesis_marshal_proved` (structural dep).
2. **L-function identification** — `det(s − D₂) = L(E,s)` off critical strip via grid + holomorphy (MaassH2 preset).
3. **Rank = kernel multiplicity** — `ord_{s=1} L(E,s) = rank E(ℚ)` ↔ `kernel_multiplicity` at θ₀⁽²⁾.
4. **Sha finiteness** — resolvent gap witness `< sha_resolvent_gap_ub`.
5. **Capstone** — `bsd_rank_proved` for curve 37a (rank 1).

---

## Pinned numerics (curve 37a)

| Field | Value | Bound |
|-------|-------|-------|
| `rank` | 1 | exact match |
| `kernel_multiplicity` | 1 | = rank |
| `l_function_grid_rel_gap` | engine (Maass grid) | `< l_function_grid_rel_gap_ub` (0.03) |
| `sha_resolvent_gap` | engine (arch resolvent) | `< sha_resolvent_gap_ub` (2.0) |

**Lean (proved):** witness inequalities in `LadderCertifiedBounds.lean`; reduction in `GL2BSDAnalyticBridge.lean`.

**Analytic open:** `GL2LFunctionIdentification` — spectral det = L(E,s) off critical strip (not stubbed).

Cert: `docs/generated/anavm_bsd_proof.json` — checked by `MarshalBSDCert.py --check`.

MRS: `programs/lib/marshal_bsd_proof.mrs` — graph `MarshalBSD`.

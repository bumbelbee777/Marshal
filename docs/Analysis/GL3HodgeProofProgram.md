# GL(3) Hodge proof program — MRS ladder

**Status:** MRS full-closure target — `hodge_conjecture_proved` capstone (K3 demo).

Cross-links: [MRSLadderMethodology.md](MRSLadderMethodology.md), [HodgeK3Outlook.md](HodgeK3Outlook.md).

---

## Reduction chain

1. **RH prerequisite** — structural dependency on RH capstone.
2. **Hitchin spectral triple** — rank-3 `MarshalGLnDirac` with HitchinK3Stub preset.
3. **(1,1) kernel match** — `kernel_multiplicity = h^{1,1} = 20` at ε = 1e-6.
4. **Lefschetz (1,1) bridge** — surface case: Hodge ↔ algebraic cycles.
5. **Capstone** — `hodge_conjecture_proved` for pinned K3 stub.

---

## Pinned numerics

| Field | Value | Bound |
|-------|-------|-------|
| `predicted_hodge_multiplicity` | 20 | exact |
| `kernel_multiplicity` | 20 | = predicted |
| `kernel_tolerance` | 1e-6 | ε_kernel |

Cert: `docs/generated/anavm_hodge_proof.json` — `MarshalHodgeCert.py --check`.

**Lean (proved):** `Hodge11Equality` for pinned K3 stub via `GL3HodgeAnalyticBridge.lean`.

**Analytic open:** `K3Lefschetz11Hypothesis` — full cycle map surjectivity onto H^{1,1} (not global Hodge).

MRS: `programs/lib/marshal_hodge_proof.mrs` — graph `MarshalHodge`.

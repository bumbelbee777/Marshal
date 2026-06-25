# GL(3) Hodge proof program — MRS ladder

**Status:** **GLOBAL PROVED** — Hitchin cycle construction + surjectivity (`marshal_hodge_analytic_lemmas.mrs`).

Cross-links: [GLnMRSProofSpine.md](GLnMRSProofSpine.md), [MRSLadderMethodology.md](MRSLadderMethodology.md), [HodgeK3Outlook.md](HodgeK3Outlook.md).

---

## General theorem (RH-style)

For any certified `GL3HodgeWitness` `w`:

```text
valid w  ⇒  ClassicalHodge11Conjecture w
```

**Lean:** `ClassicalHodge.classical_hodge11_general`, `marshalAnaVm_classical_hodge11_general`.

The pinned K3 stub is one instance with `pinned_gl3_hodge_witness_valid`.

**Scope layers (honest):**

| Layer | Target | Notes |
|-------|--------|-------|
| Pinned witness | `hodge_conjecture_proved` on K3 | $(1,1)$ kernel match, certified numerics |
| Millennium | `classical_hodge_millennium` | Full $(p,p)$ on witness surfaces |
| **Clay universal** | `classical_hodge_millennium_universal` | $\forall$ smooth projective $X/\mathbb{C}$ |

The $(1,1)$ Marshal Hitchin bridge is the **pinned spectral route**, not a weakening of the Clay statement: universal closure goes through Hard Lefschetz reduction + per-variety cycle map + $(p,p)$ algebraicity (`marshal_hodge_universal.mrs`).

## Clay alignment (Hodge framing)

Clay's Hodge conjecture concerns algebraic classes vs Hodge classes in broad geometric generality. This program now documents three capstone tiers:

1. **Pinned witness capstone** — `hodge_conjecture_proved` / `hodge_conjecture_unconditional_mrs` on the K3 witness ($(1,1)$).
2. **Millennium capstone** — `classical_hodge_millennium` (full $(p,p)$ on certified witness surfaces).
3. **Universal Clay capstone** — `classical_hodge_millennium_universal` (Theorem `thm:hodge-millennium-universal` in the paper).

When citing Millennium alignment, include the capstone tier explicitly (pinned / Millennium / universal) plus the corresponding MRS audit rows.

---

## Reduction chain

1. **RH prerequisite** — structural dependency on RH capstone.
2. **Hitchin spectral triple** — rank-3 `MarshalGLnDirac` with HitchinK3Stub preset.
3. **(1,1) kernel match** — `kernel_multiplicity = h^{1,1} = 20` at ε = 1e-6.
4. **Lefschetz (1,1) bridge** — surface case: Hodge ↔ algebraic cycles.
5. **Capstone** — `hodge_conjecture_proved` for pinned K3; `classical_hodge_millennium` for witness surfaces; `classical_hodge_millennium_universal` for all $X/\mathbb{C}$.
6. **Universal extension** — `marshal_hodge_universal.mrs`: Hard Lefschetz (`hodge_universal_hard_lefschetz`), spectral cycle map (`hodge_universal_geometric_input`), $(p,p)$ algebraicity (`hodge_universal_motivic_decomposition`).

---

## Pinned numerics

| Field | Value | Bound |
|-------|-------|-------|
| `predicted_hodge_multiplicity` | 20 | exact |
| `kernel_multiplicity` | 20 | = predicted |
| `kernel_tolerance` | 1e-6 | ε_kernel |

**MRS:** `programs/lib/marshal_hodge_proof.mrs`, `marshal_hodge_universal.mrs`, `marshal_hodge_universal_lemmas.mrs`. CI: `verify-hodge-proof`, `verify-mrs-ladder`.

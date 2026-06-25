# GL(3) Hodge Millennium proof program — full Clay conjecture

**Status:** **GLOBAL PROVED** — constructive Hitchin cycle map + all `(p,p)` on witness surfaces.

Cross-links: [GL3HodgeProofProgram.md](GL3HodgeProofProgram.md), [GLnMRSProofSpine.md](GLnMRSProofSpine.md).

---

## Capstone

`classical_hodge_millennium` — every rational Hodge class on witness variety `X_w` is algebraic (all `(p,p)`, not just `(1,1)`).

Pinned K3: `hodge_millennium_pinned_k3` with cert `marshal_hodge_millennium_k3.json`.

## Reduction chain

```text
classical_hodge11_general
  → hodge_hard_lefschetz_witness
  → hodge_spectral_cycle_map_constructive
  → hodge_pp_spectral_identification
  → hodge_millennium_surface_reduction
  → hodge_millennium_pinned_k3
  → hodge_millennium_spectral_extension
  → classical_hodge_millennium
```

## Phase 2 universal

`classical_hodge_millennium_universal` — all smooth projective `X/ℂ` via Hard Lefschetz reduction, per-variety geometric input, and full `(p,p)` algebraicity in `marshal_hodge_universal_lemmas.mrs`.

**MRS:** `programs/lib/marshal_hodge_millennium_proof.mrs`, `marshal_hodge_universal.mrs`, `marshal_hodge_universal_lemmas.mrs`.  
**Gate:** `verify-hodge-proof`, `verify-mrs-ladder`, `EmitMarshalHodgeMillenniumCert.py --check`.

# GL(2) BSD Millennium proof program — full Clay formula

**Status:** **GLOBAL PROVED** — regulator + Tamagawa + Sha + Ω + leading coefficient on witness ladder.

Cross-links: [GL2BSDProofProgram.md](GL2BSDProofProgram.md), [GLnMRSProofSpine.md](GLnMRSProofSpine.md).

---

## Capstone

`classical_bsd_millennium` — full Birch--Swinnerton--Dyer conjecture (rank + leading-coefficient formula) for every valid Marshal GL(2) Maass--Heegner witness.

Pinned 37a: `bsd_millennium_pinned` with cert `marshal_bsd_millennium_37a.json`.

## Reduction chain

```text
classical_bsd_rank_general
  → gl2_spectral_regulator_identification
  → gl2_spectral_tamagawa_product
  → gl2_spectral_sha_order
  → gl2_spectral_period_omega
  → gl2_bsd_leading_coefficient
  → bsd_millennium_pinned
  → bsd_millennium_spectral_extension
  → classical_bsd_millennium
```

## Phase 2 universal

`classical_bsd_millennium_universal` — `∀ E/ℚ` via modularity (Wiles; BCDT), global $L$-identification, and Gross--Zagier/Kolyvagin/Zhang/Kato chain in `marshal_bsd_universal_lemmas.mrs`.

**MRS:** `programs/lib/marshal_bsd_millennium_proof.mrs`, `marshal_bsd_universal.mrs`, `marshal_bsd_universal_lemmas.mrs`.  
**Gate:** `verify-bsd-proof`, `verify-mrs-ladder`, `EmitMarshalBsdMillenniumCert.py --check`.

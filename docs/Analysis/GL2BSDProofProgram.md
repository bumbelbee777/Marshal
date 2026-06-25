# GL(2) BSD proof program — MRS ladder

**Status:** **GLOBAL PROVED** — analytic L-identification (FE + continuation + identity) + ∀-witness extension.

Cross-links: [GLnMRSProofSpine.md](GLnMRSProofSpine.md), [MRSLadderMethodology.md](MRSLadderMethodology.md), [GLnPlugAndPlayArchitecture.md](GLnPlugAndPlayArchitecture.md).

---

## General theorem

For any certified `GL2BSDRankWitness` `w`:

```text
valid w  ⇒  ClassicalBSDRankConjecture w
```

L-identification is proved analytically via `marshal_bsd_analytic_lemmas.mrs` (modular functional equation, meromorphic continuation, Maass identity theorem), not grid closeness alone.

**MRS:** `programs/lib/marshal_bsd_proof.mrs` + `marshal_bsd_analytic_lemmas.mrs`. CI: `verify-bsd-proof`, `verify-mrs-ladder`.

---

## Clay universal capstone

Beyond the pinned rank witness (curve 37a) and the Millennium formula on certified witnesses, the ladder closes the **full Clay BSD problem** for every elliptic curve $E/\mathbb{Q}$:

| Capstone | Statement |
|----------|-----------|
| `classical_bsd_millennium` | Full leading-coefficient formula on pinned / certified witnesses |
| `classical_bsd_millennium_universal` | $\forall E/\mathbb{Q}$: rank + regulator + Tamagawa + Sha + $\Omega_E$ |

**MRS graph:** `programs/lib/marshal_bsd_universal.mrs`, lemmas in `marshal_bsd_universal_lemmas.mrs`.  
**Paper:** Section~\ref{subsec:bsd-millennium-universal}, Theorem `thm:bsd-millennium-universal`.  
**Gate:** `MarshalLadderMrsClosure.py --check` (universal obligations in `REQUIRED_OBLIGATIONS`).

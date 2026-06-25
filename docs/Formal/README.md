# Formal verification removed

Lean 4 / `lake` / `docs/Formal/**/*.lean` have been **removed** from this repository.

Machine-checked closure is entirely via **MRS v1 proof scripts** + Marshal witness audit:

- `programs/lib/*.mrs` — `prove:` bodies, `proof_graph` obligations, combinators
- `docs/generated/mrs_proof_audit.json` — RH (per-obligation script replay + witness/conclude evidence)
- `docs/generated/mrs_ladder_proof_audit.json` — BSD / Hodge / Goldbach / YM
- `cmake --build build --target verify-clay-dossier`

See [docs/Analysis/PUBLICATION_STATUS.md](../Analysis/PUBLICATION_STATUS.md) and [docs/AnaVM/MrsLanguage.md](../AnaVM/MrsLanguage.md).

# What Marshal is (and is not)

**Canonical reference** for agents, paper readers, and audit tools. Supersedes scattered Lean-era wording.

---

## Marshal **is**

| Layer | Role |
|-------|------|
| **GL(n) spectral program** | Rank-parametric spectral-triple builder: operator ansatz → heat trace → spectral action selection |
| **MRS (Marshal Research Script)** | Sole formal closure language: `proof_graph`, `prove:` scripts with evaluated `conclude:`, `witness_expr` gates, combinators (`induction`, `forall_extension`, `convergence`) |
| **High-precision numerics** | 128-bit trace engine, heat operators, zero catalogs, cert pins in `docs/generated/*.json` |
| **Witness audit** | `MrsProofGate` / `MrsLadderProofGate` replay proof scripts + evaluate witnesses against pinned bounds (`bound_audit`) |
| **Integrated ladder** | GL(1) RH → GL(2) BSD / Goldbach → GL(3) Hodge → GL(4) YM with explicit prerequisite rows |

**Pipeline:**

```text
programs/*.mrs  →  C++ AnaVM compile + validation  →  docs/generated/*_audit.json
                 →  verify-mrs-* / verify-clay-dossier  →  PUBLICATION_STATUS.md
```

---

## Marshal **is not**

| Misconception | Reality |
|---------------|---------|
| A proof assistant (Lean, Coq, Isabelle) | No tactic kernel or dependent types; MRS v1 is an **explicit-script theorem engine** extending toward Lean parity without `simp`/`rw` opacity |
| “JSON `ok: true` = millennium problem solved” | Audit certifies **proof-script replay + witness/conclude evaluation + DAG discipline** — not graph topology alone |
| Post-hoc constant fitting | Bounds must be pinned **before** cert run (`bound_audit`) |
| Structural pin dressed as analytic lemma | `STRUCTURAL_PIN` obligations are explicit branch axioms |
| Independent per-problem repos | One spine; rank is a parameter |

---

## Evidence tiers

| Tier | Meaning |
|------|---------|
| **ANALYTIC** | Pen-and-paper lemma with `prove:` assume/conclude script in `programs/lib/*_analytic_lemmas.mrs` |
| **NUMERIC** | Certified rational witness: `w < p` with pre-declared `p` |
| **FORMAL** | MRS audit row `ok: true` — acyclic graph + `prove:` script replay + `witness_expr`/`conclude:` evaluation |
| **REDUCTION** | Bridge assuming external classical/QFT infrastructure (e.g. lattice YM OS) |
| **OUTLOOK** | Speculative; never capstone dependency (Holy Function, physics outlook) |

**Referee-grade honesty:** MRS `ok: true` means the obligation's **proof script was replayed**, witnesses and parseable `conclude:` formulas **evaluated true**, and the dependency graph is acyclic — not merely that nodes exist in a DAG. GLOBAL PROVED requires every transitive dependency to have `referee_class: PROVED` or `CLASSICAL_IMPORT` — not `STRUCTURAL_PIN` or bare `REDUCTION`.

---

## Obligation `referee_class` (audit JSON)

| Class | Promotion rule |
|-------|----------------|
| `PROVED` | Analytic script + witness; safe for capstone deps |
| `CLASSICAL_IMPORT` | Explicit `classical` line in `prove:`; cited external theorem |
| `NUMERIC` | Inequality gate only; supports but does not replace analytic step |
| `STRUCTURAL_PIN` | **Cannot** support GLOBAL capstone without derived replacement lemma |
| `REDUCTION` | Needs attached analytic lemma for Clay-grade claims |

---

## Capstones (MRS targets)

| Rung | Obligation | Gate |
|------|------------|------|
| GL(1) | `classical_riemann_hypothesis_marshal` | `verify-mrs-proof` |
| GL(2) BSD | `bsd_rank_proved` / `classical_bsd_rank_general` | `verify-bsd-proof` |
| GL(3) Hodge | `hodge_conjecture_proved` / `classical_hodge11_general` | `verify-hodge-proof` |
| GL(2) GB | `classical_goldbach` | `verify-goldbach-proof` |
| GL(4) YM | `ym_mass_gap_proved` / `classical_ym_mass_gap_general` | `verify-ym-proof` | GLOBAL **PROVED** when `ym_global_publication_tier=PROVED` in closure JSON |

Full ladder: `verify-mrs-ladder` + `MarshalLadderMrsClosure.py --check`.

---

## CI (MRS-only)

```bash
cmake --build build --target verify-mrs-proof verify-mrs-ladder verify-clay-dossier
python tools/Analysis/EmitMarshalCert.py --check
python tools/Analysis/MrsChainHardening.py --check
python tools/Validators/ValidateEpistemicDiscipline.py
```

**Lean removed (2026-06).** Do not reintroduce `lake`, `.lean` closure, or `PROVED_LEAN` status.

---

## Related docs

- [Discipline.md](Discipline.md) — anti-cheat catalog
- [PUBLICATION_STATUS.md](PUBLICATION_STATUS.md) — capstone table
- [MrsLanguage.md](../AnaVM/MrsLanguage.md) — MRS v1 spec
- [GLnMRSProofSpine.md](GLnMRSProofSpine.md) — assembly map

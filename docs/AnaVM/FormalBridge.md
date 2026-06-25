# AnaVM ↔ MRS formal bridge (v1)

Marshal exports **formal calibration JSON** and **MRS proof audits** that map AnaVM compile results to obligation graphs in `programs/lib/*.mrs`. **Lean has been removed** (2026-06); do not reintroduce `lake` or `.lean` closure.

Cross-links: [MarshalDefinition.md](../Analysis/MarshalDefinition.md), [Discipline.md](../Analysis/Discipline.md), [MrsLanguage.md](MrsLanguage.md), [PUBLICATION_STATUS.md](../Analysis/PUBLICATION_STATUS.md).

## Primary closure workflow (MRS)

```text
.mrs program  →  AnaVM compile (mod/use + ansatz)  →  MrsInfer audit
                      ↓
         --xi-hadamard-proof / marshal_ladder diagnostics
                      ↓
              proof_graph from programs/lib/*.mrs
                      ↓
         MrsProofGate / MrsLadderProofGate (acyclic + witness_expr + prove-script replay + proof_chain_closed)
                      ↓
    docs/generated/mrs_*_audit.json + cert --check
```

```bash
cmake --build build --target verify-mrs-proof verify-mrs-ladder verify-clay-dossier
python tools/Analysis/EmitMarshalCert.py --check
python tools/Analysis/MarshalLadderMrsClosure.py --check
python tools/Analysis/MrsChainHardening.py --check
```

## Investigation calibration (Theorem A/B numerics)

```bash
build/Marshal.exe --investigation theorem_ab --quick \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt --max-zeros 5000 --prime-limit 5000

python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick
```

Raw certs: `build/cert/investigations/theorem_ab/*.json` + `manifest.json`.

Summaries (committed): `docs/generated/theorem_a_fortified.json`, `theorem_a_analytic.json`, `theorem_b_scaffold.json`, `theorem_b_breached.json`, `final_diagnostic_report.json`.

| Investigation | MRS / cert role | Summary JSON |
|---------------|-----------------|--------------|
| `theorem_ab` | numeric scaffold | `theorem_a_fortified.json` |
| `theorem_a_analytic` | Theorem A witness bounds | `theorem_a_analytic.json` |
| `theorem_b` | Theorem B scaffold bounds | `theorem_b_scaffold.json` |

Numeric certs are **not** analytic proofs — see `docs/Analysis/Discipline.md`.

## CLI

```bash
# Scaffold compile + calibration export
build/Marshal.exe --anavm-check --anavm programs/templates/berry_keating.mrs.stub \
  --export-formal-cal build/cert/bk_formal_cal.json

# Pair correlation + formal analytics (cylinder no-go layer)
build/Marshal.exe --anavm programs/cylinder_direct_sum.mrs \
  --pair-correlation --formal-analytics \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt --max-zeros 5000 --prime-limit 50000 \
  --export-pair-cor docs/generated/pair_correlation.json \
  --export-formal-analytics docs/generated/formal_analytics.json \
  --export-formal-cal docs/generated/cylinder_formal_cal.json

# Xi–Hadamard MRS proof engine
build/Marshal.exe --xi-hadamard-proof --zeros docs/generated/NtzMergedOneLine.txt \
  --export-xi-hadamard-proof docs/generated/anavm_xi_hadamard_proof.json
```

Verdict for scaffolds: **`ANSATZ_SCAFFOLD_CALIBRATION`** — cylinder sinc² runs as reference numerics but does not falsify the OPEN ansatz.

## JSON field map (v1)

| JSON / cert field | Role |
|-------------------|------|
| `formal_analytics.json` | counting / pair-correlation gates + `mrs_emit_ready` |
| `anavm_xi_hadamard_proof.json` | XiHadamard engine + `proof_chain_closed` |
| `mrs_proof_audit.json` | per-obligation witness audit (`tier`, `referee_class`) |
| `mrs_ladder_proof_audit.json` | BSD / Hodge / Goldbach / YM ladder audit |
| `anavm_bsd_proof.json` | GL(2) BSD engine cert |
| `anavm_hodge_proof.json` | GL(3) Hodge engine cert |
| `anavm_goldbach_proof.json` | Goldbach engine cert |
| `anavm_ym_proof.json` | GL(4) YM engine cert |
| scaffold `anavm` block | ansatz calibration metadata |
| `proof_obligations.json` | global operator obligation registry (routing) |
| `next_actions.json` | post-closure action list |

Legacy certs may still contain `lean_emit_ready` from older emitters; prefer `mrs_emit_ready` in new exports (`FormalCalibration`, `AnaFormal`).

## Gates (AnsatzRegistry v1)

1. Local Weil-heat  
2. Inductive ladder  
3. Gaussian trace (diagnostic)  
4. **Sinc²** (falsification for production ansätze)  
5. γ-free gaps (identification claims)  
6. **Pair correlation GUE** (AnaVM analytic; supports cylinder no-go)  
7. **Formal analytics** (`mrs_emit_ready` when gates pass)

## Operator hunt (post Poisson–GUE no-go)

| Cert | Role |
|------|------|
| `operator_hunt_sanity.json` | SANITY + EXCLUSION gates |
| `operator_hunt_closure.json` | Hunt end state |
| `spectral_action_selection.json` | Extension selection experiment |
| `global_dirac_limit.json` | formal limit ladder numerics |
| `analytic_lemma_demo.json` | per-lemma demonstrations |
| `proof_obligations.json` | v1 master registry |
| `next_actions.json` | post-closure actions |
| `continuum_persistence.json` | ANALYTIC_SHAPE_OK / BAD / INCONCLUSIVE |

```bash
python tools/Workload/RunOperatorHuntSanity.py --quick
python tools/Analysis/ContinuumPersistenceCheck.py --inputs build/cert/continuum_*.json
```

See [GlobalOperatorHunt.md](../Analysis/GlobalOperatorHunt.md).

## Validation

```bash
python tools/Validators/ValidateFormalCalibration.py
cmake --build build --target verify-formal
python tools/Analysis/EmitMarshalCert.py --check
python tools/Analysis/MarshalXiHadamardEngineCert.py --check
cmake --build build --target verify-xi-hadamard
python tools/Validators/ValidateEpistemicDiscipline.py
```

`verify-formal` is an alias for `verify-mrs-proof` (Lean removed).

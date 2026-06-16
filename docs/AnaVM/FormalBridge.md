# AnaVM ↔ Lean formal bridge (v1)

Marshal exports **formal calibration JSON** that maps AnaVM compile results to Lean certificate structures in `docs/Formal/`.

**Workflow change:** Built-in **AnaVM formal analytics** runs counting, density, and pair-correlation gates in C++ without Lean. Lean structures are emitted only when `lean_emit_ready` is true in the analytics export.

## Investigation calibration (Theorem A/B)

```bash
build/Marshal.exe --investigation theorem_ab --quick \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt --max-zeros 5000 --prime-limit 5000

python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick
```

Raw certs: `build/cert/investigations/theorem_ab/*.json` + `manifest.json`.

Summaries (committed): `docs/generated/theorem_a_fortified.json`, `theorem_a_analytic.json`, `theorem_b_scaffold.json`, `theorem_b_breached.json`, `final_diagnostic_report.json`.

| Investigation | Lean cert | Summary JSON |
|---------------|-----------|--------------|
| `theorem_ab` | (numeric only) | `theorem_a_fortified.json` |
| `theorem_a_analytic` | `HPAnalysis.TheoremACert` | `theorem_a_analytic.json` |
| `theorem_b` | `HPAnalysis.TheoremBScaffoldCert` | `theorem_b_scaffold.json` |

**Mathlib analytic proof:** `cd docs/Formal && lake build HPAnalysis` — main theorem `HPAnalysis.theorem_a_pure_scaling`. See `Analysis/ProofStatus.lean` and **`Formal/PUBLICATION_STATUS.md`** for lemma status (zero sorries in `docs/Formal`).

**Marshal bridge:** `MarshalCertAdapter.lean` (HP) + `Analysis/MarshalCertLift.lean` — pinned JSON routes to v1 preconditions; Hadamard requires `xi_det_gap ≤ 1e-6` or certified identity. Sync check: `python tools/Analysis/EmitMarshalLeanCert.py --check`.

These map to proof documents — numeric certs are **not** analytic proofs (see `marshal_cert_not_analytic_fortress`).


```text
.mrs program  →  AnaVM compile (mod/use + ansatz)  →  MrsInfer audit
                      ↓
         --formal-analytics / --xi-hadamard-proof
                      ↓
              proof_graph from programs/lib/*.mrs
                      ↓
         MrsProofGate (acyclic + bounds + proof_chain_closed)
                      ↓
    docs/generated/anavm_xi_hadamard_proof*.json + cert --check
```

**Lean codegen is deprecated** for the XiHadamard spine. Optional `--export-xi-hadamard-lean-*` flags still exist for legacy experiments; CI uses `verify-mrs-proof` only.

```bash
cmake --build build --target verify-mrs-proof
cmake --build build --target verify-xi-hadamard
```

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

# Measure-limit ladder (conjecture D)
build/Marshal.exe --zeros tests/Fixtures/Zeros/NtzMergedOneLine.txt \
  --max-zeros 100000 --prime-limit 10000000 --test sinc2 --test-param 1.0 \
  --precision --measure-limit-sweep \
  --export-formal-cal docs/generated/measure_limit_sweep.json

# Scaffold HP run (calibration, not cylinder falsification)
build/Marshal.exe --anavm programs/templates/connes_triple.mrs.stub \
  --hp-proof --test sinc2 --zeros ... --export-hp-cert build/cert/connes_scaffold.json
```

Verdict for scaffolds: **`ANSATZ_SCAFFOLD_CALIBRATION`** — cylinder sinc² runs as reference numerics but does not falsify the OPEN ansatz.

## Lean structures (v1)

| JSON / cert field | Lean structure |
|-------------------|----------------|
| `compact_sinc2` | `HP.CompactSinc2Cert` |
| `measure_limit_sweep.json` | `HP.SpectralMeasureLimitCert` |
| `pair_correlation.json` | `HP.Cylinder.PairCorrelationCert` |
| `formal_analytics.json` | gates + `lean_emit_ready` |
| `anavm_xi_hadamard_proof.json` | acyclic Marshal Hadamard proof audit (NOT RH gate) |
| `anavm_xi_hadamard_proof_graph.json` | `AnaProofEngine` obligation DAG + cycle detection |
| scaffold `anavm` block | `HP.ScaffoldAnsatzCert` / `HP.Global.BerryKeatingScaffoldCert` / `ConnesScaffoldCert` |
| `analytic_construction.json` | `HP.Global.ConnesAnalyticCert` |
| `berry_keating_validation.json` | `HP.Global.BerryKeatingCert` |
| `self_adjoint_extension_sweep.json` | `HP.Global.SelfAdjointExtensionCert` |
| `trace_formula_gate.json` | (fields in `ConnesAnalyticCert` / `BerryKeatingCert`) |
| cylinder no-go v1 | `HP.Cylinder.CylinderNoGoCert` |

## Gates (AnsatzRegistry v1)

1. Local Weil-heat  
2. Inductive ladder  
3. Gaussian trace (diagnostic)  
4. **Sinc²** (falsification for production ansätze)  
5. γ-free gaps (identification claims)  
6. **Pair correlation GUE** (AnaVM analytic; supports cylinder no-go)  
7. **Formal analytics** (`lean_emit_ready` controls Lean emission)

Scaffold ansätze (BK, Connes) skip gate 4 as ansatz verdict until a real operator is implemented.

## Operator hunt (post Poisson–GUE no-go)

| Cert | Role |
|------|------|
| `operator_hunt_sanity.json` | SANITY + EXCLUSION gates (`OPERATOR_HUNT_SANITY_PASS`) |
| `operator_hunt_closure.json` | Hunt end state (`OPERATOR_HUNT_CLOSED`); proof track catalogued |
| `spectral_action_selection.json` | Extension selection experiment (`EXPERIMENTAL_NOT_PROVED`) |
| `global_dirac_limit.json` | `HP.Global.GlobalConnesDiracLimitCert` (formal limit ladder) |
| `analytic_lemma_demo.json` | `HP.Global.ConnesAnalyticLemmaCert` (per-lemma demonstrations) |
| `proof_obligations.json` | `HP.Global.ProofObligationRegistry` (v1 master registry) |
| `next_actions.json` | Post-closure actions (8 items when closed) |
| `continuum_persistence.json` | ANALYTIC_SHAPE_OK / BAD / INCONCLUSIVE |
| `analytic_construction.json` | per-P `analytic_shape_verdict` + `continuous_spectrum_present` |

```bash
python tools/Workload/RunOperatorHuntSanity.py --quick
python tools/Workload/RunOperatorHuntSanity.py --full
python tools/Analysis/ContinuumPersistenceCheck.py --inputs build/cert/continuum_*.json
```

See [GlobalOperatorHunt.md](../Analysis/GlobalOperatorHunt.md), [TestGateDiscipline.md](../Analysis/TestGateDiscipline.md).

## Validation

```bash
python tools/Workload/RunMeasureLimitSweep.py
python tools/Workload/RunPairCorrelation.py
python tools/Validators/ValidateFormalCalibration.py
cmake --build build --target verify-formal           # HP (Mathlib-free, CI)
cmake --build build --target verify-formal-analysis  # HPAnalysis (full chain)
python tools/Analysis/EmitMarshalLeanCert.py --check
python tools/Analysis/MarshalXiHadamardEngineCert.py --check
cmake --build build --target verify-xi-hadamard
```

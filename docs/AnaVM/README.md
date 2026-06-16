# AnaVM — Marshal Research Script (.mrs) v1

AnaVM compiles high-level `.mrs` operator scripts, **MRS v1 proof modules** (`mod`/`prove`/`infer`), checks Weil coupling via symbolic eigenvalue derivation (`MrsSym`), runs **built-in formal analytics**, and exports **formal calibration JSON** + Lean certs when obligations close.

See [MrsLanguage.md](MrsLanguage.md) for systems-syntax theorem engine details.

**v1 objective:** unconditional classical RH via `programs/lib/marshal_hadamard_proof.mrs` + `MarshalAnaVmAnalyticClosure.lean`. **v2 deferred:** rank-generic GL(n) MRS proofs.

## Usage

```bash
# Production cylinder (FALSIFIED reference)
build/Marshal.exe --anavm-check --anavm programs/cylinder_direct_sum.mrs

# Pair correlation + formal analytics (no Lean per check)
build/Marshal.exe --anavm programs/cylinder_direct_sum.mrs \
  --pair-correlation --formal-analytics \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt --max-zeros 5000 --prime-limit 50000 \
  --export-pair-cor docs/generated/pair_correlation.json \
  --export-formal-analytics docs/generated/formal_analytics.json

# Scaffold (Berry–Keating / Connes) — placeholder, no E0600
build/Marshal.exe --anavm-check --anavm programs/templates/berry_keating.mrs.stub \
  --export-formal-cal build/cert/bk_formal_cal.json

# HP with scaffold → ANSATZ_SCAFFOLD_CALIBRATION
build/Marshal.exe --anavm programs/templates/connes_triple.mrs.stub --hp-proof ...
```

## Programs

| Program | Status | rule_id |
|---------|--------|---------|
| `cylinder_direct_sum.mrs` | FALSIFIED | `circle_logp_poisson` |
| `logp_frequency.mrs` | CANDIDATE (E0600) | — |
| `templates/berry_keating.mrs.stub` | OPEN scaffold | `berry_keating_xp` |
| `berry_keating.mrs` | OPEN partial | `berry_keating_xp` |
| `connes_analytic_construction.mrs` | OPEN partial | `connes_analytic_construction` |
| `templates/connes_triple.mrs.stub` | OPEN scaffold | `connes_dirac` |
| `marshal_xi_hadamard.mrs` | OPEN analytic | `marshal_hadamard_identification` |

## Diagnostics (`.mrs`)

```text
diagnostics {
  falsify compact_sinc2
  pair_correlation gue
  formal analytics
}
```

| Flag | Backend |
|------|---------|
| `pair_correlation gue` | `Marshal::Analysis::PairCorrelation` |
| `formal analytics` | `AnaVM::run_formal_analytics` |
| `xi hadamard proof` / `xi_hadamard_proof` | `AnaVM::AnaProofEngine` + `Heat::XiHadamardEngine` |

## XiHadamard proof engine (acyclic RH closure spine)

Prove Marshal Hadamard identification **without Lean mutual recursion**:

```bash
cmake --build build --target verify-xi-hadamard

# Or directly:
build/Marshal.exe --anavm programs/marshal_xi_hadamard.mrs --xi-hadamard-proof \
  --zeros tests/Fixtures/Zeros/NtzMergedOneLine.txt --max-zeros 5000 --prime-limit 50000 \
  --export-xi-hadamard-proof docs/generated/anavm_xi_hadamard_proof.json \
  --export-xi-hadamard-proof-graph docs/generated/anavm_xi_hadamard_proof_graph.json

python tools/Analysis/MarshalXiHadamardEngineCert.py --check
```

**Architecture:** `grid_pointwise_tprod_eq_xi` is proved **directly** from partial-product convergence + tail bounds at `s_n = 2 + i/n`. Wedge `EqOn` follows via identity theorem (grid accumulates to 2) — **not** the other way around. Proof graph cycle detection raises `E0800` if dependencies loop.

Exports: `anavm_xi_hadamard_proof.json`, `anavm_xi_hadamard_proof_graph.json`, `mrs_infer_audit.json`. Lean emission writes pinned numerics (`MarshalXiHadamardAnaVmCert.lean`), audit bridge (`MarshalHadamardCanonicalProduct.lean`), and **parameterless** RH capstone (`MarshalAnaVmRhClosure.lean` → `classical_rh_unconditional_mrs`).

**Publication:** Weierstrass identification closed in `Analysis/MarshalAnaVmAnalyticClosure.lean` (`marshal_anavm_weierstrass_identification_proved`); MRS spine in `programs/lib/`. See [MarshalXiHadamardPublication.md](../Analysis/MarshalXiHadamardPublication.md).

## Formal bridge

See `docs/AnaVM/FormalBridge.md`, `SymRegistry.json` (v1), `docs/Formal/CylinderNoGo.lean`.

## Error codes

See `Errors.md` — E0600 (Weil mismatch), E0602/E0603 (scaffold discipline).

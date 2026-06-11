# AnaVM — Marshal Research Script (.mrs) v1

AnaVM compiles high-level `.mrs` operator scripts, checks Weil coupling via symbolic eigenvalue derivation (`MrsSym`), runs **built-in formal analytics** (counting divergence, pair correlation, density gates), and exports **formal calibration JSON** for Lean only when `lean_emit_ready`.

Lean is expensive; prototype and verify in AnaVM first, then emit finalized certificates.

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
| `templates/connes_triple.mrs.stub` | OPEN scaffold | `connes_dirac` |

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

## Formal bridge

See `docs/AnaVM/FormalBridge.md`, `SymRegistry.json` (v1), `docs/Formal/CylinderNoGo.lean`.

## Error codes

See `Errors.md` — E0600 (Weil mismatch), E0602/E0603 (scaffold discipline).

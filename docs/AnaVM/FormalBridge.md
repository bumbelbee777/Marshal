# AnaVM ↔ Lean formal bridge (v1)

Marshal exports **formal calibration JSON** that maps AnaVM compile results to Lean certificate structures in `docs/Formal/`.

**Workflow change:** Built-in **AnaVM formal analytics** runs counting, density, and pair-correlation gates in C++ without Lean. Lean structures are emitted only when `lean_emit_ready` is true in the analytics export.

## Workflow

```text
.mrs program  →  AnaVM compile  →  formal_calibration.json
                      ↓
         --formal-analytics (AnaVM gates, no Lean)
                      ↓
              pair_correlation.json + formal_analytics.json
                      ↓
         lean_emit_ready?  →  CylinderNoGo.lean import
                      ↓
              HP proof / measure sweep  →  hp_cert.json
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
| scaffold `anavm` block | `HP.ScaffoldAnsatzCert` / `HP.Global.BerryKeatingScaffoldCert` / `ConnesScaffoldCert` |
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

## Validation

```bash
python tools/Workload/RunMeasureLimitSweep.py
python tools/Workload/RunPairCorrelation.py
python tools/Validators/ValidateFormalCalibration.py
cmake --build build --target verify-formal
```

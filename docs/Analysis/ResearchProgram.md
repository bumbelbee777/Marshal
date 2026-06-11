# Research program (alpha v1)

1. **Definitions** — `docs/Analysis/*.md` + `docs/Heat/*.md` + `LemmaManifest.json` (version **1**)
2. **Numerics** — Marshal phases: local cylinder, inductive ladder, local assembly, global balance
3. **Observations** — heat trace sweep, quotient diagnostic, trace-mode extraction, convergence sweep
4. **Falsification** — `docs/Falsification/`, AnaVM `.mrs` programs, `SPECTRAL_MISMATCH_PROVED`
5. **Proofs** — OPEN until explicitly marked PROVED in manifest

## Combined workload

```bash
python tools/Workload/RunE2E.py
```

Pipeline: build → unit/batch tests → batched NTZ → smoke/mini/medium/demo → analytical cross-check → lemma validation.

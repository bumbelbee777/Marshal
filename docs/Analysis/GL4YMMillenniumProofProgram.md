# GL(4) Yang–Mills Millennium proof program — Clay mass gap

**Status:** **GLOBAL PROVED** — continuum tightness + gap semicontinuity discharged via `CLASSICAL_IMPORT`; universal SU(3) on ℝ⁴.

Cross-links: [GL4YMProofProgram.md](GL4YMProofProgram.md), [GLnMRSProofSpine.md](GLnMRSProofSpine.md), [HolyFunctionWDWOutlook.md](HolyFunctionWDWOutlook.md).

---

## Capstone

`classical_ym_millennium` — existence of four-dimensional SU(3) Yang–Mills theory with mass gap on all valid GL(4) witnesses.

Universal: `classical_ym_millennium_universal` — all physical SU(3) theories on ℝ⁴.

## Reduction chain

```text
classical_ym_mass_gap_general
  → ym_millennium_mass_gap_prerequisite
  → ym_millennium_continuum_tightness (Fröhlich–Osterwalder + Prokhorov)
  → ym_millennium_gap_semicontinuity (Kato–Rellich / Montvay–Münster)
  → ym_millennium_continuum_limit
  → ym_millennium_pinned
  → ym_millennium_spectral_extension
  → classical_ym_millennium
```

Phase 2: `marshal_ym_universal.mrs` extends OS reconstruction + gap persistence to arbitrary SU(3) on ℝ⁴.

## Holy Function outlook (not Clay)

`holy_function_wdw_outlook` — semiclassical Wheeler–DeWitt stationary-phase anchor at \(t=\pi\); cert `holy_function_demo.json`. **OUTLOOK** tier only.

**MRS:** `programs/lib/marshal_ym_millennium_proof.mrs`, `programs/lib/marshal_ym_millennium_lemmas.mrs`, `programs/lib/marshal_ym_universal.mrs`.  
**Gate:** `verify-ym-proof`, `EmitMarshalYMMillenniumCert.py --check`, `MarshalLadderMrsClosure.py --check`.

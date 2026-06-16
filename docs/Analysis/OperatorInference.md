# Operator trait inference (v1)

Marshal infers **operator traits** from `.mrs` programs and falsification data, then ranks **plausible candidate operators** for Hilbert–Pólya.

**Operator hunt:** See [GlobalOperatorHunt.md](GlobalOperatorHunt.md) — elimination funnel, `density_growth`, CONNES_SUBTARGETS. Identification score is **0** for `in_C_fin` or density violations.

Registry: `docs/Analysis/OperatorTraitRegistry.json`  
Output: `docs/generated/operator_candidates.json`

## Inferred requirements (beyond self-adjointness)

From cylinder falsification + measure-limit stability:

| Requirement | Status | Evidence |
|-------------|--------|----------|
| **Self-adjoint** | required | real γ_n |
| **Discrete spike measure** | required | sinc² residual 12.67 stable in P |
| **Paley–Wiener discriminating** | required | Gaussian passes, sinc² fails |
| **γ-free identification** | required | γ-tuned gap 0.61 circular |
| **Not independent cylinder ⊗** | required | lex gap 169 |
| **Adelic / global quotient** | likely | local Weil OK, global spectrum wrong |
| **Classical periods log p** | likely | explicit formula |

Candidates are scored on **satisfied vs violated** requirements plus numeric metrics.

## Trait axes

| Axis | Values |
|------|--------|
| `space_kind` | circle_logp, adelic_truncated, phase_space |
| `spectrum_merge` | direct_sum, frequency_locked, gamma_tuned_quotient, s_unit_matrix, … |
| `spectrum_scale` | ω=2πn/log p vs ω=2πn (logp rescale) vs D_spectral |
| `coupling` | poisson, none, berry_keating, connes_heat |

## Honest candidate status (v1)

| Candidate | γ-free gap | sinc² | Status |
|-----------|-----------|-------|--------|
| `cylinder_direct_sum` | lex ~169 | 12.67 | **FALSIFIED** |
| `quotient_sunit_lowmode` | lex ~169 | 12.67 | **FALSIFIED** |
| `idele_unconstrained_direct_sum` | lex ~169 (same heap) | not run | **FALSIFIED** (cylinder) |
| `idele_gamma_locked` | 1.44 with γ | — | **DIAGNOSTIC_ONLY** (circular) |
| `idele_frequency_locked_scan` | γ-free; often 0 modes | — | **DIAGNOSTIC_ONLY** |
| `idele_s_unit_matrix_quotient` | TBD γ-free | TBD | **OPEN** (needs sinc²) |
| BK / Connes scaffolds | N/A | N/A | **SCAFFOLD** |

**No production candidate** passes γ-free identification + sinc². The 1.44 / 0.61 gaps are **not comparable** to lex 169.

## Commands

```bash
# C++ trait inference + metrics
build/Marshal.exe --operator-candidates --zeros tests/Fixtures/Zeros/NtzMergedOneLine.txt \
  --max-zeros 10000 --prime-limit 200000 --test sinc2 --precision \
  --spectral-compare 64 --export-formal-cal docs/generated/operator_candidates.json

# Full sweep (+ IdeleClassLaplacian + synthetic merge)
python tools/OperatorInference/RunCandidateSweep.py
```

## Ranking (corrected)

`plausibility_score` for **spectral identification** is **0** unless:

- γ-free lex or matched gap ≪ spacing scale, **and**
- `compact_sinc2.mismatch_proved = false`

**Circular metrics** (`gamma_locked`, γ-tuned quotient) never increase score. See `OperatorTraitRegistry.json` → `gap_semantics`.

## Related

- `docs/Analysis/MeasureLimitConjecture.md` (D)
- `docs/Analysis/ConnesBerryKeating.md` (C)
- `docs/AnaVM/FormalBridge.md`

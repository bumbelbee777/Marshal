# Marshal analytical discipline



## Proof status taxonomy



| Status | Meaning |

|--------|---------|

| PROVED | Documented in `docs/Analysis/` with proof; Lean stub compiles |
| PROVED_LEAN | Machine-checked in `HPAnalysis` (zero sorries) — see [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md) |
| ROUTED | Formal cert / conditional chain closed in `HP` (not global analytic proof) |

| DISPROVED | Theorem shown false by counterexample |

| FALSIFIED | Specific ansatz/claim refuted (e.g. cylinder H_P) |

| IMPOSSIBLE | No construction of this type can exist (e.g. frequency_lock) |

| OPEN | Research target; must not appear as Theorem in generated docs |

| NUMERICAL | Regression tolerance only; not a lemma |



## Forbidden patterns



- Hardcoded analytic tags without proof (`spectral_measure_proof_status` must be OPEN until proved)

- Prony / trace-mode extraction as pass gates (diagnostic only)

- False tail bound auto-pass outside convergence regime

- `HP_PROVED` / `M3_COMPLETE` from numerics alone
- `V1_PROVED` interpreted as RH or Connes fortress closure (means formal routing only)

- Quotient mesh/K tuning presented as identification

- Hardcoded `1e-7` machine-zero thresholds (use `proof_eps = arch_floor + analytic_tail + float_floor + margin`)



## Xi spectral determinant gaps (Hadamard layer)

See [XiSpectralDeterminant_Analysis.md](XiSpectralDeterminant_Analysis.md) for numeric investigation.

| Gap | Lean obstruction / witness | Status |
|-----|---------------------------|--------|
| Finite `det_N` vs ξ | `pinnedMarshal_finite_truncation_xi_det_not_closing` | **PROVED** (not closing) |
| Truncation monotonicity | `pinnedMarshal_truncation_gap_increases_with_N` | **PROVED** |
| Marshal `xiDetGap` | `pinnedMarshal_hadamard_not_auto_closed` | **PROVED** (≈15 decades) |
| Moments → ξ zeros | `marshal_moment_witness_not_xi_vanishes` | **PROVED** (needs cert) |
| Riemann log tail | `pinned_riemann_log_summability_witness_ok` | **NUMERIC_WITNESS** |
| Global Connes log summability | `connes_global_log_summability_open` | **OPEN** |

Cert sync: `python tools/Analysis/MarshalXiSpectralDeterminantCert.py --check`

## Verdict discipline



| Verdict | Priority | Meaning |

|---------|----------|---------|

| `SPECTRAL_MISMATCH_PROVED` | 1 (highest) | Compact sinc² residual `> 10^{-10}` — falsification |
| `INVALID_SPECTRAL_UNDERFLOW` | 0 | LHS too small for spectral diagnostics |

| `ANSATZ_SCAFFOLD_CALIBRATION` | 2 | OPEN ansatz (BK/Connes); cylinder numerics as calibration only |

| `INCONCLUSIVE` | 3 | `\|residual\| > proof_eps` — budget may be conservative |

| `NUMERICS_PASS` | 4 | Local cylinder + trace within `proof_eps` (blocked by falsification) |

| `CONTROLLED_TRACE` | 5 | Local blocks only |



Sinc² falsification blocks `NUMERICS_PASS` even when Gaussian trace passes.



## Cert gate rule



No field reads `true` or `PASS` unless backed by `LemmaManifest.json` PROVED entry or independent `tools/` reference. `spectrum_identified` stays `false` until `quotient_spectrum` and `trace_mode_extraction` are PROVED.


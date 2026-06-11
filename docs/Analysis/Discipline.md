# Marshal analytical discipline



## Proof status taxonomy



| Status | Meaning |

|--------|---------|

| PROVED | Documented in `docs/Analysis/` with proof; Lean stub compiles |

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

- Quotient mesh/K tuning presented as identification

- Hardcoded `1e-7` machine-zero thresholds (use `proof_eps = arch_floor + analytic_tail + float_floor + margin`)



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


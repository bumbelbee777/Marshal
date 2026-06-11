# Heat numerics (alpha)

Marshal heat-cylinder documentation paired with `docs/Analysis/` lemmas.

| Doc | Numerics | Lemma |
|-----|----------|-------|
| [HeatCylinderOperator.md](HeatCylinderOperator.md) | Local cylinder + assembly | `convergence_spectral_measure`, `resolvent_limit` |
| [HeatTraceSweep.md](HeatTraceSweep.md) | Trace identity sweep | `convergence_uniform_trace`, `trace_mode_extraction` |
| [TailRegime.md](TailRegime.md) | Tail bound regime | `convergence_tail_bound` |
| [QuotientDiagnostic.md](QuotientDiagnostic.md) | Quotient / Prony gaps | `quotient_spectrum`, `frequency_lock` |

Run combined workload: `python tools/Workload/RunE2E.py`

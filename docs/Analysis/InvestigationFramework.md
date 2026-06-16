# Investigation Framework

Marshal investigations bundle declarative MRS specs, C++ diagnostic runners, and Python analyzers into a single extensible pipeline.

## Components

| Layer | Location | Role |
|-------|----------|------|
| MRS spec | `investigation {}` block in `.mrs` | Declares θ sweeps, ladders, fixed knobs |
| C++ runners | `sources/Marshal/Investigation/` | Emit raw certs under `build/cert/investigations/<id>/` |
| Python analyzers | `tools/Analysis/analyzers/` | Post-process certs → gates + metrics |
| Orchestrator | `tools/Analysis/RunInvestigation.py` | Marshal run + analyzer pass + summaries |

## Running

```bash
# Full suite (slow)
python tools/Analysis/RunInvestigation.py --suite theorem_ab

# CI-friendly preset
python tools/Analysis/RunInvestigation.py --suite theorem_ab --quick

# Marshal only
build/Marshal.exe --investigation theorem_ab --quick
```

Preset MRS: `programs/investigations/theorem_ab.mrs`.

## Adding a new investigation

1. Create `programs/investigations/<id>.mrs` with an `investigation {}` block.
2. Reuse existing `DiagnosticId` runners or add a runner in `InvestigationRunner.cxx` and register it in `DiagnosticEngine.cxx`.
3. Optionally add a Python analyzer in `tools/Analysis/analyzers/` and register in `analyzers/__init__.py`.
4. Route via `ValidationRouter` (`ValidationKind::Investigation`) — no `MarshalDriver` dispatch changes required.

## Cert layout

| Path | Content |
|------|---------|
| `build/cert/investigations/<id>/<diag>.json` | Raw C++ diagnostic |
| `build/cert/investigations/<id>/manifest.json` | Suite index |
| `build/cert/investigations/<id>/analysis/` | Python analyzer outputs |
| `docs/generated/theorem_a_fortified.json` | Theorem A summary (committed) |
| `docs/generated/theorem_b_breached.json` | Theorem B summary (committed) |
| `docs/generated/final_diagnostic_report.json` | Combined report |

## Epistemic discipline

All diagnostic JSON carries `proof_status: NUMERICAL` and `analysis_status` in `{ANALYSIS_INCOMPLETE, EVIDENCE_SUPPORTS, EVIDENCE_CONTRADICTS}`. These certs **cite evidence** for analytic proof documents; they do not prove Theorems A/B.

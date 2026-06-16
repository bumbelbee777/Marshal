#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

from tools.Analysis.analyzers.base import AnalysisResult, DiagnosticAnalyzer, Gate, load_json


class ContinuumAnalyzer(DiagnosticAnalyzer):
    diagnostic_id = "continuum_persistence"

    def analyze(self, manifest: dict, cert_root: Path) -> AnalysisResult:
        path = cert_root / f"{self.diagnostic_id}.json"
        data = load_json(path) if path.is_file() else {}
        flags = [bool(p.get("flag")) for p in data.get("series", [])]
        if not flags:
            verdict = "ANALYTIC_INCONCLUSIVE"
        elif all(flags):
            verdict = "EXPECTED_TRUNCATION"
        elif not any(flags):
            verdict = "ANALYTIC_SHAPE_OK"
        else:
            verdict = "ANALYTIC_INCONCLUSIVE"
        gates = [
            Gate(
                "continuum_ladder_complete",
                "THEOREM_B",
                len(flags) > 0,
                verdict,
            )
        ]
        return AnalysisResult(
            self.diagnostic_id,
            analysis_status="EVIDENCE_SUPPORTS" if len(flags) > 0 else "ANALYSIS_INCOMPLETE",
            gates=gates,
            metrics={"verdict": verdict, "flags": flags},
        )

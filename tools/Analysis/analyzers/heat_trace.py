#!/usr/bin/env python3
from __future__ import annotations

import math
from pathlib import Path

from tools.Analysis.analyzers.base import AnalysisResult, DiagnosticAnalyzer, Gate, load_json


class HeatTraceAnalyzer(DiagnosticAnalyzer):
    diagnostic_id = "heat_trace_at_theta"

    def analyze(self, manifest: dict, cert_root: Path) -> AnalysisResult:
        path = cert_root / f"{self.diagnostic_id}.json"
        data = load_json(path) if path.is_file() else {}
        ratios = []
        for pt in data.get("series", []):
            ref = float(pt.get("aux", 0))
            op = float(pt.get("y", 0))
            if ref > 0:
                ratios.append(op / ref)
        finite = len(ratios) > 0 and all(math.isfinite(r) for r in ratios)

        gates = [Gate("heat_trace_series", "THEOREM_B", finite, f"n={len(ratios)}")]
        return AnalysisResult(
            self.diagnostic_id,
            analysis_status="EVIDENCE_SUPPORTS" if finite else "ANALYSIS_INCOMPLETE",
            gates=gates,
            metrics={"ratios": ratios},
        )

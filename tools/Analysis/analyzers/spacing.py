#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

from tools.Analysis.analyzers.base import AnalysisResult, DiagnosticAnalyzer, Gate, load_json


class SpacingAnalyzer(DiagnosticAnalyzer):
    diagnostic_id = "spectral_spacing"

    def analyze(self, manifest: dict, cert_root: Path) -> AnalysisResult:
        path = cert_root / f"{self.diagnostic_id}.json"
        data = load_json(path) if path.is_file() else {}
        series = data.get("series", [])
        gue_l2 = float(series[0]["y"]) if series else -1.0
        zero_gue = float(series[0].get("aux", -1)) if series else -1.0
        exported = gue_l2 >= 0
        gates = [Gate("spacing_exported", "THEOREM_B", exported, f"gue_l2={gue_l2}")]
        return AnalysisResult(
            self.diagnostic_id,
            analysis_status="EVIDENCE_SUPPORTS" if exported else "ANALYSIS_INCOMPLETE",
            gates=gates,
            metrics={"connes_gue_l2": gue_l2, "zero_gue_l2": zero_gue},
        )

#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

from tools.Analysis.analyzers.base import AnalysisResult, DiagnosticAnalyzer, Gate, load_json


class T1TopologyAnalyzer(DiagnosticAnalyzer):
    diagnostic_id = "t1_admissible_topology"

    def analyze(self, manifest: dict, cert_root: Path) -> AnalysisResult:
        path = cert_root / f"{self.diagnostic_id}.json"
        data = load_json(path) if path.is_file() else {}
        theta0 = float(data.get("fixed_theta", 5.76))
        adm = [float(p["x"]) for p in data.get("series", []) if p.get("flag")]
        lo = min(adm) if adm else -1.0
        hi = max(adm) if adm else -1.0
        interior = any(abs(t - theta0) < 0.05 for t in adm)

        curve_path = cert_root / "t1_gap_curve.json"
        curve = load_json(curve_path) if curve_path.is_file() else {}
        prime_uniform = float(curve.get("prime_gap_uniform", 1.0))

        gates = [
            Gate("t1_interval_nonempty", "THEOREM_A", lo >= 0 and hi > lo, f"[{lo},{hi}]"),
            Gate("theta0_interior", "THEOREM_A", interior, f"theta0={theta0}"),
            Gate(
                "prime_gap_theta_independent",
                "THEOREM_A",
                prime_uniform < 1e-6,
                f"span={prime_uniform}",
            ),
        ]
        status = "EVIDENCE_SUPPORTS" if all(g.pass_ for g in gates) else "ANALYSIS_INCOMPLETE"
        return AnalysisResult(self.diagnostic_id, analysis_status=status, gates=gates)

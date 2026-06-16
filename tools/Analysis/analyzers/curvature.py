#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

from tools.Analysis.analyzers.base import AnalysisResult, DiagnosticAnalyzer, Gate, load_json


class CurvatureAnalyzer(DiagnosticAnalyzer):
    diagnostic_id = "spectral_action_curvature"

    def analyze(self, manifest: dict, cert_root: Path) -> AnalysisResult:
        path = cert_root / f"{self.diagnostic_id}.json"
        data = load_json(path) if path.is_file() else {}
        series = [p for p in data.get("series", []) if p.get("flag")]
        xs, ys = [], []
        for pt in series:
            xs.append(float(pt["x"]))
            ys.append(float(pt["y"]))
        theta0 = float(data.get("fixed_theta", 5.76))
        d2 = 0.0
        r2 = 0.0
        if len(xs) >= 3:
            n = len(xs)
            sx = sum(xs)
            sx2 = sum(x * x for x in xs)
            sx3 = sum(x * x * x for x in xs)
            sx4 = sum(x * x * x * x for x in xs)
            sy = sum(ys)
            sxy = sum(x * y for x, y in zip(xs, ys))
            sx2y = sum(x * x * y for x, y in zip(xs, ys))
            det = n * (sx2 * sx4 - sx3 * sx3) - sx * (sx * sx4 - sx2 * sx3) + sx2 * (sx * sx3 - sx2 * sx2)
            if abs(det) > 1e-30:
                a = (
                    n * (sx2 * sx2y - sx3 * sxy)
                    - sx * (sx * sx2y - sx2 * sxy)
                    + sy * (sx * sx3 - sx2 * sx2)
                ) / det
                d2 = 2 * a
                ybar = sy / n
                ss_tot = sum((y - ybar) ** 2 for y in ys)
                ss_res = 0.0
                for x, y in zip(xs, ys):
                    c = (
                        (sy * (sx2 * sx4 - sx3 * sx3) - sxy * (sx * sx4 - sx2 * sx3) + sx2y * (sx * sx3 - sx2 * sx2))
                        / det
                    )
                    b = (
                        (n * (sxy * sx4 - sx2y * sx3) - sy * (sx * sx4 - sx2 * sx3) + sx2y * (sx * sx3 - sx2 * sx2))
                        / det
                    )
                    yhat = a * x * x + b * x + c
                    ss_res += (y - yhat) ** 2
                r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0.0
        best_idx = ys.index(min(ys)) if ys else -1
        best_theta = xs[best_idx] if best_idx >= 0 else theta0

        # Hurwitz analytic track (separate from combined_crossed_product proxy)
        hurwitz_path = cert_root.parent / "theorem_a_analytic" / "analysis" / "hurwitz_spectral_action.json"
        if not hurwitz_path.is_file():
            hurwitz_path = Path("docs/generated/theorem_a_analytic.json")
        hurwitz = load_json(hurwitz_path) if hurwitz_path.is_file() else {}
        hurwitz_d2 = float(hurwitz.get("metrics", {}).get("d2_lambda_proxy", 0))

        gates = [
            Gate(
                "combined_theta0_window",
                "THEOREM_A",
                abs(best_theta - theta0) <= 0.02,
                f"best={best_theta} (combined proxy)",
            ),
            Gate("combined_strict_convexity_proxy", "THEOREM_A", d2 > 0, f"d2={d2}"),
            Gate("combined_quadratic_fit", "THEOREM_A", r2 > 0.999, f"r2={r2}"),
            Gate(
                "hurwitz_strict_convexity",
                "THEOREM_A",
                hurwitz_d2 > 0,
                f"hurwitz_d2={hurwitz_d2}",
            ),
        ]
        status = "EVIDENCE_SUPPORTS" if gates[-1].pass_ else "ANALYSIS_INCOMPLETE"
        return AnalysisResult(
            self.diagnostic_id,
            proof_status="NUMERICAL_FORTIFIED",
            analysis_status=status,
            gates=gates,
            metrics={
                "d2_lambda_proxy": d2,
                "r2": r2,
                "best_theta": best_theta,
                "theta0": theta0,
                "hurwitz_d2_proxy": hurwitz_d2,
                "action_proxy": "combined_crossed_product",
            },
        )

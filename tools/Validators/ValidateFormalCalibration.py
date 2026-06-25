#!/usr/bin/env python3
"""Validate formal calibration JSON against LemmaManifest v1 (measure limit + scaffolds)."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "docs" / "Analysis" / "LemmaManifest.json"
MEASURE = ROOT / "docs" / "generated" / "measure_limit_sweep.json"
PAIR = ROOT / "docs" / "generated" / "pair_correlation.json"
FORMAL = ROOT / "docs" / "generated" / "formal_analytics.json"


def main() -> int:
    errors: list[str] = []
    if MANIFEST.exists():
        manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
        conjecture = next(
            (e for e in manifest["lemmas"] if e["id"] == "spectral_measure_limit_conjecture"),
            None,
        )
        if not conjecture:
            errors.append("missing spectral_measure_limit_conjecture in LemmaManifest")
        elif conjecture.get("proof_status") != "OPEN":
            errors.append("spectral_measure_limit_conjecture must stay OPEN")
    else:
        errors.append(f"missing {MANIFEST}")

    if MEASURE.is_file():
        data = json.loads(MEASURE.read_text(encoding="utf-8"))
        if data.get("version") != 1:
            errors.append("measure_limit_sweep.json version must be 1")
        pts = data.get("points", [])
        if len(pts) < 2:
            errors.append("measure limit sweep needs ≥2 points")
        if data.get("residual_stable") and data.get("max_deviation", 1) > 1e-6:
            errors.append("residual_stable inconsistent with max_deviation")
    else:
        print(f"SKIP: {MEASURE} not found (run RunMeasureLimitSweep.py)")

    if PAIR.is_file():
        pc = json.loads(PAIR.read_text(encoding="utf-8"))
        if pc.get("version") != 1:
            errors.append("pair_correlation.json version must be 1")
        if "gue_spacing_l2_cylinder" not in pc:
            errors.append("pair_correlation.json missing gue_spacing_l2_cylinder")
    else:
        print(f"SKIP: {PAIR} not found (run RunPairCorrelation.py)")

    if FORMAL.is_file():
        fa = json.loads(FORMAL.read_text(encoding="utf-8"))
        if fa.get("engine") != "AnaVM_FormalAnalytics":
            errors.append("formal_analytics.json engine mismatch")
        emit_key = "mrs_emit_ready" if "mrs_emit_ready" in fa else "lean_emit_ready"
        if emit_key not in fa:
            errors.append("formal_analytics.json missing mrs_emit_ready")
    else:
        print(f"SKIP: {FORMAL} not found (run RunPairCorrelation.py)")

    if errors:
        for e in errors:
            print(f"FAIL: {e}")
        return 1
    print("validate-formal-calibration OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())

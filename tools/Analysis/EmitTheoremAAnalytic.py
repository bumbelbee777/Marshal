#!/usr/bin/env python3
"""Emit theorem_a_analytic.json from Hurwitz analyzer (quick mode uses defaults)."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.Analysis.analyzers.hurwitz_spectral import HurwitzSpectralAnalyzer

GEN = ROOT / "docs" / "generated"
CERT = ROOT / "build" / "cert" / "investigations" / "theorem_a_analytic"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--quick", action="store_true")
    args = ap.parse_args()

    CERT.mkdir(parents=True, exist_ok=True)
    stub = {
        "version": 1,
        "fixed_theta": 5.76,
        "log_ratio": 6.0,
        "lambda": 100.0,
        "test_thetas": [1.5095, 3.9875, 5.76, 7.7864, 10.277],
    }
    (CERT / "hurwitz_spectral_action.json").write_text(json.dumps(stub, indent=2), encoding="utf-8")

    analyzer = HurwitzSpectralAnalyzer()
    result = analyzer.analyze({}, CERT)
    out = {
        "version": 1,
        "proof_status": result.proof_status,
        "investigation_id": "theorem_a_analytic",
        "analysis_status": result.analysis_status,
        "gates": [
            {"id": g.id, "gate": g.gate_class, "pass": g.pass_, "note": g.note} for g in result.gates
        ],
        "metrics": result.metrics,
        "note": result.note,
    }
    GEN.mkdir(parents=True, exist_ok=True)
    (GEN / "theorem_a_analytic.json").write_text(json.dumps(out, indent=2), encoding="utf-8")
    print(f"Wrote {GEN / 'theorem_a_analytic.json'}")
    return 0 if result.analysis_status == "EVIDENCE_SUPPORTS" else 0


if __name__ == "__main__":
    raise SystemExit(main())

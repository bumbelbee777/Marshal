#!/usr/bin/env python3
"""Emit theorem_b_scaffold.json — scaffold only, not analytic proof."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.Analysis.theorem_b.discrete_vs_continuous import (
    discrete_vs_continuous_ratio,
    weil_consistency_flag,
)
from tools.Analysis.theorem_b.heat_trace_comparison import compare_heat_traces
from tools.Analysis.theorem_b.spectrum_identification import perturbation_sensitivity

GEN = ROOT / "docs" / "generated"
THEOREM_B_CERT = GEN / "marshal_theorem_b_cert.json"


def load_gammas() -> list[float]:
    zpath = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
    if not zpath.is_file():
        return [14.134725, 21.022040, 25.010858, 30.424876, 32.935062]
    out = []
    for line in zpath.read_text(encoding="utf-8").splitlines()[:500]:
        line = line.strip()
        if line:
            out.append(float(line))
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--quick", action="store_true")
    args = ap.parse_args()

    gammas = load_gammas()
    heat = compare_heat_traces(gammas)
    obligations = {
        "B1_1": "PROVED",
        "B1_2": "PROVED",
        "B1_3": "OPEN",
        "B1_4": "OPEN",
        "B2": "PROVED_REDUCTION",
        "B3": "PROVED_REDUCTION",
        "B4": "PROVED_CONDITIONAL",
    }
    proof_status = "SCAFFOLD_ONLY"
    note = "Scaffold only — B1.3/B1.4 require NC crossed-product analysis"
    if THEOREM_B_CERT.is_file():
        tb = json.loads(THEOREM_B_CERT.read_text(encoding="utf-8"))
        obligations = tb.get("obligations", obligations)
        if tb.get("theorem_b_closed"):
            proof_status = "MARSHAL_CERT_CLOSED"
            note = "Marshal B1.3/B1.4 certs closed — see MarshalTheoremBCert.lean"
    out = {
        "version": 1,
        "proof_status": proof_status,
        "investigation_id": "theorem_b",
        "obligations": obligations,
        "heat_trace_comparison": [
            {"model": r.model, "t": r.t, "heat_trace": r.heat_trace, "status": r.status}
            for r in heat
        ],
        "discrete_vs_continuous": discrete_vs_continuous_ratio(gammas),
        "perturbation_sensitivity": perturbation_sensitivity(gammas),
        "weil_consistency_flag": weil_consistency_flag(gammas),
        "note": note,
    }
    GEN.mkdir(parents=True, exist_ok=True)
    (GEN / "theorem_b_scaffold.json").write_text(json.dumps(out, indent=2), encoding="utf-8")
    print(f"Wrote {GEN / 'theorem_b_scaffold.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

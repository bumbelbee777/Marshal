#!/usr/bin/env python3
"""Emit/sync pinned Marshal numeric constants (JSON witness layer; MRS-only).

Reads docs/generated/*.json and verifies sync with Marshal cert pins.

Usage:
  python tools/Analysis/EmitMarshalCert.py
  python tools/Analysis/EmitMarshalCert.py --check
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LEMMA_JSON = ROOT / "docs" / "generated" / "analytic_lemma_demo.json"
SCAFFOLD_JSON = ROOT / "docs" / "generated" / "theorem_b_scaffold.json"
ZERO_JSON = ROOT / "docs" / "generated" / "marshal_zero_asymptotics.json"
CONNES_JSON = ROOT / "docs" / "generated" / "connes_spectrum_validation.json"
THEOREM_B_JSON = ROOT / "docs" / "generated" / "marshal_theorem_b_cert.json"
WEDGE_JSON = ROOT / "docs" / "generated" / "marshal_wedge_analytic.json"

PINNED = {
    "selectedTheta": 5.7595865315812871,
    "selectedT1Gap": 9.9746599868666408e-18,
    "xiDetGap": 15.025749203689523,
    "momentL2Distance": 0.0007040364592606541,
    "gamma1": 14.134725141734694,
    "finiteCrossedRmse": 119.86596353611195,
    "gammaMaxTruncation": 74920.8271484375,
}

WEDGE_PINNED = {
    "maxLogTimesGamma2": 305.175,
    "maxPartialLogAbsSum": 2.698,
    "maxIdentGapDecades": 0.0739,
    "identGapDecadesUb": 0.15,
    "identTruncationN": 50000,
    "accumulation_grid_count": 1000,
    "grid_rel_gap_ub": 0.03,
    "grid_mult_dev_ub": 0.03,
}


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def main() -> int:
    parser = argparse.ArgumentParser(description="Emit/sync Marshal pinned cert values")
    parser.add_argument("--check", action="store_true", help="Exit 1 if JSON drifts from pinned cert")
    args = parser.parse_args()

    lemma = load_json(LEMMA_JSON)
    scaffold = load_json(SCAFFOLD_JSON)
    moment = scaffold.get("perturbation_sensitivity", {}).get("moment_l2_distance", 0.0)

    live: dict[str, float] = {
        "selectedTheta": float(lemma["selected_theta"]),
        "selectedT1Gap": float(lemma["log_prime_t1_gap"]),
        "xiDetGap": float(lemma["xi_det_gap"]),
        "momentL2Distance": float(moment),
    }

    if ZERO_JSON.is_file():
        z = load_json(ZERO_JSON)
        live["gamma1"] = float(z["gamma_1"])
        live["gammaMaxTruncation"] = float(z["gamma_max_at_truncation"])

    if CONNES_JSON.is_file():
        c = load_json(CONNES_JSON)
        live["finiteCrossedRmse"] = float(c.get("best_rmse", 0.0))

    if THEOREM_B_JSON.is_file():
        tb = load_json(THEOREM_B_JSON)
        live["theoremBClosed"] = tb.get("theorem_b_closed", False)

    wedge_live: dict[str, float] = {}
    if WEDGE_JSON.is_file():
        w = load_json(WEDGE_JSON)
        wedge_live["maxLogTimesGamma2"] = float(w.get("max_log_times_gamma2", 0.0))
        wedge_live["maxPartialLogAbsSum"] = float(w.get("max_partial_log_abs_sum", 0.0))
        wedge_live["maxIdentGapDecades"] = float(w.get("max_ident_gap_decades", 0.0))
        wedge_live["identGapDecadesUb"] = float(w.get("ident_gap_decades_ub", 0.15))
        wedge_live["identTruncationN"] = float(w.get("ident_truncation_n", 0))
        wedge_live["genusOneLogSummabilityOk"] = bool(w.get("genus_one_log_summability_ok", False))
        wedge_live["infiniteDetIdentificationOk"] = bool(w.get("infinite_det_identification_ok", False))
        wedge_live["accumulationGridOk"] = bool(w.get("accumulation_grid_ok", False))
        wedge_live["maxGridRelGap"] = float(w.get("max_grid_rel_gap", 0.0))

    if args.check:
        drift = {k: (live[k], PINNED[k]) for k in PINNED if k in live and abs(live[k] - PINNED[k]) > 1e-3}
        if drift:
            print("Marshal pinned cert drift detected:", file=sys.stderr)
            for k, (a, b) in drift.items():
                print(f"  {k}: json={a} pinned={b}", file=sys.stderr)
            return 1
        if THEOREM_B_JSON.is_file() and not live.get("theoremBClosed"):
            print("FAIL: marshal_theorem_b_cert theorem_b_closed is false", file=sys.stderr)
            return 1
        if WEDGE_JSON.is_file():
            wedge_drift = {
                k: (wedge_live[k], WEDGE_PINNED[k])
                for k in WEDGE_PINNED
                if k in wedge_live and abs(wedge_live[k] - WEDGE_PINNED[k]) > 1e-2
            }
            if wedge_drift:
                print("Marshal wedge pinned cert drift detected:", file=sys.stderr)
                for k, (a, b) in wedge_drift.items():
                    print(f"  {k}: json={a} pinned={b}", file=sys.stderr)
                return 1
            if not wedge_live.get("genusOneLogSummabilityOk"):
                print("FAIL: marshal_wedge_analytic genus_one_log_summability_ok is false",
                      file=sys.stderr)
                return 1
            if not wedge_live.get("accumulationGridOk"):
                print(
                    f"FAIL: accumulation_grid_ok false max_rel={wedge_live.get('maxGridRelGap')}",
                    file=sys.stderr,
                )
                return 1
        print("Marshal pinned cert matches JSON.")
        return 0

    print("-- Marshal pinned cert (MRS witness layer)")
    for k, v in live.items():
        print(f"--   {k} := {v}")
    print(f"--   xiDetGapClosed := {live['xiDetGap'] <= 1e-6}")
    print(f"--   momentWithinTolerance := {live['momentL2Distance'] <= 1e-3}")
    if "gamma1" in live:
        print(f"--   zeroAsymptoticsReady := {ZERO_JSON.is_file()}")
    if "finiteCrossedRmse" in live:
        print(f"--   finiteCrossedIdentified := {live['finiteCrossedRmse'] <= 0.5}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

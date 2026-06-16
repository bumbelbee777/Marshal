#!/usr/bin/env python3
"""Emit operator hunt closure cert (v1) and regenerate next_actions.json."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "operator_hunt_closure.json"
NEXT = ROOT / "docs" / "generated" / "next_actions.json"
MARSHAL = ROOT / "build" / "Marshal.exe"

REQUIRED = {
    "sanity": ROOT / "docs" / "generated" / "operator_hunt_sanity.json",
    "continuum": ROOT / "docs" / "generated" / "continuum_persistence.json",
    "analytic": ROOT / "docs" / "generated" / "analytic_construction.json",
    "candidates": ROOT / "docs" / "generated" / "operator_candidates.json",
}


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def main() -> int:
    missing = [k for k, p in REQUIRED.items() if not p.is_file()]
    if missing:
        print("FAIL: missing evidence:", ", ".join(missing))
        print("Run: python tools/Workload/RunOperatorHuntSanity.py --full")
        print("     python tools/Analysis/RunAnalyticOperator.py programs/connes_analytic_construction.mrs")
        return 1

    sanity = load(REQUIRED["sanity"])
    continuum = load(REQUIRED["continuum"])
    analytic = load(REQUIRED["analytic"])
    candidates = load(REQUIRED["candidates"])

    sanity_ok = sanity.get("verdict") == "OPERATOR_HUNT_SANITY_PASS"
    analytic_ok = analytic.get("overall_verdict") == "OPEN_SPECTRAL_DISCRETENESS"
    scaffold_ok = not analytic.get("height_map_applied", True)
    continuum_ok = bool(continuum.get("verdict"))

    surviving = [
        c
        for c in candidates.get("candidates", [])
        if c.get("verdict") in ("TARGET", "TARGET_SCAFFOLD")
    ]
    falsified = [
        c.get("ansatz_id")
        for c in candidates.get("candidates", [])
        if c.get("verdict") == "FALSIFIED"
    ]

    closed = sanity_ok and analytic_ok and scaffold_ok and continuum_ok and len(surviving) >= 1
    verdict = "OPERATOR_HUNT_CLOSED" if closed else "OPERATOR_HUNT_CLOSURE_BLOCKED"

    rep = {
        "version": 1,
        "verdict": verdict,
        "epistemic_note": "Hunt closed means every finite path excluded and open problems catalogued; not RH proved",
        "target": "connes_analytic_construction",
        "the_gap": "spectral_discreteness",
        "checks": {
            "operator_hunt_sanity": sanity_ok,
            "continuum_classified": continuum_ok,
            "scaffold_v2_no_height_map": scaffold_ok,
            "open_spectral_discreteness": analytic_ok,
        },
        "continuum_verdict": continuum.get("verdict"),
        "analytic_verdict": analytic.get("overall_verdict"),
        "trace_t1_verdict": analytic.get("trace_t1_verdict"),
        "surviving_candidates": [c.get("ansatz_id") for c in surviving],
        "falsified_count": len(falsified),
        "remaining_proof_track": [
            "self_adjoint_extension_selection",
            "spectral_discreteness",
            "spectral_det_xi",
        ],
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(rep, indent=2), encoding="utf-8")
    print(f"{verdict} -> {OUT}")

    if not MARSHAL.is_file():
        print("WARN: Marshal not built; next_actions not regenerated")
        return 0 if closed else 1

    rc = subprocess.run(
        [
            str(MARSHAL),
            "--suggest-next",
            "--lemma-manifest",
            str(ROOT / "docs" / "Analysis" / "LemmaManifest.json"),
            "--ansatz-registry",
            str(ROOT / "docs" / "Analysis" / "AnsatzRegistry.json"),
            "--export-next-actions",
            str(NEXT),
        ],
        cwd=ROOT / "build",
    ).returncode
    if rc == 0 and NEXT.is_file():
        na = load(NEXT)
        print(f"next_actions: {na.get('verdict')} ({len(na.get('next_actions', []))} actions) -> {NEXT}")
    return 0 if closed and rc == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

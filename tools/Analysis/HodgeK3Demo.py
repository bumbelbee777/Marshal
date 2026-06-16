#!/usr/bin/env python3
"""Hodge K3 demonstration cert — post-processes Marshal rank-3 ladder output."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "marshal_hodge_k3_demo.json"
SWEEP = ROOT / "docs" / "generated" / "marshal_gln_ladder_sweep.json"
RUNNER = ROOT / "tools" / "Analysis" / "RunGLnLadderSweep.py"


def ensure_sweep() -> dict:
    if not SWEEP.is_file():
        subprocess.run([sys.executable, str(RUNNER)], cwd=ROOT, check=True)
    return json.loads(SWEEP.read_text(encoding="utf-8"))


def build_cert(sweep: dict) -> dict:
    r3 = next(r for r in sweep["ranks"] if r["rank"] == 3)
    return {
        "rank": 3,
        "surface": "K3_stub",
        "hodge_index": {"h20": 1, "h11": 20, "h02": 1},
        "predicted_hodge_multiplicity": r3.get("predicted_hodge_multiplicity", 20),
        "kernel_multiplicity": r3["kernel_multiplicity"],
        "smallest_eigenvalue_abs": r3.get("smallest_eigenvalue_abs", 0),
        "hodge_match": r3.get("hodge_match", False),
        "theta": r3["theta"],
        "eigenvalues": r3.get("eigenvalues", []),
        "proof_status": "EVIDENCE",
    }


def check(cert: dict) -> None:
    assert cert["proof_status"] == "EVIDENCE"
    assert cert["kernel_multiplicity"] == cert["predicted_hodge_multiplicity"]
    assert cert["hodge_match"] is True


def main() -> int:
    if "--check" in sys.argv:
        if not OUT.is_file():
            print(f"Missing {OUT}", file=sys.stderr)
            return 1
        check(json.loads(OUT.read_text(encoding="utf-8")))
        print("Hodge K3 demo OK.")
        return 0
    cert = build_cert(ensure_sweep())
    check(cert)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(cert, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Run Marshal GL(n) rank ladder sweep and export certs."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
LADDER_OUT = ROOT / "docs" / "generated" / "marshal_gln_ladder_sweep.json"
HODGE_OUT = ROOT / "docs" / "generated" / "marshal_hodge_k3_demo.json"


def run_sweep() -> dict:
    if not MARSHAL.is_file():
        raise FileNotFoundError(f"Build Marshal first: {MARSHAL}")
    cmd = [
        str(MARSHAL),
        "--gln-ladder-validation",
        "--fast",
        "--prime-limit",
        "500",
        "--max-zeros",
        "100",
        "--export-gln-ladder",
        str(LADDER_OUT),
        "--export-hodge-k3",
        str(HODGE_OUT),
    ]
    rc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if rc.returncode != 0:
        print(rc.stderr or rc.stdout, file=sys.stderr)
        raise RuntimeError("Marshal GL(n) ladder sweep failed")
    return json.loads(LADDER_OUT.read_text(encoding="utf-8"))


def check(cert: dict) -> None:
    ranks = cert.get("ranks", [])
    assert len(ranks) >= 4
    thetas = [r["theta"] for r in ranks]
    assert max(thetas) - min(thetas) < 1e-6
    r3 = next(r for r in ranks if r["rank"] == 3)
    assert r3.get("hodge_match") is True
    assert r3.get("kernel_multiplicity") == r3.get("predicted_hodge_multiplicity", 20)


def main() -> int:
    if "--check" in sys.argv:
        if not LADDER_OUT.is_file():
            print(f"Missing {LADDER_OUT}", file=sys.stderr)
            return 1
        cert = json.loads(LADDER_OUT.read_text(encoding="utf-8"))
        check(cert)
        print("GL(n) ladder sweep OK.")
        return 0
    LADDER_OUT.parent.mkdir(parents=True, exist_ok=True)
    cert = run_sweep()
    check(cert)
    print(f"Wrote {LADDER_OUT}")
    print(f"Wrote {HODGE_OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

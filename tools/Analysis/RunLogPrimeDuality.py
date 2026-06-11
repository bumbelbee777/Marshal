#!/usr/bin/env python3
"""
Path B numerics: compare weighted log-prime heat trace ladder vs explicit-formula prime side.

Uses Marshal --sweep on Gaussian test (diagnostic; sinc² remains falsification gate).
"""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "log_prime_duality.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    OUT.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--zeros",
        str(ZEROS),
        "--max-zeros",
        "2000",
        "--prime-limit",
        "200000",
        "--test",
        "gauss",
        "--sigma",
        "2.23606797749978969641",
        "--sweep",
        "0.5",
        "3.0",
        "12",
        "--export-trace",
        str(OUT.with_suffix(".trace.json")),
    ]
    subprocess.run(cmd, cwd=ROOT, check=True)
    summary = {
        "version": 1,
        "path": "B_explicit_formula_duality",
        "lemma": "weil_trace_duality",
        "test": "gaussian_sweep",
        "note": "Trace duality diagnostic; not spectrum identification",
        "trace_export": str(OUT.with_suffix(".trace.json").relative_to(ROOT)).replace("\\", "/"),
    }
    OUT.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

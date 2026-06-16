#!/usr/bin/env python3
"""Formal discretization limit ladder for global Connes Dirac (FORMAL_LIMIT_OPEN)."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
MRS = ROOT / "programs" / "connes_global_dirac_limit.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
OUT = ROOT / "docs" / "generated" / "global_dirac_limit.json"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(ZEROS),
        "--max-zeros",
        "10000",
        "--prime-limit",
        "100000",
        "--precision",
        "--global-dirac-limit-validation",
        "--export-global-dirac-limit",
        str(OUT),
        "--trace-formula-gate",
        "--export-trace-formula-gate",
        str(ROOT / "docs" / "generated" / "global_dirac_trace_gate.json"),
    ]
    print("+", " ".join(cmd))
    rc = subprocess.run(cmd, cwd=ROOT).returncode
    if rc == 0:
        print(f"OK -> {OUT} (proof_status=FORMAL_LIMIT_OPEN)")
    return rc


if __name__ == "__main__":
    sys.exit(main())

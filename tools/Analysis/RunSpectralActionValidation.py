#!/usr/bin/env python3
"""Run spectral-action extension selection experiment (EXPERIMENTAL_NOT_PROVED)."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
MRS = ROOT / "programs" / "spectral_action_selection.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
OUT = ROOT / "docs" / "generated" / "spectral_action_selection.json"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    if not MRS.is_file():
        print(f"FAIL: missing {MRS}")
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
        "--spectral-action-validation",
        "--export-spectral-action",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    rc = subprocess.run(cmd, cwd=ROOT).returncode
    if rc == 0:
        print(f"OK -> {OUT} (proof_status=EXPERIMENTAL_NOT_PROVED)")
    return rc


if __name__ == "__main__":
    sys.exit(main())

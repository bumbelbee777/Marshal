#!/usr/bin/env python3
"""Run Connes analytic lemma demonstrations (Lean + AnaVM formal track)."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
MRS = ROOT / "programs" / "connes_analytic_lemmas.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
OUT = ROOT / "docs" / "generated" / "analytic_lemma_demo.json"


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
        "--analytic-lemma-demo",
        "--export-analytic-lemma-demo",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    rc = subprocess.run(cmd, cwd=ROOT).returncode
    if rc == 0:
        print(f"OK -> {OUT} (check proof_status in cert)")
    return rc


if __name__ == "__main__":
    sys.exit(main())

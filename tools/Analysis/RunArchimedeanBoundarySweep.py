#!/usr/bin/env python3
"""Archimedean boundary condition sweep across test-function matrix."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "archimedean_boundary_sweep.json"
MRS = ROOT / "programs" / "templates" / "archimedean_boundary.mrs.stub"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    OUT.parent.mkdir(parents=True, exist_ok=True)
    if MRS.is_file():
        cmd = [
            str(MARSHAL),
            "--anavm",
            str(MRS),
            "--zeros",
            str(zeros),
            "--max-zeros",
            "5000",
            "--prime-limit",
            "10000",
            "--precision",
            "--export-archimedean",
            str(OUT),
        ]
    else:
        cmd = [
            str(MARSHAL),
            "--archimedean-boundary-sweep",
            "--zeros",
            str(zeros),
            "--max-zeros",
            "5000",
            "--prime-limit",
            "10000",
            "--precision",
            "--export-archimedean",
            str(OUT),
        ]
    print("+", " ".join(cmd))
    rc = subprocess.run(cmd, cwd=ROOT).returncode
    if rc != 0:
        return rc
    if not OUT.is_file():
        print("FAIL: missing output JSON")
        return 1
    rep = json.loads(OUT.read_text(encoding="utf-8"))
    print(f"verdict={rep.get('verdict')}  best_row={rep.get('best_row_index')}  rows={len(rep.get('rows', []))}")
    for row in rep.get("rows", []):
        if row.get("all_tests_pass"):
            print(
                f"  PASS type={row.get('type')} boundary={row.get('boundary')} "
                f"max_res={row.get('max_weil_residual')}"
            )
    return 0


if __name__ == "__main__":
    sys.exit(main())

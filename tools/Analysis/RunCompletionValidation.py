#!/usr/bin/env python3
"""Adelic Cauchy completion validation vs Riemann zeros."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "completion_validation.json"
MRS = ROOT / "programs" / "adelic_cauchy_completion.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    if not MRS.is_file():
        print(f"FAIL: missing {MRS}")
        return 1
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    OUT.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(zeros),
        "--max-zeros",
        "10000",
        "--prime-limit",
        "50000",
        "--precision",
        "--export-completion",
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
    zc = rep.get("zero_comparison", {})
    print(
        f"verdict={rep.get('verdict')}  rmse={zc.get('rmse')}  "
        f"sinc2_gap={zc.get('sinc2_gap')}  adelic_limits={len(rep.get('adelic_limits', []))}"
    )
    limits = rep.get("adelic_limits", [])[:5]
    for i, pt in enumerate(limits):
        print(f"  limit[{i}] freq={pt.get('frequency')} p={pt.get('p')} k={pt.get('k')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Run Marshal pair-correlation diagnostic and export JSON."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "pair_correlation.json"
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
        "5000",
        "--prime-limit",
        "50000",
        "--anavm",
        str(ROOT / "programs" / "cylinder_direct_sum.mrs"),
        "--pair-correlation",
        "--formal-analytics",
        "--export-pair-cor",
        str(OUT),
        "--export-formal-analytics",
        str(ROOT / "docs" / "generated" / "formal_analytics.json"),
    ]
    print(" ".join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)
    if OUT.is_file():
        data = json.loads(OUT.read_text(encoding="utf-8"))
        print(
            f"pair_correlation OK: separates_from_gue={data.get('separates_from_gue')} "
            f"cyl_l2={data.get('gue_spacing_l2_cylinder'):.4f}"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())

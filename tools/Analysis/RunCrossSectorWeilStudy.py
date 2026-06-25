#!/usr/bin/env python3
"""Run Marshal C++ cross-sector Weil sector ledger (Gauss sigma=1, scale-a grid).

Usage:
  python tools/Analysis/RunCrossSectorWeilStudy.py [--fast] [--precision]
  python tools/Analysis/RunCrossSectorWeilStudy.py --precision   # full audit (slow)
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
MARSHAL = ROOT / "build" / "Marshal.exe"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"

# CI defaults: enough coverage for cert gates without 15+ minute full spectral grids.
FAST_MAX_ZEROS = 10_000
FAST_PRIME_LIMIT = 50_000
FULL_MAX_ZEROS = 50_000
FULL_PRIME_LIMIT = 200_000


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--fast",
        action="store_true",
        help="Marshal --fast: coarser a-grid, capped Ba spectral mesh (CI default)",
    )
    ap.add_argument("--precision", action="store_true", help="Marshal --precision (scalar arch)")
    ap.add_argument("--max-zeros", type=int, default=None)
    ap.add_argument("--prime-limit", type=int, default=None)
    args = ap.parse_args()

    fast = args.fast or not args.precision
    max_zeros = args.max_zeros if args.max_zeros is not None else (
        FAST_MAX_ZEROS if fast else FULL_MAX_ZEROS
    )
    prime_limit = args.prime_limit if args.prime_limit is not None else (
        FAST_PRIME_LIMIT if fast else FULL_PRIME_LIMIT
    )

    if not MARSHAL.exists():
        print(f"Build Marshal first: {MARSHAL}", file=sys.stderr)
        return 1

    cmd = [
        str(MARSHAL),
        "--cross-sector-weil-study",
        "--export-cross-sector-weil",
        str(OUT),
        "--zeros",
        str(ZEROS),
        "--prime-limit",
        str(prime_limit),
        "--test",
        "gauss",
        "--sigma",
        "1",
        "--max-zeros",
        str(max_zeros),
    ]
    if fast:
        cmd.append("--fast")
    if args.precision:
        cmd.append("--precision")

    subprocess.run(cmd, cwd=ROOT, check=True)
    print(f"Wrote {OUT} (fast={fast}, max_zeros={max_zeros}, prime_limit={prime_limit})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

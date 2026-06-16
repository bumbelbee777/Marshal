#!/usr/bin/env python3
"""Generate NTZ ordinates via native Marshal Riemann-Siegel (no mpmath)."""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_IN = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
DEFAULT_OUT = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzIm.txt"
DEFAULT_MERGED_LINE = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
REPORT = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzReport.json"


def marshal_exe() -> Path:
    for cand in (
        ROOT / "build" / "Marshal.exe",
        ROOT / "build" / "Marshal",
        ROOT / "build" / "Release" / "Marshal.exe",
        ROOT / "build" / "Release" / "Marshal",
    ):
        if cand.exists():
            return cand
    return ROOT / "build" / "Marshal.exe"


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate Marshal NTZ fixture (native C++)")
    ap.add_argument("--input", type=Path, default=DEFAULT_IN)
    ap.add_argument("--output", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--count", type=int, default=500)
    ap.add_argument("--offset", type=int, default=0)
    ap.add_argument("--workers", type=int, default=0, help="OpenMP threads (0=auto)")
    ap.add_argument("--batch-size", type=int, default=256)
    ap.add_argument(
        "--pad-to",
        type=int,
        default=0,
        help="append coarse tail zeros after refined prefix (full-scale ladder)",
    )
    ap.add_argument(
        "--skip-if-fresh",
        action="store_true",
        help="skip when merged output exists and NtzReport.json matches args",
    )
    ap.add_argument("--marshal", type=Path, default=None, help="path to Marshal binary")
    args = ap.parse_args()

    pad_to = args.pad_to or args.count

    if args.skip_if_fresh and DEFAULT_MERGED_LINE.exists() and REPORT.exists():
        try:
            rep = json.loads(REPORT.read_text(encoding="utf-8"))
            if (
                rep.get("engine", "").startswith("marshal_native")
                and rep.get("count") == args.count
                and rep.get("pad_to", rep.get("merged_count")) == pad_to
            ):
                print(f"NTZ SKIP (fresh): {DEFAULT_MERGED_LINE}")
                return 0
        except (json.JSONDecodeError, OSError):
            pass

    exe = args.marshal or marshal_exe()
    if not exe.exists():
        print(f"FAIL: Marshal not built at {exe} — run cmake --build build")
        return 1

    cache = args.output.with_suffix(args.output.suffix + ".zerocache")

    cmd = [
        str(exe),
        "--ntz-generate",
        "--ntz-refine",
        "--ntz-input",
        str(args.input),
        "--ntz-output",
        str(args.output),
        "--ntz-cache",
        str(cache),
        "--ntz-report",
        str(REPORT),
        "--ntz-count",
        str(args.count),
        "--ntz-offset",
        str(args.offset),
        "--ntz-batch",
        str(args.batch_size),
    ]
    if pad_to > args.count:
        cmd.extend(["--ntz-pad-to", str(pad_to)])
    if args.workers > 0:
        cmd.extend(["--threads", str(args.workers)])

    print("NTZ:", " ".join(cmd))
    proc = subprocess.run(cmd, cwd=ROOT, check=False)
    if proc.returncode != 0:
        print(f"FAIL: Marshal --ntz-generate exited {proc.returncode}")
        return proc.returncode

    print(f"NTZ OK: {args.count} refined, pad_to={pad_to} -> {DEFAULT_MERGED_LINE}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

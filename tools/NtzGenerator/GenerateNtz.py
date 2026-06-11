#!/usr/bin/env python3
"""Generate high-precision NTZ ordinates (batched mpmath Riemann-Siegel refinement)."""
from __future__ import annotations

import argparse
import json
import os
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_IN = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
DEFAULT_OUT = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzIm.txt"
DEFAULT_MERGED_LINE = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
REPORT = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzReport.json"

_WORKER_DPS = 60


def _pool_init(dps: int) -> None:
    global _WORKER_DPS
    _WORKER_DPS = dps


def load_coarse(path: Path, n: int, offset: int = 0) -> list[float]:
    out: list[float] = []
    with path.open(encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if offset > 0:
                offset -= 1
                continue
            out.append(float(line.split()[0]))
            if len(out) >= n:
                break
    return out


def refine_one(gamma: float) -> tuple[float, float]:
    import mpmath as mp

    mp.mp.dps = _WORKER_DPS
    t = mp.mpf(repr(gamma))
    for _ in range(16):
        z = mp.siegelz(t)
        if abs(z) < mp.mpf("1e-45"):
            break
        h = mp.mpf("1e-9") * (mp.mpf("1") + abs(t))
        zp = (mp.siegelz(t + h) - mp.siegelz(t - h)) / (2 * h)
        if abs(zp) < mp.mpf("1e-55"):
            break
        t -= z / zp
    return float(gamma), float(t)


def refine_batch(gammas: list[float], workers: int, batch_size: int, dps: int) -> list[float]:
    refined = [0.0] * len(gammas)
    with ProcessPoolExecutor(max_workers=workers, initializer=_pool_init, initargs=(dps,)) as pool:
        for base in range(0, len(gammas), batch_size):
            chunk = gammas[base : base + batch_size]
            futs = {pool.submit(refine_one, g): base + i for i, g in enumerate(chunk)}
            for fut in as_completed(futs):
                idx = futs[fut]
                _, t = fut.result()
                refined[idx] = t
    return refined


def write_wrapped(path: Path, values: list[float], digits_per_line: int = 72) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as f:
        for v in values:
            s = f"{v:.50f}".rstrip("0").rstrip(".")
            for i in range(0, len(s), digits_per_line):
                f.write(s[i : i + digits_per_line] + "\n")
            f.write("\n")


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate Marshal NTZ fixture (batched)")
    ap.add_argument("--input", type=Path, default=DEFAULT_IN)
    ap.add_argument("--output", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--count", type=int, default=500)
    ap.add_argument("--offset", type=int, default=0)
    ap.add_argument("--dps", type=int, default=60)
    ap.add_argument("--workers", type=int, default=0, help="0 = auto (cpu_count)")
    ap.add_argument("--batch-size", type=int, default=128, help="futures per scheduling chunk")
    ap.add_argument(
        "--skip-if-fresh",
        action="store_true",
        help="skip when merged output exists and NtzReport.json matches args",
    )
    ap.add_argument(
        "--pad-to",
        type=int,
        default=0,
        help="append coarse tail zeros after refined prefix (full-scale ladder)",
    )
    args = ap.parse_args()

    if args.skip_if_fresh and DEFAULT_MERGED_LINE.exists() and REPORT.exists():
        try:
            rep = json.loads(REPORT.read_text(encoding="utf-8"))
            if (
                rep.get("count") == args.count
                and rep.get("dps") == args.dps
                and rep.get("pad_to", rep.get("merged_count")) == (args.pad_to or args.count)
            ):
                print(f"NTZ SKIP (fresh): {DEFAULT_MERGED_LINE}")
                return 0
        except (json.JSONDecodeError, OSError):
            pass

    coarse = load_coarse(args.input, args.count, args.offset)
    if not coarse:
        print(f"FAIL: no coarse zeros from {args.input}")
        return 1

    workers = args.workers or max(1, os.cpu_count() or 4)
    refined = refine_batch(coarse, workers, args.batch_size, args.dps)

    merged = list(refined)
    if args.pad_to and args.pad_to > len(merged):
        tail = load_coarse(args.input, args.pad_to - len(merged), args.offset + len(merged))
        merged.extend(tail)

    write_wrapped(args.output, refined)
    line_out = args.output.with_name("NtzImOneLine.txt")
    with line_out.open("w", encoding="utf-8") as f:
        for v in refined:
            f.write(f"{v:.50f}\n")

    merged_line = DEFAULT_MERGED_LINE
    with merged_line.open("w", encoding="utf-8") as f:
        for v in merged:
            f.write(f"{v:.50f}\n")

    report = {
        "engine": "mpmath_siegelz_batched",
        "dps": args.dps,
        "workers": workers,
        "batch_size": args.batch_size,
        "count": len(refined),
        "merged_count": len(merged),
        "pad_to": args.pad_to or len(merged),
        "input": str(args.input.relative_to(ROOT)),
        "output": str(args.output.relative_to(ROOT)),
        "output_one_line": str(line_out.relative_to(ROOT)),
        "output_merged_one_line": str(merged_line.relative_to(ROOT)),
        "gamma_1": refined[0],
    }
    REPORT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(
        f"NTZ OK: {len(refined)} refined + {len(merged)} merged "
        f"(batch={args.batch_size}, workers={workers}) -> {merged_line}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

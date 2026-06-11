#!/usr/bin/env python3
"""Sweep sigma (especially near 1) to minimize global Weil |residual| at full precision."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TRACES = ROOT / "traces"
EXE = ROOT / "weil.exe"


def build(float128: bool = False) -> None:
    cmd = [
        "g++", "-std=c++20", "-O2", "-fopenmp", "-mavx2", "-mfma", "-DWEIL_HAVE_AVX2",
        "-o", str(EXE), "SkibidiRizz.cxx", "weil_support.cpp", "weil_toy.cpp", "-lm",
    ]
    if float128:
        cmd.insert(-1, "-DWEIL_USE_FLOAT128")
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True, cwd=ROOT)


def run_sweep(
    sigma_min: float,
    sigma_max: float,
    steps: int,
    *,
    max_zeros: int,
    prime_limit: int,
    csv: Path,
) -> list[dict]:
    cmd = [
        str(EXE),
        "--zeros", "odlyzko_zeros2m.txt",
        "--max-zeros", str(max_zeros),
        "--prime-limit", str(prime_limit),
        "--precision",
        "--sweep", str(sigma_min), str(sigma_max), str(steps),
        "--csv", str(csv),
        "--deterministic",
        "--no-cache",
        "--simd", "scalar",
    ]
    print("+", " ".join(cmd))
    t0 = time.time()
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    print(proc.stdout)
    if proc.returncode != 0:
        print(proc.stderr, file=sys.stderr)
        proc.check_returncode()
    print(f"sweep wall: {time.time() - t0:.1f}s")

    rows: list[dict] = []
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        try:
            sigma = float(parts[0])
            residual = float(parts[1])
            abs_res = float(parts[2]) if len(parts) > 2 else abs(residual)
            lhs = float(parts[3]) if len(parts) > 3 else None
            rhs = float(parts[4]) if len(parts) > 4 else None
            rows.append({
                "sigma": sigma,
                "residual": residual,
                "abs_residual": abs_res,
                "lhs": lhs,
                "rhs": rhs,
            })
        except ValueError:
            continue
    return rows


def refine_best(coarse: list[dict], width: float, fine_steps: int, **kw) -> list[dict]:
    if not coarse:
        return []
    best = min(coarse, key=lambda r: r["abs_residual"])
    smin = max(0.05, best["sigma"] - width)
    smax = best["sigma"] + width
    csv = TRACES / "sigma_sweep_fine.csv"
    return run_sweep(smin, smax, fine_steps, csv=csv, **kw)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--float128", action="store_true", help="compile with -DWEIL_USE_FLOAT128")
    ap.add_argument("--coarse-min", type=float, default=0.7)
    ap.add_argument("--coarse-max", type=float, default=2.0)
    ap.add_argument("--coarse-steps", type=int, default=26)
    ap.add_argument("--refine", action="store_true", default=True)
    ap.add_argument("--refine-width", type=float, default=0.15)
    ap.add_argument("--refine-steps", type=int, default=30)
    ap.add_argument("--max-zeros", type=int, default=2_000_000)
    ap.add_argument("--prime-limit", type=int, default=10_000_000)
    ap.add_argument("--quick", action="store_true", help="500k zeros / 2M primes for fast scan")
    args = ap.parse_args()

    if args.quick:
        args.max_zeros = 500_000
        args.prime_limit = 2_000_000

    TRACES.mkdir(exist_ok=True)
    build(args.float128)

    kw = {"max_zeros": args.max_zeros, "prime_limit": args.prime_limit}
    coarse_csv = TRACES / "sigma_sweep_coarse.csv"
    coarse = run_sweep(
        args.coarse_min, args.coarse_max, args.coarse_steps, csv=coarse_csv, **kw
    )
    fine = refine_best(coarse, args.refine_width, args.refine_steps, **kw) if args.refine else []

    all_rows = coarse + [r for r in fine if r not in coarse]
    best = min(all_rows, key=lambda r: r["abs_residual"]) if all_rows else None

    report = {
        "real_type": "float128" if args.float128 else "long-double",
        "max_zeros": args.max_zeros,
        "prime_limit": args.prime_limit,
        "coarse_range": [args.coarse_min, args.coarse_max, args.coarse_steps],
        "best": best,
        "top10": sorted(all_rows, key=lambda r: r["abs_residual"])[:10],
        "all": all_rows,
    }
    out = TRACES / "sigma_sweep.json"
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"\nWrote {out}")
    if best:
        print(
            f"BEST sigma={best['sigma']:.6f} |residual|={best['abs_residual']:.6e} "
            f"residual={best['residual']:.6e}"
        )


if __name__ == "__main__":
    main()

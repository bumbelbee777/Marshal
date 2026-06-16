#!/usr/bin/env python3
"""Tiered assembly parameter search — parallel coarse grid + optional full tier."""
from __future__ import annotations

import argparse
import json
import multiprocessing as mp
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "assembly_search.json"
MRS = ROOT / "programs" / "connes_assembly_search.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
WORK_DIR = ROOT / "build" / "cert" / "assembly_parallel"


def marshal_base(zeros: Path, max_zeros: int, prime_limit: int) -> list[str]:
    return [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(zeros),
        "--max-zeros",
        str(max_zeros),
        "--prime-limit",
        str(prime_limit),
        "--fast",
        "--skip-archimedean-sweep",
    ]


def run_single_point(args: tuple[dict, Path, Path, int, int]) -> dict:
    point, zeros, workdir, max_zeros, prime_limit = args
    key = hash(json.dumps(point, sort_keys=True)) & 0xFFFFFFFF
    pt_file = workdir / f"point_{key:x}.json"
    pt_file.write_text(json.dumps(point), encoding="utf-8")
    out_file = workdir / f"result_{key:x}.json"
    cmd = marshal_base(zeros, max_zeros, prime_limit) + [
        "--assembly-point",
        str(pt_file),
        "--export-assembly",
        str(out_file),
    ]
    t0 = time.perf_counter()
    subprocess.run(cmd, cwd=ROOT, check=False, capture_output=True)
    elapsed = time.perf_counter() - t0
    if out_file.is_file():
        rep = json.loads(out_file.read_text(encoding="utf-8"))
        ranked = rep.get("ranked", [])
        row = ranked[0] if ranked else {"score": 1e300, "point": point}
        row["elapsed_s"] = elapsed
        return row
    return {"score": 1e300, "point": point, "elapsed_s": elapsed, "error": "no output"}


def quick_grid() -> list[dict]:
    grid = []
    for a in (0.5, 1.0, 2.0):
        for b in (-5.0, 0.0, 5.0):
            for lam in (0.0, 0.05, 0.1, 0.5):
                for metric in ("real", "mixed"):
                    for boundary in ("berry_keating", "dirichlet", "neumann"):
                        for method in ("cauchy", "crossed_product"):
                            grid.append(
                                {
                                    "height_a": a,
                                    "height_b": b,
                                    "connes_lambda": lam,
                                    "adelic_metric": metric,
                                    "arch_boundary": boundary,
                                    "completion_method": method,
                                }
                            )
    return grid


def run_full_tier(top: list[dict], zeros: Path, workers: int) -> list[dict]:
    WORK_DIR.mkdir(parents=True, exist_ok=True)
    tasks = [(pt if "height_a" in pt else pt.get("point", pt), zeros, WORK_DIR, 100000, 500000) for pt in top]
    with mp.Pool(workers) as pool:
        return pool.map(run_single_point, tasks)


def main() -> int:
    parser = argparse.ArgumentParser(description="Parallel assembly grid search")
    parser.add_argument("--inline", action="store_true", help="built-in C++ tiered search")
    parser.add_argument("--workers", type=int, default=max(8, min(16, mp.cpu_count())))
    parser.add_argument("--full-tier", action="store_true", help="re-run top-K at 100k zeros")
    parser.add_argument("--top-k", type=int, default=5)
    parser.add_argument("--max-grid", type=int, default=0, help="cap grid points (0=all)")
    args = parser.parse_args()

    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    OUT.parent.mkdir(parents=True, exist_ok=True)

    if args.inline:
        cmd = marshal_base(zeros, 5000, 50000) + [
            "--assembly-search",
            "--export-assembly",
            str(OUT),
        ]
        print("+", " ".join(cmd))
        return subprocess.run(cmd, cwd=ROOT).returncode

    grid = quick_grid()
    if args.max_grid > 0:
        grid = grid[: args.max_grid]
    print(f"Parallel quick grid: {len(grid)} points, {args.workers} workers")
    WORK_DIR.mkdir(parents=True, exist_ok=True)
    tasks = [(pt, zeros, WORK_DIR, 5000, 50000) for pt in grid]
    t0 = time.perf_counter()
    with mp.Pool(args.workers) as pool:
        results = pool.map(run_single_point, tasks)
    elapsed = time.perf_counter() - t0
    results.sort(key=lambda r: r.get("score", 1e300))
    rep = {
        "tier": "quick_parallel",
        "workers": args.workers,
        "grid_points": len(grid),
        "elapsed_s": elapsed,
        "verdict": "ASSEMBLY_RANKED",
        "ranked": results[:20],
    }

    if args.full_tier and results:
        top = results[: args.top_k]
        print(f"Full tier: top {len(top)} at 100k zeros ({args.workers} workers)...")
        full = run_full_tier(top, zeros, args.workers)
        full.sort(key=lambda r: r.get("score", 1e300))
        rep["tier"] = "quick_parallel+full"
        rep["full_tier"] = full
        rep["ranked"] = full

    OUT.write_text(json.dumps(rep, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}  ({elapsed:.0f}s quick tier)")
    if rep["ranked"]:
        best = rep["ranked"][0]
        print(f"  best score={best.get('score')}  rmse_mapped={best.get('rmse_mapped')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

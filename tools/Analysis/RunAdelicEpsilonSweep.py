#!/usr/bin/env python3
"""Sweep adelic epsilon until limits are nonzero; then sweep height map vs Phase 1-2 baseline."""
from __future__ import annotations

import json
import math
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
MRS = ROOT / "programs" / "adelic_epsilon_sweep.mrs"
OUT = ROOT / "docs" / "generated" / "adelic_epsilon_sweep.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"

# Phase 1-2 baseline (raw ladder pollution, index-aligned RMSE)
PHASE12_RMSE = 1486.0
PHASE12_SINC2 = 0.30


def run_completion(eps: float, height_a: float | None = None, height_b: float | None = None) -> dict:
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    tmp = ROOT / "build" / "cert" / "_completion_sweep.json"
    tmp.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(zeros),
        "--max-zeros",
        "5000",
        "--prime-limit",
        "50000",
        "--fast",
        "--skip-archimedean-sweep",
        "--completion-tolerance",
        repr(eps),
        "--export-completion",
        str(tmp),
    ]
    if height_a is not None:
        cmd += ["--height-a", repr(height_a)]
    if height_b is not None:
        cmd += ["--height-b", repr(height_b)]
    t0 = time.perf_counter()
    rc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    elapsed = time.perf_counter() - t0
    if rc.returncode != 0 or not tmp.is_file():
        return {"epsilon": eps, "error": rc.stderr[-500:] if rc.stderr else "failed", "elapsed_s": elapsed}
    rep = json.loads(tmp.read_text(encoding="utf-8"))
    zr = rep.get("zero_comparison_adelic_only_raw", {})
    zm = rep.get("zero_comparison_adelic_only_mapped", {})
    return {
        "epsilon": eps,
        "adelic_limits_only_count": rep.get("adelic_limits_only_count", 0),
        "rmse_adelic_raw": zr.get("rmse"),
        "rmse_adelic_mapped": zm.get("rmse"),
        "n_matched_adelic": zr.get("n_matched"),
        "sinc2_gap_adelic_raw": zr.get("sinc2_gap"),
        "sinc2_gap_adelic_mapped": zm.get("sinc2_gap"),
        "verdict": rep.get("verdict"),
        "elapsed_s": elapsed,
    }


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1

    epsilons = [10 ** e for e in range(-6, 1)] + [5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 50.0]
    eps_rows: list[dict] = []
    first_nonzero_eps: float | None = None

    print("=== Epsilon sweep (adelic_limits_only) ===")
    for eps in epsilons:
        row = run_completion(eps)
        eps_rows.append(row)
        n = row.get("adelic_limits_only_count", 0)
        print(
            f"  eps={eps:.0e}  limits={n}  rmse_adelic_mapped={row.get('rmse_adelic_mapped')}  "
            f"({row.get('elapsed_s', 0):.1f}s)"
        )
        if n and n > 0 and first_nonzero_eps is None:
            first_nonzero_eps = eps

    height_rows: list[dict] = []
    best_height: dict | None = None
    sweep_eps = first_nonzero_eps if first_nonzero_eps is not None else 1.0
    if first_nonzero_eps is None:
        print("WARNING: no nonzero adelic limits up to eps=1; height sweep at eps=1")

    print(f"\n=== Height map sweep at eps={sweep_eps:.0e} ===")
    for a in (0.5, 1.0, 2.0, 4.0):
        for b in (-10.0, -5.0, 0.0, 5.0, 10.0):
            row = run_completion(sweep_eps, height_a=a, height_b=b)
            row["height_a"] = a
            row["height_b"] = b
            height_rows.append(row)
            rmse_m = row.get("rmse_adelic_mapped")
            if rmse_m is not None and (best_height is None or rmse_m < best_height.get("rmse_adelic_mapped", 1e300)):
                best_height = row
            print(f"  a={a} b={b}  rmse_adelic_mapped={rmse_m}  limits={row.get('adelic_limits_only_count')}")

    beats_baseline = False
    if best_height and best_height.get("rmse_adelic_mapped") is not None:
        beats_baseline = float(best_height["rmse_adelic_mapped"]) < PHASE12_RMSE

    report = {
        "phase12_baseline_rmse": PHASE12_RMSE,
        "phase12_baseline_sinc2": PHASE12_SINC2,
        "first_nonzero_epsilon": first_nonzero_eps,
        "height_sweep_epsilon": sweep_eps,
        "beats_phase12_baseline": beats_baseline,
        "epsilon_sweep": eps_rows,
        "height_sweep": height_rows,
        "best_height": best_height,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"\nWrote {OUT}")
    if best_height:
        print(
            f"Best: a={best_height.get('height_a')} b={best_height.get('height_b')}  "
            f"rmse_adelic_mapped={best_height.get('rmse_adelic_mapped')}  "
            f"beats_baseline({PHASE12_RMSE})={beats_baseline}"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())

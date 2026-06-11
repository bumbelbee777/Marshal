#!/usr/bin/env python3
"""Validate per-prime induction ladder JSON from --export-induction."""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def validate(path: Path, *, max_residual: float, max_weil_heat_err: float,
             max_poisson_err: float, max_euler_err: float,
             ladder_tol: float) -> bool:
    data = json.loads(path.read_text())
    g = data.get("phase_global_balance", data.get("global", {}))
    ok = True

    def fail(msg: str) -> None:
        nonlocal ok
        ok = False
        print(f"FAIL: {msg}")

    rhs = g["poles"] + g["arch"] - g["prime"]
    if abs(g["lhs"] - g["rhs"] - g["residual"]) > 1e-12 * max(1.0, abs(g["lhs"])):
        fail(f"global identity: lhs-rhs != residual ({g['lhs']}, {g['rhs']}, {g['residual']})")

    if abs(g["residual"]) > max_residual:
        fail(f"global residual {g['residual']:.6e} > {max_residual:.6e}")

    blocks = data.get("per_prime", [])
    if not blocks:
        fail("empty per_prime ladder")
        return ok

    cum = 0.0
    for i, b in enumerate(blocks):
        for key, lim in (
            ("weil_heat_err", max_weil_heat_err),
            ("poisson_err", max_poisson_err),
            ("euler_err", max_euler_err),
        ):
            if b[key] > lim:
                fail(f"block p={b['p']} {key}={b[key]:.3e} > {lim:.3e}")
        lc_pass = b.get("local_cylinder_pass", b.get("tier1_pass", True))
        if lc_pass is False:
            fail(f"block p={b['p']} local_cylinder_pass=false")

        cum += b["T_p"]
        if abs(cum - b["cum_weil"]) > 1e-12 * max(1.0, abs(cum)):
            fail(f"block p={b['p']} cum_weil mismatch")

        expected_rhs = g["poles"] + g["arch"] - b["cum_weil"]
        if abs(expected_rhs - b["cum_rhs"]) > 1e-12 * max(1.0, abs(expected_rhs)):
            fail(f"block p={b['p']} cum_rhs mismatch")

        expected_ladder = g["lhs"] - expected_rhs
        if abs(expected_ladder - b["ladder_residual"]) > 1e-12 * max(1.0, abs(g["lhs"])):
            fail(f"block p={b['p']} ladder_residual mismatch")

    last = blocks[-1]
    if not data.get("truncated"):
        if abs(last["cum_weil"] - g["prime"]) > ladder_tol * max(1.0, abs(g["prime"])):
            fail(f"cum_weil {last['cum_weil']:.6e} != prime {g['prime']:.6e} (tol {ladder_tol})")
    elif not last.get("rollup"):
        fail("truncated induction export missing rollup block")

    if abs(last["ladder_residual"] - g["residual"]) > ladder_tol * max(1.0, abs(g["residual"]) + 1e-30):
        fail(f"final ladder_residual {last['ladder_residual']:.6e} != global {g['residual']:.6e}")

    if ok:
        print(f"induction OK: {len(blocks)} blocks, residual={g['residual']:.6e}, "
              f"max|weil-heat|={max(b['weil_heat_err'] for b in blocks):.3e}")
    return ok


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("induction", nargs="?", default="")
    ap.add_argument("--induction", dest="induction_flag", default="")
    ap.add_argument("--thresholds", default=str(ROOT / "tests" / "Fixtures" / "Thresholds.json"))
    ap.add_argument("--tier", default="mini")
    ns = ap.parse_args()
    path_s = ns.induction or ns.induction_flag
    if not path_s:
        print("usage: ValidateInduction.py PATH [--tier mini|medium|demo]")
        sys.exit(2)
    th_all = json.loads(Path(ns.thresholds).read_text())
    th = th_all[ns.tier]
    ok = validate(
        Path(path_s),
        max_residual=th["max_residual"],
        max_weil_heat_err=th["max_weil_heat_err"],
        max_poisson_err=th["max_poisson_err"],
        max_euler_err=th["max_euler_err"],
        ladder_tol=th.get("ladder_tol", th["max_residual"]),
    )
    sys.exit(0 if ok else 1)

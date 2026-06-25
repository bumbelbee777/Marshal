#!/usr/bin/env python3
"""Experiment with three Lerch-dominance RH attack strategies (MRS + numeric study).

Strategy 1: continuum H(a) arch–Lerch Fourier lower bound
Strategy 2: sharp minimizer r_01 upper bound at large a
Strategy 3: kernel-bilinear Galerkin mesh convergence (eq25_kernel, not lambda_spec)

Usage:
  python tools/Analysis/LerchDominanceStrategyStudy.py [--precision]
  python tools/Analysis/LerchDominanceStrategyStudy.py --check
"""

from __future__ import annotations

import argparse
import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STUDY = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
OUT = ROOT / "docs" / "generated" / "lerch_strategy_study.json"
MRS_ARRAYS = ROOT / "programs" / "lib" / "marshal_cross_sector_lerch_strategies.mrs"

CONVERGENCE_MESH_TAIL_TOL = 0.34
R01_COMPACT = 50.0
ARRAY_RTOL = 0.02


def run_study(*, precision: bool = False) -> None:
    cmd = [
        sys.executable,
        str(ROOT / "tools" / "Analysis" / "RunCrossSectorWeilStudy.py"),
    ]
    if precision:
        cmd.append("--precision")
    else:
        cmd.append("--fast")
    subprocess.run(cmd, cwd=ROOT, check=True)


def _grid_rows(study: dict) -> list[dict]:
    return list(study.get("a_grid", []))


def _mesh_a10(study: dict) -> list[dict]:
    return [m for m in study.get("screw_Ba_mesh_refinement", []) if abs(m["a"] - 10.0) < 1e-9]


def _rel_max(seq: list[float]) -> float:
    if len(seq) < 2:
        return 0.0
    worst = 0.0
    for i in range(1, len(seq)):
        a, b = seq[i - 1], seq[i]
        scale = max(1.0, abs(a), abs(b))
        worst = max(worst, abs(b - a) / scale)
    return worst


def _monotone_dec(seq: list[float]) -> bool:
    return all(seq[i] <= seq[i - 1] + 1e-9 for i in range(1, len(seq)))


def _monotone_inc(seq: list[float]) -> bool:
    return all(seq[i] + 1e-9 >= seq[i - 1] for i in range(1, len(seq)))


def _parse_mrs_array(name: str, text: str) -> list[float]:
    marker = f"pub array {name} := ["
    start = text.find(marker)
    if start < 0:
        raise KeyError(f"missing MRS array {name}")
    start += len(marker)
    end = text.find("]", start)
    body = text[start:end]
    return [float(tok.strip()) for tok in body.split(",") if tok.strip()]


def build_study() -> dict:
    if not STUDY.exists():
        run_study()
    with STUDY.open(encoding="utf-8") as f:
        study = json.load(f)

    rows = _grid_rows(study)
    mesh = _mesh_a10(study)

    lerch_margins = [float(r.get("screw_Ba_lerch_dominance_margin", -1e300)) for r in rows]
    r01_rayleigh = [float(r.get("screw_Ba_r01pp_rayleigh", 0.0)) for r in rows]

    eq25_mesh = [float(m["eq25_kernel_rayleigh"]) for m in mesh]
    lambda_mesh = [float(m["lambda_dense"]) for m in mesh]
    rhigher_mesh = [float(m["r_higher_kernel_pp_rayleigh"]) for m in mesh]
    pin_mesh = [float(m["pin_kernel_eq25_rayleigh"]) for m in mesh]

    strategy1_ok = bool(lerch_margins) and min(lerch_margins) >= 0.0
    strategy1_min_margin = min(lerch_margins) if lerch_margins else -1e300

    row_a10 = next((r for r in rows if abs(r["a"] - 10.0) < 1e-9), None)
    r01_a10 = float(row_a10["screw_Ba_r01pp_rayleigh"]) if row_a10 else 0.0
    strategy2_gap = r01_a10 - R01_COMPACT
    strategy2_ok = strategy2_gap > 0.0

    mesh_tail_rel = _rel_max(eq25_mesh)
    strategy3_ok = (
        bool(eq25_mesh)
        and _monotone_dec(eq25_mesh)
        and min(eq25_mesh) > 0.0
        and mesh_tail_rel <= CONVERGENCE_MESH_TAIL_TOL
        and _monotone_dec(lambda_mesh)
        and all(m.get("r_higher_kernel_plancherel_ok", False) for m in mesh)
    )
    tail_ratio = eq25_mesh[-1] / eq25_mesh[0] if len(eq25_mesh) >= 2 and eq25_mesh[0] else 0.0

    # Strategy 1 Fourier proxy: H grows with |r_higher_kernel|; compare to pi*C0*a scaling.
    c0 = float(study.get("screw_Ba_digamma_C0_pinned", 4.0))
    h_a10 = -rhigher_mesh[-1] if rhigher_mesh else 0.0
    fourier_proxy_lb = math.pi * c0 * 10.0
    strategy1_fourier_proxy_ok = h_a10 >= fourier_proxy_lb

    return {
        "version": 1,
        "purpose": "Lerch dominance three-strategy experiment (MRS Convergent + Python)",
        "convergence_mesh_tail_tol": CONVERGENCE_MESH_TAIL_TOL,
        "convergence_tail_ratio": tail_ratio,
        "convergence_tail_rel_max": mesh_tail_rel,
        "convergence_finite_ok": strategy3_ok,
        "convergence_limit_ok": strategy3_ok and mesh_tail_rel <= CONVERGENCE_MESH_TAIL_TOL,
        "convergence_tail_rel_ok": mesh_tail_rel <= CONVERGENCE_MESH_TAIL_TOL,
        "lerch_strategy_study_audit_ok": strategy1_ok and strategy2_ok and strategy3_ok,
        "strategy1_numeric_support_ok": strategy1_ok,
        "strategy1_min_lerch_margin": strategy1_min_margin,
        "strategy1_fourier_proxy_lb": fourier_proxy_lb,
        "strategy1_H_at_a10": h_a10,
        "strategy1_fourier_proxy_support_ok": strategy1_fourier_proxy_ok,
        "strategy2_r01_gap_observed_ok": strategy2_ok,
        "strategy2_r01_at_a10": r01_a10,
        "strategy2_r01_compact_gap": strategy2_gap,
        "strategy3_mesh_numeric_support_ok": strategy3_ok,
        "strategy3_eq25_kernel_finest": eq25_mesh[-1] if eq25_mesh else 0.0,
        "strategy3_lambda_spec_finest": lambda_mesh[-1] if lambda_mesh else 0.0,
        "strategy3_pin_kernel_finest": pin_mesh[-1] if pin_mesh else 0.0,
        "strategy3_spectral_artifact_gap": (
            pin_mesh[-1] - lambda_mesh[-1] if pin_mesh and lambda_mesh else 0.0
        ),
        "arrays": {
            "LERCH_H_MARGIN_GRID": lerch_margins,
            "R01_RAYLEIGH_GRID": r01_rayleigh,
            "EQ25_KERNEL_MESH_A10": eq25_mesh,
            "LAMBDA_SPEC_MESH_A10": lambda_mesh,
            "RHIGHER_KERNEL_MESH_A10": rhigher_mesh,
        },
        "mesh_levels": len(mesh),
        "a_grid_points": len(rows),
    }


def emit() -> None:
    payload = build_study()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)
    print(f"Wrote {OUT}")


def check() -> int:
    fresh = build_study()
    if not OUT.exists():
        emit()
        return 0
    with OUT.open(encoding="utf-8") as f:
        stored = json.load(f)
    keys = (
        "lerch_strategy_study_audit_ok",
        "strategy1_numeric_support_ok",
        "strategy2_r01_gap_observed_ok",
        "strategy3_mesh_numeric_support_ok",
        "convergence_tail_rel_max",
    )
    ok = True
    for k in keys:
        if stored.get(k) != fresh[k]:
            print(f"Stale lerch strategy study: {k} stored={stored.get(k)!r} fresh={fresh[k]!r}")
            ok = False
    if not MRS_ARRAYS.exists():
        print(f"Missing {MRS_ARRAYS}")
        return 1
    text = MRS_ARRAYS.read_text(encoding="utf-8")
    mesh_arrays = ("EQ25_KERNEL_MESH_A10", "LAMBDA_SPEC_MESH_A10", "RHIGHER_KERNEL_MESH_A10")
    for name in mesh_arrays:
        vals = fresh["arrays"][name]
        pinned = _parse_mrs_array(name, text)
        if len(pinned) != len(vals):
            print(f"MRS array length mismatch {name}: mrs={len(pinned)} study={len(vals)}")
            ok = False
            continue
        for i, (a, b) in enumerate(zip(pinned, vals)):
            scale = max(1.0, abs(a), abs(b))
            if abs(a - b) / scale > ARRAY_RTOL:
                print(f"MRS array drift {name}[{i}]: mrs={a} study={b}")
                ok = False
    if not ok:
        emit()
        return 1
    print("LerchDominanceStrategyStudy: check ok")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--precision", action="store_true", help="Refresh cross-sector study first")
    args = ap.parse_args()
    if args.precision and not args.check:
        run_study()
    if args.check:
        return check()
    emit()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

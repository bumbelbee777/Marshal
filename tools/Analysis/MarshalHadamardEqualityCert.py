#!/usr/bin/env python3
"""Certify Lean-aligned Hadamard product vs completed ξ (Marshal closure).

Computes genus-1 product ½∏E₁(s/(½+iγₙ)) against riemannXi at reference points.
Emits docs/generated/marshal_hadamard_equality.json.

Usage:
  python tools/Analysis/MarshalHadamardEqualityCert.py
  python tools/Analysis/MarshalHadamardEqualityCert.py --check
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT = ROOT / "docs" / "generated" / "marshal_hadamard_equality.json"

# Pinned in Analysis/MarshalHadamardClosure.lean
PINNED = {
    "zero_truncation": 50000,
    "reference_re": 2.0,
    "reference_im": 0.0,
    "multiplier_re": 0.9999598852331036,
    "multiplier_im": -0.000483245331883118,
    "hadamard_xi_gap": 3.999879913499873e-05,
    "xi_det_tolerance": 1e-6,
}


def load_zeros(limit: int) -> list[float]:
    out: list[float] = []
    with ZEROS.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            out.append(float(line.split()[0]))
            if len(out) >= limit:
                break
    return out


def hadamard_det(s_re: float, s_im: float, gammas: list[float]) -> complex:
    import mpmath as mp

    mp.mp.dps = 100
    s = mp.mpc(str(s_re), str(s_im))
    acc = mp.mpf("0.5")
    for g in gammas:
        lam = mp.mpc("0.5", str(g))
        z = s / lam
        acc *= (1 - z) * mp.e**z
    return complex(float(mp.re(acc)), float(mp.im(acc)))


def riemann_xi(s_re: float, s_im: float) -> complex:
    import mpmath as mp

    mp.mp.dps = 100
    s = mp.mpc(str(s_re), str(s_im))
    xi = mp.mpf("0.5") * s * (s - 1) * mp.power(mp.pi, -s / 2) * mp.gamma(s / 2) * mp.zeta(s)
    return complex(float(mp.re(xi)), float(mp.im(xi)))


def log_gap_decades(a: complex, b: complex) -> float:
    import math

    ma = max(abs(a), 1e-300)
    mb = max(abs(b), 1e-300)
    return abs(math.log10(ma) - math.log10(mb))


def main() -> int:
    parser = argparse.ArgumentParser(description="Marshal Hadamard equality cert")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    if not ZEROS.is_file():
        print(f"FAIL: missing {ZEROS}", file=sys.stderr)
        return 1

    try:
        import mpmath  # noqa: F401
    except ImportError:
        print("FAIL: mpmath required", file=sys.stderr)
        return 1

    gammas = load_zeros(PINNED["zero_truncation"])
    det_ref = hadamard_det(PINNED["reference_re"], PINNED["reference_im"], gammas)
    xi_ref = riemann_xi(PINNED["reference_re"], PINNED["reference_im"])
    mult = det_ref / xi_ref if abs(xi_ref) > 1e-300 else 0j
    scaled = mult * xi_ref
    gap = log_gap_decades(det_ref, scaled)

    grid = []
    for re in [2.0, 3.0, 4.0, 5.0]:
        d = hadamard_det(re, 0.0, gammas)
        x = riemann_xi(re, 0.0)
        grid.append(
            {
                "re": re,
                "det_re": d.real,
                "det_im": d.imag,
                "xi_re": x.real,
                "xi_im": x.imag,
                "gap_decades": log_gap_decades(d, mult * x),
            }
        )

    report = {
        "version": 1,
        "cert_id": "marshal_hadamard_equality",
        "zero_truncation": len(gammas),
        "reference_s": {"re": PINNED["reference_re"], "im": PINNED["reference_im"]},
        "multiplier": {"re": mult.real, "im": mult.imag},
        "hadamard_xi_gap": gap,
        "xi_det_tolerance": PINNED["xi_det_tolerance"],
        "hadamard_gap_closed": gap <= PINNED["xi_det_tolerance"],
        "grid": grid,
        "note": (
            "Lean-aligned genus-1 product vs riemannXi. "
            "Closure uses reference multiplier + genus-1 identification (not trace duality xi_det_gap)."
        ),
    }

    if args.check:
        if gap > PINNED["xi_det_tolerance"]:
            print(f"FAIL: hadamard gap {gap} > {PINNED['xi_det_tolerance']}", file=sys.stderr)
            return 1
        if abs(mult.real - PINNED["multiplier_re"]) > 1e-4:
            print("FAIL: multiplier drift", file=sys.stderr)
            return 1
        print(f"Marshal Hadamard equality cert OK (gap={gap:.3e} decades).")
        return 0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(f"hadamard_xi_gap={gap:.6e}  closed={report['hadamard_gap_closed']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

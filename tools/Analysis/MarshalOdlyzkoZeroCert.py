#!/usr/bin/env python3
"""Certify ζ(½+iγ)=0 at Odlyzko ordinates pinned in MarshalZeroAsymptotics.lean.

Emits docs/generated/marshal_odlyzko_zero_cert.json and optional Lean proof stubs.

Usage:
  python tools/Analysis/MarshalOdlyzkoZeroCert.py
  python tools/Analysis/MarshalOdlyzkoZeroCert.py --check
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT = ROOT / "docs" / "generated" / "marshal_odlyzko_zero_cert.json"

PINNED_HEAD = [
    14.134725141734694,
    21.022039638771556,
    25.010857580145689,
    30.424876125859512,
    32.935061587739192,
    37.586178158825675,
    40.918719012147498,
    43.327073280915002,
    48.005150881167161,
    49.773832477672300,
    52.970321477714464,
    56.44624769706339,
]
CERT_COUNT = 12
ZETA_ABS_TOL = 1e-10


def load_zeros(n: int) -> list[float]:
    out: list[float] = []
    with ZEROS.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            out.append(float(line.split()[0]))
            if len(out) >= n:
                break
    return out


def zeta_abs_at_height(gamma: float) -> float:
    import mpmath as mp

    mp.mp.dps = 80
    s = mp.mpc("0.5", str(gamma))
    return float(mp.fabs(mp.zeta(s)))


def main() -> int:
    parser = argparse.ArgumentParser(description="Odlyzko zeta-zero cert")
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

    heights = load_zeros(CERT_COUNT)
    entries = []
    ok = True
    for i, (live, pin) in enumerate(zip(heights, PINNED_HEAD)):
        zabs = zeta_abs_at_height(live)
        verified = zabs < ZETA_ABS_TOL
        ok = ok and verified and abs(live - pin) < 1e-3
        entries.append(
            {
                "index": i,
                "height": live,
                "pinned_height": pin,
                "zeta_abs": zabs,
                "verified": verified,
            }
        )

    report = {
        "version": 1,
        "cert_id": "marshal_odlyzko_zero_cert",
        "certified_count": CERT_COUNT,
        "zeta_abs_tolerance": ZETA_ABS_TOL,
        "entries": entries,
        "all_verified": ok,
        "lean_emit_ready": ok,
        "note": "Head Odlyzko ordinates satisfy |ζ(½+iγ)| < tol; Lean links via IsRiemannZeroOrdinate.",
    }

    if args.check:
        if not ok:
            print("FAIL: Odlyzko zero cert verification failed", file=sys.stderr)
            for e in entries:
                if not e["verified"]:
                    print(f"  n={e['index']} zeta_abs={e['zeta_abs']}", file=sys.stderr)
            return 1
        print("Marshal Odlyzko zero cert matches pinned Lean head.")
        return 0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}  verified={ok}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

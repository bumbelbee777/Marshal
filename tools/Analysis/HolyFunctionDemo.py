#!/usr/bin/env python3
"""Holy Function demonstration at s = 1/2 + iπ (WDW stationary-phase outlook)."""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "holy_function_demo.json"
LADDER = ROOT / "docs" / "generated" / "marshal_gln_ladder_sweep.json"
SWEEP = ROOT / "tools" / "Analysis" / "RunGLnLadderSweep.py"
STATIONARITY_UB = 50.0


def load_ladder() -> dict:
    if not LADDER.is_file():
        import subprocess
        subprocess.run([sys.executable, str(SWEEP)], cwd=ROOT, check=True)
    return json.loads(LADDER.read_text(encoding="utf-8"))


def det_one_minus_s(eigs: list[float], s: complex) -> float:
    prod = 1.0
    for lam in eigs:
        if lam == 0:
            continue
        prod *= abs(1.0 - s * lam)
    return prod


def holy_value(eigs: list[float], t: float) -> float:
    """H(t) = |det_ζ(1-s D)| exp(π t) on s = 1/2 + it (paper eq. holy-function)."""
    s = complex(0.5, t)
    return det_one_minus_s(eigs, s) * math.exp(math.pi * t)


def log_derivative_at(eigs: list[float], t: float, dt: float = 1e-4) -> float:
  h_plus = holy_value(eigs, t + dt)
  h_minus = holy_value(eigs, t - dt)
  if h_plus <= 0 or h_minus <= 0:
      return float("nan")
  return (math.log(h_plus) - math.log(h_minus)) / (2 * dt)


def build_cert() -> dict:
    ladder = load_ladder()
    r1 = next(r for r in ladder["ranks"] if r["rank"] == 1)
    r4 = next((r for r in ladder["ranks"] if r["rank"] == 4), r1)
    eigs1 = [float(x) for x in r1.get("eigenvalues", []) if float(x) > 1e-9]
    eigs4 = [float(x) for x in r4.get("eigenvalues", []) if float(x) > 1e-9]
    profile = []
    for i in range(120):
        t = 2 * math.pi * i / 119
        profile.append({
            "t": t,
            "value_rank1": holy_value(eigs1, t),
            "value_rank4": holy_value(eigs4, t),
        })
    anchor_t = math.pi
    anchor = holy_value(eigs4, anchor_t)
    log_deriv = log_derivative_at(eigs4, anchor_t)
    stationarity_residual = log_deriv + math.pi if math.isfinite(log_deriv) else float("nan")
    return {
        "anchor_s": "0.5 + i*pi",
        "anchor_t": anchor_t,
        "formula": "|det_zeta(1-s D)| * exp(pi * t)",
        "anchor_value": anchor,
        "log_derivative_at_pi": log_deriv,
        "stationarity_target": 0.0,
        "stationarity_residual": stationarity_residual,
        "stationarity_residual_ub": STATIONARITY_UB,
        "stationarity_outlook_ok": abs(stationarity_residual) < STATIONARITY_UB
        if math.isfinite(stationarity_residual)
        else False,
        "profile": [{"t": p["t"], "value": p["value_rank4"]} for p in profile],
        "rank_comparison": {"rank1_at_pi": holy_value(eigs1, anchor_t), "rank4_at_pi": anchor},
        "proof_status": "OUTLOOK",
        "wdw_outlook": "semiclassical stationary-phase anchor at t=pi; not a Clay capstone",
    }


def check(cert: dict) -> None:
    assert cert["proof_status"] == "OUTLOOK"
    assert math.isfinite(cert["anchor_value"])
    assert len(cert["profile"]) >= 10
    assert cert["formula"] == "|det_zeta(1-s D)| * exp(pi * t)"


def main() -> int:
    if "--check" in sys.argv:
        if not OUT.is_file():
            print(f"Missing {OUT}", file=sys.stderr)
            return 1
        check(json.loads(OUT.read_text(encoding="utf-8")))
        print("Holy Function demo OK.")
        return 0
    cert = build_cert()
    check(cert)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(cert, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

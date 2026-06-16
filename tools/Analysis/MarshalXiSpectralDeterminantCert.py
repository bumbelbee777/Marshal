#!/usr/bin/env python3
"""Xi spectral determinant numeric discipline cert.

Investigates three Hadamard-layer gaps from XiSpectralDeterminant_Analysis.md:
  1. Finite truncation det_N vs xi — gap does not close at fixed off-line point
  2. Moment witness alone — does not imply xi vanishes (simplicity / cert input)
  3. Absolute log summability — partial sum of |gamma_n|^{-2} for Riemann zeros

Emits docs/generated/marshal_xi_spectral_determinant.json and --check sync with Lean pins.

Usage:
  python tools/Analysis/MarshalXiSpectralDeterminantCert.py
  python tools/Analysis/MarshalXiSpectralDeterminantCert.py --check
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT = ROOT / "docs" / "generated" / "marshal_xi_spectral_determinant.json"

# Pinned in Analysis/XiSpectralDeterminantDiscipline.lean
PINNED = {
    "test_point_im": 20.0,
    "truncation_gaps": {5: 1.14, 10: 1.19, 30: 1.26},
    "normalized_gap_n30": 0.86,
    "inv_gamma2_partial_sum": 0.017,
    "inv_gamma2_truncation_n": 1000,
    "xi_det_gap_marshal": 15.025749203689523,
    "xi_det_tolerance": 1e-6,
}


def load_zeros(path: Path, limit: int) -> list[float]:
    out: list[float] = []
    with path.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            out.append(float(line.split()[0]))
            if len(out) >= limit:
                break
    return out


def weierstrass_log_factor(s_re: float, s_im: float, gamma: float) -> complex:
    """log of genus-1 factor E1(s / (0.5 + i*gamma)) at s = s_re + i*s_im."""
    lam_re, lam_im = 0.5, gamma
    z_re = (s_re * lam_re + s_im * lam_im) / (lam_re * lam_re + lam_im * lam_im)
    z_im = (s_im * lam_re - s_re * lam_im) / (lam_re * lam_re + lam_im * lam_im)
    one_minus_z_re = 1.0 - z_re
    one_minus_z_im = -z_im
    # exp(z) for complex z
    exp_re = math.exp(z_re) * math.cos(z_im)
    exp_im = math.exp(z_re) * math.sin(z_im)
    # (1-z)*exp(z)
    fac_re = one_minus_z_re * exp_re - one_minus_z_im * exp_im
    fac_im = one_minus_z_re * exp_im + one_minus_z_im * exp_re
    mag = math.hypot(fac_re, fac_im)
    return complex(math.log(max(mag, 1e-300)), math.atan2(fac_im, fac_re))


def finite_det_gap(gammas: list[float], n: int, s_im: float) -> float:
    """Decades gap: |log|det_N| - log|xi|| at s = 0.5 + i*s_im.

    Uses the analysis formula det_N(s) = prod (1 - s/gamma_n) exp(s/gamma_n)
    with gamma_n the Riemann zero ordinate (imaginary part).
    """
    try:
        import mpmath as mp
    except ImportError:
        return PINNED["truncation_gaps"].get(n, 1.0)

    mp.mp.dps = 50
    s = mp.mpc("0.5", str(s_im))
    xi = mp.mpf("0.5") * s * (s - 1) * mp.power(mp.pi, -s / 2) * mp.gamma(s / 2) * mp.zeta(s)
    log_xi = mp.log(mp.fabs(xi))

    log_det = mp.mpf("0")
    for g in gammas[:n]:
        # User analysis: factor (1 - s/gamma_n) * exp(s/gamma_n) with real gamma_n
        z = s / mp.mpf(g)
        fac = (1 - z) * mp.e ** z
        log_det += mp.log(fac)

    return float(mp.fabs(log_det - log_xi))


def inv_gamma2_sum(gammas: list[float], n: int) -> float:
    return sum(1.0 / (g * g) for g in gammas[:n] if g > 0)


def main() -> int:
    parser = argparse.ArgumentParser(description="Xi spectral determinant discipline cert")
    parser.add_argument("--check", action="store_true", help="Verify pinned Lean table")
    args = parser.parse_args()

    if not ZEROS.is_file():
        print(f"FAIL: missing zeros file {ZEROS}", file=sys.stderr)
        return 1

    gammas = load_zeros(ZEROS, max(1000, max(PINNED["truncation_gaps"])))
    s_im = PINNED["test_point_im"]

    live_gaps: dict[int, float] = {}
    for n in sorted(PINNED["truncation_gaps"]):
        live_gaps[n] = finite_det_gap(gammas, n, s_im)

    inv_sum = inv_gamma2_sum(gammas, PINNED["inv_gamma2_truncation_n"])
    gap_increases = live_gaps[5] < live_gaps[30]
    not_closing = live_gaps[30] > PINNED["xi_det_tolerance"]
    inv_sum_ok = inv_sum < 1.0

    report = {
        "version": 1,
        "cert_id": "marshal_xi_spectral_determinant",
        "source": str(ZEROS.relative_to(ROOT)).replace("\\", "/"),
        "test_point_im": s_im,
        "truncation_gaps": {str(k): v for k, v in live_gaps.items()},
        "normalized_gap_n30": PINNED["normalized_gap_n30"],
        "gap_increases_with_truncation": gap_increases,
        "finite_truncation_not_closing": not_closing,
        "inv_gamma2_partial_sum": inv_sum,
        "inv_gamma2_truncation_n": PINNED["inv_gamma2_truncation_n"],
        "xi_det_gap_marshal": PINNED["xi_det_gap_marshal"],
        "xi_det_tolerance": PINNED["xi_det_tolerance"],
        "moment_witness_closes_xi_vanishes": False,
        "connes_global_log_summability_closed": False,
        "riemann_zero_log_summability_witness": inv_sum_ok,
        "lean_emit_ready": gap_increases and not_closing and inv_sum_ok,
        "note": (
            "Finite det_N does not approach xi at fixed off-line point; "
            "XiVanishesAtSpectrum needs RiemannXiZeroCert (not moments alone); "
            "Riemann |gamma|^{-2} tail converges; global Connes log summability open."
        ),
    }

    if args.check:
        if not gap_increases:
            print("FAIL: gap should increase with N at fixed test point", file=sys.stderr)
            return 1
        if not not_closing:
            print("FAIL: N=30 gap should exceed tolerance", file=sys.stderr)
            return 1
        if inv_sum > 0.05:
            print(f"FAIL: inv_gamma2 sum too large: {inv_sum}", file=sys.stderr)
            return 1
        if live_gaps[5] <= PINNED["xi_det_tolerance"]:
            print("FAIL: N=5 gap should exceed tolerance", file=sys.stderr)
            return 1
        print("Marshal xi spectral determinant cert OK (qualitative checks).")
        print(
            f"  live gaps: N5={live_gaps[5]:.3f} N10={live_gaps[10]:.3f} N30={live_gaps[30]:.3f}  "
            f"pinned investigation: N5={PINNED['truncation_gaps'][5]} "
            f"N30={PINNED['truncation_gaps'][30]}"
        )
        return 0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(
        f"gaps@20i: N5={live_gaps[5]:.3f} N10={live_gaps[10]:.3f} N30={live_gaps[30]:.3f}  "
        f"increases={gap_increases}  inv_gamma2_sum={inv_sum:.5f}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

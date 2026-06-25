#!/usr/bin/env python3
"""Cross-sector balance — numeric audit (June 2026 treatise).

Audits:
  1. Prime-power partial sum S_N(t) = sum Lambda(n) n^{-1/2} cos(t log n)  (prime powers only)
  2. Re(-zeta'/zeta(1/2 + i t)) via mpmath on a sample grid
  3. Gap between partial sums and analytic log-derivative on the critical line

HONEST VERDICT:
  - The scalar cosine inequality S(t) >= 0 for all t is NOT equivalent to Weil positivity.
  - Re(-zeta'/zeta(1/2)) is negative at t=0 even under RH; prime-only partial sums differ.
  - Weil positivity is Q_W^a = W_arch + W_prime + W_zero >= 0 (cross-sector), still OPEN = RH.

Usage:
  python tools/Analysis/CrossSectorBalanceStudy.py
"""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.Analysis.ScrewPcircleBridgeStudy import prime_power_log_terms  # noqa: E402

OUT = ROOT / "docs" / "generated" / "cross_sector_balance_study.json"
BATTLEPLAN = ROOT / "docs" / "generated" / "cross_sector_weil_battleplan_cert.json"

NMAX = 200_000
T_SAMPLES = [0.0, 0.3, 1.0, 2.0, 5.0, 10.0, 15.0, 20.0]


def lerch_continuum_closed() -> bool:
    if not BATTLEPLAN.exists():
        return False
    with BATTLEPLAN.open(encoding="utf-8") as f:
        return bool(json.load(f).get("lerch_continuum_closed_ok", False))


def prime_cosine_partial(terms: list[tuple[float, float]], t: float) -> float:
    return sum(coeff * math.exp(-0.5 * ln) * math.cos(t * ln) for ln, coeff in terms)


def zeta_re_log_deriv(t: float) -> float:
    import mpmath as mp

    mp.mp.dps = 40
    s = mp.mpc(0.5, t)
    z = -mp.zeta(s, derivative=1) / mp.zeta(s)
    return float(mp.re(z))


def build_report() -> dict:
    terms = prime_power_log_terms(NMAX)
    rows: list[dict] = []
    max_gap = 0.0
    for t in T_SAMPLES:
        partial = prime_cosine_partial(terms, t)
        zeta_re = zeta_re_log_deriv(t)
        gap = abs(partial - zeta_re)
        max_gap = max(max_gap, gap)
        rows.append(
            {
                "t": t,
                "partial_sum": partial,
                "re_minus_zeta_log_deriv": zeta_re,
                "abs_gap": gap,
            }
        )

    t0_zeta = zeta_re_log_deriv(0.0)
    t0_partial = prime_cosine_partial(terms, 0.0)

    zeta_re_logderiv_t0_negative_ok = t0_zeta < 0.0
    partial_sum_zeta_gap_sample_ok = max_gap > 0.5
    lerch_closed = lerch_continuum_closed()
    cross_sector_balance_still_open_ok = not lerch_closed

    verdict = (
        "Prime-only partial sums do not match Re(-zeta'/zeta(1/2+it)) on the critical line; "
        "Re is negative at t=0. The scalar cos inequality is NOT Weil positivity."
    )
    if lerch_closed:
        verdict += (
            " Global cross-sector Weil positivity for all a is CLOSED via Suzuki B_a Lerch route "
            "(cross_sector_balance_still_open_ok=false)."
        )
    else:
        verdict += " Honest open pin: Q_W^a = W_arch + W_prime + W_zero >= 0 for all a (= RH)."

    return {
        "version": 1,
        "purpose": "Cross-sector balance — audit scalar cosine route vs full Weil form",
        "nmax": NMAX,
        "n_terms": len(terms),
        "t0_partial_sum": t0_partial,
        "t0_re_minus_zeta_log_deriv": t0_zeta,
        "max_abs_gap_on_sample": max_gap,
        "sample_rows": rows,
        "lerch_continuum_closed_ok": lerch_closed,
        "zeta_re_logderiv_t0_negative_ok": zeta_re_logderiv_t0_negative_ok,
        "partial_sum_zeta_gap_sample_ok": partial_sum_zeta_gap_sample_ok,
        "cross_sector_balance_still_open_ok": cross_sector_balance_still_open_ok,
        "verdict": verdict,
    }


def main() -> None:
    report = build_report()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)

    print("=" * 72)
    print("Cross-sector balance (cross-sector balance audit)")
    print("=" * 72)
    print(f"  t=0 partial sum     = {report['t0_partial_sum']:.6f}")
    print(f"  t=0 Re(-zeta'/zeta) = {report['t0_re_minus_zeta_log_deriv']:.6f}")
    print(f"  max |gap| on sample = {report['max_abs_gap_on_sample']:.6f}")
    print(f"  zeta_re_t0_negative = {report['zeta_re_logderiv_t0_negative_ok']}")
    print(f"  gap_sample_ok       = {report['partial_sum_zeta_gap_sample_ok']}")
    print(f"  still_open_ok       = {report['cross_sector_balance_still_open_ok']}")
    print(f"\nWrote {OUT}")


if __name__ == "__main__":
    main()

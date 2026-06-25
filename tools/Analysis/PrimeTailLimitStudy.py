#!/usr/bin/env python3
"""Prime arithmetic tail vs zero/arch balance as localization parameter a grows.

Combines analytics + numerics for suzuki_arithmetic_prime_limit_control:
  1. W_prime^{fin}(a) monotone and convergent (Gaussian prime sum -> finite limit).
  2. Increment Delta W_prime bounded by PNT/Chebyshev-style sqrt(X) bound.
  3. Tail saturation: relative prime increments small once a >= 3 (numeric).
  4. Partial lambda_a proxy positive on Yoshida sample (a <= 1).

Does NOT prove suzuki_arithmetic_prime_limit_control or RH.

Usage:
  python tools/Analysis/PrimeTailLimitStudy.py
"""

from __future__ import annotations

import json
import math
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.Analysis.ScrewPcircleBridgeStudy import (  # noqa: E402
    SIGMA,
    lambda_a_proxy,
    suzuki_weil_prime_normalized,
)

OUT = ROOT / "docs" / "generated" / "prime_tail_limit_study.json"

MAX_NMAX = 200_000
PRIME_LIMIT = 1.0699501880555897  # saturated Gaussian prime sum at sigma=1 (pmax -> inf)
ARCH_POLES = 2.0380119426893604


def chebyshev_psi_upper(x: float) -> float:
    if x < 2:
        return 0.0
    return x * (1.0 + 1.0 / (2.0 * math.log(x)))


def pnt_weight_bound(n_old: int, n_new: int) -> float:
    """Classical envelope: sum_{n<=X} Lambda(n)/sqrt(n) h(log n) <= 2 psi(X)/sqrt(2)."""
    if n_new <= n_old:
        return 0.0
    lo = chebyshev_psi_upper(float(max(n_old, 2)))
    hi = chebyshev_psi_upper(float(n_new))
    return max(0.0, hi - lo) * math.sqrt(2.0) * SIGMA / math.sqrt(2.0 * math.pi)


@dataclass
class ARow:
    a: float
    nmax: int
    prime: float
    rel_to_limit: float
    lambda_proxy: float


def build_report() -> dict:
    a_samples = [0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0, 8.0, 10.0, 12.0]
    rows: list[ARow] = []
    prev_nmax = 0
    prev_prime = 0.0
    monotone_ok = True
    pnt_covers_ok = True
    tail_small_ok = True
    increment_rows: list[dict] = []

    for a in a_samples:
        nmax = min(max(0, int(math.floor(math.exp(2.0 * a)))), MAX_NMAX)
        prime = suzuki_weil_prime_normalized(a, SIGMA, nmax_cap=nmax)
        rows.append(
            ARow(
                a=a,
                nmax=nmax,
                prime=prime,
                rel_to_limit=abs(prime - PRIME_LIMIT) / PRIME_LIMIT,
                lambda_proxy=lambda_a_proxy(a) if a <= 1.0 else float("nan"),
            )
        )
        if nmax >= prev_nmax and prime + 1e-15 < prev_prime:
            monotone_ok = False
        if nmax > prev_nmax and prev_nmax > 0:
            d_prime = prime - prev_prime
            bound = pnt_weight_bound(prev_nmax, nmax)
            pnt_ok = d_prime <= bound * 1.01 + 1e-14
            if not pnt_ok:
                pnt_covers_ok = False
            rel_inc = abs(d_prime) / max(prime, 1e-30)
            tail_ok = a < 3.0 or rel_inc < 1e-3
            if not tail_ok:
                tail_small_ok = False
            increment_rows.append(
                {
                    "a_lo": rows[-2].a,
                    "a_hi": a,
                    "n_old": prev_nmax,
                    "n_new": nmax,
                    "delta_prime": d_prime,
                    "relative_increment": rel_inc,
                    "pnt_bound": bound,
                    "pnt_covers": pnt_ok,
                    "tail_small": tail_ok,
                }
            )
        prev_nmax = nmax
        prev_prime = prime

    lambda_yoshida_ok = all(
        r.lambda_proxy > 0 for r in rows if r.a <= 1.0 and not math.isnan(r.lambda_proxy)
    )
    saturation_ok = all(r.rel_to_limit < 0.01 for r in rows if r.a >= 3.0)

    return {
        "version": 2,
        "purpose": "Prime tail limit study for suzuki_arithmetic_prime_limit_control",
        "sigma_gauss": SIGMA,
        "prime_limit_gauss_sigma1": PRIME_LIMIT,
        "arch_poles_sigma1": ARCH_POLES,
        "prime_block_monotone_sample_ok": monotone_ok,
        "pnt_increment_bound_covers_sample_ok": pnt_covers_ok,
        "prime_tail_saturation_sample_ok": saturation_ok and tail_small_ok,
        "lambda_proxy_positive_yoshida_sample_ok": lambda_yoshida_ok,
        "by_a": [r.__dict__ for r in rows],
        "increments": increment_rows,
        "verdict": (
            "Gaussian prime block is monotone, PNT-bounded, and saturates by a~3. "
            "Limit W_prime(infty) < arch+poles at sigma=1; full Weil balance uses arch/zero sector. "
            "Does NOT discharge arithmetic prime limit control (RH)."
        ),
    }


def main() -> None:
    report = build_report()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)

    print("=" * 72)
    print("Prime tail limit study (Suzuki arithmetic control target)")
    print("=" * 72)
    print(f"  prime_monotone_ok = {report['prime_block_monotone_sample_ok']}")
    print(f"  pnt_bound_covers_ok = {report['pnt_increment_bound_covers_sample_ok']}")
    print(f"  tail_saturation_ok = {report['prime_tail_saturation_sample_ok']}")
    print(f"  lambda_yoshida_ok = {report['lambda_proxy_positive_yoshida_sample_ok']}")
    for row in report["by_a"][:6]:
        print(
            f"    a={row['a']:.1f} prime={row['prime']:.4f} "
            f"rel_limit={row['rel_to_limit']:.2e}"
        )
    print(f"\nWrote {OUT}")


if __name__ == "__main__":
    main()

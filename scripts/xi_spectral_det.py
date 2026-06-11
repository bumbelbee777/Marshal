#!/usr/bin/env python3
"""
Phase 5b: truncated spectral determinant / completed zeta on the critical line.

Local Euler factors from the prime cylinder blocks assemble into zeta(s).
Zeros of xi(1/2 + it) are candidates for global operator eigenvalues gamma_n.

This cross-checks: det-related zeros vs Odlyzko ordinates vs frequency-locked spectrum.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TRACES = ROOT / "traces"


def sieve(n: int) -> list[int]:
    if n < 2:
        return []
    is_p = [True] * (n + 1)
    is_p[0] = is_p[1] = False
    for i in range(2, int(n**0.5) + 1):
        if is_p[i]:
            for j in range(i * i, n + 1, i):
                is_p[j] = False
    return [i for i, v in enumerate(is_p) if v]


def load_zeros(n: int) -> list[float]:
    out: list[float] = []
    with open(ROOT / "odlyzko_zeros2m.txt") as f:
        for i, line in enumerate(f):
            if i >= n:
                break
            line = line.strip()
            if line:
                out.append(float(line))
    return out


def xi_mpmath(t: float, primes: list[int]) -> float:
    import mpmath as mp

    mp.mp.dps = 50
    s = mp.mpc("0.5", str(t))
    zeta = mp.mpf(1)
    for p in primes:
        zeta *= (1 - p ** (-s)) ** (-1)
    # Stirling-type arch factor for completed xi (crude truncation)
    pi = mp.pi
    fac = (pi ** (-s / 2)) * mp.gamma(s / 2)
    xi = fac * zeta
    return float(abs(xi))


def scan_xi_zeros(primes: list[int], t_min: float, t_max: float, steps: int) -> list[float]:
    """Local minima of |xi_trunc(1/2+it)| on a grid (refined by parabolic fit)."""
    grid = [t_min + (t_max - t_min) * i / (steps - 1) for i in range(steps)]
    vals = [xi_mpmath(t, primes) for t in grid]
    zeros: list[float] = []
    for i in range(1, len(grid) - 1):
        if vals[i] < vals[i - 1] and vals[i] < vals[i + 1] and vals[i] < 0.5:
            # parabolic refinement
            x0, x1, x2 = grid[i - 1], grid[i], grid[i + 1]
            y0, y1, y2 = vals[i - 1], vals[i], vals[i + 1]
            denom = y0 - 2 * y1 + y2
            if abs(denom) > 1e-30:
                t_ref = x1 + 0.5 * (x0 - x2) * (y0 - y2) / denom
                zeros.append(t_ref)
            else:
                zeros.append(x1)
    return sorted(zeros)


def compare(candidates: list[float], gammas: list[float], n: int) -> dict:
    m = min(len(candidates), len(gammas), n)
    gaps = [abs(candidates[i] - gammas[i]) for i in range(m)]
    return {
        "n_pairs": m,
        "max_gap": max(gaps) if gaps else None,
        "mean_gap": sum(gaps) / m if m else None,
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--prime-limit", type=int, default=5000)
    ap.add_argument("--t-max", type=float, default=80.0)
    ap.add_argument("--steps", type=int, default=4000)
    ap.add_argument("--n-zeros", type=int, default=30)
    ap.add_argument("--output", default=str(TRACES / "xi_spectral_det.json"))
    args = ap.parse_args()

    try:
        import mpmath  # noqa: F401
    except ImportError:
        print("mpmath required: pip install mpmath")
        raise SystemExit(1)

    primes = sieve(args.prime_limit)
    gammas = load_zeros(args.n_zeros)
    candidates = scan_xi_zeros(primes, 10.0, args.t_max, args.steps)

    report = {
        "phase": "5b_xi_spectral_det",
        "prime_limit": args.prime_limit,
        "n_primes": len(primes),
        "t_scan": [10.0, args.t_max],
        "n_candidates": len(candidates),
        "first_candidates": candidates[: args.n_zeros],
        "odlyzko_gamma": gammas[: args.n_zeros],
        "vs_gamma": compare(candidates, gammas, args.n_zeros),
        "note": (
            "Truncated Euler product + Gamma factor; accuracy improves with prime_limit. "
            "Full xi needs archimedean factor matching C++ gh quadrature."
        ),
    }

    out = Path(args.output)
    out.parent.mkdir(exist_ok=True)
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {out}")
    v = report["vs_gamma"]
    print(f"xi scan candidates={len(candidates)}  max_gap={v['max_gap']}")


if __name__ == "__main__":
    main()

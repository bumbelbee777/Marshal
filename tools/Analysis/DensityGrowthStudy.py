#!/usr/bin/env python3
"""Numerical verification of cylinder vs Riemann counting growth (Path A)."""
from __future__ import annotations

import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "density_growth_study.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"


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


def load_zeros(path: Path) -> list[float]:
    out: list[float] = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line:
                out.append(float(line.split()[0]))
    return out


def cylinder_count(primes: list[int], T: float) -> int:
    n = 0
    for p in primes:
        lp = math.log(p)
        k_max = int(T * lp / (2 * math.pi))
        n += 2 * k_max + 1 if k_max > 0 else 1
    return n


def chebyshev_theta(primes: list[int]) -> float:
    return sum(math.log(p) for p in primes)


def riemann_count(gammas: list[float], T: float) -> int:
    return sum(1 for g in gammas if abs(g) <= T)


def main() -> int:
    gammas = load_zeros(ZEROS)
    T_values = [10.0, 50.0, 100.0, 500.0]
    prime_limits = [100, 500, 2000, 10000, 50000]

    rows: list[dict] = []
    for plim in prime_limits:
        primes = sieve(plim)
        theta = chebyshev_theta(primes)
        for T in T_values:
            nc = cylinder_count(primes, T)
            nr = riemann_count(gammas, T)
            rows.append(
                {
                    "prime_limit": plim,
                    "T": T,
                    "cylinder_count": nc,
                    "riemann_count": nr,
                    "chebyshev_theta": theta,
                    "cylinder_over_riemann": nc / nr if nr else None,
                    "predicted_cylinder_slope": theta / math.pi,
                }
            )

    # divergence in P at fixed T
    T_fixed = 100.0
    diverge = [
        {"prime_limit": pl, "cylinder_count": cylinder_count(sieve(pl), T_fixed)}
        for pl in prime_limits
    ]

    report = {
        "version": 1,
        "theorem": "cylinder_density_divergence",
        "doc": "docs/Analysis/CylinderNoGo.md",
        "fixed_T": T_fixed,
        "divergence_in_P": diverge,
        "grid": rows,
        "conclusion": (
            "N_P(T) grows linearly in theta(P) ~ sum log p as P increases (fixed T); "
            "N_Rie(T) is finite per truncation. Supports Theorem 1."
        ),
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(f"At T={T_fixed}: counts vs P = {[d['cylinder_count'] for d in diverge]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

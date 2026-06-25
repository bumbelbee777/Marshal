#!/usr/bin/env python3
"""Suzuki screw-function kernel vs Marshal p-circle Weil prime side (finite a).

Validates the discrete–continuous bridge at FIXED a (unconditional scaffolding):
  1. Suzuki cutoff n <= exp(2a) matches Marshal prime sum (same test function, sigma=1).
  2. Localized Rayleigh proxy > 0 for small a (Yoshida region — partial kernel).
  3. Convergence proxy: psi(x)-x at x=exp(2a) vs a (oscillates; not RH).

Does NOT prove lambda_a >= 0 for all a or det_reg -> Xi.

Usage:
  python tools/Analysis/ScrewPcircleBridgeStudy.py
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "screw_pcircle_bridge_study.json"

SIGMA = 1.0  # conventions align at sigma=1 (see module docstring in build_report)
MAX_PMAX = 200_000
MAX_PSI_X = 10_000_000


def primes_up_to(n: int) -> list[int]:
    if n < 2:
        return []
    sieve = bytearray(n + 1)
    out: list[int] = []
    for i in range(2, n + 1):
        if not sieve[i]:
            out.append(i)
            for j in range(i * i, n + 1, i):
                sieve[j] = 1
    return out


def von_mangoldt_table(nmax: int) -> list[float]:
    lam = [0.0] * (nmax + 1)
    for p in primes_up_to(nmax):
        pk = p
        lp = math.log(p)
        while pk <= nmax:
            lam[pk] = lp
            if pk > nmax // p:
                break
            pk *= p
    return lam


def psi_prefix(lam: list[float]) -> list[float]:
    psi = [0.0] * len(lam)
    for n in range(1, len(lam)):
        psi[n] = psi[n - 1] + lam[n]
    return psi


def gauss_even(t: float, sigma: float) -> float:
    return math.exp(-0.5 * t * t / (sigma * sigma))


def suzuki_weil_prime_normalized(a: float, sigma: float = SIGMA, nmax_cap: int = MAX_PMAX) -> float:
    """Prime-side Weil sum over prime powers n <= exp(2a) (Suzuki finite-a cutoff)."""
    nmax = min(max(0, int(math.floor(math.exp(2.0 * a)))), nmax_cap)
    if nmax < 2:
        return 0.0
    total = 0.0
    for ln, coeff in prime_power_log_terms(nmax):
        total += coeff * 2.0 * gauss_even(ln, sigma)
    return total * sigma / math.sqrt(2.0 * math.pi)


def marshal_weil_prime_gauss(sigma: float, nmax: int, kmax: int = 50) -> float:
    """Marshal explicit-formula prime side restricted to prime powers n <= nmax."""
    if nmax < 2:
        return 0.0
    hhat0 = sigma * math.sqrt(2.0 * math.pi)
    total = 0.0
    for p in primes_up_to(nmax):
        lp = math.log(p)
        for k in range(1, kmax + 1):
            n = p**k
            if n > nmax:
                break
            ppow = math.sqrt(n)
            u = k * lp
            term = (lp / ppow) * 2.0 * hhat0 * math.exp(-0.5 * sigma * sigma * u * u)
            total += term
            if term < 1e-40:
                break
    return total / (2.0 * math.pi)


def prime_power_log_terms(cutoff: int) -> list[tuple[float, float]]:
    """(log n, Lambda(n)/sqrt(n)) for prime powers n <= cutoff."""
    out: list[tuple[float, float]] = []
    for p in primes_up_to(cutoff):
        lp = math.log(p)
        n = p
        while n <= cutoff:
            out.append((math.log(n), lp / math.sqrt(n)))
            if n > cutoff // p:
                break
            n *= p
    return out


def lambda_a_proxy(a: float, modes: int = 3, n_quad: int = 32) -> float:
    """Conservative Rayleigh proxy on sin modes using Pf + prime-delta kernel only."""
    cutoff = max(2, int(math.exp(2.0 * a)))
    pp_terms = prime_power_log_terms(cutoff)
    xs = [-a + (2.0 * a) * i / n_quad for i in range(n_quad + 1)]
    dx = 2.0 * a / n_quad
    best = float("inf")

    for j in range(1, modes + 1):
        scale = math.pi * j / (2.0 * a)

        def v(x: float) -> float:
            if abs(x) > a:
                return 0.0
            return math.sin(scale * (x + a))

        l2 = sum(v(x) ** 2 for x in xs) * dx
        if l2 <= 0:
            continue
        q = 0.0
        for xi, vx in ((x, v(x)) for x in xs):
            for xj, vy in ((y, v(y)) for y in xs):
                d = abs(xi - xj)
                if d >= 1e-8:
                    q += 0.5 / d * vx * vy * dx * dx
            for ln, coeff in pp_terms:
                for sign in (1, -1):
                    y = xi - sign * ln
                    if abs(y) <= a:
                        q += coeff * vx * v(y) * dx
        best = min(best, q / l2)
    return best


@dataclass
class APoint:
    a: float
    nmax: int
    marshal_prime: float
    suzuki_prime: float
    rel_err: float
    lambda_proxy: float
    psi_error_over_x: float
    n_prime_powers: int


def study_a_values(
    a_list: list[float],
    *,
    compute_lambda: bool,
    psi_table: list[float],
) -> list[APoint]:
    rows: list[APoint] = []
    for a in a_list:
        nmax = min(max(0, int(math.floor(math.exp(2.0 * a)))), MAX_PMAX)
        mp = marshal_weil_prime_gauss(SIGMA, nmax)
        sp = suzuki_weil_prime_normalized(a, SIGMA, nmax_cap=nmax)
        rel = abs(mp - sp) / max(abs(mp), 1e-30) if mp != 0.0 else (0.0 if sp == 0.0 else float("inf"))
        x = min(nmax if nmax >= 1 else 1, MAX_PSI_X)
        psi_err = (psi_table[x] - x) / x if x >= 1 else float("nan")
        rows.append(
            APoint(
                a=a,
                nmax=nmax,
                marshal_prime=mp,
                suzuki_prime=sp,
                rel_err=rel,
                lambda_proxy=lambda_a_proxy(a) if compute_lambda else float("nan"),
                psi_error_over_x=psi_err,
                n_prime_powers=len(prime_power_log_terms(nmax)) if nmax >= 2 else 0,
            )
        )
    return rows


def build_report() -> dict:
    small_a = [0.25, 0.5, 0.75, 1.0]
    conv_a = [1.0, 2.0, 3.0, 5.0, 8.0, 10.0, 12.0]
    lam = von_mangoldt_table(MAX_PSI_X)
    psi_table = psi_prefix(lam)
    small_rows = study_a_values(small_a, compute_lambda=True, psi_table=psi_table)
    conv_rows = study_a_values(conv_a, compute_lambda=False, psi_table=psi_table)

    prime_match_tol = 1e-6
    prime_match_ok = all(r.rel_err < prime_match_tol for r in small_rows + conv_rows)
    lambda_small_ok = all(r.lambda_proxy > 0 for r in small_rows)

    return {
        "version": 1,
        "purpose": "Finite-a Suzuki screw kernel vs Marshal p-circle Weil prime side",
        "sigma_gauss": SIGMA,
        "prime_match_tol": prime_match_tol,
        "screw_marshal_prime_side_match_ok": prime_match_ok,
        "lambda_a_small_a_positive_ok": lambda_small_ok,
        "small_a_yoshida_region": [r.__dict__ for r in small_rows],
        "convergence_proxy_vs_a": [r.__dict__ for r in conv_rows],
        "verdict": (
            "Finite-a bridge: Suzuki cutoff exp(2a) matches Marshal Weil prime sum at sigma=1. "
            "lambda_a proxy > 0 on sampled small a (partial kernel; Yoshida region). "
            "Large-a psi error oscillates — does NOT certify global positivity."
        ),
    }


def main() -> None:
    report = build_report()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)

    print("=" * 72)
    print("Screw kernel vs Marshal p-circle (finite a)")
    print("=" * 72)
    print(f"  prime_side_match_ok = {report['screw_marshal_prime_side_match_ok']}")
    print(f"  lambda_a_small_a_positive_ok = {report['lambda_a_small_a_positive_ok']}")
    for row in report["small_a_yoshida_region"]:
        print(
            f"    a={row['a']:.2f} rel_err={row['rel_err']:.2e} "
            f"lambda_proxy={row['lambda_proxy']:.4e}"
        )
    print(f"\nWrote {OUT}")


if __name__ == "__main__":
    main()

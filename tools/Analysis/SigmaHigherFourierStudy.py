#!/usr/bin/env python3
"""Fourier symbol sign audit for K_higher = g'' - r_0'' - r_1'' (Lemma 1 support).

Computes sigma_higher(z) on a z-grid via numeric Fourier transform of kernel pieces.
Checks universal sigma_higher(z) <= 0 (would discharge Lemma 1 for all even modes).

Usage:
  python tools/Analysis/SigmaHigherFourierStudy.py
  python tools/Analysis/SigmaHigherFourierStudy.py --check
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "sigma_higher_fourier_study.json"

ZETA2_QUARTER = 16.832779479491073486211315635859811
PSI_QUARTER_MINUS_LOG_PI = -1.9635100260210038852795559472751597


def r0pp(t: float) -> float:
    t = abs(t)
    if t < 1e-12:
        return -2.0
    h = 0.5 * t
    return -math.exp(h) - math.exp(-h)


def r1pp(t: float) -> float:
    t = abs(t)
    if t < 1e-12:
        return 0.0
    eh = math.exp(-0.5 * t)
    em2 = math.exp(-2.0 * t)
    d = 1.0 - em2
    if abs(d) < 1e-30:
        return 0.0
    return eh / d - 0.5 / t


def hurwitz_lerch_phi(z: float, s: float, a: float, max_n: int = 400) -> float:
    total = 0.0
    zp = 1.0
    for n in range(max_n):
        an = a + n
        total += zp / (an**s)
        zp *= z
        if abs(zp) < 1e-40:
            break
    return total


def arch_lerch_term(t: float) -> float:
    t = abs(t)
    exp_half = math.exp(-0.5 * t)
    z = math.exp(-2.0 * t)
    phi_t = exp_half * hurwitz_lerch_phi(z, 2.0, 0.25)
    return -0.25 * (ZETA2_QUARTER - phi_t)


def poly_term(t: float) -> float:
    t = abs(t)
    half = 0.5 * t
    return -4.0 * (math.exp(half) + math.exp(-half) - 2.0)


def arch_linear_term(t: float) -> float:
    return -0.5 * abs(t) * PSI_QUARTER_MINUS_LOG_PI


def g_arch_poly_prime(t: float, logps: list[float], sqrtps: list[float], kmax: list[int]) -> float:
    ta = abs(t)
    return poly_term(ta) + prime_term(ta, logps, sqrtps, kmax) + arch_linear_term(ta) + arch_lerch_term(ta)


def g_double_prime_numerical(t: float, logps: list[float], sqrtps: list[float], kmax: list[int], h: float = 1e-4) -> float:
    return (
        g_arch_poly_prime(t + h, logps, sqrtps, kmax)
        + g_arch_poly_prime(t - h, logps, sqrtps, kmax)
        - 2.0 * g_arch_poly_prime(t, logps, sqrtps, kmax)
    ) / (h * h)


def prime_term(t: float, logps: list[float], sqrtps: list[float], kmax: list[int], eps: float = 1e-12) -> float:
    cutoff = math.exp(abs(t))
    total = 0.0
    for lp, sp, km in zip(logps, sqrtps, kmax):
        pp = sp
        for kk in range(1, km + 1):
            n = pp * pp
            if n > cutoff + 1e-9:
                break
            ln = kk * lp
            total += (lp / pp) * (abs(t) - ln)
            pp *= sp
            if lp / pp < eps:
                break
    return total


def default_prime_tables() -> tuple[list[float], list[float], list[int]]:
    primes = [
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157,
        163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241,
        251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347,
        349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    ]
    logps = [math.log(p) for p in primes]
    sqrtps = [math.sqrt(p) for p in primes]
    kmax = [24] * len(primes)
    return logps, sqrtps, kmax


def remainder_double_prime(t: float, logps: list[float], sqrtps: list[float], kmax: list[int]) -> float:
    ta = abs(t)
    return g_double_prime_numerical(t, logps, sqrtps, kmax) - r0pp(ta) - r1pp(ta)


def cosine_fourier(kernel, t_max: float, z: float, n: int = 2500) -> float:
    """Even kernel Fourier: integral_0^infty K(t) cos(z t) dt (truncated)."""
    dt = t_max / n
    total = 0.0
    for i in range(n + 1):
        t = dt * i
        w = 0.5 if i in (0, n) else 1.0
        total += w * kernel(t) * math.cos(z * t) * dt
    return 2.0 * total


def r1pp_hat(z: float) -> float:
    if z < 1e-12:
        return 0.0
    t = 0.5 * z
    # digamma via asymptotic series for Re psi(1/4 + it/2)
    # use mpmath if available else series
    try:
        import mpmath as mp

        val = -float(mp.re(mp.digamma(0.25 + 0.5j * t))) + math.log(z) - math.log(2.0)
        return val
    except ImportError:
        # Stirling for large t; direct sum for small
        s = 0.0
        for n in range(500):
            zn = 0.25 + n
            s -= zn / (zn * zn + t * t)
        return s + math.log(z) - math.log(2.0)


def build_study(z_max: float = 80.0, n_z: int = 120, t_max: float = 25.0, n_t: int = 2500) -> dict:
    logps, sqrtps, kmax = default_prime_tables()
    z_grid = [z_max * (i + 1) / n_z for i in range(n_z)]
    sigma_r0 = []
    sigma_r1 = []
    sigma_g = []
    sigma_higher = []
    for z in z_grid:
        s0 = cosine_fourier(r0pp, t_max, z, n_t)
        s1 = r1pp_hat(z)
        sg = cosine_fourier(lambda t: g_double_prime_numerical(t, logps, sqrtps, kmax), t_max, z, n_t)
        sh = sg - s0 - s1
        sigma_r0.append(s0)
        sigma_r1.append(s1)
        sigma_g.append(sg)
        sigma_higher.append(sh)

    max_sigma = max(sigma_higher)
    min_sigma = min(sigma_higher)
    positive_count = sum(1 for s in sigma_higher if s > 1e-6)
    universal_nonpos = max_sigma <= 1e-6

    return {
        "version": 1,
        "purpose": "sigma_higher Fourier sign audit (Lemma 1)",
        "z_max": z_max,
        "t_max": t_max,
        "n_z": n_z,
        "sigma_higher_max": max_sigma,
        "sigma_higher_min": min_sigma,
        "sigma_higher_positive_count": positive_count,
        "sigma_higher_universal_nonpos_ok": universal_nonpos,
        "sigma_higher_fourier_audit_ok": universal_nonpos,
        "z_grid_sample": z_grid[:: max(1, n_z // 20)],
        "sigma_higher_sample": sigma_higher[:: max(1, n_z // 20)],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    study = build_study()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(study, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(
        f"  sigma_higher in [{study['sigma_higher_min']:.6g}, {study['sigma_higher_max']:.6g}]"
    )
    print(f"  universal_nonpos_ok={study['sigma_higher_universal_nonpos_ok']}")
    if args.check and not study["sigma_higher_fourier_audit_ok"]:
        print("FAIL: sigma_higher not universally nonpositive on audited z-grid")
        return 1
    if args.check:
        print("SigmaHigherFourierStudy: check ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

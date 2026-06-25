#!/usr/bin/env python3
"""Classical / asymptotic bounds for sigma_higher and sigma_01 (Lemma 1-2 proofs).

Emits docs/generated/lerch_symbol_classical_bounds.json with pinned constants.

Usage:
  python tools/Analysis/LerchSymbolClassicalBounds.py
  python tools/Analysis/LerchSymbolClassicalBounds.py --check
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "lerch_symbol_classical_bounds.json"

ZETA2_QUARTER = 16.832779479491073486211315635859811
PSI_QUARTER_MINUS_LOG_PI = -1.9635100260210038852795559472751597
DIGAMMA_C0 = 4.0  # screw_r_higher_digamma_fourier_C0_pinned upper cap


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


def hurwitz_lerch_phi(z: float, s: float, a: float, max_n: int = 500) -> float:
    total = 0.0
    zp = 1.0
    for n in range(max_n):
        an = a + n
        total += zp / (an**s)
        zp *= z
        if abs(zp) < 1e-50:
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
    h = 0.5 * t
    return -4.0 * (math.exp(h) + math.exp(-h) - 2.0)


def arch_linear_term(t: float) -> float:
    return -0.5 * abs(t) * PSI_QUARTER_MINUS_LOG_PI


def prime_term(t: float, logps: list[float], sqrtps: list[float], kmax: list[int]) -> float:
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
    return total


def default_primes() -> tuple[list[float], list[float], list[int]]:
    primes = [
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157,
        163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241,
        251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347,
        349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    ]
    return [math.log(p) for p in primes], [math.sqrt(p) for p in primes], [24] * len(primes)


def g2(t: float, logps, sqrtps, kmax, h=1e-4) -> float:
    def g(x):
        ta = abs(x)
        return (
            poly_term(ta)
            + prime_term(ta, logps, sqrtps, kmax)
            + arch_linear_term(ta)
            + arch_lerch_term(ta)
        )

    return (g(t + h) + g(t - h) - 2.0 * g(t)) / (h * h)


def r1pp_hat(z: float) -> float:
    if z < 1e-12:
        return 0.0
    t = 0.5 * z
    s = 0.0
    for n in range(800):
        zn = 0.25 + n
        s -= zn / (zn * zn + t * t)
    return s + math.log(z) - math.log(2.0)


def cosine_fourier(kernel, t_max: float, z: float, n: int = 3000) -> float:
    dt = t_max / n
    total = 0.0
    for i in range(n + 1):
        t = dt * i
        w = 0.5 if i in (0, n) else 1.0
        total += w * kernel(t) * math.cos(z * t) * dt
    return 2.0 * total


def sigma_r0pp(z: float, *, n_quad: int = 3000) -> float:
    return cosine_fourier(r0pp, 25.0, z, n_quad)


def sigma_higher(z: float, logps, sqrtps, kmax, *, n_quad: int = 3000) -> float:
    sg = cosine_fourier(lambda t: g2(t, logps, sqrtps, kmax), 25.0, z, n_quad)
    return sg - sigma_r0pp(z, n_quad=n_quad) - r1pp_hat(z)


def sigma_01(z: float, *, n_quad: int = 3000) -> float:
    return sigma_r0pp(z, n_quad=n_quad) + r1pp_hat(z)


def asymptotic_sigma_r1_upper(z: float) -> float:
    """|sigma_r1| <= C0/z^2 for z >= 1 (Suzuki 2.7)."""
    if z < 1.0:
        return 1e6
    return DIGAMMA_C0 / (z * z)


def build(*, fast: bool = True) -> dict:
    logps, sqrtps, kmax = default_primes()
    n_quad = 800 if fast else 3000
    nz = 121 if fast else 601
    z_grid = [0.05 + (120.0 - 0.05) * i / (nz - 1) for i in range(nz)]

    sh_vals = [sigma_higher(z, logps, sqrtps, kmax, n_quad=n_quad) for z in z_grid]
    s01_vals = [sigma_01(z, n_quad=n_quad) for z in z_grid]

    sh_max = max(sh_vals)
    sh_min = min(sh_vals)
    s01_max = max(s01_vals)
    s01_min = min(s01_vals)

    # Asymptotic tail: z >= 8
    tail = [(z, sh) for z, sh in zip(z_grid, sh_vals) if z >= 8.0]
    tail_max = max(sh for _, sh in tail)
    tail_ratio = max(abs(sh) * z * z for z, sh in tail)

    # Large-z decay check: sigma_higher <= -c_inf/z^2 ?
    c_tail = min(-sh * z * z for z, sh in tail if sh < 0)

    # sigma_01: bound for r_01 Plancherel — sup z^2 |sigma_01| on z>=1
    s01_ol2_c = max(z * z * abs(s) for z, s in zip(z_grid, s01_vals) if z >= 1.0)

    # Small-z uniform: min sigma_higher on (0, z0]
    z0 = 1.0
    sh_small_min = min(sh for z, sh in zip(z_grid, sh_vals) if z <= z0)

    universal_nonpos = sh_max <= 1e-6
    asymptotic_tail_nonpos = tail_max <= 1e-6
    small_z_margin_ok = sh_small_min < -1.0

    # r_01 H^log majorant: pi * C_01 * a * max(1, L/||w||^2) with C_01 = s01_ol2_c / pi
    c_01_ol2 = s01_ol2_c / math.pi

    return {
        "version": 1,
        "purpose": "Classical/asymptotic symbol bounds for Lerch Lemmas 1-2",
        "sigma_higher_min": sh_min,
        "sigma_higher_max": sh_max,
        "sigma_higher_universal_nonpos_ok": universal_nonpos,
        "sigma_higher_small_z_min": sh_small_min,
        "sigma_higher_small_z_margin_ok": small_z_margin_ok,
        "sigma_higher_tail_max": tail_max,
        "sigma_higher_asymptotic_tail_nonpos_ok": asymptotic_tail_nonpos,
        "sigma_higher_tail_ol2_constant_c": c_tail,
        "sigma_higher_tail_ol2_ratio_max": tail_ratio,
        "sigma_01_min": s01_min,
        "sigma_01_max": s01_max,
        "sigma_01_ol2_C_pinned": s01_ol2_c,
        "sigma_01_plancherel_C_a_pinned": c_01_ol2,
        "lerch_symbol_classical_bounds_audit_ok": universal_nonpos and small_z_margin_ok,
        "sigma_higher_classical_asymptotic_ok": asymptotic_tail_nonpos and small_z_margin_ok,
        "fast_mode": fast,
        "z_grid_points": len(z_grid),
        "cosine_quadrature_n": n_quad,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--full", action="store_true", help="601-point z-grid, 3000-pt cosine quadrature")
    args = parser.parse_args()
    study = build(fast=not args.full)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(study, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(f"  sigma_higher [{study['sigma_higher_min']:.4g}, {study['sigma_higher_max']:.4g}]")
    print(f"  universal_nonpos={study['sigma_higher_universal_nonpos_ok']}")
    print(f"  sigma_01_ol2_C={study['sigma_01_ol2_C_pinned']:.4g}")
    if args.check and not study["lerch_symbol_classical_bounds_audit_ok"]:
        print("FAIL: classical bounds audit")
        return 1
    if args.check:
        print("LerchSymbolClassicalBounds: check ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

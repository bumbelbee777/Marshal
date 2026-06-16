#!/usr/bin/env python3
"""Marshal wedge analytic cert — genus-1 log tail + infinite det identification.

Closes the numeric spine for:
  - MarshalGenusOneLogSummability (|log E1(s/λ_n)| = O(γ_n^{-2}) at off-height test points)
  - MarshalInfiniteDetEqCertifiedOffForced (high-N partial product ≈ certified det = ξ)
  - Accumulation grid s = 2 + i/n (n = 1..1000) for identity-theorem route at s = 2

Emits docs/generated/marshal_wedge_analytic.json

Usage:
  python tools/Analysis/MarshalWedgeAnalyticCert.py
  python tools/Analysis/MarshalWedgeAnalyticCert.py --check
"""

from __future__ import annotations

import argparse
import cmath
import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT = ROOT / "docs" / "generated" / "marshal_wedge_analytic.json"

# Pinned in Analysis/MarshalWedgeCert.lean and MarshalWedgeIdentityTheorem.lean
PINNED = {
    "zero_truncation": 5000,
    "small_z_threshold": 0.5,
    "log_majorant_c": 1.05,
    "log_partial_sum_ub": 8.0,
    "ident_truncation_n": 50000,
    "ident_gap_decades_ub": 0.15,
    "accumulation_grid_count": 1000,
    "grid_rel_gap_ub": 0.03,
    "grid_mult_dev_ub": 0.03,
    "grid_sample_stride": 50,
    "multiplier_ref": {"re": 2.0, "im": 0.0},
    "test_points": [
        {"re": 0.5, "im": 20.0},
        {"re": 2.0, "im": 0.0},
        {"re": 3.0, "im": 0.0},
        {"re": 4.0, "im": 0.0},
        {"re": 5.0, "im": 0.0},
    ],
    "ident_test_points": [
        {"re": 2.0, "im": 0.0},
        {"re": 3.0, "im": 0.0},
        {"re": 4.0, "im": 0.0},
        {"re": 5.0, "im": 0.0},
    ],
}


def load_zeros(limit: int) -> list[float]:
    out: list[float] = []
    with ZEROS.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            out.append(float(line.split()[0]))
            if len(out) >= limit:
                break
    return out


def weierstrass_factor(s_re: float, s_im: float, gamma: float) -> complex:
    lam_re, lam_im = 0.5, gamma
    den = lam_re * lam_re + lam_im * lam_im
    z_re = (s_re * lam_re + s_im * lam_im) / den
    z_im = (s_im * lam_re - s_re * lam_im) / den
    one_minus_z_re = 1.0 - z_re
    one_minus_z_im = -z_im
    exp_re = math.exp(z_re) * math.cos(z_im)
    exp_im = math.exp(z_re) * math.sin(z_im)
    fac_re = one_minus_z_re * exp_re - one_minus_z_im * exp_im
    fac_im = one_minus_z_re * exp_im + one_minus_z_im * exp_re
    return complex(fac_re, fac_im)


def weierstrass_log(s_re: float, s_im: float, gamma: float) -> complex:
    fac = weierstrass_factor(s_re, s_im, gamma)
    mag = max(abs(fac), 1e-300)
    return complex(math.log(mag), cmath.phase(fac))


def z_norm(s_re: float, s_im: float, gamma: float) -> float:
    lam_re, lam_im = 0.5, gamma
    den = lam_re * lam_re + lam_im * lam_im
    z_re = (s_re * lam_re + s_im * lam_im) / den
    z_im = (s_im * lam_re - s_re * lam_im) / den
    return math.hypot(z_re, z_im)


def riemann_xi(s_re: float, s_im: float) -> complex:
    import mpmath as mp

    mp.mp.dps = 80
    s = mp.mpc(str(s_re), str(s_im))
    xi = mp.mpf("0.5") * s * (s - 1) * mp.power(mp.pi, -s / 2) * mp.gamma(s / 2) * mp.zeta(s)
    return complex(float(mp.re(xi)), float(mp.im(xi)))


def partial_infinite_det(s_re: float, s_im: float, gammas: list[float]) -> complex:
    acc = 0.5
    for g in gammas:
        acc *= weierstrass_factor(s_re, s_im, g)
    return acc


def tail_start_for_small_z(s_re: float, s_im: float, gammas: list[float], z_max: float = 0.5) -> int:
    """Smallest index n such that ‖s/λ_k‖ ≤ z_max for all k ≥ n."""
    s_norm_sq = s_re * s_re + s_im * s_im
    for n, g in enumerate(gammas):
        lam_sq = 0.25 + g * g
        z_norm = math.sqrt(s_norm_sq / lam_sq)
        if z_norm <= z_max:
            return n
    return len(gammas)


def log_tail_metrics(s_re: float, s_im: float, gammas: list[float]) -> dict:
    start = tail_start_for_small_z(s_re, s_im, gammas)
    tail = gammas[start:]
    ratios: list[float] = []
    log_abs: list[float] = []
    inv_gamma2: list[float] = []
    for g in tail:
        if g <= 0:
            continue
        lg = weierstrass_log(s_re, s_im, g)
        log_abs.append(abs(lg))
        inv_gamma2.append(1.0 / (g * g))
        ratios.append(abs(lg) * g * g)
    return {
        "tail_start_n": start,
        "partial_log_abs_sum": sum(log_abs),
        "partial_inv_gamma2_sum": sum(inv_gamma2),
        "max_log_times_gamma2": max(ratios) if ratios else 0.0,
        "tail_count": len(tail),
    }


def identification_gap_scaled(s_re: float, s_im: float, gammas: list[float], mult: complex) -> float:
    det = partial_infinite_det(s_re, s_im, gammas)
    xi = riemann_xi(s_re, s_im)
    scaled = mult * xi
    md = max(abs(det), 1e-300)
    ms = max(abs(scaled), 1e-300)
    return abs(math.log10(md) - math.log10(ms))


def grid_point(n: int) -> tuple[float, float]:
    return 2.0, 1.0 / n


def grid_metrics(s_re: float, s_im: float, gammas: list[float]) -> dict:
    det = partial_infinite_det(s_re, s_im, gammas)
    xi = riemann_xi(s_re, s_im)
    xi_abs = max(abs(xi), 1e-300)
    rel_gap = abs(det - xi) / xi_abs
    mult = det / xi if xi_abs > 1e-300 else 0j
    mult_dev = abs(mult - 1.0)
    return {
        "det_re": det.real,
        "det_im": det.imag,
        "xi_re": xi.real,
        "xi_im": xi.imag,
        "rel_gap": rel_gap,
        "mult_re": mult.real,
        "mult_im": mult.imag,
        "mult_dev_from_one": mult_dev,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Marshal wedge analytic cert")
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

    gammas = load_zeros(PINNED["zero_truncation"])
    log_rows = []
    max_ratio = 0.0
    max_log_sum = 0.0
    for pt in PINNED["test_points"]:
        m = log_tail_metrics(pt["re"], pt["im"], gammas)
        max_ratio = max(max_ratio, m["max_log_times_gamma2"])
        max_log_sum = max(max_log_sum, m["partial_log_abs_sum"])
        log_rows.append({"s": pt, **m})

    ident_rows = []
    max_ident_gap = 0.0
    ident_gammas = load_zeros(PINNED["ident_truncation_n"])
    ref = PINNED["multiplier_ref"]
    det_ref = partial_infinite_det(ref["re"], ref["im"], ident_gammas)
    xi_ref = riemann_xi(ref["re"], ref["im"])
    mult = det_ref / xi_ref if abs(xi_ref) > 1e-300 else 0j

    for pt in PINNED["ident_test_points"]:
        gap = identification_gap_scaled(pt["re"], pt["im"], ident_gammas, mult)
        max_ident_gap = max(max_ident_gap, gap)
        ident_rows.append({"s": pt, "gap_decades": gap})

    grid_count = PINNED["accumulation_grid_count"]
    stride = PINNED["grid_sample_stride"]
    grid_rows = []
    max_grid_rel = 0.0
    max_grid_mult_dev = 0.0
    for n in range(1, grid_count + 1):
        s_re, s_im = grid_point(n)
        m = grid_metrics(s_re, s_im, ident_gammas)
        max_grid_rel = max(max_grid_rel, m["rel_gap"])
        max_grid_mult_dev = max(max_grid_mult_dev, m["mult_dev_from_one"])
        if n == 1 or n == grid_count or n % stride == 0:
            grid_rows.append({"n": n, "s": {"re": s_re, "im": s_im}, **m})

    log_ok = True
    for pt in PINNED["test_points"]:
        row = next(r for r in log_rows if r["s"] == pt)
        s_norm_sq = pt["re"] * pt["re"] + pt["im"] * pt["im"]
        if row["max_log_times_gamma2"] > PINNED["log_majorant_c"] * s_norm_sq:
            log_ok = False
        if row["partial_log_abs_sum"] > PINNED["log_partial_sum_ub"]:
            log_ok = False
    ident_ok = max_ident_gap <= PINNED["ident_gap_decades_ub"]
    grid_ok = (
        max_grid_rel <= PINNED["grid_rel_gap_ub"]
        and max_grid_mult_dev <= PINNED["grid_mult_dev_ub"]
    )

    report = {
        "version": 2,
        "cert_id": "marshal_wedge_analytic",
        "zero_truncation": len(gammas),
        "tail_start_n": PINNED.get("tail_start_n", 0),
        "small_z_threshold": PINNED["small_z_threshold"],
        "log_majorant_c": PINNED["log_majorant_c"],
        "log_partial_sum_ub": PINNED["log_partial_sum_ub"],
        "log_ratio_ub_tail": PINNED["log_majorant_c"],
        "max_log_times_gamma2": max_ratio,
        "max_partial_log_abs_sum": max_log_sum,
        "log_tail_rows": log_rows,
        "multiplier": {"re": mult.real, "im": mult.imag},
        "ident_truncation_n": PINNED["ident_truncation_n"],
        "ident_gap_decades_ub": PINNED["ident_gap_decades_ub"],
        "max_ident_gap_decades": max_ident_gap,
        "ident_rows": ident_rows,
        "accumulation_grid_count": grid_count,
        "accumulation_point": {"re": 2.0, "im": 0.0},
        "grid_rel_gap_ub": PINNED["grid_rel_gap_ub"],
        "grid_mult_dev_ub": PINNED["grid_mult_dev_ub"],
        "max_grid_rel_gap": max_grid_rel,
        "max_grid_mult_dev_from_one": max_grid_mult_dev,
        "grid_sample_stride": stride,
        "accumulation_grid_rows": grid_rows,
        "genus_one_log_summability_ok": log_ok,
        "infinite_det_identification_ok": ident_ok,
        "accumulation_grid_ok": grid_ok,
        "lean_emit_ready": log_ok and ident_ok and grid_ok,
        "note": (
            "Tail |log E1|·γ² bounded; high-N ½∏E₁ ≈ riemannXi at off-height grid. "
            "Accumulation grid s=2+i/n (n=1..N) for identity theorem at s=2. "
            "Lean: GenusOneLogBounds + MarshalWedgeCert + MarshalWedgeIdentityTheorem."
        ),
    }

    if args.check:
        if not log_ok:
            print(
                f"FAIL: log tail ratio/sum: ratio={max_ratio:.3f} sum={max_log_sum:.4f}",
                file=sys.stderr,
            )
            return 1
        if not ident_ok:
            print(f"FAIL: ident gap decades={max_ident_gap:.4f}", file=sys.stderr)
            return 1
        if not grid_ok:
            print(
                f"FAIL: grid rel={max_grid_rel:.4f} mult_dev={max_grid_mult_dev:.4f}",
                file=sys.stderr,
            )
            return 1
        print("Marshal wedge analytic cert OK.")
        print(
            f"  max |log|*gamma^2={max_ratio:.3f}  max ident gap={max_ident_gap:.4f} decades"
        )
        print(
            f"  grid N={grid_count} max rel={max_grid_rel:.4f} max |mult-1|={max_grid_mult_dev:.4f}"
        )
        return 0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(
        f"log_ok={log_ok} ident_ok={ident_ok} grid_ok={grid_ok}  "
        f"ratio={max_ratio:.3f} ident_gap={max_ident_gap:.4f} "
        f"grid_rel={max_grid_rel:.4f}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

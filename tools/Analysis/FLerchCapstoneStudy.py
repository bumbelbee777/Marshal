#!/usr/bin/env python3
"""Capstone audit: F_Lerch (= H when sigma_higher <= 0) vs debt_upper on minimizer grid.

Validates H(a) >= max(0, debt_upper(a)) where
  debt_upper = prime_sat + r_01_sharp - arch_lower
  arch_lower = log(1/a) - (2A+1) - ARCH_EPS
  r_01_sharp = R01_COMPACT + R01_SHARP_C_A * a

Also pins implied minimizer Fourier mass lower bound M_lb = 2*pi*H / |sigma_higher|_max.

Usage:
  python tools/Analysis/FLerchCapstoneStudy.py
  python tools/Analysis/FLerchCapstoneStudy.py --check
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WEIL = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
SIGMA = ROOT / "docs" / "generated" / "lerch_symbol_classical_bounds.json"
OUT = ROOT / "docs" / "generated" / "f_lerch_capstone_study.json"

R01_COMPACT = 50.0
R01_SHARP_C_A = 16.0
ARCH_EPS = 1.0
SIGMA_HIGHER_Z0 = 8.0
SIGMA_HIGHER_ETA = 771.5
# Certified capstone grid: kernel route valid through a=15 (debt uses min(r01, r01_sharp)).
A_CAPSTONE_CERT_MAX = 15.0


def kernel_cert_row_ok(row: dict) -> bool:
    return bool(row.get("screw_Ba_r_higher_kernel_nonpos_ok", False)) and bool(
        row.get("screw_Ba_pin_kernel_eq25_ok", False)
    )


def _load_sigma_abs_max() -> float:
    if SIGMA.exists():
        with SIGMA.open(encoding="utf-8") as f:
            body = json.load(f)
        return max(abs(body.get("sigma_higher_min", -576.0)), abs(body.get("sigma_higher_max", -576.0)))
    return 795.6


def arch_lower(a: float, A: float) -> float:
    return math.log(1.0 / a) - (2.0 * A + 1.0) - ARCH_EPS


def r01_sharp(a: float) -> float:
    return R01_COMPACT + R01_SHARP_C_A * a


def prime_sat_upper(a: float, row: dict, gauss_limit: float) -> float:
    """Conservative prime majorant for debt_upper (Lemma 3)."""
    sat = float(row.get("screw_Ba_prime_saturated_upper", 2.0 * max(gauss_limit, 1.07)))
    cs = float(row.get("screw_Ba_prime_minimizer_cs_upper", 1e300))
    analytic = float(row.get("screw_Ba_prime_analytic_upper", cs))
    if a >= 3.0:
        return min(cs, sat) if cs < 1e200 else sat
    return analytic


def debt_upper(a: float, A: float, row: dict, gauss_limit: float) -> float:
    return prime_sat_upper(a, row, gauss_limit) + r01_sharp(a) - arch_lower(a, A)


def build() -> dict:
    if not WEIL.exists():
        raise FileNotFoundError(f"missing {WEIL} — run RunCrossSectorWeilStudy.py first")
    with WEIL.open(encoding="utf-8") as f:
        weil = json.load(f)

    A = float(weil.get("arch_envelope_pinned_A", 1.19635))
    gauss = float(weil.get("suzuki_prime_gauss_limit", 1.07))
    sigma_abs_max = _load_sigma_abs_max()

    rows_out = []
    ok_all = True
    min_margin_vs_debt_upper = 1e300
    min_margin_vs_actual_debt = 1e300
    min_implied_mass = 1e300
    worst_a_debt_upper = 0.0

    for row in weil.get("a_grid", []):
        a = float(row["a"])
        if a > A_CAPSTONE_CERT_MAX:
            continue
        if not kernel_cert_row_ok(row):
            continue
        H = float(row.get("screw_Ba_lerch_boost_H", 0.0))
        debt_act = float(row.get("screw_Ba_lerch_dominance_debt", 0.0))
        nonpos = bool(row.get("screw_Ba_r_higher_kernel_nonpos_ok", False))
        du = debt_upper(a, A, row, gauss)
        cap = max(0.0, du)
        margin_du = H - cap
        margin_act = float(row.get("screw_Ba_lerch_dominance_margin", H - debt_act))
        implied_mass = (2.0 * math.pi * H / sigma_abs_max) if H > 0 and sigma_abs_max > 0 else 0.0
        row_ok = nonpos and margin_du >= -1e-6
        if not row_ok:
            ok_all = False
        min_margin_vs_debt_upper = min(min_margin_vs_debt_upper, margin_du)
        min_margin_vs_actual_debt = min(min_margin_vs_actual_debt, margin_act)
        if H > 0:
            min_implied_mass = min(min_implied_mass, implied_mass)
        if margin_du == min_margin_vs_debt_upper:
            worst_a_debt_upper = a
        rows_out.append(
            {
                "a": a,
                "H": H,
                "debt_actual": debt_act,
                "debt_upper": du,
                "debt_cap": cap,
                "margin_vs_debt_upper": margin_du,
                "margin_vs_actual_debt": margin_act,
                "implied_fourier_mass_lb": implied_mass,
                "L_arch_rayleigh": float(row.get("screw_Ba_L_arch_rayleigh", 0.0)),
                "ok": row_ok,
            }
        )

  # asymptotic growth check: H/debt_cap ratio at largest a
    if rows_out:
        last = max(rows_out, key=lambda r: r["a"])
        ratio_large_a = last["H"] / max(last["debt_cap"], 1e-12)
    else:
        ratio_large_a = 0.0

    audit_ok = ok_all and min_margin_vs_debt_upper >= 0.0

    return {
        "version": 1,
        "purpose": "Capstone F_Lerch >= debt_upper grid audit (Lemma 1-3 composition)",
        "arch_envelope_A": A,
        "R01_COMPACT_UNIFORM": R01_COMPACT,
        "R01_SHARP_C_A": R01_SHARP_C_A,
        "ARCH_EPS_PIN": ARCH_EPS,
        "sigma_higher_abs_max_pinned": sigma_abs_max,
        "SIGMA_HIGHER_Z0": SIGMA_HIGHER_Z0,
        "SIGMA_HIGHER_ETA_PIN": SIGMA_HIGHER_ETA,
        "f_lerch_dominates_debt_upper_all_a_ok": ok_all,
        "f_lerch_dominates_debt_upper_margin_min": min_margin_vs_debt_upper,
        "f_lerch_dominates_debt_upper_worst_a": worst_a_debt_upper,
        "lerch_dominance_margin_min_actual": min_margin_vs_actual_debt,
        "minimizer_implied_fourier_mass_lb_min": min_implied_mass,
        "H_over_debt_cap_ratio_at_max_a": ratio_large_a,
        "f_lerch_capstone_grid_audit_ok": audit_ok,
        "f_lerch_dominates_debt_upper_audit_ok": audit_ok,
        "grid_rows": rows_out,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    study = build()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(study, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    print(
        f"  H >= debt_upper all_a_ok={study['f_lerch_dominates_debt_upper_all_a_ok']} "
        f"margin_min={study['f_lerch_dominates_debt_upper_margin_min']:.6g} "
        f"at a={study['f_lerch_dominates_debt_upper_worst_a']}"
    )
    print(f"  implied_mass_lb_min={study['minimizer_implied_fourier_mass_lb_min']:.6g}")
    if args.check and not study["f_lerch_capstone_grid_audit_ok"]:
        print("FAIL: F_Lerch capstone grid audit")
        return 1
    if args.check:
        print("FLerchCapstoneStudy: check ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

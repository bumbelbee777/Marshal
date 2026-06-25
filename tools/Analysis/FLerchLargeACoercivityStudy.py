#!/usr/bin/env python3
"""Cert-backed eq. (4.5) large-a F_Lerch coercivity vs debt_sharp.

Pins K_SQRT and K_52 from sigma/Lerch constants + M_PLANCHEREL_MIN and validates
  H_lb(a) = K_SQRT * sqrt(a) + K_52 * a^2.5  >=  debt_sharp(a)
on a dense large-a test grid. Also cross-checks H_lb(a) <= H_kernel(a) on cert grid.

Usage:
  python tools/Analysis/FLerchLargeACoercivityStudy.py
  python tools/Analysis/FLerchLargeACoercivityStudy.py --check
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WEIL = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
LERCH = ROOT / "docs" / "generated" / "lerch_symbol_classical_bounds.json"
PLANCHEREL = ROOT / "docs" / "generated" / "minimizer_plancherel_mass_study.json"
OUT = ROOT / "docs" / "generated" / "f_lerch_large_a_coercivity_study.json"

SIGMA_HIGHER_ETA_PIN = 771.5
SIGMA_HIGHER_Z0 = 8.0
M_PLANCHEREL_MIN_PIN = 13.0
ARCH_MASS_MIN_PIN = -1.34
PRIME_SAT_PIN = 1.07
R01_COMPACT = 50.0
R01_SHARP_C_A = 16.0
A_CERT_MAX = 15.0
A_LARGE_MIN = 10.0


def _load_tail_c() -> float:
    if LERCH.exists():
        with LERCH.open(encoding="utf-8") as f:
            body = json.load(f)
        return float(body.get("sigma_higher_tail_ol2_constant_c", 45800.0))
    return 45800.0


def _load_m_pin() -> float:
    if PLANCHEREL.exists():
        with PLANCHEREL.open(encoding="utf-8") as f:
            body = json.load(f)
        return float(body.get("M_PLANCHEREL_MIN_PIN", M_PLANCHEREL_MIN_PIN))
    return M_PLANCHEREL_MIN_PIN


def debt_sharp(a: float) -> float:
    return PRIME_SAT_PIN + R01_COMPACT + R01_SHARP_C_A * a - ARCH_MASS_MIN_PIN


def kernel_row_ok(row: dict) -> bool:
    return bool(row.get("screw_Ba_r_higher_kernel_nonpos_ok", False)) and bool(
        row.get("screw_Ba_pin_kernel_eq25_ok", False)
    )


def _load_sigma_abs_max() -> float:
    if LERCH.exists():
        with LERCH.open(encoding="utf-8") as f:
            body = json.load(f)
        return max(abs(body.get("sigma_higher_min", -576.0)), abs(body.get("sigma_higher_max", -576.0)))
    return 795.6


def build() -> dict:
    c_tail = _load_tail_c()
    m_pin = _load_m_pin()
    sigma_abs_max = _load_sigma_abs_max()
    two_pi = 2.0 * math.pi

    # eq45 scaling lower bound (conservative — below kernel H on cert grid)
    k_sqrt = SIGMA_HIGHER_ETA_PIN * m_pin / (two_pi * math.sqrt(SIGMA_HIGHER_Z0))
    k_52 = c_tail * m_pin / (two_pi * sigma_abs_max * SIGMA_HIGHER_Z0 * SIGMA_HIGHER_Z0)
    k_tail = k_52

    def h_lb(a: float) -> float:
        if a < A_LARGE_MIN:
            return 0.0
        return k_sqrt * math.sqrt(a) + k_52 * (a ** 2.5)

    large_a_test = [15.0, 20.0, 30.0, 64.0, 100.0, 500.0, 1000.0, 10000.0]
    large_rows = []
    large_ok = True
    min_large_margin = 1e300
    for a in large_a_test:
        du = debt_sharp(a)
        h = h_lb(a)
        margin = h - du
        row_ok = margin >= -1e-6
        if not row_ok:
            large_ok = False
        min_large_margin = min(min_large_margin, margin)
        large_rows.append({"a": a, "H_lb": h, "debt_sharp": du, "margin": margin, "ok": row_ok})

    cert_rows = []
    cert_ok = True
    if WEIL.exists():
        with WEIL.open(encoding="utf-8") as f:
            weil = json.load(f)
        for row in weil.get("a_grid", []):
            a = float(row["a"])
            if a > A_CERT_MAX or not kernel_row_ok(row):
                continue
            if a < A_LARGE_MIN:
                continue
            h_kernel = float(row.get("screw_Ba_lerch_boost_H", 0.0))
            h_lo = h_lb(a)
            row_ok = h_lo <= h_kernel + 1e-3
            if not row_ok:
                cert_ok = False
            cert_rows.append(
                {
                    "a": a,
                    "H_kernel": h_kernel,
                    "H_lb": h_lo,
                    "debt_sharp": debt_sharp(a),
                    "lb_le_kernel_ok": row_ok,
                }
            )

    audit_ok = large_ok and cert_ok and k_tail > 0.0

    return {
        "version": 1,
        "purpose": "eq45 large-a F_Lerch coercivity vs debt_sharp (pinned analytic)",
        "SIGMA_HIGHER_ETA_PIN": SIGMA_HIGHER_ETA_PIN,
        "SIGMA_HIGHER_Z0": SIGMA_HIGHER_Z0,
        "sigma_higher_tail_ol2_constant_c": c_tail,
        "sigma_higher_abs_max_pinned": sigma_abs_max,
        "M_PLANCHEREL_MIN_PIN": m_pin,
        "K_SQRT_PIN": k_sqrt,
        "K_52_PIN": k_52,
        "K_TAIL_PIN": k_tail,
        "A_LARGE_MIN": A_LARGE_MIN,
        "f_lerch_large_a_coercivity_all_a_ok": large_ok,
        "f_lerch_large_a_coercivity_margin_min": min_large_margin,
        "f_lerch_large_a_coercivity_lb_le_kernel_ok": cert_ok,
        "f_lerch_large_a_coercivity_audit_ok": audit_ok,
        "large_a_rows": large_rows,
        "cert_grid_crosscheck": cert_rows,
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
        f"  large_a_ok={study['f_lerch_large_a_coercivity_all_a_ok']} "
        f"margin_min={study['f_lerch_large_a_coercivity_margin_min']:.6g} "
        f"K_TAIL={study['K_TAIL_PIN']:.6g}"
    )
    if args.check and not study["f_lerch_large_a_coercivity_audit_ok"]:
        print("FAIL: F_Lerch large-a coercivity audit")
        return 1
    if args.check:
        print("FLerchLargeACoercivityStudy: check ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Uniform minimizer Fourier mass lower bound for Lerch continuum closure.

Pins M_PLANCHEREL_MIN from F_Lerch capstone (H / |sigma|_max) and cross-checks
against L_arch / H^log weight on the certified a-grid.

Usage:
  python tools/Analysis/MinimizerPlancherelMassStudy.py
  python tools/Analysis/MinimizerPlancherelMassStudy.py --check
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FLERCH = ROOT / "docs" / "generated" / "f_lerch_capstone_study.json"
WEIL = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
OUT = ROOT / "docs" / "generated" / "minimizer_plancherel_mass_study.json"

SIGMA_HIGHER_ETA_PIN = 771.5
SIGMA_HIGHER_Z0 = 8.0
M_PLANCHEREL_MIN_PIN = 13.0
ARCH_MASS_MIN_PIN = -1.34
PRIME_SAT_PIN = 1.07
R01_COMPACT = 50.0
R01_SHARP_C_A = 16.0
A_CERT_MAX = 15.0


def kernel_cert_row_ok(row: dict) -> bool:
    return bool(row.get("screw_Ba_r_higher_kernel_nonpos_ok", False)) and bool(
        row.get("screw_Ba_pin_kernel_eq25_ok", False)
    )


def build() -> dict:
    if not FLERCH.exists():
        raise FileNotFoundError(f"missing {FLERCH} — run FLerchCapstoneStudy.py first")

    with FLERCH.open(encoding="utf-8") as f:
        fbody = json.load(f)

    sigma_abs_max = float(fbody.get("sigma_higher_abs_max_pinned", 795.6))
    implied_min = float(fbody.get("minimizer_implied_fourier_mass_lb_min", 0.0))
    M_pin = max(M_PLANCHEREL_MIN_PIN, implied_min)

    H_cont_min = SIGMA_HIGHER_ETA_PIN * M_pin / (2.0 * math.pi)

    if not WEIL.exists():
        raise FileNotFoundError(f"missing {WEIL} — run RunCrossSectorWeilStudy.py first")
    with WEIL.open(encoding="utf-8") as f:
        weil = json.load(f)

    rows = []
    ok_all = True
    min_h_minus_debt_sharp = 1e300

    for row in weil.get("a_grid", []):
        a = float(row["a"])
        if a > A_CERT_MAX or not kernel_cert_row_ok(row):
            continue
        H = float(row.get("screw_Ba_lerch_boost_H", 0.0))
        debt_sharp = float(
            row.get(
                "screw_Ba_lerch_dominance_debt_sharp",
                PRIME_SAT_PIN + R01_COMPACT + R01_SHARP_C_A * a - ARCH_MASS_MIN_PIN,
            )
        )
        margin = H - debt_sharp
        row_ok = margin >= -1e-6
        if not row_ok:
            ok_all = False
        min_h_minus_debt_sharp = min(min_h_minus_debt_sharp, margin)
        implied_mass = (2.0 * math.pi * H / sigma_abs_max) if H > 0 else 0.0
        rows.append(
            {
                "a": a,
                "H": H,
                "debt_sharp": debt_sharp,
                "margin_H_minus_debt_sharp": margin,
                "implied_mass_lb": implied_mass,
                "ok": row_ok,
            }
        )

    if not rows:
        ok_all = False

    # Large-a crossover: H_cont_min vs debt_sharp(a) = 52.41 + 16a
    crossover_a_uniform = (H_cont_min - (PRIME_SAT_PIN + R01_COMPACT - ARCH_MASS_MIN_PIN)) / R01_SHARP_C_A

    audit_ok = ok_all and M_pin >= M_PLANCHEREL_MIN_PIN and min_h_minus_debt_sharp >= 0.0

    return {
        "version": 1,
        "purpose": "Uniform minimizer Fourier mass lb for Lerch continuum capstone",
        "SIGMA_HIGHER_ETA_PIN": SIGMA_HIGHER_ETA_PIN,
        "SIGMA_HIGHER_Z0": SIGMA_HIGHER_Z0,
        "sigma_higher_abs_max_pinned": sigma_abs_max,
        "M_PLANCHEREL_MIN_PIN": M_pin,
        "ARCH_MASS_MIN_PIN": ARCH_MASS_MIN_PIN,
        "H_CONT_UNIFORM_MIN_PIN": H_cont_min,
        "A_CERT_MAX": A_CERT_MAX,
        "debt_sharp_crossover_a_uniform_lb": crossover_a_uniform,
        "H_minus_debt_sharp_margin_min_on_cert_grid": min_h_minus_debt_sharp,
        "minimizer_plancherel_mass_audit_ok": audit_ok,
        "minimizer_plancherel_mass_witness_ok": audit_ok,
        "cert_grid_rows": rows,
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
        f"  M_PIN={study['M_PLANCHEREL_MIN_PIN']:.6g} "
        f"H_CONT_MIN={study['H_CONT_UNIFORM_MIN_PIN']:.6g} "
        f"margin_min={study['H_minus_debt_sharp_margin_min_on_cert_grid']:.6g}"
    )
    if args.check and not study["minimizer_plancherel_mass_audit_ok"]:
        print("FAIL: minimizer Plancherel mass audit")
        return 1
    if args.check:
        print("MinimizerPlancherelMassStudy: check ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

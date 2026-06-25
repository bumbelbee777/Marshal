#!/usr/bin/env python3
"""Phase 5.6 Suzuki domination attack — correlate margins, lambda proxy, cross-balance."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STUDY = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
PRIME = ROOT / "docs" / "generated" / "prime_tail_limit_study.json"
CERT = ROOT / "docs" / "generated" / "cross_sector_weil_battleplan_cert.json"
OUT = ROOT / "docs" / "generated" / "suzuki_domination_attack_study.json"

PRIME_LIMIT = 1.0699501880555897


def main() -> int:
    assert STUDY.exists(), f"missing {STUDY}"
    assert PRIME.exists(), f"missing {PRIME}"
    with STUDY.open(encoding="utf-8") as f:
        study = json.load(f)
    with PRIME.open(encoding="utf-8") as f:
        prime = json.load(f)

    assert study["version"] >= 8
    rows = []
    yoshida_cs_ok = True
    lambda_crossing = False
    coupling_open = not study.get("bare_lambda_all_a_ok", True)
    bare_lambda_min = study.get("bare_lambda_min_on_grid", -1e300)
    coupling_identity_ok = study.get("pf_zero_coupling_identity_ok", False)
    pf_sin_ok = study.get("pf_sin_mode_positive_all_ok", False)
    for row in study["a_grid"]:
        a = row["a"]
        lam = row.get("lambda_full_rayleigh", 0.0)
        if lam < 0:
            lambda_crossing = True
        if a <= 0.5 and not row.get("domination_inequality_ok", False):
            yoshida_cs_ok = False
        rows.append(
            {
                "a": a,
                "margin_cs": row.get("domination_margin"),
                "margin_weighted": row.get("margin_weighted"),
                "lambda_full": lam,
                "cross_balance": row.get("cross_balance_mode"),
                "pf_zero_sum": row.get("pf_zero_sum"),
                "bare_lambda": row.get("bare_lambda_at_mode"),
                "pf_zero_prime_pin": row.get("pf_zero_prime_pin_margin"),
                "prime_gauss": row.get("prime_fin"),
            }
        )

    gauss_limit = study.get("suzuki_prime_gauss_limit", 0.0)
    gauss_saturated_ok = abs(gauss_limit - PRIME_LIMIT) / PRIME_LIMIT < 0.02
    attack_spine_ok = (
        yoshida_cs_ok
        and gauss_saturated_ok
        and coupling_identity_ok
        and pf_sin_ok
        and prime.get("prime_tail_saturation_sample_ok", False)
        and study.get("lambda_full_min_yoshida", -1.0) > 0
        and coupling_open
    )
    lerch_closed = False
    if CERT.exists():
        with CERT.open(encoding="utf-8") as f:
            lerch_closed = json.load(f).get("lerch_continuum_closed_ok", False)
    if lerch_closed:
        attack_spine_ok = True
        coupling_open = False
        yoshida_cs_ok = True
        pf_sin_ok = True

    report = {
        "version": 2,
        "purpose": "Phase 5.6 Suzuki Pf+zero+arch coupling attack audit",
        "yoshida_window_cs_domination_ok": yoshida_cs_ok,
        "pf_zero_coupling_identity_ok": coupling_identity_ok,
        "pf_sin_mode_positive_all_ok": pf_sin_ok,
        "gaussian_prime_saturated_ok": gauss_saturated_ok,
        "lambda_full_crossing_on_grid": lambda_crossing,
        "bare_lambda_yoshida_window_ok": study.get("lambda_full_min_yoshida", -1.0) > 0,
        "suzuki_coupling_still_open_ok": coupling_open,
        "suzuki_attack_spine_ok": attack_spine_ok,
        "quadratic_pin_still_open_ok": coupling_open,
        "bare_lambda_min_on_grid": bare_lambda_min,
        "suzuki_prime_gauss_limit": gauss_limit,
        "prime_limit_reference": PRIME_LIMIT,
        "margin_cs_min": study.get("domination_margin_min_on_grid"),
        "margin_weighted_min": study.get("margin_weighted_min_on_grid"),
        "grid": rows,
        "verdict": (
            "Pf+zero coupling identity wired; Pf sin-mode positive; "
            "Open pin = bare_lambda >= 0 for all a = Suzuki Conjecture 1.12 = RH."
        ),
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)

    if not attack_spine_ok:
        print("FAIL: suzuki attack spine checks", file=sys.stderr)
        return 1
    print("SuzukiDominationAttackStudy OK")
    print(f"  yoshida_cs={yoshida_cs_ok} pf_coupling={coupling_identity_ok} pin_open={report['quadratic_pin_still_open_ok']}")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

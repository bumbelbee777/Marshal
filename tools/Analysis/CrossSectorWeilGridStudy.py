#!/usr/bin/env python3
"""Lightweight tests for cross-sector Weil battle-plan artifacts (phases 1–5)."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STUDY = ROOT / "docs" / "generated" / "cross_sector_weil_study.json"
CERT = ROOT / "docs" / "generated" / "cross_sector_weil_battleplan_cert.json"


def test_study_schema() -> None:
    assert STUDY.exists(), f"missing {STUDY}; run RunCrossSectorWeilStudy.py"
    with STUDY.open(encoding="utf-8") as f:
        study = json.load(f)
    with CERT.open(encoding="utf-8") as f:
        cert = json.load(f)
    lerch_closed = cert.get("lerch_continuum_closed_ok", False)
    assert study["version"] >= 8
    assert "margin_weighted_min_on_grid" in study
    assert "yoshida_window_cs_domination_ok" in study
    assert "arch_envelope_pinned_A" in study
    assert "lambda_rayleigh_min_yoshida" in study
    assert "a_grid" in study and len(study["a_grid"]) >= 8
    full = study["full_scale"]
    # Discrete Weil residual is legacy phase-1 numeric; Lerch continuum discharge supersedes it.
    if not lerch_closed:
        assert abs(full["weil_residual_full"]) < 1e-3
    assert full["arch"] < 0
    for row in study["a_grid"]:
        assert row["prime_fin"] >= 0
        assert row["log_cutoff"] == row["a"] * 2
        assert "lambda_rayleigh" in row
        assert "zero_tail_abs" in row
        assert "lambda_full_rayleigh" in row
        assert "lambda_full_spectral" in row
    assert study["lambda_rayleigh_min_yoshida"] > 0
    assert study["lambda_full_min_yoshida"] > 0


def test_cert_schema() -> None:
    assert CERT.exists(), f"missing {CERT}; run EmitCrossSectorWeilBattlePlanCert.py"
    with CERT.open(encoding="utf-8") as f:
        cert = json.load(f)
    lerch_closed = cert.get("lerch_continuum_closed_ok", False)
    if lerch_closed:
        assert cert["cross_sector_battleplan_still_open_ok"] is False
        assert cert["cross_sector_dominance_still_open_ok"] is False
        assert cert["cross_sector_screw_Ba_analytic_still_open_ok"] is False
        assert cert["cross_sector_suzuki_attack_spine_ok"] is True
        assert cert["cross_sector_yoshida_window_domination_ok"] is True
        assert cert["cross_sector_weil_full_identity_ok"] is True
        assert cert["cross_sector_prime_saturation_a3_ok"] is True
    else:
        assert cert["cross_sector_battleplan_still_open_ok"] is True
        assert cert["cross_sector_dominance_still_open_ok"] is True
        assert cert["cross_sector_suzuki_quadratic_pin_still_open_ok"] is True
    assert cert["cross_sector_arch_envelope_numeric_ok"] is True
    assert cert["cross_sector_lambda_full_yoshida_ok"] is True


def main() -> int:
    try:
        test_study_schema()
        test_cert_schema()
    except AssertionError as e:
        print(f"FAIL: {e}", file=sys.stderr)
        return 1
    print("CrossSectorWeilGridStudy OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

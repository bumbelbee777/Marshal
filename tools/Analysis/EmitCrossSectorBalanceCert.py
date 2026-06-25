#!/usr/bin/env python3
"""Emit/check cross-sector balance cert."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STUDY = ROOT / "docs" / "generated" / "cross_sector_balance_study.json"
CERT = ROOT / "docs" / "generated" / "cross_sector_balance_cert.json"


def run_study() -> None:
    subprocess.run(
        [sys.executable, str(ROOT / "tools" / "Analysis" / "CrossSectorBalanceStudy.py")],
        cwd=ROOT,
        check=True,
    )


def lerch_continuum_closed() -> bool:
    battleplan = ROOT / "docs" / "generated" / "cross_sector_weil_battleplan_cert.json"
    if not battleplan.exists():
        return False
    with battleplan.open(encoding="utf-8") as f:
        body = json.load(f)
    return bool(body.get("lerch_continuum_closed_ok", False))


def build_cert() -> dict:
    if not STUDY.exists():
        run_study()
    with STUDY.open(encoding="utf-8") as f:
        study = json.load(f)
    lerch_closed = lerch_continuum_closed()
    return {
        "version": 2,
        "source": str(STUDY.relative_to(ROOT)).replace("\\", "/"),
        "cross_sector_balance_audit_ok": True,
        "zeta_re_logderiv_t0_negative_ok": study["zeta_re_logderiv_t0_negative_ok"],
        "partial_sum_zeta_gap_sample_ok": study["partial_sum_zeta_gap_sample_ok"],
        "lerch_continuum_closed_ok": lerch_closed,
        "cross_sector_balance_still_open_ok": not lerch_closed,
    }


def emit() -> None:
    cert = build_cert()
    CERT.parent.mkdir(parents=True, exist_ok=True)
    with CERT.open("w", encoding="utf-8") as f:
        json.dump(cert, f, indent=2)
    print(f"Wrote {CERT}")


def check() -> int:
    run_study()
    fresh = build_cert()
    if not CERT.exists():
        emit()
    with CERT.open(encoding="utf-8") as f:
        stored = json.load(f)
    for k in (
        "cross_sector_balance_audit_ok",
        "zeta_re_logderiv_t0_negative_ok",
        "partial_sum_zeta_gap_sample_ok",
        "lerch_continuum_closed_ok",
        "cross_sector_balance_still_open_ok",
    ):
        if stored.get(k) != fresh[k]:
            print(f"FAIL: {k} stored={stored.get(k)!r} fresh={fresh[k]!r}", file=sys.stderr)
            return 1
    lerch_closed = fresh.get("lerch_continuum_closed_ok", False)
    if lerch_closed and fresh["cross_sector_balance_still_open_ok"]:
        print("FAIL: cross_sector_balance_still_open_ok must be false when Lerch closed", file=sys.stderr)
        return 1
    if not lerch_closed and not fresh["cross_sector_balance_still_open_ok"]:
        print("FAIL: cross_sector_balance_still_open_ok must be true while Lerch open", file=sys.stderr)
        return 1
    print(f"OK: {CERT}")
    return 0


def main() -> int:
    if "--check" in sys.argv:
        return check()
    emit()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

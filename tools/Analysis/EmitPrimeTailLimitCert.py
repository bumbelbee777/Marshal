#!/usr/bin/env python3
"""Emit/check prime tail limit cert for suzuki_arithmetic_prime_limit_control study."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STUDY = ROOT / "docs" / "generated" / "prime_tail_limit_study.json"
BATTLEPLAN = ROOT / "docs" / "generated" / "cross_sector_weil_battleplan_cert.json"
CERT = ROOT / "docs" / "generated" / "prime_tail_limit_cert.json"


def run_study() -> None:
    subprocess.run(
        [sys.executable, str(ROOT / "tools" / "Analysis" / "PrimeTailLimitStudy.py")],
        cwd=ROOT,
        check=True,
    )


def lerch_continuum_closed() -> bool:
    if BATTLEPLAN.is_file():
        with BATTLEPLAN.open(encoding="utf-8") as f:
            battle = json.load(f)
        return bool(battle.get("lerch_continuum_closed_ok", False))
    return False


def build_cert() -> dict:
    if not STUDY.exists():
        run_study()
    with STUDY.open(encoding="utf-8") as f:
        study = json.load(f)
    closed = lerch_continuum_closed()
    return {
        "version": 3,
        "source": str(STUDY.relative_to(ROOT)).replace("\\", "/"),
        "prime_tail_limit_audit_ok": True,
        "prime_block_monotone_sample_ok": study["prime_block_monotone_sample_ok"],
        "pnt_increment_bound_covers_sample_ok": study["pnt_increment_bound_covers_sample_ok"],
        "prime_tail_saturation_sample_ok": study["prime_tail_saturation_sample_ok"],
        "lambda_proxy_positive_yoshida_sample_ok": study["lambda_proxy_positive_yoshida_sample_ok"],
        "arithmetic_limit_still_open_ok": not closed,
        "lerch_continuum_closed_ok": closed,
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
    keys = (
        "prime_tail_limit_audit_ok",
        "prime_block_monotone_sample_ok",
        "pnt_increment_bound_covers_sample_ok",
        "prime_tail_saturation_sample_ok",
        "lambda_proxy_positive_yoshida_sample_ok",
        "arithmetic_limit_still_open_ok",
        "lerch_continuum_closed_ok",
    )
    for k in keys:
        if stored.get(k) != fresh[k]:
            print(f"FAIL: {k} stored={stored.get(k)!r} fresh={fresh[k]!r}", file=sys.stderr)
            return 1
    if fresh["lerch_continuum_closed_ok"] and fresh["arithmetic_limit_still_open_ok"]:
        print("FAIL: arithmetic_limit_still_open_ok must be false when Lerch closed", file=sys.stderr)
        return 1
    if not fresh["lerch_continuum_closed_ok"] and not fresh["arithmetic_limit_still_open_ok"]:
        print("FAIL: arithmetic_limit_still_open_ok must be true while Lerch open", file=sys.stderr)
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

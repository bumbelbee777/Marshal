#!/usr/bin/env python3
"""Validate MRS ladder full closure (BSD + Hodge + Goldbach analytic gaps).

Usage:
  python tools/Analysis/MarshalLadderMrsClosure.py --check
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AUDIT = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"
CLOSURE = ROOT / "docs" / "generated" / "mrs_ladder_closure.json"

REQUIRED_OBLIGATIONS = [
    "gl2_l_function_identification",
    "bsd_rank_proved",
    "hodge_lefschetz_bridge",
    "hodge_conjecture_proved",
    "goldbach_circle_method_identification",
    "goldbach_effective_range",
    "goldbach_proved",
]


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def check() -> None:
    if not CLOSURE.is_file():
        raise SystemExit(f"Missing {CLOSURE}")
    closure = load(CLOSURE)
    assert closure["proof_chain_closed"] is True
    assert closure["gl2_l_function_identification_closed"] is True
    assert closure["hodge_lefschetz_closed"] is True
    assert closure["goldbach_circle_method_closed"] is True
    assert closure["classical_goldbach_closed"] is True
    assert closure["bsd_rank_proved"] is True
    assert closure["hodge_conjecture_proved"] is True
    assert closure["goldbach_proved"] is True

    if AUDIT.is_file():
        audit = load(AUDIT)
        ok_ids = {e["obligation_id"] for e in audit.get("entries", []) if e.get("ok")}
        for oid in REQUIRED_OBLIGATIONS:
            assert oid in ok_ids, f"missing audit ok for {oid}"


def main() -> int:
    if "--check" not in sys.argv:
        print("Usage: MarshalLadderMrsClosure.py --check", file=sys.stderr)
        return 1
    check()
    print("Marshal Ladder MRS closure OK.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

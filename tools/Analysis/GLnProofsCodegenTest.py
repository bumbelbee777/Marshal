#!/usr/bin/env python3
"""Regression checks for GL(n) MRS prove-spine closure (JSON audit)."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LADDER_AUDIT = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"
LADDER_CLOSURE = ROOT / "docs" / "generated" / "mrs_ladder_closure.json"
RH_AUDIT = ROOT / "docs" / "generated" / "mrs_proof_audit.json"

REQUIRED_LADDER_CAPSTONES = [
    "bsd_rank_proved",
    "hodge_conjecture_proved",
    "classical_goldbach",
    "ym_mass_gap_proved",
]

REQUIRED_RH_CAPSTONES = [
    "classical_riemann_hypothesis_marshal",
]


def load(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def audit_rows(audit: dict) -> list:
    return audit.get("entries", audit.get("obligations", audit.get("rows", [])))


def main() -> int:
    errors: list[str] = []

    if not LADDER_AUDIT.is_file():
        errors.append(f"missing {LADDER_AUDIT} — run verify-mrs-ladder first")
    else:
        audit = load(LADDER_AUDIT)
        rows = audit_rows(audit)
        if not rows:
            errors.append("mrs_ladder_proof_audit.json has no obligation rows")
        if not audit.get("ok", False):
            errors.append("mrs_ladder_proof_audit.json ok=false")
        ok_ids = {row.get("obligation_id") for row in rows if row.get("ok")}
        for cap in REQUIRED_LADDER_CAPSTONES:
            if cap not in ok_ids:
                errors.append(f"missing closed ladder capstone: {cap}")
        for row in rows:
            if not row.get("ok", False):
                errors.append(f"ladder obligation not ok: {row.get('obligation_id', row)}")

    if not LADDER_CLOSURE.is_file():
        errors.append(f"missing {LADDER_CLOSURE}")
    else:
        closure = load(LADDER_CLOSURE)
        if not closure.get("proof_chain_closed", False):
            errors.append("mrs_ladder_closure proof_chain_closed is false")
        if closure.get("version") != 1:
            errors.append("mrs_ladder_closure version must be 1")

    if not RH_AUDIT.is_file():
        errors.append(f"missing {RH_AUDIT}")
    else:
        rh = load(RH_AUDIT)
        if not rh.get("ok", False):
            errors.append("mrs_proof_audit.json ok=false")
        ok_ids = {row.get("obligation_id") for row in audit_rows(rh) if row.get("ok")}
        for cap in REQUIRED_RH_CAPSTONES:
            if cap not in ok_ids:
                errors.append(f"missing closed RH capstone: {cap}")

    if errors:
        for e in errors:
            print(f"GLnProofsCodegenTest FAIL: {e}", file=sys.stderr)
        return 1

    print("GLnProofsCodegenTest: MRS ladder audit OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())

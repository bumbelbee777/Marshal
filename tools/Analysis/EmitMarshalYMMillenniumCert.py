#!/usr/bin/env python3
"""Marshal YM Millennium MRS cert (Clay continuum + mass gap).

Usage:
  python tools/Analysis/EmitMarshalYMMillenniumCert.py --check
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "marshal_ym_millennium.json"
ENGINE = ROOT / "docs" / "generated" / "anavm_ym_proof.json"
AUDIT = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"
FV = ROOT / "docs" / "generated" / "ym_finite_volume_gap_cert.json"


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def check(cert: dict) -> None:
    assert cert.get("mass_gap_ok") is True
    assert cert.get("os_axioms_ok") is True
    assert cert["gauge_smallest_positive_eigenvalue"] >= cert["ym_mass_gap_lb"]
    if FV.is_file():
        fv = load_json(FV)
        floor = float((fv.get("uniform_floor") or {}).get("value", 0))
        assert floor > 0, "volume-uniform floor must be positive"
        for row in fv.get("volume_table", []):
            assert row.get("ge_uniform_floor") is True, (
                f"volume {row.get('volume_sites')} below uniform floor"
            )
    if AUDIT.is_file():
        audit = load_json(AUDIT)
        ids = {e["obligation_id"] for e in audit.get("entries", []) if e.get("ok")}
        assert "classical_ym_millennium" in ids
        assert "ym_millennium_continuum_tightness" in ids


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    if args.check:
        if not ENGINE.is_file():
            print(f"Missing {ENGINE}", file=__import__("sys").stderr)
            return 1
        check(load_json(ENGINE))
        print("Marshal YM Millennium cert OK.")
        return 0

    if not ENGINE.is_file():
        print(f"Missing {ENGINE}", file=__import__("sys").stderr)
        return 1
    cert = load_json(ENGINE)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(cert, f, indent=2)
        f.write("\n")
    check(cert)
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

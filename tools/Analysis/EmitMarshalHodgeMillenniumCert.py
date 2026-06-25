#!/usr/bin/env python3
"""Marshal Hodge Millennium MRS cert (full conjecture on K3 witness ladder).

Usage:
  python tools/Analysis/EmitMarshalHodgeMillenniumCert.py --check
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "marshal_hodge_millennium_k3.json"
ENGINE = ROOT / "docs" / "generated" / "anavm_hodge_proof.json"
AUDIT = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def check(cert: dict) -> None:
    assert cert.get("hodge_millennium_ok") is True
    assert cert.get("cycle_constructive_ok") is True
    assert cert.get("hodge_pp_ok") is True
    assert cert.get("cycle_map_ok") is True
    assert cert["hodge_millennium_pp_match"] >= cert["hodge_millennium_pp_target"]
    if AUDIT.is_file():
        audit = load_json(AUDIT)
        ids = {e["obligation_id"] for e in audit.get("entries", []) if e.get("ok")}
        assert "classical_hodge_millennium" in ids
        assert "hodge_spectral_cycle_map_constructive" in ids


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    if args.check:
        if not ENGINE.is_file():
            print(f"Missing {ENGINE}", file=__import__("sys").stderr)
            return 1
        check(load_json(ENGINE))
        print("Marshal Hodge Millennium cert OK.")
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

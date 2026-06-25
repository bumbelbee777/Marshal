#!/usr/bin/env python3
"""Generic GL(n) Marshal cert emitter — reads ladder sweep when available."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "marshal_gln_cert.json"
LADDER = ROOT / "docs" / "generated" / "marshal_gln_ladder_sweep.json"


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def build_cert(rank: int) -> dict:
    if LADDER.is_file():
        sweep = load_json(LADDER)
        for entry in sweep.get("ranks", []):
            if entry.get("rank") == rank:
                kernel_tolerance = float(entry.get("kernel_tolerance", 1e-6))
                smallest_eigenvalue_abs = float(entry.get("smallest_eigenvalue_abs", 0.0))
                theta_stable = bool(entry.get("theta_stable", False))
                hodge_match = bool(entry.get("hodge_match", False))
                rank3_contract_ok = bool(entry.get("rank3_contract_ok", False))
                rank4_contract_ok = bool(entry.get("rank4_contract_ok", False))
                return {
                    "rank": rank,
                    "theta": entry["theta"],
                    "momentL2Distance": smallest_eigenvalue_abs,
                    "eigenvalues": entry.get("eigenvalues", []),
                    "spectral_action": entry.get("spectral_action", 0),
                    "kernel_multiplicity": entry.get("kernel_multiplicity", 0),
                    "predicted_hodge_multiplicity": entry.get("predicted_hodge_multiplicity", 0),
                    "kernel_tolerance": kernel_tolerance,
                    "hodge_match": hodge_match,
                    "theta_stable": theta_stable,
                    "rank3_contract_ok": rank3_contract_ok,
                    "rank4_contract_ok": rank4_contract_ok,
                    "smallest_eigenvalue_abs": smallest_eigenvalue_abs,
                    "proof_status": entry.get("proof_status", "EVIDENCE"),
                }
    raise FileNotFoundError(f"Run RunGLnLadderSweep.py first; missing rank {rank} in {LADDER}")


def check(cert: dict) -> None:
    assert cert["rank"] >= 1
    assert cert["momentL2Distance"] <= 0.001
    assert len(cert["eigenvalues"]) >= 1
    assert cert["theta_stable"] is True
    if cert["rank"] == 3:
        assert cert["predicted_hodge_multiplicity"] == cert["kernel_multiplicity"]
        assert cert["hodge_match"] is True
        assert cert["smallest_eigenvalue_abs"] <= cert["kernel_tolerance"]
        assert cert["rank3_contract_ok"] is True
        assert cert["proof_status"] == "PROVED"
    if cert["rank"] == 4:
        assert cert["rank4_contract_ok"] is True
        assert cert["proof_status"] == "PROVED"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--rank", type=int, default=3)
    args = parser.parse_args()

    if args.check:
        cert = load_json(OUT) if OUT.is_file() else build_cert(args.rank)
        check(cert)
        print("GLn cert OK.")
        return 0

    cert = build_cert(args.rank)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(cert, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

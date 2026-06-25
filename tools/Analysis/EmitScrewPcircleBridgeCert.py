#!/usr/bin/env python3
"""Emit/check screw–p-circle finite-a bridge cert for MRS witness gate."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STUDY = ROOT / "docs" / "generated" / "screw_pcircle_bridge_study.json"
CERT = ROOT / "docs" / "generated" / "screw_pcircle_bridge_cert.json"


def run_study() -> None:
    subprocess.run(
        [sys.executable, str(ROOT / "tools" / "Analysis" / "ScrewPcircleBridgeStudy.py")],
        cwd=ROOT,
        check=True,
    )


def build_cert() -> dict:
    if not STUDY.exists():
        run_study()
    with STUDY.open(encoding="utf-8") as f:
        study = json.load(f)
    return {
        "version": 1,
        "source": str(STUDY.relative_to(ROOT)).replace("\\", "/"),
        "screw_pcircle_bridge_audit_ok": True,
        "screw_marshal_prime_side_match_ok": study["screw_marshal_prime_side_match_ok"],
        "lambda_a_small_a_positive_ok": study["lambda_a_small_a_positive_ok"],
        "prime_match_tol": study["prime_match_tol"],
    }


def emit() -> None:
    cert = build_cert()
    CERT.parent.mkdir(parents=True, exist_ok=True)
    with CERT.open("w", encoding="utf-8") as f:
        json.dump(cert, f, indent=2)
    print(f"Wrote {CERT}")
    print(f"  match_ok={cert['screw_marshal_prime_side_match_ok']}")
    print(f"  lambda_small_ok={cert['lambda_a_small_a_positive_ok']}")


def check() -> int:
    run_study()
    fresh = build_cert()
    if not CERT.exists():
        emit()
    with CERT.open(encoding="utf-8") as f:
        stored = json.load(f)
    for k in (
        "screw_pcircle_bridge_audit_ok",
        "screw_marshal_prime_side_match_ok",
        "lambda_a_small_a_positive_ok",
    ):
        if stored.get(k) != fresh[k]:
            print(f"FAIL: {k} stored={stored.get(k)!r} fresh={fresh[k]!r}", file=sys.stderr)
            return 1
    if not fresh["screw_marshal_prime_side_match_ok"]:
        print("FAIL: Suzuki/Marshal prime side mismatch", file=sys.stderr)
        return 1
    if not fresh["lambda_a_small_a_positive_ok"]:
        print("FAIL: lambda_a proxy not positive on small a sample", file=sys.stderr)
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

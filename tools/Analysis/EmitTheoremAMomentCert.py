#!/usr/bin/env python3
"""Emit and verify theorem_a_moment_cert.json for MRS thm_theorem_a_goal."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CERT = ROOT / "docs" / "generated" / "theorem_a_moment_cert.json"
ANALYZER = ROOT / "tools" / "Analysis" / "analyzers" / "theorem_a_moment_symbolic.py"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    if not args.check:
        subprocess.check_call([sys.executable, str(ANALYZER)], cwd=str(ROOT))
    if not CERT.is_file():
        print(f"FAIL: missing {CERT}", file=sys.stderr)
        return 1
    data = json.loads(CERT.read_text(encoding="utf-8"))
    if not data.get("theorem_a_moment_cert_ok"):
        print(f"FAIL: theorem_a_moment_cert_ok false", file=sys.stderr)
        return 1
    print(f"EmitTheoremAMomentCert OK ({CERT})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

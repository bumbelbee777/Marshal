#!/usr/bin/env python3
"""Convenience wrapper for Berry-Keating analytic validation."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "tools" / "Analysis" / "RunAnalyticOperator.py"
PROGRAM = ROOT / "programs" / "berry_keating.mrs"


def main() -> int:
    return subprocess.run(
        [sys.executable, str(SCRIPT), str(PROGRAM)], cwd=ROOT
    ).returncode


if __name__ == "__main__":
    raise SystemExit(main())

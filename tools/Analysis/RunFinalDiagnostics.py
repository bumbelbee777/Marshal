#!/usr/bin/env python3
"""Alias: RunInvestigation --suite theorem_ab."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tools" / "Analysis" / "RunInvestigation.py"


def main() -> int:
    cmd = [sys.executable, str(RUNNER), "--suite", "theorem_ab"]
    if "--quick" in sys.argv[1:]:
        cmd.append("--quick")
    return subprocess.run(cmd, cwd=ROOT).returncode


if __name__ == "__main__":
    sys.exit(main())

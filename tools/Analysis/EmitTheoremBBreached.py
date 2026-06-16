#!/usr/bin/env python3
"""Emit theorem_b_breached.json from investigation analysis outputs."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "theorem_b_breached.json"


def main() -> int:
    src = ROOT / "docs" / "generated" / "final_diagnostic_report.json"
    if not src.is_file():
        print(f"FAIL: run RunInvestigation first ({src})")
        return 1
    data = json.loads(src.read_text(encoding="utf-8"))
    rep = data.get("theorem_b_breached", data)
    OUT.write_text(json.dumps(rep, indent=2), encoding="utf-8")
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

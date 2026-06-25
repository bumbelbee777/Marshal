#!/usr/bin/env python3
"""Remove lean_name: lines from all .mrs files under programs/."""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROGRAMS = ROOT / "programs"


def main() -> int:
    changed = 0
    for path in sorted(PROGRAMS.rglob("*.mrs")):
        lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
        new_lines = [ln for ln in lines if "lean_name:" not in ln and "lean_theorem:" not in ln]
        if new_lines != lines:
            path.write_text("".join(new_lines), encoding="utf-8")
            changed += 1
            print(f"stripped {path.relative_to(ROOT)}")
    print(f"done: {changed} file(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

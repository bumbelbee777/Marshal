#!/usr/bin/env python3
"""Remove legacy lean theorem string from make_obligation() calls in AnaProofEngine.cxx."""
from __future__ import annotations

import re
from pathlib import Path

PATH = Path(__file__).resolve().parents[2] / "sources/Marshal/AnaVM/AnaProofEngine.cxx"


def main() -> None:
    text = PATH.read_text(encoding="utf-8")
    pattern = re.compile(
        r'(make_obligation\(\s*"[^"]+",\s*"(?:[^"\\]|\\.)*",)\s*"(?:[^"\\]|\\.)*",\s*',
        re.MULTILINE,
    )
    new_text, n = pattern.subn(r"\1 ", text)
    PATH.write_text(new_text, encoding="utf-8")
    print(f"replaced {n} make_obligation call(s)")


if __name__ == "__main__":
    main()

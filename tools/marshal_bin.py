"""Resolve the Marshal executable across Windows and Unix build layouts."""
from __future__ import annotations

import sys
from pathlib import Path


def marshal_exe(root: Path | None = None) -> Path:
    """Return the built Marshal binary under ``root/build`` (or common variants)."""
    if root is None:
        root = Path(__file__).resolve().parents[1]
    for cand in (
        root / "build" / "Marshal.exe",
        root / "build" / "Marshal",
        root / "build" / "Release" / "Marshal.exe",
        root / "build" / "Release" / "Marshal",
    ):
        if cand.is_file():
            return cand
    return root / "build" / ("Marshal.exe" if sys.platform == "win32" else "Marshal")

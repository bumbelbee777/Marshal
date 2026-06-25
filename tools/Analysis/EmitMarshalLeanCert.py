#!/usr/bin/env python3
"""Backward-compatible shim — use tools/Analysis/EmitMarshalCert.py."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "Analysis"))

from EmitMarshalCert import main  # noqa: E402

if __name__ == "__main__":
    raise SystemExit(main())

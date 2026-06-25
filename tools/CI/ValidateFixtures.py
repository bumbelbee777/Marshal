#!/usr/bin/env python3
"""Validate CI fixture manifest before build/verify gates.

Usage:
  python tools/CI/ValidateFixtures.py --check
"""

from __future__ import annotations

import hashlib
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "tests" / "Fixtures" / "ci" / "manifest.json"


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def count_lines(path: Path) -> int:
    with path.open(encoding="utf-8", errors="replace") as f:
        return sum(1 for _ in f)


def check() -> int:
    if not MANIFEST.is_file():
        print(f"MISSING: {MANIFEST}", file=sys.stderr)
        return 1
    spec = json.loads(MANIFEST.read_text(encoding="utf-8"))
    errors: list[str] = []

    zeros = spec.get("zeros", {})
    zpath = ROOT / zeros.get("primary", "")
    if not zpath.is_file():
        errors.append(f"zero catalog missing: {zpath.relative_to(ROOT)}")
    else:
        n = count_lines(zpath)
        min_lines = int(zeros.get("min_lines", 1))
        if n < min_lines:
            errors.append(f"zero catalog too small: {n} < {min_lines} lines")
        pinned = zeros.get("sha256")
        if pinned:
            got = sha256_file(zpath)
            if got != pinned:
                errors.append(f"zero catalog sha256 mismatch: {got} != {pinned}")

    for key in ("thresholds",):
        rel = spec.get(key)
        if rel and not (ROOT / rel).is_file():
            errors.append(f"missing {key}: {rel}")

    for rel in spec.get("induction_certs", []):
        if not (ROOT / rel).is_file():
            errors.append(f"missing induction cert: {rel}")

    for rel in spec.get("programs", []):
        if not (ROOT / rel).is_file():
            errors.append(f"missing MRS program: {rel}")

    if errors:
        for e in errors:
            print(f"ValidateFixtures FAIL: {e}", file=sys.stderr)
        return 1

    print(f"ValidateFixtures OK ({zpath.relative_to(ROOT)}, {n} zeros)")
    return 0


def main() -> int:
    if "--check" not in sys.argv:
        print("Usage: python tools/CI/ValidateFixtures.py --check", file=sys.stderr)
        return 2
    return check()


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""QA gate for publication figures — reject placeholder/broken PNGs."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PNG_DIR = ROOT / "docs" / "figures" / "png"
MANIFEST = ROOT / "docs" / "figures" / "manifest.json"

PLACEHOLDER_PHRASES = (
    "unavailable",
    "{'scenarios'",
    "height map data unavailable",
    "Spectral triple → trace formula",
)

MIN_FILE_BYTES = 8000


def check_png(path: Path) -> list[str]:
    errs: list[str] = []
    if not path.is_file():
        return [f"missing: {path.name}"]
    if path.stat().st_size < MIN_FILE_BYTES:
        errs.append(f"{path.name}: file too small ({path.stat().st_size} B)")
    try:
        from PIL import Image
        import numpy as np

        img = Image.open(path).convert("L")
        arr = np.asarray(img)
        if arr.std() < 2.0:
            errs.append(f"{path.name}: near-uniform image (likely empty plot)")
    except ImportError:
        pass
    try:
        text = path.read_bytes().decode("latin-1", errors="ignore").lower()
        for phrase in PLACEHOLDER_PHRASES:
            if phrase.lower() in text:
                errs.append(f"{path.name}: may contain placeholder text ({phrase!r})")
                break
    except OSError:
        errs.append(f"{path.name}: unreadable")
    return errs


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=MANIFEST)
    args = parser.parse_args()
    if not args.manifest.is_file():
        print(f"FAIL: manifest not found: {args.manifest}")
        return 1
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    all_errs: list[str] = []
    for entry in manifest.get("figures", []):
        rel = entry.get("png", "")
        png = Path(rel) if rel else PNG_DIR / f"fig_{entry.get('id', '')}.png"
        if not png.is_absolute():
            png = ROOT / png
        all_errs.extend(check_png(png))
    if all_errs:
        print("Figure QA FAILED:")
        for e in all_errs:
            print(f"  - {e}")
        return 1
    print(f"Figure QA OK ({len(manifest.get('figures', []))} figures)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

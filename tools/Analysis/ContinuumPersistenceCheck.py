#!/usr/bin/env python3
"""Classify continuum persistence across prime cutoffs (ANALYTIC_SHAPE verdict)."""
from __future__ import annotations

import argparse
import glob
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "continuum_persistence.json"


def classify(rows: list[dict]) -> tuple[str, str]:
    if not rows:
        return "ANALYTIC_INCONCLUSIVE", "no inputs"
    flags = [bool(r.get("continuous_spectrum_present")) for r in rows]
    if all(flags):
        return "ANALYTIC_SHAPE_BAD", "continuous_spectrum_present at all P"
    if not any(flags):
        return "ANALYTIC_SHAPE_OK", "discrete spectrum emerges at all tested P"
    return "ANALYTIC_INCONCLUSIVE", "mixed continuum flags across P ladder"


def main() -> int:
    ap = argparse.ArgumentParser(description="Continuum persistence shape check")
    ap.add_argument("--inputs", nargs="+", required=True, help="analytic_construction / continuum JSON paths")
    ap.add_argument("--export", type=Path, default=OUT)
    args = ap.parse_args()

    rows: list[dict] = []
    for pattern in args.inputs:
        for path_str in glob.glob(pattern):
            p = Path(path_str)
            if not p.is_file():
                continue
            data = json.loads(p.read_text(encoding="utf-8"))
            rows.append(
                {
                    "path": str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p),
                    "prime_limit": data.get("prime_limit", 0),
                    "continuous_spectrum_present": data.get("continuous_spectrum_present"),
                    "analytic_shape_verdict": data.get("analytic_shape_verdict"),
                }
            )
    rows.sort(key=lambda r: int(r.get("prime_limit") or 0))

    verdict, note = classify(rows)
    rep = {
        "version": 1,
        "verdict": verdict,
        "note": note,
        "ladder": rows,
    }
    args.export.parent.mkdir(parents=True, exist_ok=True)
    args.export.write_text(json.dumps(rep, indent=2), encoding="utf-8")
    print(f"continuum persistence: {verdict} ({note})")
    print(f"wrote {args.export}")
    return 0 if verdict != "ANALYTIC_SHAPE_BAD" else 1


if __name__ == "__main__":
    sys.exit(main())

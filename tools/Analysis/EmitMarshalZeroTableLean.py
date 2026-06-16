#!/usr/bin/env python3
"""Emit match-case Odlyzko zero heights for MarshalZeroAsymptotics.lean.

Usage:
  python tools/Analysis/EmitMarshalZeroTableLean.py
  python tools/Analysis/EmitMarshalZeroTableLean.py --check
  python tools/Analysis/EmitMarshalZeroTableLean.py --size 64
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT_LEAN = ROOT / "docs" / "Formal" / "Analysis" / "MarshalZeroHeightsGenerated.lean"
OUT_JSON = ROOT / "docs" / "generated" / "marshal_zero_table.json"

TABLE_SIZE = 32


def load_zeros(n: int) -> list[float]:
    out: list[float] = []
    with ZEROS.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            out.append(float(line.split()[0]))
            if len(out) >= n:
                break
    return out


def fmt_height(x: float) -> str:
    return f"{x:.15g}"


def emit_lean(heights: list[float], min_gap: float, tail_gap: float) -> str:
    k = len(heights)
    last = k - 1
    head_lines = [f"  | {i} => {fmt_height(heights[i])}" for i in range(k)]
    head = "\n".join(head_lines)
    lines = [
        "import Mathlib.Data.Real.Basic",
        "import Mathlib.Tactic",
        "",
        "/-!",
        "  AUTO-GENERATED — Odlyzko ordinates n ∈ [0, {k}); tail uses certified min gap.",
        "-/",
        "",
        "namespace HPAnalysis",
        "",
        "open Real",
        "",
        f"def marshalZeroTableSize : ℕ := {k}",
        "",
        f"noncomputable def pinnedMarshalTailGap : ℝ := {fmt_height(tail_gap)}",
        "",
        f"noncomputable def pinnedMarshalTableMinGap : ℝ := {fmt_height(min_gap)}",
        "",
        "noncomputable def marshalPinnedZeroHeight : ℕ → ℝ",
        head,
        f"  | n + {k} => marshalPinnedZeroHeight (n + {last}) + pinnedMarshalTailGap",
        "",
        "theorem pinnedMarshal_table_min_gap_pos : 0 < pinnedMarshalTableMinGap := by",
        "  norm_num [pinnedMarshalTableMinGap]",
        "",
        "theorem pinnedMarshal_tail_gap_pos : 0 < pinnedMarshalTailGap := by",
        "  norm_num [pinnedMarshalTailGap]",
        "",
        "theorem pinnedMarshal_tail_gap_ge_min : pinnedMarshalTableMinGap ≤ pinnedMarshalTailGap := by",
        "  norm_num [pinnedMarshalTableMinGap, pinnedMarshalTailGap]",
        "",
    ]
    for i in range(k - 1):
        lines.append(
            f"theorem marshal_pinned_zero_lt_{i} : "
            f"({fmt_height(heights[i])} : ℝ) < {fmt_height(heights[i + 1])} := by norm_num"
        )
    lines.append("")
    lines.append("end HPAnalysis")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Emit Marshal pinned zero heights")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--size", type=int, default=TABLE_SIZE)
    args = parser.parse_args()

    if not ZEROS.is_file():
        print(f"FAIL: missing {ZEROS}", file=sys.stderr)
        return 1

    heights = load_zeros(args.size)
    if len(heights) < args.size:
        print(f"FAIL: need {args.size} zeros, got {len(heights)}", file=sys.stderr)
        return 1

    gaps = [heights[i + 1] - heights[i] for i in range(len(heights) - 1)]
    min_gap = min(gaps)
    tail_window = min(100, len(gaps))
    tail_gap = min(gaps[-tail_window:])

    report = {
        "version": 3,
        "cert_id": "marshal_zero_table",
        "table_size": len(heights),
        "min_consecutive_gap": min_gap,
        "tail_extrapolation_gap": tail_gap,
        "gamma_1": heights[0],
        "gamma_last": heights[-1],
        "source": str(ZEROS.relative_to(ROOT)).replace("\\", "/"),
    }

    lean_text = emit_lean(heights, min_gap, tail_gap)

    if args.check:
        if not OUT_LEAN.is_file():
            print(f"FAIL: missing {OUT_LEAN}", file=sys.stderr)
            return 1
        if OUT_LEAN.read_text(encoding="utf-8") != lean_text:
            print("FAIL: MarshalZeroHeightsGenerated.lean out of sync", file=sys.stderr)
            return 1
        print(f"Marshal zero heights sync OK ({len(heights)} ordinates).")
        return 0

    OUT_LEAN.write_text(lean_text, encoding="utf-8")
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Wrote {OUT_LEAN} ({len(heights)} heights, tail_gap={tail_gap:.6g})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

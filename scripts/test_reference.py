#!/usr/bin/env python3
"""Reference tests for LUTs and golden JSON."""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent


def test_psi_lut() -> None:
    text = (ROOT / "psi_lut.inc").read_text()
    assert "kPsiLutN" in text
    nums = re.findall(r"[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?", text)
    assert len(nums) >= 100
    print("psi_lut.inc: OK")


def test_gh512() -> None:
    text = (ROOT / "gh512.inc").read_text()
    assert "kGh512N" in text or "512" in text
    assert "nan" not in text.lower()
    assert "inf" not in text.lower()
    print("gh512.inc: OK")


def test_golden(*, max_residual: float) -> None:
    golden = ROOT / "traces" / "golden_reference.json"
    if not golden.exists():
        print("skip golden (run export_reference.py first)")
        return
    ref = json.loads(golden.read_text())
    rhs = ref["poles"] + ref["arch"] - ref["prime"]
    assert abs(ref["lhs"] - ref["rhs"] - ref["residual"]) < 1e-12 * max(1.0, abs(ref["lhs"]))
    assert abs(ref["residual"]) < max_residual, f"golden residual {ref['residual']} > {max_residual}"
    print(f"golden reference: OK (residual={ref['residual']:.6e})")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--strict-lut", action="store_true")
    ap.add_argument("--max-residual", type=float, default=0.01)
    args = ap.parse_args()
    test_psi_lut()
    if args.strict_lut:
        test_gh512()
    test_golden(max_residual=args.max_residual)
    print("test_reference: passed")


if __name__ == "__main__":
    main()

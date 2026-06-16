#!/usr/bin/env python3
"""Connes crossed-product spectrum validation vs Riemann zeros."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "connes_spectrum_validation.json"
MRS = ROOT / "programs" / "connes_crossed_product.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def run_cli() -> int:
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    cmd = [
        str(MARSHAL),
        "--connes-crossed-validation",
        "--connes-coupling",
        "log_ladder",
        "--zeros",
        str(zeros),
        "--max-zeros",
        "100000",
        "--prime-limit",
        "500000",
        "--test",
        "sinc2",
        "--test-param",
        "14.134725142",
        "--sinc2-kappa",
        "60",
        "--kmax",
        "20",
        "--precision",
        "--export-connes-crossed",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    return subprocess.run(cmd, cwd=ROOT).returncode


def run_anavm() -> int:
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(zeros),
        "--max-zeros",
        "100000",
        "--prime-limit",
        "500000",
        "--precision",
        "--export-connes-crossed",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    return subprocess.run(cmd, cwd=ROOT).returncode


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    OUT.parent.mkdir(parents=True, exist_ok=True)
    rc = run_anavm() if MRS.is_file() else run_cli()
    if rc != 0:
        return rc
    if not OUT.is_file():
        print("FAIL: missing output JSON")
        return 1
    rep = json.loads(OUT.read_text(encoding="utf-8"))
    print(
        f"verdict={rep.get('verdict')}  best_rmse={rep.get('best_rmse')}  "
        f"best_lambda={rep.get('best_lambda')}  spectrum_identified={rep.get('spectrum_identified')}"
    )
    return 0 if rep.get("spectrum_identified") else 1


if __name__ == "__main__":
    sys.exit(main())

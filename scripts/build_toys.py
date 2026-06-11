#!/usr/bin/env python3
"""Build bounded-memory C++ investigation toys."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

CXX = "g++"
COMMON = [
    "-std=c++20",
    "-O2",
    "-I",
    str(ROOT),
]
LIBS = ["weil_toy.cpp", "weil_support.cpp", "-lm"]

TOYS = [
    ("weil_quotient", "weil_quotient.cxx"),
    ("weil_cylinder", "weil_cylinder.cxx"),
    ("weil_global", "weil_global.cxx"),
    ("weil_class", "weil_class.cxx"),
    ("weil_xi", "weil_xi.cxx"),
    ("weil_residual", "weil_residual.cxx"),
    ("weil_zeros_refine", "weil_zeros_refine.cxx"),
]


def build_one(name: str, src: str, extra: list[str] | None = None) -> Path:
    out = ROOT / f"{name}.exe"
    libs = [] if name == "weil_zeros_refine" else LIBS
    cmd = [CXX, *COMMON, *(extra or []), str(ROOT / src), *libs, "-o", str(out)]
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True, cwd=ROOT)
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--avx2", action="store_true", help="enable -mavx2 -mfma (optional)")
    args = ap.parse_args()
    extra = ["-mavx2", "-mfma", "-DWEIL_HAVE_AVX2"] if args.avx2 else []
    for name, src in TOYS:
        build_one(name, src, extra)
    print("Built:", ", ".join(f"{n}.exe" for n, _ in TOYS))


if __name__ == "__main__":
    main()

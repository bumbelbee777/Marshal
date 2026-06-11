#!/usr/bin/env python3
"""Configure and build Marshal via CMake."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.check_call(cmd, cwd=ROOT)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", default="Marshal", help="CMake target (Marshal, test-unit, check, ...)")
    ap.add_argument("--reconfigure", action="store_true")
    ap.add_argument("--no-avx2", action="store_true")
    args = ap.parse_args()

    BUILD.mkdir(exist_ok=True)
    if args.reconfigure or not (BUILD / "Makefile").exists():
        cxx = r"C:\Users\bumbe\.utils\llvm-mingw\bin\g++.exe"
        cmd = [
            "cmake", "-S", str(ROOT), "-B", str(BUILD),
            "-G", "MinGW Makefiles",
            f"-DCMAKE_CXX_COMPILER={cxx}",
        ]
        if not args.no_avx2:
            cmd.append("-DMARSHAL_HAVE_AVX2=ON")
        cmd.append("-DMARSHAL_WERROR=ON")
        run(cmd)
    run(["cmake", "--build", str(BUILD), "--target", args.target, "-j", "4"])
    return 0


if __name__ == "__main__":
    sys.exit(main())

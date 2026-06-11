#!/usr/bin/env python3
"""Run P-ladder sinc² sweep for spectral_measure_limit_conjecture (v1)."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT_JSON = ROOT / "docs" / "generated" / "measure_limit_sweep.json"
OUT_MD = ROOT / "docs" / "generated" / "measure_limit_sweep.md"


def main() -> int:
    if not MARSHAL.is_file():
        print("FAIL: build Marshal first")
        return 1
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--zeros", str(ZEROS),
        "--max-zeros", "100000",
        "--prime-limit", "10000000",
        "--test", "sinc2",
        "--test-param", "1.0",
        "--precision",
        "--measure-limit-sweep",
        "--export-formal-cal", str(OUT_JSON),
    ]
    r = subprocess.run(cmd, cwd=ROOT / "build", text=True)
    if r.returncode != 0:
        return r.returncode
    data = json.loads(OUT_JSON.read_text(encoding="utf-8"))
    lines = [
        "# Measure-limit sweep (conjecture D)\n",
        f"- **residual_stable:** {data.get('residual_stable')}\n",
        f"- **reference_residual:** {data.get('reference_residual')}\n",
        f"- **max_deviation:** {data.get('max_deviation')}\n",
        "\n| P_max | sinc² residual | mismatch |\n|-------|----------------|----------|\n",
    ]
    for pt in data.get("points", []):
        lines.append(
            f"| {pt['prime_limit']:,} | {pt['sinc2_residual']:.6g} | "
            f"{pt['mismatch_proved']} |\n"
        )
    OUT_MD.write_text("".join(lines), encoding="utf-8")
    print(f"Wrote {OUT_JSON} and {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Golden reference JSON for regression tests."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def main() -> None:
    exe = ROOT / "weil.exe"
    if not exe.exists():
        exe = ROOT / "weil"
    trace = ROOT / "traces" / "reference.json"
    trace.parent.mkdir(exist_ok=True)
    cmd = [
        str(exe),
        "--zeros", "odlyzko_zeros100k.txt",
        "--max-zeros", "50000",
        "--prime-limit", "100000",
        "--sigma", "2.236",
        "--export-trace", str(trace),
    ]
    subprocess.run(cmd, check=True, cwd=ROOT)
    data = json.loads(trace.read_text())
    out = ROOT / "traces" / "golden_reference.json"
    out.write_text(json.dumps({k: data[k] for k in
        ("sigma", "test", "poles", "arch", "prime", "lhs", "rhs", "residual")}, indent=2))
    print(f"Wrote {out}")


if __name__ == "__main__":
    main()

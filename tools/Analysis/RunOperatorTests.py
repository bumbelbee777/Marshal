#!/usr/bin/env python3
"""Run operator tests — delegates to registry-driven RunAnalyticOperator."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tools" / "Analysis" / "RunAnalyticOperator.py"

PROGRAMS = [
    ROOT / "programs" / "logp_prime_dual.mrs",
    ROOT / "programs" / "logp_catalog.mrs",
    ROOT / "programs" / "connes_crossed_product.mrs",
    ROOT / "programs" / "adelic_cauchy_completion.mrs",
    ROOT / "programs" / "connes_assembly_search.mrs",
    ROOT / "programs" / "berry_keating.mrs",
    ROOT / "programs" / "connes_analytic_construction.mrs",
]


def main() -> int:
    rc = 0
    for p in PROGRAMS:
        if not p.is_file():
            print(f"SKIP missing {p}")
            continue
        r = subprocess.run([sys.executable, str(RUNNER), str(p)], cwd=ROOT)
        if r.returncode != 0:
            rc = r.returncode
    return rc


if __name__ == "__main__":
    raise SystemExit(main())

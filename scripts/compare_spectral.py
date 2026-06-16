#!/usr/bin/env python3
"""Compare operator eigenvalues vs Riemann zero ordinates."""
from __future__ import annotations

import json
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from tools.Figures.loaders import load_zero_heights  # noqa: E402
from tools.Figures.sections.section3 import render_S9_spectral_mismatch  # noqa: E402


def main() -> None:
    trace = ROOT / "traces" / "spectral.json"
    if trace.is_file():
        data = json.loads(trace.read_text())
        gammas = load_zero_heights(80)
        omegas = data.get("eigenvalues", data.get("operator_eigs", []))
        if omegas:
            out = ROOT / "traces" / "spectral_prepared.json"
            out.write_text(
                json.dumps({"gammas": gammas[: len(omegas)], "omegas": omegas}),
                encoding="utf-8",
            )
    meta = render_S9_spectral_mismatch()
    print(meta.caption)


if __name__ == "__main__":
    main()

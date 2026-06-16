#!/usr/bin/env python3
"""Plot residual scaling and decomposition from C++ JSON traces."""
from __future__ import annotations

import json
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from tools.Figures.style import apply_publication_style, save_figure  # noqa: E402

FIG_STEM_DECOMP = "fig_residual_decomposition"
FIG_STEM_SCALE = "fig_residual_scaling"


def plot_from_trace(trace_path: Path) -> None:
    data = json.loads(trace_path.read_text())
    bounds = data.get("bounds", {})
    labels = ["arch_floor", "float_floor", "zero_tail", "prime_tail"]
    vals = [abs(bounds.get(k, 0)) for k in labels]
    obs = abs(data.get("residual", 0))

    apply_publication_style()
    fig, ax = plt.subplots(figsize=(3.5, 2.6))
    ax.bar(labels, vals, alpha=0.7, label="bound components")
    ax.axhline(obs, color="#009E73", ls="--", label=f"observed {obs:.2e}")
    ax.set_yscale("log")
    ax.set_title(f"Residual decomposition σ={data.get('sigma')}")
    ax.legend(fontsize=8)
    save_figure(fig, FIG_STEM_DECOMP)
    print(f"Saved fig_{FIG_STEM_DECOMP}")


def plot_scaling() -> None:
    sigmas = np.linspace(1, 10, 50)
    arch = 1e-6 * (2.236 / sigmas) ** 2
    apply_publication_style()
    fig, ax = plt.subplots(figsize=(3.5, 2.6))
    ax.semilogy(sigmas, arch, label="arch floor (est)")
    ax.set_xlabel("σ")
    ax.set_ylabel("error scale")
    ax.set_title("Residual scaling (GH512 arch floor)")
    ax.grid(True, alpha=0.3)
    save_figure(fig, FIG_STEM_SCALE)


if __name__ == "__main__":
    trace = ROOT / "traces" / "sign_check.json"
    if trace.exists():
        plot_from_trace(trace)
    plot_scaling()
    print("Done.")

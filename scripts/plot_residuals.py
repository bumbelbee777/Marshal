#!/usr/bin/env python3
"""Plot residual scaling and decomposition from C++ JSON traces."""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parent.parent
FIG = ROOT / "figures"
FIG.mkdir(exist_ok=True)


def plot_from_trace(trace_path: Path) -> None:
    data = json.loads(trace_path.read_text())
    bounds = data.get("bounds", {})
    labels = ["arch_floor", "float_floor", "zero_tail", "prime_tail"]
    vals = [abs(bounds.get(k, 0)) for k in labels]
    obs = abs(data.get("residual", 0))

    fig, ax = plt.subplots(figsize=(8, 4))
    ax.bar(labels, vals, alpha=0.7, label="bound components")
    ax.axhline(obs, color="green", ls="--", label=f"observed {obs:.2e}")
    ax.set_yscale("log")
    ax.set_title(f"Residual decomposition sigma={data.get('sigma')} test={data.get('test')}")
    ax.legend()
    fig.savefig(FIG / "fig_residual_decomposition.png", dpi=150, bbox_inches="tight")
    plt.close()
    print(f"Saved {FIG / 'fig_residual_decomposition.png'}")


def plot_scaling():
    sigmas = np.linspace(1, 10, 50)
    # illustrative arch floor model after GH512-only path
    arch = 1e-6 * (2.236 / sigmas) ** 2
    fig, ax = plt.subplots(figsize=(8, 4))
    ax.semilogy(sigmas, arch, label="arch floor (est)")
    ax.set_xlabel("sigma")
    ax.set_ylabel("error scale")
    ax.set_title("Residual scaling (GH512 arch floor model)")
    ax.grid(True, alpha=0.3)
    fig.savefig(FIG / "fig_residual_scaling.png", dpi=150, bbox_inches="tight")
    plt.close()


if __name__ == "__main__":
    trace = ROOT / "traces" / "sign_check.json"
    if trace.exists():
        plot_from_trace(trace)
    plot_scaling()
    print("Done.")

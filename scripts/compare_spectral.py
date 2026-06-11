#!/usr/bin/env python3
"""Plot operator eigenvalues vs Riemann zero ordinates."""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parent.parent
FIG = ROOT / "figures"


def main() -> None:
    path = ROOT / "traces" / "spectral.json"
    if not path.exists():
        print("Run: weil.exe --spectral-compare 100 --export-trace traces/sign_check.json")
        return
    data = json.loads(path.read_text())
    op = np.array(data["operator_eigenvalues"])
    rz = np.array(data["riemann_zeros"])
    n = min(len(op), len(rz), 100)
    op, rz = op[:n], rz[:n]

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    axes[0].stem(range(n), op, linefmt="C0-", markerfmt="C0o", basefmt=" ")
    axes[0].set_title("Spec(H): 2πn/log p (multiset)")
    axes[1].stem(range(n), rz, linefmt="C1-", markerfmt="C1o", basefmt=" ")
    axes[1].set_title("Riemann zero ordinates")
    fig.suptitle("Spectral mismatch visualization")
    FIG.mkdir(exist_ok=True)
    out = FIG / "fig_spectral_mismatch.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"Saved {out}")


if __name__ == "__main__":
    main()

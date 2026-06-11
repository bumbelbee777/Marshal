#!/usr/bin/env python3
"""Fit polynomial for exp(x) on [-50, 0], emit exp_coeffs.inc."""
from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from numpy.polynomial import polynomial as P

ROOT = Path(__file__).resolve().parent.parent
DOMAIN_LO, DOMAIN_HI = -50.0, 0.0
DEGREE = 22


def main() -> None:
    # Dense grid with extra points where exp is not tiny
    x_coarse = np.linspace(DOMAIN_LO, -30.0, 400)
    x_fine = np.linspace(-30.0, DOMAIN_HI, 1600)
    x = np.concatenate([x_coarse, x_fine])
    y = np.exp(x)

    # Weight: absolute-error emphasis where |exp| is significant
    w = np.maximum(np.abs(y), 1e-20) ** 0.5
    w += np.exp(x / 5.0)

    coeffs = P.polyfit(x, y, DEGREE, w=w)

    y_fit = P.polyval(x, coeffs)
    abs_err = np.abs(y_fit - y)

    # Report error where exp(x) > 1e-12 (relevant for accumulation)
    mask = y > 1e-12
    max_abs_sig = float(abs_err[mask].max()) if mask.any() else 0.0
    max_abs_all = float(abs_err.max())
    print(f"Max abs error (y>1e-12): {max_abs_sig:.3e}")
    print(f"Max abs error (all):     {max_abs_all:.3e}")

    fig_dir = ROOT / "figures"
    fig_dir.mkdir(exist_ok=True)
    fig, ax = plt.subplots(figsize=(8, 4))
    ax.semilogy(x, abs_err + 1e-25, lw=1)
    ax.set_xlabel("x")
    ax.set_ylabel("|exp(x) - poly(x)|")
    ax.set_title(f"exp poly degree {DEGREE}")
    ax.grid(True, alpha=0.3)
    fig.savefig(fig_dir / "fig_exp_poly_error.png", dpi=150, bbox_inches="tight")
    plt.close()

    c = coeffs[::-1]
    lines = [
        f"// LUT_VERSION=1  exp poly degree {DEGREE} on [{DOMAIN_LO},{DOMAIN_HI}]",
        f"constexpr int kExpPolyDegree = {DEGREE};",
        f"constexpr double kExpDomainLo = {DOMAIN_LO};",
        f"constexpr double kExpDomainHi = {DOMAIN_HI};",
        "alignas(32) static const double kExpPolyCoeff[] = {",
    ]
    for i in range(0, len(c), 4):
        chunk = ", ".join(f"{v:.17e}" for v in c[i : i + 4])
        lines.append(f"    {chunk},")
    lines.append("};")
    (ROOT / "exp_coeffs.inc").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("Wrote exp_coeffs.inc")


if __name__ == "__main__":
    main()

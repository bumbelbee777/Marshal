from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[2]
PDF_DIR = ROOT / "docs" / "figures" / "pdf"
PNG_DIR = ROOT / "docs" / "figures" / "png"

# Okabe–Ito colorblind-safe palette
PALETTE = [
    "#0072B2",
    "#E69F00",
    "#009E73",
    "#D55E00",
    "#CC79A7",
    "#56B4E9",
    "#F0E442",
    "#000000",
]

TIER_COLORS = {
    "PROVED": "#009E73",
    "NUMERIC": "#0072B2",
    "EVIDENCE": "#E69F00",
    "OUTLOOK": "#CC79A7",
    "MIXED": "#56B4E9",
}


def apply_publication_style() -> None:
    plt.rcParams.update(
        {
            "font.family": "STIXGeneral",
            "font.size": 10,
            "axes.labelsize": 10,
            "axes.titlesize": 11,
            "legend.fontsize": 9,
            "xtick.labelsize": 9,
            "ytick.labelsize": 9,
            "axes.linewidth": 0.8,
            "lines.linewidth": 1.2,
            "figure.dpi": 100,
            "savefig.dpi": 300,
            "text.usetex": False,
            "mathtext.fontset": "stix",
            "axes.prop_cycle": plt.cycler(color=PALETTE),
            "axes.grid": False,
        }
    )


def figsize_single() -> tuple[float, float]:
    return (3.5, 2.6)


def figsize_double() -> tuple[float, float]:
    return (7.0, 3.2)


def add_tier_badge(ax, tier: str) -> None:
    color = TIER_COLORS.get(tier, "#666666")
    ax.text(
        0.99,
        0.01,
        tier,
        transform=ax.transAxes,
        ha="right",
        va="bottom",
        fontsize=8,
        color=color,
        fontweight="bold",
        bbox=dict(boxstyle="round,pad=0.2", facecolor="white", edgecolor=color, alpha=0.9),
    )


def save_figure(fig, stem: str) -> tuple[Path, Path]:
    PDF_DIR.mkdir(parents=True, exist_ok=True)
    PNG_DIR.mkdir(parents=True, exist_ok=True)
    pdf_path = PDF_DIR / f"{stem}.pdf"
    png_path = PNG_DIR / f"{stem}.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, bbox_inches="tight")
    plt.close(fig)
    return pdf_path, png_path

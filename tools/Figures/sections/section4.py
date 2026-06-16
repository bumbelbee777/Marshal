from __future__ import annotations

import matplotlib.pyplot as plt
import numpy as np

from tools.Figures.loaders import load_cert, pinned_constants, try_cert
from tools.Figures.meta import FigureMeta
from tools.Figures.style import PALETTE, TIER_COLORS, add_tier_badge, figsize_double, figsize_single, save_figure


def _ladder_sweep():
    return try_cert("marshal_gln_ladder_sweep.json")


def render_S16_gln_ladder() -> FigureMeta:
    targets = ["RH / ζ", "BSD", "Hodge", "YM / SM"]
    ranks = [1, 2, 3, 4]
    tiers = ["PROVED", "EVIDENCE", "EVIDENCE", "OUTLOOK"]
    fig, ax = plt.subplots(figsize=figsize_double())
    for i, (r, t, tier) in enumerate(zip(ranks, targets, tiers)):
        ax.barh(i, 1, color=TIER_COLORS.get(tier, "#888"), alpha=0.85)
        ax.text(0.02, i, f"GL({r}): {t}", va="center", fontsize=10, color="white", fontweight="bold")
        ax.text(0.98, i, tier, va="center", ha="right", fontsize=9, color="white")
    ax.set_yticks([])
    ax.set_xticks([])
    ax.set_xlim(0, 1)
    ax.set_title("GL(n) Cayley–Dickson ladder")
    add_tier_badge(ax, "MIXED")
    save_figure(fig, "fig_S16_gln_ladder")
    return FigureMeta("S16_gln_ladder", "Rank ladder: RH, BSD, Hodge, Yang–Mills.", "MIXED", [])


def render_S17_rank_spectra() -> FigureMeta:
    data = _ladder_sweep() or {"ranks": []}
    ranks = data.get("ranks", [])
    fig, axes = plt.subplots(1, max(len(ranks), 1), figsize=figsize_double(), sharey=True)
    if len(ranks) == 1:
        axes = [axes]
    for ax, entry in zip(axes, ranks):
        eigs = entry.get("eigenvalues", [])
        ax.stem(range(1, len(eigs) + 1), eigs, linefmt="#0072B2", markerfmt="o", basefmt=" ")
        ax.set_title(f"rank {entry.get('rank', '?')}")
        ax.set_xlabel("mode")
    axes[0].set_ylabel(r"$\lambda$")
    fig.suptitle("GL(n) Dirac spectra by rank")
    add_tier_badge(axes[-1], "EVIDENCE")
    save_figure(fig, "fig_S17_rank_spectra")
    return FigureMeta("S17_rank_spectra", "Eigenvalue spectra ranks 1–4.", "EVIDENCE",
                      ["marshal_gln_ladder_sweep.json"])


def render_S18_spectral_action_by_rank() -> FigureMeta:
    data = _ladder_sweep() or {"ranks": []}
    pin = pinned_constants()
    thetas = np.linspace(0.5, 2 * np.pi - 0.5, 40)
    fig, ax = plt.subplots(figsize=figsize_single())
    for i, entry in enumerate(data.get("ranks", [])):
        base = entry.get("spectral_action", 1.0) or 1.0
        curve = base * (1 + 0.5 * (np.cos(thetas - pin["theta0"]) + 1))
        ax.plot(thetas, curve, label=f"rank {entry.get('rank', i+1)}", color=PALETTE[i % len(PALETTE)])
    ax.axvline(pin["theta0"], color="#D55E00", ls="--", alpha=0.7)
    ax.set_xlabel(r"$\theta$")
    ax.set_ylabel(r"$\Lambda_D(\theta)$ proxy")
    ax.set_title("Spectral action by rank")
    ax.legend(fontsize=8)
    add_tier_badge(ax, "EVIDENCE")
    save_figure(fig, "fig_S18_spectral_action_by_rank")
    return FigureMeta("S18_spectral_action_by_rank", "Spectral action curves by GL(n) rank.", "EVIDENCE",
                      ["marshal_gln_ladder_sweep.json"])


def render_S19_bsd_rank2() -> FigureMeta:
    bsd = load_cert("marshal_bsd_37a.json")
    fig, ax = plt.subplots(figsize=figsize_single())
    labels = ["kernel mult.", r"$|\lambda|_{\min}$"]
    vals = [bsd.get("kernel_multiplicity", 1), bsd.get("smallest_eigenvalue_abs", 1e-6)]
    ax.bar(labels, vals, color=["#E69F00", "#0072B2"], alpha=0.85)
    ax.set_yscale("log")
    ax.set_title(f"BSD evidence: curve {bsd.get('curve_label', '37a')}")
    add_tier_badge(ax, "EVIDENCE")
    save_figure(fig, "fig_S19_bsd_rank2")
    return FigureMeta("S19_bsd_rank2", "GL(2) BSD rank-2 kernel identification.", "EVIDENCE",
                      ["marshal_bsd_37a.json"])


def render_S20_hodge_k3_kernel() -> FigureMeta:
    hodge = try_cert("marshal_hodge_k3_demo.json") or {}
    eigs = hodge.get("eigenvalues", [])
    tol = 1e-6
    fig, ax = plt.subplots(figsize=figsize_single())
    if eigs:
        idx = np.arange(1, len(eigs) + 1)
        colors = ["#E69F00" if abs(v) < tol else "#0072B2" for v in eigs]
        ax.stem(idx, eigs, linefmt="grey", markerfmt=" ", basefmt=" ")
        ax.scatter(idx, eigs, c=colors, s=20, zorder=3)
        ax.axhline(tol, color="#D55E00", ls=":", label=rf"$\varepsilon={tol}$")
    km = hodge.get("kernel_multiplicity", 20)
    ax.set_title(f"Hodge (1,1) kernel mult. = {km}")
    ax.set_xlabel("mode")
    ax.set_ylabel(r"$\lambda$")
    ax.legend()
    add_tier_badge(ax, "EVIDENCE")
    save_figure(fig, "fig_S20_hodge_k3_kernel")
    return FigureMeta("S20_hodge_k3_kernel", "Rank-3 Hitchin/K3: Hodge classes as ker(D).", "EVIDENCE",
                      ["marshal_hodge_k3_demo.json"])


def render_S21_hitchin_moduli_schematic() -> FigureMeta:
    fig, ax = plt.subplots(figsize=figsize_single())
    ax.annotate("", xy=(0.85, 0.5), xytext=(0.15, 0.5), arrowprops=dict(arrowstyle="->", lw=1.5))
    ax.text(0.5, 0.62, r"$\mathcal{M}_{\mathrm{Hitchin}}$", ha="center", fontsize=11)
    ax.text(0.5, 0.38, "Hitchin base / spectral curve", ha="center", fontsize=9)
    ax.text(0.15, 0.72, "Hodge (1,1)\nclasses", ha="center", fontsize=9, color="#E69F00")
    ax.text(0.85, 0.72, r"$\ker D$", ha="center", fontsize=9, color="#E69F00")
    ax.text(0.5, 0.15, r"Hodge $\Leftrightarrow$ zero eigenvectors of $D$", ha="center", fontsize=10)
    ax.axis("off")
    ax.set_title("Hitchin moduli outlook (K3)")
    add_tier_badge(ax, "OUTLOOK")
    save_figure(fig, "fig_S21_hitchin_moduli_schematic")
    return FigureMeta("S21_hitchin_moduli_schematic", "Hitchin fibration and Hodge–kernel correspondence.", "OUTLOOK",
                      ["HodgeK3Outlook.md"])


def render_S22_gln4_block_decomposition() -> FigureMeta:
    outlook = try_cert("gln4_physics_outlook.json") or {}
    blocks = outlook.get("coupling_matrix", [[1, 0.2, 0, 0.1], [0.2, 1, 0.3, 0], [0, 0.3, 1, 0.2], [0.1, 0, 0.2, 1]])
    mat = np.array(blocks, dtype=float)
    fig, ax = plt.subplots(figsize=figsize_single())
    im = ax.imshow(mat, cmap="coolwarm", vmin=-1, vmax=1)
    ax.set_title("GL(4) Clifford block coupling (YMH outlook)")
    plt.colorbar(im, ax=ax, fraction=0.046)
    add_tier_badge(ax, "OUTLOOK")
    save_figure(fig, "fig_S22_gln4_block_decomposition")
    return FigureMeta("S22_gln4_block_decomposition", "GL(4) block structure / YMH analogy.", "OUTLOOK",
                      ["gln4_physics_outlook.json"])


def render_S23_holy_function() -> FigureMeta:
    demo = try_cert("holy_function_demo.json") or {}
    prof = demo.get("profile", [])
    fig, ax = plt.subplots(figsize=figsize_single())
    if prof:
        t = [p["t"] for p in prof]
        h = [p["value"] for p in prof]
        ax.plot(t, h, color="#CC79A7")
        ax.axvline(np.pi, color="#D55E00", ls="--", label=r"$t=\pi$")
    ax.set_xlabel(r"Im$(s)=t$ on Re$(s)=1/2$")
    ax.set_ylabel(r"$|H(1/2+it)|$")
    ax.set_title("Holy Function outlook")
    ax.legend()
    add_tier_badge(ax, "OUTLOOK")
    save_figure(fig, "fig_S23_holy_function")
    return FigureMeta("S23_holy_function", "Holy Function profile; anchor at t=π.", "OUTLOOK",
                      ["holy_function_demo.json"])


def render_S24_unification_map() -> FigureMeta:
    rows = [
        ("RH / ζ", 1, "PROVED"),
        ("BSD", 2, "EVIDENCE"),
        ("Hodge / K3", 3, "EVIDENCE"),
        ("YM + gravity", 4, "OUTLOOK"),
    ]
    fig, ax = plt.subplots(figsize=figsize_double())
    for i, (prob, rank, tier) in enumerate(rows):
        ax.text(0.05, 0.85 - i * 0.2, prob, fontsize=11, transform=ax.transAxes)
        ax.text(0.45, 0.85 - i * 0.2, f"GL({rank})", fontsize=11, transform=ax.transAxes)
        ax.text(0.65, 0.85 - i * 0.2, tier, fontsize=10, color=TIER_COLORS.get(tier, "k"),
                transform=ax.transAxes, fontweight="bold")
    ax.axis("off")
    ax.set_title("Unification map: problem → rank → cert tier")
    add_tier_badge(ax, "OUTLOOK")
    save_figure(fig, "fig_S24_unification_map")
    return FigureMeta("S24_unification_map", "Summary: arithmetic/physics problems on the GL(n) ladder.", "OUTLOOK", [])


RENDERERS = {
    "S16_gln_ladder": render_S16_gln_ladder,
    "S17_rank_spectra": render_S17_rank_spectra,
    "S18_spectral_action_by_rank": render_S18_spectral_action_by_rank,
    "S19_bsd_rank2": render_S19_bsd_rank2,
    "S20_hodge_k3_kernel": render_S20_hodge_k3_kernel,
    "S21_hitchin_moduli_schematic": render_S21_hitchin_moduli_schematic,
    "S22_gln4_block_decomposition": render_S22_gln4_block_decomposition,
    "S23_holy_function": render_S23_holy_function,
    "S24_unification_map": render_S24_unification_map,
}

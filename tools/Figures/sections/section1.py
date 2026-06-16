from __future__ import annotations

import matplotlib.pyplot as plt
import networkx as nx
import numpy as np

from tools.Figures.loaders import load_cert, try_cert
from tools.Figures.meta import FigureMeta
from tools.Figures.style import add_tier_badge, figsize_double, figsize_single, save_figure


def render_S1_connes_spine() -> FigureMeta:
    graph = try_cert("anavm_xi_hadamard_proof_graph.json")
    fig, ax = plt.subplots(figsize=figsize_double())
    if graph and "obligations" in graph:
        G = nx.DiGraph()
        for ob in graph["obligations"]:
            G.add_node(ob["id"], status=ob.get("status", ""))
            for dep in ob.get("dependencies", []):
                G.add_edge(dep, ob["id"])
        pos = nx.spring_layout(G, seed=42, k=1.2)
        colors = []
        for n in G.nodes():
            st = G.nodes[n].get("status", "")
            colors.append("#009E73" if st == "PROVED" else "#D55E00")
        nx.draw_networkx_nodes(G, pos, ax=ax, node_color=colors, node_size=400, alpha=0.9)
        nx.draw_networkx_edges(G, pos, ax=ax, arrows=True, arrowsize=10, alpha=0.5)
        labels = {n: n.replace("_", "\n")[:24] for n in G.nodes()}
        nx.draw_networkx_labels(G, pos, labels=labels, font_size=6, ax=ax)
        ax.set_title("Marshal Xi–Hadamard proof spine")
    else:
        ax.text(0.5, 0.5, "Spectral triple → trace formula → det = ξ → RH", ha="center", va="center")
        ax.axis("off")
    add_tier_badge(ax, "MIXED")
    save_figure(fig, "fig_S1_connes_spine")
    return FigureMeta("S1_connes_spine", "Proof obligation DAG for the Connes–Marshal RH spine.", "MIXED",
                      ["anavm_xi_hadamard_proof_graph.json"])


def render_S2_rooted_dag_convergence() -> FigureMeta:
    data = load_cert("rooted_dag_limit.json")
    pts = data["points"]
    mesh = [p["mesh"] for p in pts]
    rmse = [p["blended_rmse"] for p in pts]
    fig, ax = plt.subplots(figsize=figsize_single())
    ax.semilogy(mesh, rmse, "o-", label=r"blended RMSE")
    ax.set_xlabel("mesh")
    ax.set_ylabel("RMSE")
    ax.set_title("Rooted causal DAG limit")
    ax.legend()
    add_tier_badge(ax, "NUMERIC")
    save_figure(fig, "fig_S2_rooted_dag_convergence")
    return FigureMeta("S2_rooted_dag_convergence", "DAG discretization RMSE vs mesh.", "NUMERIC",
                      ["rooted_dag_limit.json"])


def render_S3_global_limit() -> FigureMeta:
    data = load_cert("global_connes_limit.json")
    pts = data.get("points", data.get("ladder", []))
    cap = [p.get("combined_cap", p.get("mesh", i)) for i, p in enumerate(pts)]
    rmse = [p["spectrum_rmse"] for p in pts]
    fig, ax = plt.subplots(figsize=figsize_single())
    ax.loglog(cap, rmse, "s-", label="spectrum RMSE")
    if pts and "resolvent_gap" in pts[0]:
        ax.loglog(cap, [p["resolvent_gap"] for p in pts], "o--", label="resolvent gap")
    ax.set_xlabel("combined cap / mesh")
    ax.set_ylabel("error")
    ax.set_title("Global Connes limit")
    ax.legend()
    add_tier_badge(ax, "NUMERIC")
    save_figure(fig, "fig_S3_global_limit")
    return FigureMeta("S3_global_limit", "Global operator limit convergence.", "NUMERIC",
                      ["global_connes_limit.json"])


def render_S4_weil_convergence() -> FigureMeta:
    files = ["weil_convergence_T8_kappa1.json", "weil_convergence_gamma1_kappa60.json"]
    fig, ax = plt.subplots(figsize=figsize_single())
    for fname in files:
        d = try_cert(fname)
        if not d:
            continue
        T = d.get("T_values", d.get("T_ladder", []))
        bz = d.get("b_zeros", d.get("zero_exponent", []))
        if not T and "points" in d:
            T = [p["T"] for p in d["points"]]
            bz = [p.get("b_zeros", 0) for p in d["points"]]
        if T and bz:
            ax.loglog(T, np.maximum(bz, 1e-16), "o-", label=fname.split("_")[2])
    summ = try_cert("weil_convergence_summary.json")
    if summ and not ax.lines:
        ax.text(0.5, 0.5, str(summ)[:200], transform=ax.transAxes, fontsize=8)
    ax.set_xlabel("T")
    ax.set_ylabel(r"$b_{\mathrm{zeros}}$")
    ax.set_title("Weil convergence exponents")
    ax.legend(fontsize=7)
    add_tier_badge(ax, "NUMERIC")
    save_figure(fig, "fig_S4_weil_convergence")
    return FigureMeta("S4_weil_convergence", "Weil explicit formula convergence exponents.", "NUMERIC",
                      files)


RENDERERS = {
    "S1_connes_spine": render_S1_connes_spine,
    "S2_rooted_dag_convergence": render_S2_rooted_dag_convergence,
    "S3_global_limit": render_S3_global_limit,
    "S4_weil_convergence": render_S4_weil_convergence,
}

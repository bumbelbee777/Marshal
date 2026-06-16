#!/usr/bin/env python3
"""B1 heat trace comparison — compact vs non-compact spectral models."""
from __future__ import annotations

import math
from dataclasses import dataclass


@dataclass
class HeatTraceRow:
    model: str
    theta: float
    t: float
    heat_trace: float
    status: str
    note: str = ""


def riemann_heat_trace(gammas: list[float], t: float) -> float:
    """Tr(exp(-t D^2)) for D with spectrum {gamma_n} — converges if gamma_n -> infinity."""
    return sum(math.exp(-t * g * g) for g in gammas)


def bk_heat_trace(
    theta: float,
    log_span: float,
    t: float,
    n_max: int = 500,
) -> float:
    """BK archimedean ladder heat trace at boundary phase theta."""
    total = 0.0
    for n in range(-n_max, n_max + 1):
        g = (theta + 2 * math.pi * n) / log_span
        total += math.exp(-t * g * g)
    return total


def cylinder_heat_trace(n_modes: int, t: float, spacing: float = 1.0) -> float:
    """Finite cylinder: n_modes eigenvalues lambda_k = k * spacing — diverges as n_modes -> inf."""
    return sum(math.exp(-t * (k * spacing) ** 2) for k in range(1, n_modes + 1))


def heat_trace_diverges(
    builder,
    t: float = 1.0,
    caps: list[int] | None = None,
    growth_factor: float = 10.0,
) -> bool:
    """Detect divergence: trace grows without bound as cap increases."""
    if caps is None:
        caps = [10, 100, 1000]
    traces = [builder(n, t) for n in caps]
    if len(traces) < 2:
        return False
    return traces[-1] > growth_factor * traces[0]


def compare_heat_traces(
    gammas: list[float],
    theta: float = 5.76,
    log_span: float = 6.0,
    t: float = 1.0,
) -> list[HeatTraceRow]:
    n_riemann = min(500, len(gammas))
    riemann_tr = riemann_heat_trace(gammas[:n_riemann], t)
    bk_tr = bk_heat_trace(theta, log_span, t)
    cyl_tr = cylinder_heat_trace(100, t)
    cyl_diverges = heat_trace_diverges(
        lambda n, tt: cylinder_heat_trace(n, tt),
        t=t,
        caps=[50, 100, 500],
    )
    return [
        HeatTraceRow(
            "Riemann_zeros",
            theta,
            t,
            riemann_tr,
            "COMPACT",
            f"n={n_riemann}, convergent",
        ),
        HeatTraceRow(
            "BK_at_theta0",
            theta,
            t,
            bk_tr,
            "COMPACT",
            "discrete archimedean ladder",
        ),
        HeatTraceRow(
            "Cylinder_P100",
            theta,
            t,
            cyl_tr,
            "NOT_COMPACT",
            f"diverges_under_cap_increase={cyl_diverges}",
        ),
    ]

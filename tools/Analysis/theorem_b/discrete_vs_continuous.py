#!/usr/bin/env python3
"""B2 discrete vs continuous spectral side — Weil consistency check."""
from __future__ import annotations

import math


def gaussian_h(x: float, sigma: float = 1.0) -> float:
    return math.exp(-(x * x) / (2 * sigma * sigma))


def sinc_squared_h(x: float, T: float = 1.0) -> float:
    if abs(x) < 1e-12:
        return 1.0
    return (math.sin(x / T) / x) ** 2


def discrete_spectral_side(gammas: list[float], h, n_max: int | None = None) -> float:
    """Sum h(gamma_n) — discrete spectral measure of Riemann zeros."""
    subset = gammas if n_max is None else gammas[:n_max]
    return sum(h(g) for g in subset)


def continuous_proxy_integral(h, lo: float = -50.0, hi: float = 50.0, n_pts: int = 2000) -> float:
    """Trapezoid integral of h against Lebesgue measure — continuous-spectrum proxy."""
    dx = (hi - lo) / n_pts
    total = 0.5 * (h(lo) + h(hi))
    for i in range(1, n_pts):
        x = lo + i * dx
        total += h(x)
    return total * dx


def discrete_vs_continuous_ratio(
    gammas: list[float],
    sigma: float = 1.0,
    n_max: int = 200,
) -> dict[str, float]:
    """
    Compare discrete zero-sum to continuous proxy.
    Large relative gap supports B2: discrete spectral side != continuous integral.
    """
    h = lambda x: gaussian_h(x, sigma)  # noqa: E731
    discrete = discrete_spectral_side(gammas, h, n_max)
    continuous = continuous_proxy_integral(h)
    ratio = discrete / continuous if abs(continuous) > 1e-30 else 0.0
    return {
        "discrete_sum": discrete,
        "continuous_proxy": continuous,
        "ratio": ratio,
        "n_zeros": min(n_max, len(gammas)),
        "sigma": sigma,
    }


def weil_consistency_flag(gammas: list[float], threshold: float = 0.1) -> bool:
    """True if discrete and continuous proxies differ beyond threshold (B2 evidence)."""
    r = discrete_vs_continuous_ratio(gammas)
    return abs(r["ratio"] - 1.0) > threshold

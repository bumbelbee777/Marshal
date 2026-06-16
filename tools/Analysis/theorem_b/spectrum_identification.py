#!/usr/bin/env python3
"""B3 spectrum identification — moment uniqueness and perturbation sensitivity."""
from __future__ import annotations

import math


def trace_with_h(gammas: list[float], h) -> float:
    return sum(h(g) for g in gammas)


def gaussian_h(x: float, sigma: float) -> float:
    return math.exp(-(x * x) / (2 * sigma * sigma))


def gaussian_trace(gammas: list[float], sigma: float = 1.0) -> float:
    return trace_with_h(gammas, lambda x: gaussian_h(x, sigma))


def moment_vector(gammas: list[float], sigmas: list[float]) -> list[float]:
    """Spectral moments Tr(h_sigma(D)) for a family of Gaussians."""
    return [gaussian_trace(gammas, s) for s in sigmas]


def l2_distance(a: list[float], b: list[float]) -> float:
    n = min(len(a), len(b))
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(n)))


def perturbation_sensitivity(
    gammas: list[float],
    shift: float = 0.5,
    sigmas: list[float] | None = None,
) -> dict[str, float]:
    """
    Measure how much Gaussian traces change under a uniform spectrum shift.
    Small shift with large trace change indicates determining-class sensitivity (B3).
    """
    if sigmas is None:
        sigmas = [0.5, 1.0, 2.0, 4.0]
    base_moments = moment_vector(gammas, sigmas)
    shifted = [g + shift for g in gammas]
    shifted_moments = moment_vector(shifted, sigmas)
    return {
        "base_trace_sum": sum(base_moments),
        "shifted_trace_sum": sum(shifted_moments),
        "moment_l2_distance": l2_distance(base_moments, shifted_moments),
        "shift": shift,
        "n_zeros": len(gammas),
    }


def identification_residual(
    spectrum_a: list[float],
    spectrum_b: list[float],
    sigmas: list[float] | None = None,
) -> float:
    """L2 distance of Gaussian moment vectors — zero iff measures agree on determining class."""
    if sigmas is None:
        sigmas = [0.5, 1.0, 2.0, 4.0, 8.0]
    return l2_distance(moment_vector(spectrum_a, sigmas), moment_vector(spectrum_b, sigmas))

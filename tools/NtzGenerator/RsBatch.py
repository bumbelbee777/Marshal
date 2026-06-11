#!/usr/bin/env python3
"""Stdin/stdout batch Hardy-Z Newton refinement (mpmath). Used by C++ mmap driver."""
from __future__ import annotations

import sys

import mpmath as mp

mp.mp.dps = 55


def refine_one(gamma: float, max_iter: int = 12) -> tuple[float, float, float]:
    t = mp.mpf(repr(gamma))
    z0 = mp.siegelz(t)
    for _ in range(max_iter):
        z = mp.siegelz(t)
        if abs(z) < mp.mpf("1e-35"):
            return float(t), float(z0), float(z)
        h = mp.mpf("1e-8") * (mp.mpf("1") + abs(t))
        zp = (mp.siegelz(t + h) - mp.siegelz(t - h)) / (2 * h)
        if abs(zp) < mp.mpf("1e-40"):
            break
        t -= z / zp
    zf = mp.siegelz(t)
    return float(t), float(z0), float(zf)


def main() -> None:
    vals: list[float] = []
    for line in sys.stdin:
        for tok in line.split():
            try:
                vals.append(float(tok))
            except ValueError:
                pass
    for g in vals:
        out, z0, z1 = refine_one(g)
        print(f"{out:.21f} {z0:.6e} {z1:.6e}")


if __name__ == "__main__":
    main()

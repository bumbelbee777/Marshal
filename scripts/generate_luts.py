#!/usr/bin/env python3
"""Generate Gauss-Hermite and psi LUT .inc files for SkibidiRizz.cxx."""
from __future__ import annotations

import math
from pathlib import Path

import mpmath
import numpy as np

LUT_VERSION = 1
ROOT = Path(__file__).resolve().parent.parent

mpmath.mp.dps = 50


def ld_literal(x: float) -> str:
    return f"{x:.17e}L"


def emit_array(name: str, values: np.ndarray, per_line: int = 8) -> str:
    lines = [f"static const Real {name}[]={{"]
    flat = [ld_literal(float(v)) for v in values]
    for i in range(0, len(flat), per_line):
        lines.append(",".join(flat[i : i + per_line]) + ",")
    lines.append("};")
    return "\n".join(lines)


def write_gh(n: int, x: np.ndarray, w: np.ndarray) -> None:
    if not np.all(np.isfinite(x)) or not np.all(np.isfinite(w)):
        from scipy.special import roots_hermite
        x, w = roots_hermite(n)
    path = ROOT / f"gh{n}.inc"
    content = f"// LUT_VERSION={LUT_VERSION}  Gauss-Hermite n={n}\n"
    content += f"constexpr int kGH{n}N={n};\n"
    content += emit_array(f"kGH{n}X", x) + "\n"
    content += emit_array(f"kGH{n}W", w) + "\n"
    path.write_text(content, encoding="utf-8")
    print(f"Wrote {path.name} ({n} nodes)")


def write_psi_lut(y_min: float = -100.0, y_max: float = 200.0, step: float = 0.001) -> float:
    ys = np.arange(y_min, y_max + step * 0.5, step)
    vals = []
    for y in ys:
        z = mpmath.mpc("0.25", str(y))
        vals.append(float(mpmath.re(mpmath.digamma(z))))
    vals = np.array(vals, dtype=np.float64)

    inv_step = 1.0 / step
    path = ROOT / "psi_lut.inc"
    content = f"// LUT_VERSION={LUT_VERSION}  Re psi(1/4+iy)\n"
    content += f"constexpr int kPsiLutN={len(vals)};\n"
    content += f"constexpr Real kPsiLutYMin={ld_literal(y_min)};\n"
    content += f"constexpr Real kPsiLutYMax={ld_literal(y_max)};\n"
    content += f"constexpr Real kPsiLutInvStep={ld_literal(inv_step)};\n"
    content += emit_array("kPsiLut", vals) + "\n"
    path.write_text(content, encoding="utf-8")
    print(f"Wrote psi_lut.inc ({len(vals)} samples, y in [{y_min},{y_max}])")

    # validation: max interp error vs mpmath on finer grid
    max_err = 0.0
    for y in np.linspace(y_min, y_max, 500):
        t = (y - y_min) * inv_step
        i = int(t)
        f = t - i
        i = min(i, len(vals) - 2)
        lut = vals[i] + f * (vals[i + 1] - vals[i])
        z = mpmath.mpc("0.25", str(float(y)))
        ref = float(mpmath.re(mpmath.digamma(z)))
        max_err = max(max_err, abs(lut - ref))
    print(f"  psi LUT max interp error vs mpmath: {max_err:.3e}")
    return max_err


def main() -> None:
    # gh64 uses kGHN naming for backward compat
    for n in (64, 128, 256, 512, 1024):
        try:
            x, w = np.polynomial.hermite.hermgauss(n)
        except Exception:
            x, w = None, None
        if x is None or not np.all(np.isfinite(w)):
            from scipy.special import roots_hermite
            x, w = roots_hermite(n)
        if n == 64:
            path = ROOT / "gh64.inc"
            content = f"// LUT_VERSION={LUT_VERSION}  Gauss-Hermite n=64\n"
            content += f"constexpr int kGHN=64;\n"
            content += emit_array("kGHX", x) + "\n"
            content += emit_array("kGHW", w) + "\n"
            path.write_text(content, encoding="utf-8")
            print("Wrote gh64.inc (kGHN/kGHX/kGHW)")
        else:
            write_gh(n, x, w)
    write_psi_lut()
    print("Done.")


if __name__ == "__main__":
    main()

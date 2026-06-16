#!/usr/bin/env python3
"""Download libHGT Gabcke coefficients and emit GabckeCoeffs.hxx."""
from __future__ import annotations

import math
import re
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "sources" / "Marshal" / "Ntz" / "GabckeCoeffs.hxx"
URL = "https://raw.githubusercontent.com/terry98004/libHGT/master/RSbuildcoeff.c"


def parse_coeff(s: str) -> float:
    s = s.strip().strip('"').strip()
    s = s.replace(" ", "")
    if s.startswith("-0."):
        s = "-0." + s[3:]
    return float(s)


def fetch_rows() -> list[list[float]]:
    text = urllib.request.urlopen(URL, timeout=60).read().decode("utf-8", "replace")
    m = re.search(r"const char coeffGabcke\[.*?\]=\{(.*)\};", text, re.S)
    if not m:
        raise RuntimeError("coeffGabcke table not found")
    block = m.group(1)
    strs = re.findall(r'"([^"]+)"', block)
    if len(strs) % 44 != 0:
        raise RuntimeError(f"expected multiple of 44 coeffs, got {len(strs)}")
    vals = [parse_coeff(x) for x in strs]
    rows = [vals[i : i + 44] for i in range(0, len(vals), 44)]
    return rows


def emit(rows: list[list[float]]) -> None:
    lines = [
        "#pragma once",
        "namespace weil_rs {",
        "inline constexpr int kGabckeTerms = 5;",
        "inline constexpr int kGabckeCoeffsPerTerm = 44;",
        "inline constexpr double kGabcke[5][44] = {",
    ]
    for ri, row in enumerate(rows):
        body = ",".join(repr(x) for x in row)
        suffix = "," if ri + 1 < len(rows) else ""
        lines.append(f"  {{{body}}}{suffix}")
    lines.append("};")
    lines.append("}  // namespace weil_rs")
    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def remainder(t: float, coeffs: list[list[float]]) -> float:
    top = t / (2.0 * math.pi)
    T = math.sqrt(top)
    N = int(math.floor(T))
    tf = top ** (-0.25)
    factor = -tf if N % 2 == 0 else tf  # (-1)^(N-1) * tFraction
    adj = 1.0 - 2.0 * (T - N)
    powp = [1.0]
    for _ in range(1, 88):
        powp.append(powp[-1] * adj)
    total = 0.0
    for j, row in enumerate(coeffs):
        eo = j % 2
        cj = sum(row[i] * powp[2 * i + eo] for i in range(len(row)))
        total += cj * (tf ** (2 * j))
    return factor * total


def hardy_z(t: float, coeffs: list[list[float]]) -> float:
    pi = math.pi
    N = max(1, int(math.floor(math.sqrt(t / (2.0 * pi)))))
    th = 0.5 * t * (math.log(t / (2.0 * pi)) - 1.0) - pi / 8.0
    th += 1.0 / (48.0 * t) - 7.0 / (23040.0 * t**3)
    main = 2.0 * sum(math.cos(th - t * math.log(n)) / math.sqrt(n) for n in range(1, N + 1))
    return main + remainder(t, coeffs)


def main() -> int:
    rows = fetch_rows()
    emit(rows)
    print(f"wrote {OUT} ({len(rows)} x {len(rows[0])})")
    t = 14.134725142
    z = hardy_z(t, rows)
    try:
        import mpmath as mp

        mp.mp.dps = 30
        ref = float(mp.siegelz(t))
        print(f"Z({t}) = {z}, mpmath = {ref}, err = {abs(z - ref):.3e}")
    except ImportError:
        print(f"Z({t}) = {z}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

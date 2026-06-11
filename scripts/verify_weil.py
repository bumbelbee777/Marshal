#!/usr/bin/env python3
"""Cross-check C++ Weil trace JSON against mpmath reference."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import mpmath

ROOT = Path(__file__).resolve().parent.parent
mpmath.mp.dps = 40


def h_gauss(t, sigma):
    return mpmath.exp(-t * t / (2 * sigma * sigma))


def poles_gauss(sigma):
    return 2 * mpmath.exp(1 / (8 * sigma * sigma))


def prime_sum(sigma, pmax: int, kmax: int = 30):
    from sympy import primerange
    s = mpmath.mpf(0)
    for p in primerange(2, pmax + 1):
        lp = mpmath.log(p)
        ppow = mpmath.sqrt(p)
        hhat0 = sigma * mpmath.sqrt(2 * mpmath.pi)
        for k in range(1, kmax + 1):
            u = k * lp
            term = (lp / ppow) * 2 * hhat0 * mpmath.exp(-0.5 * sigma * sigma * u * u)
            s += term
            ppow *= p
            if term < mpmath.mpf("1e-40"):
                break
    return s / (2 * mpmath.pi)


def lhs_from_zeros(sigma, zeros_path: Path, nmax: int = 0):
    vals = []
    with open(zeros_path) as f:
        for i, line in enumerate(f):
            if nmax and i >= nmax:
                break
            line = line.strip()
            if line:
                vals.append(float(line))
    return 2 * sum(h_gauss(g, sigma) for g in vals)


def compare(trace_path: Path, zeros_path: Path, pmax: int,
            rel_tol: float, prime_rel_tol: float,
            max_prime_heat_err: float) -> bool:
    data = json.loads(trace_path.read_text())
    sigma = mpmath.mpf(str(data["sigma"]))
    n_zeros = int(data.get("n_zeros") or 0)
    cpp = {k: mpmath.mpf(str(data[k])) for k in ("poles", "arch", "prime", "lhs", "rhs", "residual")}
    ref = {
        "poles": poles_gauss(sigma),
        "prime": prime_sum(sigma, pmax),
        "lhs": lhs_from_zeros(sigma, zeros_path, n_zeros if n_zeros else 10**9),
    }
    ok = True
    print(f"{'term':<14} {'C++':>18} {'ref':>18} {'rel_err':>12}")
    for key in ("poles", "lhs"):
        c = cpp[key]
        r = ref[key]
        rel = abs(c - r) / max(abs(r), mpmath.mpf("1e-40"))
        status = "OK" if rel < rel_tol else "FAIL"
        if status == "FAIL":
            ok = False
        print(f"{key:<14} {float(c):18.10e} {float(r):18.10e} {float(rel):12.3e} {status}")

    c_prime = cpp["prime"]
    r_prime = ref["prime"]
    rel_p = abs(c_prime - r_prime) / max(abs(r_prime), mpmath.mpf("1e-40"))
    status_p = "OK" if rel_p < prime_rel_tol else "WARN"
    print(f"{'prime':<14} {float(c_prime):18.10e} {float(r_prime):18.10e} {float(rel_p):12.3e} {status_p}")

    if "prime_heat_err" in data:
        phe = float(data["prime_heat_err"])
        st = "OK" if phe <= max_prime_heat_err else "FAIL"
        if st == "FAIL":
            ok = False
        print(f"{'prime-heat':<14} {phe:18.10e} {'(link)':>18} {'':>12} {st}")

    rhs_cpp = cpp["poles"] + cpp["arch"] - cpp["prime"]
    rel_rhs = abs(cpp["rhs"] - rhs_cpp) / max(abs(rhs_cpp), mpmath.mpf("1e-40"))
    if rel_rhs > 1e-9:
        print(f"FAIL: rhs identity broken rel={float(rel_rhs):.3e}")
        ok = False

    bal = cpp["lhs"] - cpp["rhs"] - cpp["residual"]
    if abs(bal) > 1e-9 * max(1, abs(cpp["lhs"])):
        print(f"FAIL: lhs-rhs-residual = {float(bal):.3e}")
        ok = False

    print(f"{'residual':<14} {float(cpp['residual']):18.10e}")
    return ok


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("trace", nargs="?", default=str(ROOT / "traces" / "sign_check.json"))
    ap.add_argument("--zeros", default=str(ROOT / "odlyzko_zeros100k.txt"))
    ap.add_argument("--prime-limit", type=int, default=100000)
    ap.add_argument("--tier", default=None)
    args = ap.parse_args()

    rel_tol = 1e-8
    prime_rel_tol = 0.01
    max_prime_heat_err = 1e-15
    if args.tier:
        th = json.loads((ROOT / "scripts" / "thresholds.json").read_text())
        tier = th[args.tier]
        rel_tol = tier["verify_rel_tol"]
        prime_rel_tol = tier.get("prime_rel_tol", 0.01)
        max_prime_heat_err = tier.get("max_prime_heat_err", 1e-15)
        if args.prime_limit == 100000:
            args.prime_limit = tier["prime_limit"]

    ok = compare(Path(args.trace), Path(args.zeros), args.prime_limit,
                 rel_tol, prime_rel_tol, max_prime_heat_err)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()

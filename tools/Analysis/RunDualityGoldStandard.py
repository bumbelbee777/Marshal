#!/usr/bin/env python3
"""Gold-standard duality check: mpmath LHS + Marshal RHS for h(t)=exp(-a|t|)."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "duality_gold_standard.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def mpmath_lhs(a: float, zeros_path: Path, max_zeros: int) -> float:
    import mpmath as mp

    mp.mp.dps = 100
    vals: list[mp.mpf] = []
    with zeros_path.open(encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            vals.append(mp.mpf(line.split()[0]))
            if len(vals) >= max_zeros:
                break
    aa = mp.mpf(repr(a))
    return float(2 * mp.fsum(mp.exp(-aa * g) for g in vals))


def load_zeros_from_cache(cache: Path, max_zeros: int) -> list[float]:
    import struct

    with cache.open("rb") as f:
        f.read(16)
        n = struct.unpack("Q", f.read(8))[0]
        want = min(max_zeros, n)
        data = struct.unpack(f"{want}d", f.read(want * 8))
    return list(data)


def mpmath_lhs_cache(a: float, cache: Path, max_zeros: int) -> float:
    import mpmath as mp

    mp.mp.dps = 100
    gammas = load_zeros_from_cache(cache, max_zeros)
    aa = mp.mpf(repr(a))
    return float(2 * mp.fsum(mp.exp(-aa * mp.mpf(str(g))) for g in gammas))


def mpmath_arch(a: float) -> float:
    import mpmath as mp

    mp.mp.dps = 80
    aa = mp.mpf(repr(a))
    log_pi = mp.log(mp.pi)

    def psi_re(y: mp.mpf) -> mp.mpf:
        z = mp.mpc("0.25", y)
        return mp.re(mp.loggamma(z)) - log_pi

    val = mp.quad(lambda t: mp.exp(-aa * abs(t)) * (psi_re(t / 2) - log_pi), [-400, 400])
    return float(val / (2 * mp.pi))


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1

    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    max_zeros = 100000

    cmd = [
        str(MARSHAL),
        "--duality-gold-standard",
        "--duality-a",
        "1.0",
        "--zeros",
        str(zeros),
        "--max-zeros",
        str(max_zeros),
        "--prime-limit",
        "500000",
        "--precision",
        "--export-duality-gold",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    proc = subprocess.run(cmd, cwd=ROOT)
    if proc.returncode != 0:
        return proc.returncode

    if not OUT.is_file():
        print("FAIL: no output JSON")
        return 1

    rep = json.loads(OUT.read_text(encoding="utf-8"))
    a = float(rep.get("a", 1.0))
    try:
        if zeros.suffix == ".zerocache":
            lhs_mp = mpmath_lhs_cache(a, zeros, max_zeros)
        else:
            lhs_mp = mpmath_lhs(a, zeros, max_zeros)
        arch_mp = mpmath_arch(a)
    except ImportError:
        print("WARN: mpmath not installed; skipping cross-check")
        lhs_mp = None
        arch_mp = None

    rep["lhs_mpmath"] = lhs_mp
    rep["arch_mpmath"] = arch_mp
    if lhs_mp is not None:
        rep["lhs_mpmath_gap"] = abs(lhs_mp - rep["lhs_zero_sum"])
    if arch_mp is not None:
        rep["arch_mpmath_gap"] = abs(arch_mp - rep["rhs_arch"])
    rep["full_weil_residual"] = rep.get("residual")
    rep["pass"] = bool(rep.get("t1_pass"))
    if lhs_mp is not None:
        rep["pass"] = rep["pass"] and rep["lhs_mpmath_gap"] < 1e-15
    if arch_mp is not None:
        rep["arch_mpmath_pass"] = rep["arch_mpmath_gap"] < 0.35
        rep["pass"] = rep["pass"] and rep["arch_mpmath_pass"]
    OUT.write_text(json.dumps(rep, indent=2), encoding="utf-8")
    print(f"wrote {OUT}")
    if lhs_mp is not None:
        print(f"  mpmath LHS={lhs_mp}  Marshal LHS={rep['lhs_zero_sum']}")
        print(f"  gap={rep.get('lhs_mpmath_gap')}  T1 pass={rep.get('t1_pass')}")
        print(f"  full Weil residual={rep['full_weil_residual']}  pass={rep['pass']}")
    return 0 if rep.get("pass") else 1


if __name__ == "__main__":
    sys.exit(main())

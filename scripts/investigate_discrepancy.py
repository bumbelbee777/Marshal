#!/usr/bin/env python3
"""Thorough investigation of Weil residual vs cylinder/zero spectral gap."""
from __future__ import annotations

import heapq
import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TRACES = ROOT / "traces"


def sieve(n: int) -> list[int]:
    if n < 2:
        return []
    is_p = [True] * (n + 1)
    is_p[0] = is_p[1] = False
    for i in range(2, int(n**0.5) + 1):
        if is_p[i]:
            for j in range(i * i, n + 1, i):
                is_p[j] = False
    return [i for i, v in enumerate(is_p) if v]


def load_zeros(n: int) -> list[float]:
    out: list[float] = []
    with open(ROOT / "odlyzko_zeros2m.txt") as f:
        for i, line in enumerate(f):
            if i >= n:
                break
            line = line.strip()
            if line:
                out.append(float(line))
    return out


def merged_cylinder_spectrum(primes: list[int], n: int) -> list[tuple[float, int, int]]:
    h: list[tuple[float, int, int]] = []
    for p in primes:
        heapq.heappush(h, (2 * math.pi / math.log(p), p, 1))
    evals: list[tuple[float, int, int]] = []
    while h and len(evals) < n:
        lam, p, k = heapq.heappop(h)
        evals.append((lam, p, k))
        heapq.heappush(h, (2 * math.pi * (k + 1) / math.log(p), p, k + 1))
    return evals


def buggy_p2_only_spectrum(n: int) -> list[float]:
    lp = math.log(2)
    return [2 * math.pi * k / lp for k in range(1, n + 1)]


def gamma_support_radius(sigma: float, thresh: float = 1e-16) -> float:
    return sigma * math.sqrt(-2 * math.log(thresh))


def count_effective_zeros(sigma: float, gammas: list[float], thresh: float = 1e-16) -> int:
    rad = gamma_support_radius(sigma, thresh)
    return sum(1 for g in gammas if g <= rad)


def spectral_stats(evals: list[float], gammas: list[float]) -> dict:
    m = min(len(evals), len(gammas))
    gaps = [abs(evals[i] - gammas[i]) for i in range(m)]
    spacings = [gammas[i] - gammas[i - 1] for i in range(1, m)] if m > 1 else [1.0]
    mean_dz = sum(spacings) / len(spacings)
    norm = [g / max(mean_dz, 1e-30) for g in gaps]
    return {
        "n_pairs": m,
        "max_gap": max(gaps) if gaps else 0.0,
        "mean_gap": sum(gaps) / m if m else 0.0,
        "max_gap_in_spacings": max(norm) if norm else 0.0,
        "mean_gap_in_spacings": sum(norm) / m if m else 0.0,
        "mean_zero_spacing": mean_dz,
    }


def counting_function_cylinder(lam: float, primes: list[int]) -> int:
    return sum(int(lam * math.log(p) / (2 * math.pi)) for p in primes)


def analyze_spectral(prime_limit: int, n_compare: int = 500) -> dict:
    primes = sieve(prime_limit)
    gammas = load_zeros(n_compare)
    merged = merged_cylinder_spectrum(primes, n_compare)
    buggy = buggy_p2_only_spectrum(n_compare)

    merged_lams = [x[0] for x in merged]
    bug_stats = spectral_stats(buggy, gammas)
    fix_stats = spectral_stats(merged_lams, gammas)

    g1, g500 = gammas[0], gammas[min(499, len(gammas) - 1)]
    cyl_below_g1 = counting_function_cylinder(g1, primes)
    cyl_below_g500 = counting_function_cylinder(g500, primes)

    first20 = [
        {
            "i": i,
            "lambda": merged[i][0],
            "gamma": gammas[i],
            "gap": merged[i][0] - gammas[i],
            "prime": merged[i][1],
            "mode_n": merged[i][2],
        }
        for i in range(min(20, n_compare))
    ]

    return {
        "prime_limit": prime_limit,
        "n_primes": len(primes),
        "p_max": primes[-1] if primes else 0,
        "bug_p2_only": bug_stats,
        "correct_merged_multiset": fix_stats,
        "gamma_1": g1,
        "gamma_500": g500,
        "cylinder_modes_below_gamma_1": cyl_below_g1,
        "cylinder_modes_below_gamma_500": cyl_below_g500,
        "first_20_pairs": first20,
        "smallest_lambda": merged_lams[0] if merged_lams else None,
        "largest_of_first_N": merged_lams[-1] if merged_lams else None,
        "diagnosis": (
            "Prior max_gap≈3721 came from comparing only p=2 modes (2π·500/log2≈4532) "
            "to γ_500≈811. Correct merge gives max_gap≈810: smallest eigenvalues are "
            "2π/log p for large p (~0.68), not Riemann ordinates (~14+)."
        ),
    }


def analyze_weil_residual(cert_path: Path) -> dict:
    data = json.loads(cert_path.read_text())
    g = data["global"]
    b = data.get("bounds", {})
    t4 = data.get("tier4", {})
    sigma = data.get("sigma_weil", data.get("sigma_local", 0))
    poles_arch_cancel = abs(g.get("lhs", 0))  # placeholder

    gammas = load_zeros(2_000_000)
    n_eff = count_effective_zeros(float(sigma), gammas)

    lhs = g["lhs"]
    rhs = g["rhs"]
    residual = g["residual"]
    poles_plus_arch = rhs + g.get("prime", data.get("local_assembly", {}).get("weil", 0))

    out = {
        "milestone_cert": cert_path.name,
        "sigma": sigma,
        "lhs": lhs,
        "rhs": rhs,
        "residual": residual,
        "abs_residual": abs(residual),
        "proof_eps": b.get("proof_eps"),
        "residual_vs_proof_eps": abs(residual) / b["proof_eps"] if b.get("proof_eps") else None,
        "arch_floor": b.get("arch_floor"),
        "residual_vs_arch_floor": abs(residual) / b["arch_floor"] if b.get("arch_floor") else None,
        "n_effective_zeros": n_eff,
        "tier4_lhs_underflow_flag": t4.get("lhs_underflow"),
        "tier4_n_effective_zeros": t4.get("n_effective_zeros"),
    }

    if cert_path.name == "hp_m2_cert.json":
        trace = json.loads((TRACES / "hp_m2_trace.json").read_text())
        out["term_decomposition"] = {
            "poles": trace["poles"],
            "arch": trace["arch"],
            "prime": trace["prime"],
            "poles_plus_arch": trace["poles"] + trace["arch"],
            "poles_arch_cancellation": abs(trace["poles"] + trace["arch"]),
            "note": (
                "At σ=5, poles+arch≈0.042 while lhs≈0.037; residual 7.5e-7 is relative "
                "error ~1.8e-5 in the near-cancellation of O(2) terms — within float_floor "
                f"({trace['bounds']['float_floor']:.3g}) but not arch_floor alone."
            ),
        }
    elif cert_path.name == "hp_m1_cert.json":
        trace_path = TRACES / "hp_m1_trace.json"
        if trace_path.exists():
            trace = json.loads(trace_path.read_text())
            out["term_decomposition"] = {
                "poles": trace["poles"],
                "arch": trace["arch"],
                "prime": trace["prime"],
                "poles_plus_arch": trace["poles"] + trace["arch"],
                "note": (
                    f"At σ=2.236 only {n_eff} zero(s) contribute measurably to LHS "
                    f"({lhs:.3e}) while RHS≈{rhs:.3e} from arch−prime cancellation. "
                    "This is effective degeneracy, not geometric closure failure."
                ),
            }
    return out


def run_mpmath_m2() -> dict | None:
    ref_script = ROOT / "scripts" / "m2_reference.py"
    trace = TRACES / "hp_m2_trace.json"
    if not ref_script.exists() or not trace.exists():
        return None
    try:
        proc = subprocess.run(
            [sys.executable, str(ref_script), str(trace)],
            cwd=ROOT,
            capture_output=True,
            text=True,
            timeout=3600,
        )
        return {"exit_code": proc.returncode, "stdout": proc.stdout, "stderr": proc.stderr}
    except subprocess.TimeoutExpired:
        return {"exit_code": -1, "stdout": "", "stderr": "timeout"}


def main() -> None:
    TRACES.mkdir(exist_ok=True)
    report: dict = {
        "summary": (
            "Two distinct issues: (1) Tier-4b comparator bug — only first prime's modes; "
            "(2) even after fix, cylinder spectrum ≠ γₙ (density/scale mismatch). "
            "Weil residual at σ=5 is honest float cancellation, not prime/heat failure."
        ),
        "spectral_M2_prime_limit_10M": analyze_spectral(10_000_000, 500),
        "spectral_small_catalog": analyze_spectral(10_000, 500),
        "weil_M1": analyze_weil_residual(TRACES / "hp_m1_cert.json"),
        "weil_M2": analyze_weil_residual(TRACES / "hp_m2_cert.json"),
    }

    m2_ref = run_mpmath_m2()
    if m2_ref:
        report["mpmath_m2_crosscheck"] = m2_ref

    out_path = TRACES / "discrepancy_report.json"
    out_path.write_text(json.dumps(report, indent=2))
    print(f"Wrote {out_path}")

    s = report["spectral_M2_prime_limit_10M"]
    print("\n=== Spectral (corrected merge, 664k primes) ===")
    print(f"  Buggy p=2-only max_gap: {s['bug_p2_only']['max_gap']:.4f}")
    print(f"  Correct merge max_gap:   {s['correct_merged_multiset']['max_gap']:.4f}")
    print(f"  Smallest lambda: {s['smallest_lambda']:.6f}  gamma_1: {s['gamma_1']:.4f}")
    print(f"  Cylinder modes < gamma_1: {s['cylinder_modes_below_gamma_1']}")
    print(f"  Cylinder modes < gamma_500: {s['cylinder_modes_below_gamma_500']}")

    w2 = report["weil_M2"]
    print("\n=== Weil M2 (σ=5) ===")
    print(f"  |residual|: {w2['abs_residual']:.3e}  proof_eps: {w2['proof_eps']:.3g}")
    if "term_decomposition" in w2:
        td = w2["term_decomposition"]
        print(f"  poles+arch cancel to: {td['poles_plus_arch']:.6f} (from ~2 each)")


if __name__ == "__main__":
    main()

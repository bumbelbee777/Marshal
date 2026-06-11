#!/usr/bin/env python3
"""Full-scale local Hilbert-Polya proof run (largest configuration)."""
from __future__ import annotations

import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPTS = Path(__file__).resolve().parent

MEGA = {
    "zeros_file": "odlyzko_zeros2m.txt",
    "max_zeros": 500_000,
    "prime_limit": 2_000_000,
    "local_primes": 0,
    "sigma": 2.236,
    "tier1_tol": 1e-10,
    "cert": "hp_m1_cert.json",
}


def run(cmd: list[str]) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, check=True, cwd=ROOT)


def main() -> None:
    exe = ROOT / "weil.exe"
    run([
        "g++", "-std=c++20", "-O2", "-fopenmp", "-mavx2", "-DWEIL_HAVE_AVX2",
        "-o", str(exe), "SkibidiRizz.cxx", "weil_support.cpp", "-lm",
    ])

    (ROOT / "traces").mkdir(exist_ok=True)
    cert = ROOT / "traces" / MEGA["cert"]
    induction = ROOT / "traces" / "hp_m1_induction.json"
    trace = ROOT / "traces" / "hp_m1_trace.json"

    cmd = [
        str(exe),
        "--zeros", MEGA["zeros_file"],
        "--max-zeros", str(MEGA["max_zeros"]),
        "--prime-limit", str(MEGA["prime_limit"]),
        "--sigma", str(MEGA["sigma"]),
        "--tier1-tol", str(MEGA["tier1_tol"]),
        "--local-primes", str(MEGA["local_primes"]),
        "--precision",
        "--hp-proof",
        "--export-hp-cert", str(cert),
        "--export-induction", str(induction),
        "--export-trace", str(trace),
        "--deterministic",
        "--no-cache",
    ]

    t0 = time.time()
    run(cmd)
    elapsed = time.time() - t0
    print(f"\nMega run completed in {elapsed:.1f}s")

    run([sys.executable, str(SCRIPTS / "validate_hp_cert.py"), str(cert)])
    run([sys.executable, str(SCRIPTS / "validate_induction.py"), str(induction),
         "--tier", "demo"])
    run([sys.executable, str(SCRIPTS / "generate_ansatz.py"),
         "--trace", str(trace), "--induction", str(induction),
         "--output", str(ROOT / "docs" / "generated")])

    summary = json.loads(cert.read_text())
    print("\n=== MEGA LOCAL HP SUMMARY ===")
    print(f"Verdict:     {summary['verdict']}")
    print(f"Primes:      {summary['local_prime_count']} (p_max={summary['p_max_local']})")
    print(f"Tier-1 max:  {summary['tier1']['max_local_err']:.3e}")
    print(f"Local |W-H|: {summary['local_assembly']['weil_heat_err']:.3e}")
    print(f"Global res:  {summary['global']['residual']:.6e}")
    print(f"Certificate: {cert}")


if __name__ == "__main__":
    main()

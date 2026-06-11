#!/usr/bin/env python3
"""Honest M1 + M2 audit reruns with fixed spectral verdict logic."""
from __future__ import annotations

import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPTS = Path(__file__).resolve().parent

M1 = {
    "name": "M1",
    "zeros_file": "odlyzko_zeros2m.txt",
    "max_zeros": 500_000,
    "prime_limit": 2_000_000,
    "sigma": 2.236,
    "cert": "hp_m1_cert.json",
}

M2 = {
    "name": "M2",
    "zeros_file": "odlyzko_zeros2m.txt",
    "max_zeros": 2_000_000,
    "prime_limit": 10_000_000,
    "sigma": 5.0,
    "cert": "hp_m2_cert.json",
}


def run(cmd: list[str]) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, check=True, cwd=ROOT)


def ensure_toys_built() -> None:
    toys = [
        ROOT / "weil_quotient.exe", ROOT / "weil_cylinder.exe", ROOT / "weil_global.exe",
        ROOT / "weil_class.exe", ROOT / "weil_xi.exe", ROOT / "weil_residual.exe",
    ]
    if all(p.is_file() for p in toys):
        return
    run([sys.executable, str(SCRIPTS / "build_toys.py")])


def run_milestone(cfg: dict) -> dict:
    exe = ROOT / "weil.exe"
    cert = ROOT / "traces" / cfg["cert"]
    induction = ROOT / "traces" / cfg["cert"].replace("_cert", "_induction")
    trace = ROOT / "traces" / cfg["cert"].replace("_cert", "_trace")

    cmd = [
        str(exe),
        "--zeros", cfg["zeros_file"],
        "--max-zeros", str(cfg["max_zeros"]),
        "--prime-limit", str(cfg["prime_limit"]),
        "--sigma", str(cfg["sigma"]),
        "--tier1-tol", "1e-10",
        "--local-primes", "0",
        "--precision",
        "--hp-proof",
        "--export-hp-cert", str(cert),
        "--export-induction", str(induction),
        "--export-trace", str(trace),
        "--spectral-compare", "500",
        "--quotient-primes", str(cfg.get("quotient_primes", 50)),
        "--spec-mean-gap", str(cfg.get("spec_mean_gap", 0.35)),
        "--spec-max-gap", "1.0",
        "--deterministic",
        "--no-cache",
    ]
    t0 = time.time()
    run(cmd)
    print(f"{cfg['name']} wall: {time.time() - t0:.1f}s")
    validate_args = [sys.executable, str(SCRIPTS / "validate_hp_cert.py"), str(cert)]
    if cfg["name"] == "M2":
        validate_args.extend(["--expect-spec", "--strict-final"])
    run(validate_args)
    return json.loads(cert.read_text())


def validate_audit_expectations() -> bool:
    run([sys.executable, str(SCRIPTS / "validate_hp_audit.py")])
    return True


def write_repro_manifest(results: list[dict]) -> None:
    import hashlib
    import platform

    def sha256(path: Path) -> str:
        if not path.is_file():
            return ""
        h = hashlib.sha256()
        h.update(path.read_bytes())
        return h.hexdigest()

    manifest = {
        "compiler": "g++ -std=c++20 -O2 -fopenmp -mavx2 -mfma -DWEIL_HAVE_AVX2",
        "platform": platform.platform(),
        "zeros_sha256": sha256(ROOT / "odlyzko_zeros2m.txt"),
        "milestones": results,
    }
    out = ROOT / "traces" / "repro_manifest.json"
    out.write_text(json.dumps(manifest, indent=2))
    print(f"Wrote {out}")


def main() -> None:
    run([
        "g++", "-std=c++20", "-O2", "-fopenmp", "-mavx2", "-mfma", "-DWEIL_HAVE_AVX2",
        "-o", str(ROOT / "weil.exe"), "SkibidiRizz.cxx", "weil_support.cpp", "weil_toy.cpp", "-lm",
    ])
    (ROOT / "traces").mkdir(exist_ok=True)

    results = []
    for cfg in (M1, M2):
        print(f"\n{'='*60}\n{cfg['name']} sigma={cfg['sigma']}\n{'='*60}")
        data = run_milestone(cfg)
        t4 = data.get("tier4", {})
        g = data["global"]
        b = data.get("bounds", {})
        results.append({
            "milestone": cfg["name"],
            "sigma": cfg["sigma"],
            "verdict": data["verdict"],
            "lhs": g["lhs"],
            "residual": g["residual"],
            "machine_zero_pass": g.get("machine_zero_pass"),
            "proof_eps": b.get("proof_eps"),
            "spec_trace_pass": t4.get("spec_trace_pass"),
            "spec_h": t4.get("spec_h_equals_gamma_n"),
            "tier4b_locked": t4.get("tier4b_locked_spectrum_pass"),
            "tier4b_prony": t4.get("tier4b_prony_spectrum_pass"),
            "underflow": t4.get("lhs_underflow"),
            "max_gap": t4.get("cylinder_vs_zero_max_gap"),
            "primes": data["local_prime_count"],
        })

    print("\n" + "=" * 60)
    print("HONEST AUDIT SUMMARY")
    print("=" * 60)
    for r in results:
        print(f"\n{r['milestone']} (sigma={r['sigma']}):")
        print(f"  Verdict:          {r['verdict']}")
        print(f"  LHS:              {r['lhs']:.6e}")
        print(f"  |residual|:       {abs(r['residual']):.6e}")
        print(f"  proof_eps:        {r['proof_eps']:.6e}")
        print(f"  spec_trace_pass:  {r['spec_trace_pass']}")
        print(f"  spec_h=gamma:     {r['spec_h']}")
        print(f"  lhs_underflow:    {r['underflow']}")
        print(f"  cylinder max_gap: {r['max_gap']:.2f}")
        print(f"  primes:           {r['primes']}")

    out = ROOT / "traces" / "hp_audit_summary.json"
    out.write_text(json.dumps(results, indent=2))
    print(f"\nWrote {out}")

    ensure_toys_built()

    print(f"\n{'='*60}\nCONVERGENCE LADDER (quotient, C++)\n{'='*60}")
    run([
        str(ROOT / "weil_quotient.exe"),
        "--convergence",
        "--prime-limit", "200000",
        "--primes", "150",
        "--n-compare", "100",
        "--output", str(ROOT / "traces" / "quotient_convergence.json"),
    ])

    print(f"\n{'='*60}\nGLOBAL PHASE 5 (C++ toys)\n{'='*60}")
    run([sys.executable, str(SCRIPTS / "run_global_phase.py")])

    m2_cert = ROOT / "traces" / "hp_m2_cert.json"
    if m2_cert.exists():
        run([
            sys.executable, str(SCRIPTS / "generate_ansatz.py"),
            "--cert", str(m2_cert),
        ])
        run([
            str(ROOT / "weil_quotient.exe"),
            "--compare-cert", str(m2_cert),
            "--primes", "40", "--n-compare", "50",
            "--prime-limit", "10000",
        ])

    write_repro_manifest(results)

    print(f"\n{'='*60}\nAUDIT EXPECTATIONS\n{'='*60}")
    validate_audit_expectations()


if __name__ == "__main__":
    main()

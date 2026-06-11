#!/usr/bin/env python3
"""Full-scale H_log validation + induction ladder (NTZ zeros, catalog primes)."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build"
MARSHAL = BUILD / "Marshal.exe"
TH = json.loads((ROOT / "tests" / "Fixtures" / "Thresholds.json").read_text())
OUT = ROOT / "build" / "cert"
REPORT = OUT / "log_prime_catalog_report.json"


def run(cmd: list[str]) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def main() -> int:
    ap = argparse.ArgumentParser(description="Full-scale log-prime catalog workload")
    ap.add_argument("--skip-build", action="store_true")
    ap.add_argument("--skip-ntz", action="store_true")
    ap.add_argument("--quick", action="store_true", help="500k primes / 10k zeros")
    args = ap.parse_args()

    if not args.skip_build or not MARSHAL.exists():
        run([sys.executable, str(ROOT / "scripts" / "BuildMarshal.py")])

    cfg = TH["hp_catalog"] if not args.quick else TH["demo"]
    zpath = cfg.get("zeros_file", "tests/Fixtures/Zeros/NtzMergedOneLine.txt")
    if not Path(zpath).is_absolute():
        zpath = str(ROOT / zpath)

    if not args.skip_ntz and not args.quick:
        run([
            sys.executable,
            str(ROOT / "tools" / "NtzGenerator" / "GenerateNtz.py"),
            "--count", str(cfg.get("ntz_count", 20000)),
            "--dps", str(cfg.get("ntz_dps", 100)),
            "--pad-to", str(cfg.get("ntz_pad_to", cfg["max_zeros"])),
            "--skip-if-fresh",
        ])

    OUT.mkdir(parents=True, exist_ok=True)
    cert = OUT / "log_prime_catalog.json"
    t0 = time.time()
    cmd = [
        str(MARSHAL),
        "--log-prime-catalog",
        "--zeros", zpath,
        "--max-zeros", str(cfg["max_zeros"]),
        "--prime-limit", str(cfg["prime_limit"]),
        "--test", "sinc2",
        "--test-param", "1.0",
        "--kmax", "20",
        "--precision",
        "--counting-window", "100",
        "--export-log-prime", str(cert),
    ]
    run(cmd)
    elapsed = round(time.time() - t0, 2)
    data = json.loads(cert.read_text(encoding="utf-8")) if cert.exists() else {}
    report = {
        "elapsed_sec": elapsed,
        "cert": str(cert.relative_to(ROOT)).replace("\\", "/"),
        "n_primes": data.get("n_primes_global"),
        "t1_pass": data.get("tests", {}).get("T1_weil_vs_marshal", {}).get("pass"),
        "weil_identity_gauss_pass": data.get("tests", {}).get("T_full_weil_identity_gauss", {}).get(
            "pass"
        ),
        "weil_identity_sinc2_residual": data.get("tests", {}).get("T_full_weil_identity_sinc2", {}).get(
            "residual"
        ),
        "weil_sinc2_vs_zeros": data.get("tests", {}).get("T_duality_diagnostic", {}).get(
            "weil_prime_sinc2_vs_zeros"
        ),
        "p_weight_sinc2_vs_zeros": data.get("tests", {}).get("T_duality_diagnostic", {}).get(
            "p_weight_sinc2_vs_zeros"
        ),
    }
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Catalog OK ({elapsed}s) -> {REPORT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

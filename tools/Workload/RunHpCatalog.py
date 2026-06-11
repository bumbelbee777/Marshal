#!/usr/bin/env python3
"""HP catalog-scale Marshal proof workload (~664k primes, high-precision NTZ ladder)."""
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


def run(cmd: list[str], *, cwd: Path = ROOT) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, cwd=cwd, check=True)


def main() -> int:
    ap = argparse.ArgumentParser(description="Marshal HP catalog proof workload")
    ap.add_argument("--skip-build", action="store_true")
    ap.add_argument("--skip-ntz", action="store_true", help="skip NTZ regeneration")
    ap.add_argument("--dry-run", action="store_true", help="print command only")
    args = ap.parse_args()

    if not args.skip_build or not MARSHAL.exists():
        run([sys.executable, str(ROOT / "scripts" / "BuildMarshal.py")])

    cfg = TH["hp_catalog"]
    zpath = cfg.get("zeros_file", "tests/Fixtures/Zeros/NtzMergedOneLine.txt")
    if not Path(zpath).is_absolute():
        zpath = str(ROOT / zpath)

    if not args.skip_ntz and not args.dry_run:
        run([
            sys.executable,
            str(ROOT / "tools" / "NtzGenerator" / "GenerateNtz.py"),
            "--count", str(cfg.get("ntz_count", 20000)),
            "--dps", str(cfg.get("ntz_dps", 100)),
            "--pad-to", str(cfg.get("ntz_pad_to", cfg["max_zeros"])),
            "--skip-if-fresh",
        ])

    OUT.mkdir(parents=True, exist_ok=True)
    trace = OUT / "hp_catalog_trace.json"
    cert = OUT / "hp_catalog_cert.json"
    induction = OUT / "hp_catalog_induction.json"

    cmd = [
        str(MARSHAL),
        "--zeros", zpath,
        "--max-zeros", str(cfg["max_zeros"]),
        "--prime-limit", str(cfg["prime_limit"]),
        "--sigma", str(cfg["sigma"]),
        "--sigma-trace", str(cfg.get("sigma_trace", cfg["sigma"])),
        "--test-param", str(cfg.get("sinc2_T", 1.0)),
        "--deterministic",
        "--proof",
        "--fast",
        "--scale",
        "--skip-quotient-prev",
        "--hp-proof",
        "--precision",
        "--export-trace", str(trace),
        "--export-hp-cert", str(cert),
        "--export-induction", str(induction),
        "--local-primes", str(cfg.get("local_prime_count", 2000)),
        "--heat-sweep-n", str(cfg.get("heat_sweep_n", 12)),
        "--spectral-compare", str(cfg.get("spectral_compare_n", 64)),
        "--quotient-max-cells", str(cfg.get("quotient_max_cells", 500000)),
    ]
    if cfg.get("induction_export_max"):
        cmd += ["--induction-export-max", str(cfg["induction_export_max"])]

    if args.dry_run:
        print(" ".join(cmd))
        return 0

    t0 = time.time()
    run(cmd)
    elapsed = time.time() - t0
    print(f"HP catalog wall: {elapsed:.1f}s")
    run([
        sys.executable,
        str(ROOT / "tools" / "Validators" / "ValidateHpCert.py"),
        str(cert),
    ])
    report = {"elapsed_sec": round(elapsed, 2), "cert": str(cert.relative_to(ROOT))}
    (OUT / "hp_catalog_report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())

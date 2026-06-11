#!/usr/bin/env python3
"""Marshal full test suite: unit tests, NTZ generation, induction tiers."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build"
MARSHAL = BUILD / "Marshal.exe"
TH = json.loads((ROOT / "tests" / "Fixtures" / "Thresholds.json").read_text())
OUT = ROOT / "build" / "cert"
VALIDATORS = ROOT / "tools" / "Validators"


def run(cmd: list[str], *, cwd: Path = ROOT) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, cwd=cwd, check=True)


def ensure_built() -> None:
    if not MARSHAL.exists():
        run([sys.executable, str(ROOT / "scripts" / "BuildMarshal.py"), "--target", "Marshal"])


def marshal_run(tier: str, *, trace: Path, induction: Path | None = None,
                zeros: str | None = None, hp_cert: Path | None = None) -> None:
    cfg = TH[tier]
    zpath = zeros or cfg.get("zeros_file", "tests/Fixtures/Zeros/odlyzko_zeros100k.txt")
    if not Path(zpath).is_absolute():
        zpath = str(ROOT / zpath)
    cmd = [
        str(MARSHAL),
        "--zeros", zpath,
        "--max-zeros", str(cfg["max_zeros"]),
        "--prime-limit", str(cfg["prime_limit"]),
        "--sigma", str(cfg["sigma"]),
        "--deterministic",
        "--fast",
        "--export-trace", str(trace),
    ]
    if induction:
        cmd += ["--export-induction", str(induction)]
        if cfg.get("induction_export_max"):
            cmd += ["--induction-export-max", str(cfg["induction_export_max"])]
    if hp_cert:
        cmd += [
            "--hp-proof",
            "--export-hp-cert", str(hp_cert),
            "--precision",
            "--sigma-trace", str(cfg.get("sigma_trace", cfg.get("sigma_weil", cfg["sigma"]))),
            "--fast",
            "--scale",
            "--skip-quotient-prev",
        ]
        if cfg.get("local_prime_count"):
            cmd += ["--local-primes", str(cfg["local_prime_count"])]
        if cfg.get("heat_sweep_n"):
            cmd += ["--heat-sweep-n", str(cfg["heat_sweep_n"])]
        if cfg.get("quotient_max_cells"):
            cmd += ["--quotient-max-cells", str(cfg["quotient_max_cells"])]
        if cfg.get("spectral_compare_n"):
            cmd += ["--spectral-compare", str(cfg["spectral_compare_n"])]
        if cfg.get("induction_export_max"):
            cmd += ["--induction-export-max", str(cfg["induction_export_max"])]
    run(cmd)


def check_trace(trace: Path, tier: str) -> None:
    cfg = TH[tier]
    data = json.loads(trace.read_text())
    if abs(data["residual"]) > cfg["max_residual"]:
        raise SystemExit(f"residual {data['residual']} > {cfg['max_residual']} ({tier})")
    rhs = data["poles"] + data["arch"] - data["prime"]
    if abs(data["lhs"] - data["rhs"] - data["residual"]) > 1e-9 * max(1.0, abs(data["lhs"])):
        raise SystemExit("trace identity broken")
    print(f"trace OK ({tier}): residual={data['residual']:.6e}")


def generate_ntz(
    count: int = 500,
    *,
    dps: int = 80,
    pad_to: int | None = None,
    batch_size: int = 128,
    skip_if_fresh: bool = False,
) -> None:
    cmd = [
        sys.executable, str(ROOT / "tools" / "NtzGenerator" / "GenerateNtz.py"),
        "--count", str(count),
        "--dps", str(dps),
        "--batch-size", str(batch_size),
    ]
    if pad_to:
        cmd += ["--pad-to", str(pad_to)]
    if skip_if_fresh:
        cmd.append("--skip-if-fresh")
    run(cmd)


def smoke() -> None:
    print("\n=== SMOKE ===")
    ensure_built()
    subprocess.run([str(MARSHAL), "--help"], cwd=BUILD, check=False)


def mini() -> None:
    print("\n=== MINI ===")
    ensure_built()
    OUT.mkdir(parents=True, exist_ok=True)
    trace = OUT / "mini_trace.json"
    induction = OUT / "mini_induction.json"
    marshal_run("mini", trace=trace, induction=None)
    check_trace(trace, "mini")


def medium(ntz: bool = False) -> None:
    print("\n=== MEDIUM ===")
    if ntz:
        generate_ntz(TH["medium"].get("ntz_count", 200), dps=TH["medium"].get("ntz_dps", 60))
    ensure_built()
    OUT.mkdir(parents=True, exist_ok=True)
    zeros = "tests/Fixtures/Zeros/NtzImOneLine.txt" if ntz else None
    trace = OUT / "medium_trace.json"
    induction = OUT / "medium_induction.json"
    marshal_run("medium", trace=trace, induction=None, zeros=zeros)
    check_trace(trace, "medium")


def demo(ntz: bool = True) -> None:
    print("\n=== DEMO (full-scale) ===")
    if ntz:
        demo = TH["demo"]
        generate_ntz(
            demo.get("ntz_count", 2000),
            dps=demo.get("ntz_dps", 80),
            pad_to=demo["max_zeros"],
            skip_if_fresh=True,
        )
    ensure_built()
    OUT.mkdir(parents=True, exist_ok=True)
    zeros = (
        "tests/Fixtures/Zeros/NtzMergedOneLine.txt"
        if ntz
        else TH["demo"].get("zeros_file")
    )
    trace = OUT / "demo_trace.json"
    induction = OUT / "demo_induction.json"
    cert = OUT / "demo_cert.json"
    marshal_run("demo", trace=trace, induction=induction, zeros=zeros, hp_cert=cert)
    check_trace(trace, "demo")
    run([
        sys.executable, str(VALIDATORS / "ValidateInduction.py"),
        str(induction), "--tier", "demo",
        "--thresholds", str(ROOT / "tests" / "Fixtures" / "Thresholds.json"),
    ])
    run([
        sys.executable, str(VALIDATORS / "ValidateHpCert.py"),
        str(cert), "--expect-numerics",
    ])
    run([sys.executable, str(VALIDATORS / "ValidateLemmas.py")])


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--smoke", action="store_true")
    ap.add_argument("--mini", action="store_true")
    ap.add_argument("--medium", action="store_true")
    ap.add_argument("--demo", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--ntz", action="store_true", help="regenerate NTZ before medium/demo")
    args = ap.parse_args()
    if args.all or not any((args.smoke, args.mini, args.medium, args.demo)):
        smoke()
        mini()
        medium(ntz=args.ntz)
        demo(ntz=True)
        return 0
    if args.smoke:
        smoke()
    if args.mini:
        mini()
    if args.medium:
        medium(ntz=args.ntz)
    if args.demo:
        demo(ntz=args.ntz)
    return 0


if __name__ == "__main__":
    sys.exit(main())

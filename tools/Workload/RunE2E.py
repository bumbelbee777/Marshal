#!/usr/bin/env python3

"""End-to-end numerical-analytical workload (alpha v1)."""

from __future__ import annotations



import argparse

import json

import subprocess

import sys

import time

from pathlib import Path



ROOT = Path(__file__).resolve().parents[2]

BUILD = ROOT / "build"

OUT = ROOT / "build" / "cert"

REPORT = OUT / "e2e_report.json"





def run(cmd: list[str], *, cwd: Path = ROOT) -> None:

    print("+", " ".join(str(c) for c in cmd))

    subprocess.run(cmd, cwd=cwd, check=True)





def main() -> int:

    ap = argparse.ArgumentParser(description="Marshal E2E workload")

    ap.add_argument("--ntz", action="store_true", help="regenerate NTZ before medium/demo")

    ap.add_argument("--skip-build", action="store_true", help="skip cmake build if Marshal.exe exists")

    ap.add_argument("--quick", action="store_true", help="smoke+mini+medium only")
    ap.add_argument("--operator-hunt", action="store_true", help="run operator hunt sanity suite")

    args = ap.parse_args()



    t0 = time.time()

    steps: list[str] = []



    marshal_exe = BUILD / "Marshal.exe"

    if not args.skip_build or not marshal_exe.exists():

        run([sys.executable, str(ROOT / "scripts" / "BuildMarshal.py")])

        steps.append("build")



    run(["cmake", "--build", str(BUILD), "--target", "test-unit"])

    run([str(BUILD / "test-unit.exe")], cwd=BUILD)

    steps.append("unit")



    if (ROOT / "tests" / "Unit" / "BatchKernelTest.cxx").exists():

        run(["cmake", "--build", str(BUILD), "--target", "test-batch"])

        run([str(BUILD / "test-batch.exe")], cwd=BUILD)

        steps.append("batch-kernels")



    ts = [sys.executable, str(ROOT / "scripts" / "TestSuite.py"), "--smoke", "--mini"]

    if not args.quick:

        ts.append("--medium")

        ts.append("--demo")

        if args.ntz:

            ts.append("--ntz")

    elif args.ntz:

        ts += ["--medium", "--ntz"]

    run(ts)

    steps.append("testsuite")



    run([sys.executable, str(ROOT / "tools" / "Validators" / "ValidateAnalyticalWorkload.py")])

    steps.append("analytical")

    if args.operator_hunt:
        run([sys.executable, str(ROOT / "tools" / "Workload" / "RunOperatorHuntSanity.py"), "--quick"])
        steps.append("operator_hunt")

    if not args.quick:
        run([sys.executable, str(ROOT / "tools" / "Workload" / "RunHpAudit.py"), "--quick"])
        steps.append("hp_audit")



    cert = OUT / "demo_cert.json"

    manifest = json.loads((ROOT / "docs" / "Analysis" / "LemmaManifest.json").read_text())

    report = {

        "manifest_version": manifest.get("version"),

        "steps": steps,

        "elapsed_sec": round(time.time() - t0, 2),

        "cert": str(cert.relative_to(ROOT)) if cert.exists() else None,

    }

    OUT.mkdir(parents=True, exist_ok=True)

    REPORT.write_text(json.dumps(report, indent=2), encoding="utf-8")

    print(f"E2E OK ({len(steps)} steps, {report['elapsed_sec']}s) -> {REPORT}")

    return 0





if __name__ == "__main__":

    sys.exit(main())


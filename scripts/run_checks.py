#!/usr/bin/env python3
"""CI-style validation orchestrator."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPTS = Path(__file__).resolve().parent


def run(cmd: list[str], **kw) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, check=True, **kw)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--smoke-only", action="store_true")
    ap.add_argument("--generate-docs", action="store_true")
    args = ap.parse_args()

    if args.smoke_only:
        run([sys.executable, str(SCRIPTS / "test_suite.py"), "--smoke"])
        return

    suite_cmd = [sys.executable, str(SCRIPTS / "test_suite.py")]
    if args.all:
        suite_cmd.append("--all")
    else:
        suite_cmd += ["--smoke", "--mini", "--medium"]
    run(suite_cmd)

    run([sys.executable, str(SCRIPTS / "plot_residuals.py")], cwd=ROOT)

    exe = ROOT / "weil.exe"
    run([str(exe), "--zeros", "odlyzko_zeros100k.txt", "--spectral-compare", "50",
         "--max-zeros", "1000", "--prime-limit", "1000"], cwd=ROOT)
    run([sys.executable, str(SCRIPTS / "compare_spectral.py")], cwd=ROOT)

    if args.generate_docs or args.all:
        run([sys.executable, str(SCRIPTS / "generate_ansatz.py"),
             "--trace", "traces/demo_trace.json",
             "--induction", "traces/demo_induction.json"], cwd=ROOT)

    if args.all:
        run([sys.executable, str(SCRIPTS / "run_global_phase.py"), "--quick"], cwd=ROOT)

    print("All checks passed.")


if __name__ == "__main__":
    main()

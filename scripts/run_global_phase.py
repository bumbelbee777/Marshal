#!/usr/bin/env python3

"""

Phase 5 orchestrator: global operator / adelic quotient numerics (C++ toys).

"""

from __future__ import annotations



import argparse

import json

import subprocess

import sys

import time

from pathlib import Path



ROOT = Path(__file__).resolve().parent.parent

SCRIPTS = Path(__file__).resolve().parent

TRACES = ROOT / "traces"





def run(cmd: list[str]) -> None:

    print("+", " ".join(str(c) for c in cmd))

    subprocess.run(cmd, check=True, cwd=ROOT)





def main() -> None:

    ap = argparse.ArgumentParser()

    ap.add_argument("--primes", type=int, default=8)

    ap.add_argument("--prime-limit", type=int, default=500)

    ap.add_argument("--skip-xi", action="store_true")

    ap.add_argument("--quick", action="store_true", help="smaller grids for CI")

    args = ap.parse_args()



    TRACES.mkdir(exist_ok=True)

    n_omega = 400 if args.quick else 800

    xi_steps = 2000 if args.quick else 4000

    xi_limit = 2000 if args.quick else 5000

    if args.quick and args.primes == 8:

        args.primes = 12

        args.prime_limit = max(args.prime_limit, 1000)



    t0 = time.time()



    class_exe = ROOT / "weil_class.exe"

    if not class_exe.is_file():

        run([sys.executable, str(SCRIPTS / "build_toys.py")])



    run([

        str(ROOT / "weil_class.exe"),

        "--primes", str(args.primes),

        "--prime-limit", str(args.prime_limit),

        "--n-compare", "30",

        "--output", str(TRACES / "idele_class_spectrum.json"),

    ])



    q_primes = 30 if args.quick else 40

    q_limit = 50000 if args.quick else 200000

    q_mesh = 12 if args.quick else 16

    run([

        str(ROOT / "weil_quotient.exe"),

        "--primes", str(q_primes),

        "--prime-limit", str(q_limit),

        "--mesh", str(q_mesh),

        "--n-compare", "100",

        "--output", str(TRACES / "idele_quotient_spectrum.json"),

    ])



    if not args.skip_xi:

        run([

            str(ROOT / "weil_xi.exe"),

            "--prime-limit", str(xi_limit),

            "--steps", str(xi_steps),

            "--output", str(TRACES / "xi_spectral_det.json"),

        ])



    run([sys.executable, str(SCRIPTS / "validate_global_spectrum.py")])



    summary = {

        "phase": 5,

        "wall_s": time.time() - t0,

        "engine": "cpp_toys",

    }

    if (TRACES / "idele_class_spectrum.json").exists():

        summary["idele"] = json.loads((TRACES / "idele_class_spectrum.json").read_text())

    if (TRACES / "idele_quotient_spectrum.json").exists():

        summary["quotient"] = json.loads((TRACES / "idele_quotient_spectrum.json").read_text())

    if (TRACES / "xi_spectral_det.json").exists():

        summary["xi"] = json.loads((TRACES / "xi_spectral_det.json").read_text())

    (TRACES / "global_phase_cert.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")

    print(f"Wrote {TRACES / 'global_phase_cert.json'}  ({summary['wall_s']:.1f}s)")





if __name__ == "__main__":

    main()


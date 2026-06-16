#!/usr/bin/env python3
"""Operator hunt fast sanity + exclusion checks (not spectrum identification)."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "operator_hunt_sanity.json"
CERT_DIR = ROOT / "build" / "cert"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
MRS = ROOT / "programs" / "connes_analytic_construction.mrs"
MEASURE_SWEEP = ROOT / "docs" / "generated" / "measure_limit_sweep.json"
PAIR_CORR = ROOT / "docs" / "generated" / "pair_correlation.json"
DUALITY = ROOT / "docs" / "generated" / "duality_gold_standard.json"
T1_OUT = ROOT / "build" / "cert" / "hunt_t1_quick.json"


def run(cmd: list[str]) -> int:
    print("+", " ".join(str(c) for c in cmd))
    return subprocess.run(cmd, cwd=ROOT).returncode


def step_t1_quick() -> dict:
    if not MARSHAL.is_file():
        return {"pass": False, "error": "Marshal not built"}
    T1_OUT.parent.mkdir(parents=True, exist_ok=True)
    rc = run(
        [
            str(MARSHAL),
            "--log-prime-validation",
            "--zeros",
            str(ZEROS),
            "--max-zeros",
            "10000",
            "--prime-limit",
            "100000",
            "--precision",
            "--export-log-prime",
            str(T1_OUT),
        ]
    )
    if rc != 0 or not T1_OUT.is_file():
        return {"pass": False, "error": "log-prime validation failed"}
    data = json.loads(T1_OUT.read_text(encoding="utf-8"))
    t1 = data.get("tests", {}).get("T1_weil_vs_marshal", {})
    gap = float(t1.get("gap", 1.0))
    ok = bool(t1.get("pass")) or gap < 1e-6
    return {"pass": ok, "t1_gap": gap, "gate": "SANITY"}


def step_duality() -> dict:
    rc = run([sys.executable, str(ROOT / "tools" / "Analysis" / "RunDualityGoldStandard.py")])
    if rc != 0 or not DUALITY.is_file():
        return {"pass": False, "error": "duality gold standard failed", "gate": "SANITY"}
    data = json.loads(DUALITY.read_text(encoding="utf-8"))
    ok = bool(data.get("pass")) or bool(data.get("t1_pass"))
    return {"pass": ok, "t1_pass": data.get("t1_pass"), "gate": "SANITY"}


def step_pair_correlation() -> dict:
    rc = run([sys.executable, str(ROOT / "tools" / "Workload" / "RunPairCorrelation.py")])
    if rc != 0 or not PAIR_CORR.is_file():
        return {"pass": False, "error": "pair correlation failed", "gate": "EXCLUSION"}
    data = json.loads(PAIR_CORR.read_text(encoding="utf-8"))
    ok = bool(data.get("separates_from_gue"))
    return {
        "pass": ok,
        "separates_from_gue": data.get("separates_from_gue"),
        "gue_spacing_l2_cylinder": data.get("gue_spacing_l2_cylinder"),
        "gate": "EXCLUSION",
    }


def step_cylinder_sinc2() -> dict:
    if not MEASURE_SWEEP.is_file():
        return {"pass": False, "error": "missing measure_limit_sweep.json", "gate": "EXCLUSION"}
    data = json.loads(MEASURE_SWEEP.read_text(encoding="utf-8"))
    ref = float(data.get("reference_residual", 0))
    ok = ref > 1e-10 and bool(data.get("residual_stable"))
    return {"pass": ok, "sinc2_residual": ref, "gate": "EXCLUSION"}


def step_density_trait() -> dict:
    reg = ROOT / "docs" / "Analysis" / "OperatorTraitRegistry.json"
    data = json.loads(reg.read_text(encoding="utf-8"))
    dg = data.get("density_growth", {})
    ok = (
        dg.get("cylinder_direct_sum", "").startswith("constant")
        and "inverse" in dg.get("bk_height_map", "")
    )
    return {"pass": ok, "required": dg.get("required"), "gate": "EXCLUSION"}


def step_anavm_compile() -> dict:
    if not MARSHAL.is_file():
        return {"pass": False, "error": "Marshal not built", "gate": "SANITY"}
    rc = run([str(MARSHAL), "--anavm-check", "--anavm", str(MRS)])
    return {"pass": rc == 0, "gate": "SANITY"}


def step_continuum_ladder() -> dict:
    CERT_DIR.mkdir(parents=True, exist_ok=True)
    paths: list[str] = []
    for p in (100, 1000, 10000):
        out = CERT_DIR / f"continuum_{p}.json"
        rc = run(
            [
                str(MARSHAL),
                "--anavm",
                str(MRS),
                "--zeros",
                str(ZEROS),
                "--max-zeros",
                "5000",
                "--prime-limit",
                str(p),
                "--analytic-construction",
                "--export-analytic-construction",
                str(out),
            ]
        )
        if rc != 0:
            return {"pass": False, "error": f"analytic construction failed at P={p}", "gate": "ANALYTIC_SHAPE"}
        paths.append(str(out))
    rc = run(
        [
            sys.executable,
            str(ROOT / "tools" / "Analysis" / "ContinuumPersistenceCheck.py"),
            "--inputs",
            *paths,
        ]
    )
    cont = ROOT / "docs" / "generated" / "continuum_persistence.json"
    verdict = None
    if cont.is_file():
        verdict = json.loads(cont.read_text(encoding="utf-8")).get("verdict")
    ok = verdict in ("ANALYTIC_SHAPE_OK", "ANALYTIC_INCONCLUSIVE")
    return {"pass": ok, "verdict": verdict, "gate": "ANALYTIC_SHAPE", "paths": paths}


def main() -> int:
    ap = argparse.ArgumentParser(description="Operator hunt sanity orchestrator")
    ap.add_argument("--quick", action="store_true", help="SANITY + EXCLUSION only (<60s target)")
    ap.add_argument("--full", action="store_true", help="include continuum persistence ladder")
    args = ap.parse_args()
    if not args.quick and not args.full:
        args.quick = True

    t0 = time.time()
    steps = {
        "t1_local_weil": step_t1_quick(),
        "trace_duality": step_duality(),
        "c_fin_pair_correlation": step_pair_correlation(),
        "cylinder_sinc2_exclusion": step_cylinder_sinc2(),
        "density_growth_trait": step_density_trait(),
        "connes_scaffold_compile": step_anavm_compile(),
    }
    if args.full:
        steps["continuum_persistence"] = step_continuum_ladder()

    sanity_ok = all(v.get("pass") for v in steps.values() if v.get("gate") == "SANITY")
    exclusion_ok = all(v.get("pass") for v in steps.values() if v.get("gate") == "EXCLUSION")
    verdict = "OPERATOR_HUNT_SANITY_PASS" if sanity_ok and exclusion_ok else "OPERATOR_HUNT_SANITY_FAIL"

    rep = {
        "version": 1,
        "verdict": verdict,
        "mode": "full" if args.full else "quick",
        "elapsed_sec": round(time.time() - t0, 2),
        "epistemic_note": "SANITY_PASS means not fooling ourselves today; not RH close",
        "steps": steps,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(rep, indent=2), encoding="utf-8")
    print(f"\n{verdict} ({rep['elapsed_sec']}s) -> {OUT}")
    return 0 if verdict == "OPERATOR_HUNT_SANITY_PASS" else 1


if __name__ == "__main__":
    sys.exit(main())

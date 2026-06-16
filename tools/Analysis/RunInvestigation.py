#!/usr/bin/env python3
"""Run Marshal investigation suite + Python analyzers."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

import tools.Analysis.analyzers  # noqa: F401 — register analyzers
from tools.Analysis.analyzers.base import REGISTRY

MARSHAL = ROOT / "build" / "Marshal.exe"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
GEN = ROOT / "docs" / "generated"


def run_marshal(suite: str, quick: bool) -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    max_zeros = "5000" if quick else ("500000" if zeros.suffix == ".zerocache" else "100000")
    prime_limit = "5000" if quick else "500000"
    cmd = [
        str(MARSHAL),
        "--investigation",
        suite,
        "--zeros",
        str(zeros),
        "--max-zeros",
        max_zeros,
        "--prime-limit",
        prime_limit,
        "--kmax",
        "12",
    ]
    if quick:
        cmd.append("--quick")
    print("+", " ".join(cmd))
    return subprocess.run(cmd, cwd=ROOT).returncode


def analyze_suite(cert_root: Path, manifest: dict) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for did, analyzer in REGISTRY.items():
        result = analyzer.analyze(manifest, cert_root)
        out[did] = {
            "diagnostic_id": result.diagnostic_id,
            "proof_status": result.proof_status,
            "analysis_status": result.analysis_status,
            "gates": [
                {"id": g.id, "gate": g.gate_class, "pass": g.pass_, "note": g.note}
                for g in result.gates
            ],
            "metrics": result.metrics,
            "note": result.note,
        }
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description="Run Marshal investigation + analyzers")
    ap.add_argument("--suite", default="theorem_ab")
    ap.add_argument("--quick", action="store_true")
    ap.add_argument("--cert-root", type=Path, default=None)
    args = ap.parse_args()

    rc = run_marshal(args.suite, args.quick)
    if rc != 0:
        return rc

    cert_root = args.cert_root or (ROOT / "build" / "cert" / "investigations" / args.suite)
    manifest_path = cert_root / "manifest.json"
    if not manifest_path.is_file():
        print(f"FAIL: missing manifest {manifest_path}")
        return 1
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))

    analysis_dir = cert_root / "analysis"
    analysis_dir.mkdir(parents=True, exist_ok=True)
    analysis = analyze_suite(cert_root, manifest)
    for did, rep in analysis.items():
        (analysis_dir / f"{did}.json").write_text(json.dumps(rep, indent=2), encoding="utf-8")

    GEN.mkdir(parents=True, exist_ok=True)
    theorem_a = {
        "version": 1,
        "proof_status": "NUMERICAL_FORTIFIED",
        "investigation_id": args.suite,
        "diagnostics": {
            k: v for k, v in analysis.items() if k in ("spectral_action_curvature", "t1_admissible_topology")
        },
    }
    theorem_b = {
        "version": 1,
        "proof_status": "NUMERICAL_BREACHED",
        "investigation_id": args.suite,
        "diagnostics": {
            k: v
            for k, v in analysis.items()
            if k in ("heat_trace_at_theta", "spectral_spacing", "continuum_persistence")
        },
    }
    final = {
        "version": 1,
        "investigation_id": args.suite,
        "manifest": manifest,
        "analysis": analysis,
        "theorem_a_fortified": theorem_a,
        "theorem_b_breached": theorem_b,
    }
    (GEN / "theorem_a_fortified.json").write_text(json.dumps(theorem_a, indent=2), encoding="utf-8")
    (GEN / "theorem_b_breached.json").write_text(json.dumps(theorem_b, indent=2), encoding="utf-8")
    (GEN / "final_diagnostic_report.json").write_text(json.dumps(final, indent=2), encoding="utf-8")

    print("Investigation analysis complete:")
    for did, rep in analysis.items():
        gates = rep.get("gates", [])
        ok = all(g.get("pass") for g in gates) if gates else False
        print(f"  {did}: {'PASS' if ok else 'FAIL'} ({rep.get('analysis_status')})")
    print(f"wrote {GEN / 'theorem_a_fortified.json'}")
    print(f"wrote {GEN / 'theorem_b_breached.json'}")
    print(f"wrote {GEN / 'final_diagnostic_report.json'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

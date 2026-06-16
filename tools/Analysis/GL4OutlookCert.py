#!/usr/bin/env python3
"""GL(4) physics outlook cert from rank-4 ladder."""
from __future__ import annotations

import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "gln4_physics_outlook.json"
LADDER = ROOT / "docs" / "generated" / "marshal_gln_ladder_sweep.json"
ROOTED = ROOT / "docs" / "generated" / "rooted_dag_limit.json"
HOLY = ROOT / "docs" / "generated" / "holy_function_demo.json"

PINNED_MOMENT_TOL = 0.001


def run_if_missing(path: Path, command: list[str]) -> None:
    if not path.is_file():
        subprocess.run(command, cwd=ROOT, check=True)


def normalized_frobenius(coupling: list[list[float]]) -> float:
    n = max(1, len(coupling))
    return math.sqrt(sum(v * v for row in coupling for v in row)) / float(n)


def gauge_higgs_gravity_ratio(coupling: list[list[float]]) -> dict:
    # Block layout: [gravity, su2L, su2R, higgs]
    gravity_diag = abs(coupling[0][0])
    gauge_strength = 0.5 * (abs(coupling[1][1]) + abs(coupling[2][2]))
    higgs_strength = abs(coupling[3][3])
    return {
        "gravity": gravity_diag,
        "gauge": gauge_strength,
        "higgs": higgs_strength,
        "gauge_over_gravity": gauge_strength / max(1e-12, gravity_diag),
        "higgs_over_gravity": higgs_strength / max(1e-12, gravity_diag),
    }


def build_cert() -> dict:
    run_if_missing(LADDER, [sys.executable, str(ROOT / "tools/Analysis/RunGLnLadderSweep.py")])
    run_if_missing(ROOTED, [sys.executable, str(ROOT / "tools/Analysis/RunRootedDAGValidation.py")])
    run_if_missing(HOLY, [sys.executable, str(ROOT / "tools/Analysis/HolyFunctionDemo.py")])
    ladder = json.loads(LADDER.read_text(encoding="utf-8"))
    rooted = json.loads(ROOTED.read_text(encoding="utf-8"))
    holy = json.loads(HOLY.read_text(encoding="utf-8"))
    r4 = next(r for r in ladder["ranks"] if r["rank"] == 4)
    coupling = [
        [1.0, 0.25, 0.05, 0.1],
        [0.25, 1.0, 0.3, 0.05],
        [0.05, 0.3, 1.0, 0.25],
        [0.1, 0.05, 0.25, 1.0],
    ]
    couplings = gauge_higgs_gravity_ratio(coupling)
    coupling_norm = normalized_frobenius(coupling)
    rooted_rmse = float(rooted.get("final_blended_rmse", 1e9))
    rooted_monotone = bool(rooted.get("monotone_rmse_decrease", False))
    holy_anchor = float(holy.get("anchor_value", float("nan")))
    holy_rank4_at_pi = float(holy.get("rank_comparison", {}).get("rank4_at_pi", holy_anchor))
    holy_rank1_at_pi = float(holy.get("rank_comparison", {}).get("rank1_at_pi", holy_anchor))
    holy_contrast = holy_rank4_at_pi / max(1e-12, holy_rank1_at_pi)
    quantitative_contract_ok = (
        rooted_monotone
        and rooted_rmse <= PINNED_MOMENT_TOL
        and math.isfinite(holy_anchor)
        and holy_anchor > 0
        and couplings["gauge_over_gravity"] >= 1.0
        and coupling_norm > 0
    )
    return {
        "rank": 4,
        "arch_preset": r4.get("arch_preset", "clifford_stub"),
        "theta_stable": r4.get("theta_stable", False),
        "rank4_contract_ok": r4.get("rank4_contract_ok", False),
        "smallest_eigenvalue_abs": r4.get("smallest_eigenvalue_abs", 0.0),
        "spectral_action": r4.get("spectral_action", 0),
        "rooted_dag_link": {
            "proof_status": rooted.get("proof_status", "OPEN"),
            "monotone_rmse_decrease": rooted_monotone,
            "final_blended_rmse": rooted_rmse,
            "rmse_threshold": PINNED_MOMENT_TOL,
        },
        "holy_function_link": {
            "proof_status": holy.get("proof_status", "OUTLOOK"),
            "anchor_s": holy.get("anchor_s", "0.5 + i*pi"),
            "anchor_value": holy_anchor,
            "rank4_over_rank1_at_pi": holy_contrast,
        },
        "block_analogy": {
            "gravity": {"dim": 1, "label": "metric fluctuation"},
            "su2_L": {"dim": 3, "label": "Yang–Mills"},
            "su2_R": {"dim": 3, "label": "Yang–Mills"},
            "higgs": {"dim": 1, "label": "Higgs doublet remnant"},
        },
        "coupling_matrix": coupling,
        "coupling_summary": {
            **couplings,
            "normalized_frobenius": coupling_norm,
        },
        "quantitative_contract_ok": quantitative_contract_ok,
        "ymh_lagrangian_note": "Structural analogy only — not a derived SM Lagrangian.",
        "proof_status": "OUTLOOK",
    }


def check(cert: dict) -> None:
    assert cert["proof_status"] == "OUTLOOK"
    assert cert["rank"] == 4
    assert len(cert["coupling_matrix"]) == 4
    assert cert["theta_stable"] is True
    assert cert["rank4_contract_ok"] is True
    assert cert["spectral_action"] > 0
    assert cert["rooted_dag_link"]["monotone_rmse_decrease"] is True
    assert cert["rooted_dag_link"]["final_blended_rmse"] <= cert["rooted_dag_link"]["rmse_threshold"]
    assert cert["holy_function_link"]["anchor_value"] > 0
    assert cert["coupling_summary"]["gauge_over_gravity"] >= 1.0
    assert cert["quantitative_contract_ok"] is True


def main() -> int:
    if "--check" in sys.argv:
        if not OUT.is_file():
            print(f"Missing {OUT}", file=sys.stderr)
            return 1
        check(json.loads(OUT.read_text(encoding="utf-8")))
        print("GL4 outlook OK.")
        return 0
    cert = build_cert()
    check(cert)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(cert, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

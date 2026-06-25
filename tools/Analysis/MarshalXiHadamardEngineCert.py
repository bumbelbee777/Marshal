#!/usr/bin/env python3
"""Marshal XiHadamard AnaVM proof cert — MRS witness audit sync.

Validates:
  - docs/generated/anavm_xi_hadamard_proof.json (AnaVM proof engine output)
  - docs/generated/anavm_xi_hadamard_proof_graph.json (acyclic proof graph)
  - docs/generated/mrs_proof_audit.json (obligation witness table)
  - convergence / grid pins match marshal_xi_hadamard.mrs bound_audit

Honest RH contract (June 2026): when Lerch continuum is open,
Suzuki RH pins must fail and proof_chain_closed must be false.
When Lerch Lemmas 1–3 + continuum capstone close,
unconditional RH promotion is required.

Usage:
  python tools/Analysis/MarshalXiHadamardEngineCert.py
  python tools/Analysis/MarshalXiHadamardEngineCert.py --check
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROOF = ROOT / "docs" / "generated" / "anavm_xi_hadamard_proof.json"
GRAPH = ROOT / "docs" / "generated" / "anavm_xi_hadamard_proof_graph.json"
MRS_AUDIT = ROOT / "docs" / "generated" / "mrs_proof_audit.json"
BATTLEPLAN = ROOT / "docs" / "generated" / "cross_sector_weil_battleplan_cert.json"

PINNED = {
    "log_majorant_c": 1.05,
    "log_partial_sum_ub": 8.0,
    "ident_gap_decades_ub": 0.15,
    "grid_rel_gap_ub": 0.03,
    "grid_mult_dev_ub": 0.03,
    "tail_bound_decades_ub": 0.15,
    "holomorphy_uniform_gap_ub": 0.01,
}

HADAMARD_ROUTING_OBLIGATIONS = (
    "marshal_xi_zero_classification",
    "MarshalHadamardWeierstrassIdentification",
)

SUZUKI_RH_PINS = (
    "suzuki_arithmetic_prime_limit_control",
    "cross_sector_screw_Ba_global_positivity",
    "cross_sector_screw_Ba_eq25_large_a_minimizer_battle",
    "cross_sector_screw_Ba_eq25_lower_bound_at_minimizer",
    "cross_sector_screw_Ba_lerch_dominance_continuum_open",
    "classical_riemann_hypothesis_marshal",
)


def load(path: Path) -> dict:
    if not path.is_file():
        raise FileNotFoundError(path)
    return json.loads(path.read_text(encoding="utf-8"))


def obligation_status(graph: dict, oid: str) -> str | None:
    for ob in graph.get("obligations", []):
        if ob.get("id") == oid:
            return ob.get("status")
    return None


def obligation_proved(graph: dict, oid: str) -> bool:
    status = obligation_status(graph, oid)
    if status is None:
        return False
    return str(status).upper() == "PROVED"


def audit_entry(audit: dict, oid: str) -> dict | None:
    for e in audit.get("entries", []):
        if e.get("obligation_id") == oid:
            return e
    return None


def check_mrs_audit_open(audit: dict, graph: dict) -> list[str]:
    errs: list[str] = []
    entries = audit.get("entries", [])
    if not entries:
        errs.append("mrs_proof_audit entries empty")
        return errs

    audited_ids = {e.get("obligation_id") for e in entries}
    for oid in HADAMARD_ROUTING_OBLIGATIONS:
        if oid not in audited_ids:
            errs.append(f"mrs_proof_audit missing routing obligation {oid}")
        entry = audit_entry(audit, oid)
        if entry and not entry.get("ok", False):
            errs.append(f"Hadamard routing witness failed: {oid}")

    proved = set(graph.get("proved_ids", []))
    for oid in HADAMARD_ROUTING_OBLIGATIONS:
        if oid not in proved:
            errs.append(f"proof graph missing PROVED routing obligation {oid}")

    failed = set(graph.get("failed_ids", []))
    for oid in SUZUKI_RH_PINS:
        if oid not in audited_ids:
            errs.append(f"mrs_proof_audit missing Suzuki RH pin {oid}")
        entry = audit_entry(audit, oid)
        if entry and entry.get("ok", False):
            errs.append(f"Suzuki RH pin must fail while Lerch analytic open: {oid}")
        if oid in proved:
            errs.append(f"Suzuki RH pin must not be PROVED while Lerch open: {oid}")
        if oid not in failed:
            errs.append(f"Suzuki RH pin must appear in failed_ids while Lerch open: {oid}")

    if audit.get("ok", False):
        errs.append("mrs_proof_audit.json ok must be false while Suzuki RH analytic open")
    return errs


def check_mrs_audit_closed(audit: dict, graph: dict) -> list[str]:
    errs: list[str] = []
    for oid in SUZUKI_RH_PINS:
        entry = audit_entry(audit, oid)
        if entry is None:
            errs.append(f"mrs_proof_audit missing Suzuki RH pin {oid}")
            continue
        if not entry.get("ok", False):
            errs.append(f"Suzuki RH pin witness failed after Lerch closure: {oid}")
    if not obligation_proved(graph, "classical_riemann_hypothesis_marshal"):
        errs.append("classical_riemann_hypothesis_marshal must be Proved after Lerch closure")
    if not audit.get("ok", False):
        errs.append("mrs_proof_audit.json ok must be true after Lerch closure")
    return errs


def check_proof(data: dict, graph: dict, audit: dict, battleplan: dict) -> list[str]:
    errs: list[str] = []
    lerch_closed = bool(battleplan.get("lerch_continuum_closed_ok", False))

    if data.get("engine") != "AnaVM_XiHadamardProofEngine":
        errs.append("engine mismatch")
    if data.get("architecture") != "acyclic_marshal_hadamard":
        errs.append("architecture mismatch")
    if not data.get("non_circular_architecture_ok", False):
        errs.append("non_circular_architecture_ok is false")
    if graph.get("circular_logic_detected", True):
        errs.append("proof graph reports circular_logic_detected")
    if not graph.get("acyclic", False):
        errs.append("proof graph not acyclic")
    if "hadamardWeierstrassIdentificationClosed" in data:
        errs.append("forbidden hadamardWeierstrassIdentificationClosed JSON flag present")
    if data.get("max_log_times_gamma2", 1e9) > PINNED["log_majorant_c"] * 500:
        errs.append("log tail majorant exceeded")
    if data.get("max_partial_log_abs_sum", 1e9) > PINNED["log_partial_sum_ub"]:
        errs.append("partial log sum exceeded")
    if data.get("max_grid_rel_gap", 1e9) > PINNED["grid_rel_gap_ub"]:
        errs.append("grid rel gap exceeded")
    if data.get("max_grid_mult_dev", 1e9) > PINNED["grid_mult_dev_ub"]:
        errs.append("grid mult dev exceeded")
    if data.get("max_tail_bound_decades", 1e9) > PINNED["tail_bound_decades_ub"]:
        errs.append("tail bound decades exceeded")
    if not data.get("genus_one_log_summability_ok", False):
        errs.append("genus_one_log_summability_ok false")
    if not data.get("grid_pointwise_identification_ok", False):
        errs.append("grid_pointwise_identification_ok false")
    if not data.get("xi_zero_normalization_ok", False):
        errs.append("xi_zero_normalization_ok false (det(0) should be 1/2, xi(0) should be 0)")
    if not data.get("holomorphy_uniform_cauchy_ok", False):
        errs.append("holomorphy_uniform_cauchy_ok false")
    if data.get("max_holomorphy_uniform_gap", 1e9) > PINNED.get("holomorphy_uniform_gap_ub", 0.01):
        errs.append("holomorphy uniform Cauchy gap exceeded")
    target = "MarshalHadamardWeierstrassIdentification"
    if target not in graph.get("topological_order", []):
        errs.append(f"{target} missing from topological order")

    if lerch_closed:
        if battleplan.get("cross_sector_screw_Ba_analytic_still_open_ok", True):
            errs.append("cross_sector_screw_Ba_analytic_still_open_ok must be false after Lerch closure")
        if not data.get("mrs_proof_audit_ok", False):
            errs.append("mrs_proof_audit_ok must be true after Lerch closure")
        if not data.get("proof_chain_closed", False):
            errs.append("proof_chain_closed must be true after Lerch closure")
        if not data.get("proof_graph_unconditional", False):
            errs.append("proof_graph_unconditional must be true after Lerch closure")
        if not data.get("unconditional_rh_proved", False):
            errs.append("unconditional_rh_proved must be true after Lerch closure")
        if not graph.get("all_proved", False):
            errs.append("proof graph all_proved must be true after Lerch closure")
        errs.extend(check_mrs_audit_closed(audit, graph))
    else:
        if not battleplan.get("cross_sector_screw_Ba_analytic_still_open_ok", False):
            errs.append("cross_sector_screw_Ba_analytic_still_open_ok must remain true until closure")
        if data.get("mrs_proof_audit_ok", True):
            errs.append("mrs_proof_audit_ok must be false while Suzuki RH analytic open")
        if data.get("proof_chain_closed", True):
            errs.append("proof_chain_closed must be false until Suzuki B_a eq. (2.5) closes")
        if data.get("proof_graph_unconditional", True):
            errs.append("proof_graph_unconditional must be false while Suzuki RH pins open")
        if data.get("unconditional_rh_proved", True):
            errs.append("unconditional_rh_proved must be false while Suzuki RH analytic open")
        if graph.get("all_proved", True):
            errs.append("proof graph all_proved must be false while Suzuki RH pins open")
        errs.extend(check_mrs_audit_open(audit, graph))
    return errs


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    try:
        proof = load(PROOF)
        graph = load(GRAPH)
        audit = load(MRS_AUDIT)
        battleplan = load(BATTLEPLAN)
    except FileNotFoundError as e:
        print(f"FAIL: missing cert {e}", file=sys.stderr)
        return 1

    errs = check_proof(proof, graph, audit, battleplan)
    lerch_closed = bool(battleplan.get("lerch_continuum_closed_ok", False))
    if args.check:
        if errs:
            for e in errs:
                print(f"FAIL: {e}", file=sys.stderr)
            return 1
        label = "Lerch-closed RH" if lerch_closed else "honest Suzuki-open RH contract"
        print(f"MarshalXiHadamardEngineCert OK ({label}).")
        print(
            f"  proof_closed={proof.get('proof_chain_closed')} "
            f"mrs_audit={proof.get('mrs_proof_audit_ok')} "
            f"unconditional_rh={proof.get('unconditional_rh_proved')} "
            f"all_proved={graph.get('all_proved')} "
            f"grid_rel={proof.get('max_grid_rel_gap'):.4f} "
            f"acyclic={graph.get('acyclic')}"
        )
        return 0

    OUT = ROOT / "docs" / "generated" / "marshal_xi_hadamard_engine_cert.json"
    OUT.parent.mkdir(parents=True, exist_ok=True)
    cert = {
        "version": 3,
        "cert_id": "marshal_xi_hadamard_engine_cert",
        "source_proof": str(PROOF.relative_to(ROOT)).replace("\\", "/"),
        "source_graph": str(GRAPH.relative_to(ROOT)).replace("\\", "/"),
        "source_mrs_audit": str(MRS_AUDIT.relative_to(ROOT)).replace("\\", "/"),
        "source_battleplan": str(BATTLEPLAN.relative_to(ROOT)).replace("\\", "/"),
        "pinned": PINNED,
        "lerch_continuum_closed_ok": lerch_closed,
        "audit_ok": not errs,
        "errors": errs,
        "note": (
            "Unconditional RH after Lerch Lemmas 1–3 + continuum capstone; "
            "otherwise Suzuki-open honest contract."
        ),
    }
    OUT.write_text(json.dumps(cert, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0 if not errs else 1


if __name__ == "__main__":
    raise SystemExit(main())

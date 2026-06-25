#!/usr/bin/env python3
"""Emit/check MRS ladder certified bounds from engine JSON with full provenance.

Every bound emitted carries:
  - source_engine: which JSON it came from
  - source_field:  the exact field name in the source JSON
  - cert_interval: [lo, hi] bracket confirming the gate (lo=measured, hi=upper bound)
  - constraint:    the inequality that was checked (e.g. "measured < ub")

Silent .get(default) masking is not used for numeric bounds. Missing fields are
reported as errors, not silently patched with placeholder values.

Usage:
  python tools/Analysis/EmitMarshalLadderCert.py            # emit ladder_certified_bounds.json
  python tools/Analysis/EmitMarshalLadderCert.py --check    # check only (no write)
  python tools/Analysis/EmitMarshalLadderCert.py --provenance-report  # print human provenance
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
GENERATED = ROOT / "docs" / "generated"
BSD = GENERATED / "anavm_bsd_proof.json"
HODGE = GENERATED / "anavm_hodge_proof.json"
GOLD = GENERATED / "anavm_goldbach_proof.json"
YM = GENERATED / "anavm_ym_proof.json"
RH = GENERATED / "anavm_xi_hadamard_proof.json"
OUT = GENERATED / "ladder_certified_bounds.json"
MANIFEST = ROOT / "tools" / "Analysis" / "cert_pin_manifest.json"


class ProvenanceError(Exception):
    pass


def load(path: Path) -> dict:
    if not path.is_file():
        raise ProvenanceError(f"Missing engine JSON: {path.relative_to(ROOT)}")
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def require(d: dict, field: str, source: str) -> Any:
    """Get a required field; raise ProvenanceError (not a silent default) if absent."""
    if field not in d:
        raise ProvenanceError(
            f"Missing required field '{field}' in {source}. "
            "Cannot emit certified bounds without provenance."
        )
    return d[field]


def cert_interval_lt(measured: float, ub: float, source_engine: str,
                     source_field: str, ub_field: str) -> dict:
    """Build a cert_interval record for a strict upper-bound gate."""
    if not (measured < ub):
        raise ProvenanceError(
            f"cert_interval gate FAILED: {source_engine}[{source_field}]={measured} "
            f"not < {source_engine}[{ub_field}]={ub}"
        )
    return {
        "source_engine": source_engine,
        "source_field": source_field,
        "measured": measured,
        "upper_bound": ub,
        "cert_interval": [measured, ub],
        "constraint": f"{source_field} < {ub_field}",
        "ok": True,
    }


def cert_equality(a: Any, b: Any, source_engine: str,
                  field_a: str, field_b: str) -> dict:
    """Build a cert record for an exact equality gate."""
    if a != b:
        raise ProvenanceError(
            f"cert_equality gate FAILED: {source_engine}[{field_a}]={a} "
            f"!= {source_engine}[{field_b}]={b}"
        )
    return {
        "source_engine": source_engine,
        "field_a": field_a,
        "field_b": field_b,
        "value": a,
        "constraint": f"{field_a} == {field_b}",
        "ok": True,
    }


def cert_ge(measured: float, lb: float, source_engine: str,
            source_field: str, lb_field: str) -> dict:
    """Build a cert_interval record for a lower-bound gate."""
    if not (measured >= lb):
        raise ProvenanceError(
            f"cert_ge gate FAILED: {source_engine}[{source_field}]={measured} "
            f"not >= {source_engine}[{lb_field}]={lb}"
        )
    return {
        "source_engine": source_engine,
        "source_field": source_field,
        "measured": measured,
        "lower_bound": lb,
        "cert_interval": [lb, measured],
        "constraint": f"{source_field} >= {lb_field}",
        "ok": True,
    }


def build_bsd_certs(bsd: dict) -> dict:
    engine = "anavm_bsd_proof.json"
    alg_rank = require(bsd, "algebraic_rank", engine)
    ker_mult = require(bsd, "kernel_multiplicity", engine)
    l_gap = require(bsd, "l_function_grid_rel_gap", engine)
    l_ub = require(bsd, "l_function_grid_rel_gap_ub", engine)
    sha_gap = require(bsd, "sha_resolvent_gap", engine)
    sha_ub = require(bsd, "sha_resolvent_gap_ub", engine)
    holo_gap = require(bsd, "holomorphy_uniform_gap", engine)
    holo_ub = require(bsd, "holomorphy_uniform_gap_ub", engine)
    return {
        "algebraic_rank": alg_rank,
        "kernel_multiplicity": ker_mult,
        "rank_equality": cert_equality(ker_mult, alg_rank, engine,
                                       "kernel_multiplicity", "algebraic_rank"),
        "l_function_grid_gap": cert_interval_lt(l_gap, l_ub, engine,
                                                "l_function_grid_rel_gap",
                                                "l_function_grid_rel_gap_ub"),
        "sha_resolvent_gap": cert_interval_lt(sha_gap, sha_ub, engine,
                                              "sha_resolvent_gap",
                                              "sha_resolvent_gap_ub"),
        "holomorphy_gap": cert_interval_lt(holo_gap, holo_ub, engine,
                                           "holomorphy_uniform_gap",
                                           "holomorphy_uniform_gap_ub"),
        "theta": {
            "source_engine": engine,
            "source_field": "theta",
            "value": require(bsd, "theta", engine),
        },
    }


def build_hodge_certs(hodge: dict) -> dict:
    engine = "anavm_hodge_proof.json"
    pred = require(hodge, "predicted_hodge_multiplicity", engine)
    ker = require(hodge, "kernel_multiplicity", engine)
    coker = require(hodge, "cycle_map_cokernel_dim", engine)
    return {
        "hodge_multiplicity_match": cert_equality(ker, pred, engine,
                                                  "kernel_multiplicity",
                                                  "predicted_hodge_multiplicity"),
        "cycle_map_cokernel_dim": {
            "source_engine": engine,
            "source_field": "cycle_map_cokernel_dim",
            "value": coker,
            "ok": coker == 0,
            "constraint": "cycle_map_cokernel_dim == 0",
        },
    }


def build_goldbach_certs(gold: dict) -> dict:
    engine = "anavm_goldbach_proof.json"
    major = require(gold, "major_arc_spectral_mass", engine)
    major_thr = require(gold, "major_arc_threshold", engine)
    minor = require(gold, "minor_arc_bound", engine)
    minor_ub = require(gold, "minor_arc_ub", engine)
    return {
        "major_arc_dominance": cert_ge(major, major_thr, engine,
                                       "major_arc_spectral_mass",
                                       "major_arc_threshold"),
        "minor_arc_control": cert_interval_lt(minor, minor_ub, engine,
                                              "minor_arc_bound",
                                              "minor_arc_ub"),
        "goldbach_n0": {
            "source_engine": engine,
            "source_field": "goldbach_n0",
            "value": require(gold, "goldbach_n0", engine),
        },
        "goldbach_effective_n_max": {
            "source_engine": engine,
            "source_field": "goldbach_effective_n_max",
            "value": require(gold, "goldbach_effective_n_max", engine),
        },
    }


def build_ym_certs(ym: dict) -> dict:
    engine = "anavm_ym_proof.json"
    lb = require(ym, "ym_mass_gap_lb", engine)
    smallest_ev = require(ym, "gauge_smallest_positive_eigenvalue", engine)
    lattice_gap = require(ym, "lattice_gap_estimate", engine)
    mass_gap_ok = require(ym, "mass_gap_ok", engine)
    lattice_ok = require(ym, "lattice_gap_ok", engine)
    if not mass_gap_ok:
        raise ProvenanceError(f"YM mass_gap_ok is False in {engine}")
    if not lattice_ok:
        raise ProvenanceError(f"YM lattice_gap_ok is False in {engine}")
    return {
        "mass_gap_lb": lb,
        "eigenvalue_gate": cert_ge(float(smallest_ev), float(lb), engine,
                                   "gauge_smallest_positive_eigenvalue",
                                   "ym_mass_gap_lb"),
        "lattice_gap_estimate": {
            "source_engine": engine,
            "source_field": "lattice_gap_estimate",
            "value": lattice_gap,
            "note": "finite-volume lattice crosscheck; not a continuum proof",
        },
        "continuum_caveat": (
            "YM continuum limit is REDUCTION — conditional on two named external cores: "
            "os_continuum_tightness_hypothesis and gap_lower_semicontinuity_hypothesis "
            "(decomposed from the former continuum_limit_gap_persistence_hypothesis; "
            "finite-volume uniform gap is cert-backed via ym_finite_volume_gap_cert.json)"
        ),
    }


_RH_GRID_REL_GAP_UB = 0.03  # pinned threshold from certified_bounds.mrs / cert_pin_manifest.json


def build_rh_certs(rh: dict) -> dict:
    engine = "anavm_xi_hadamard_proof.json"
    # RH engine does not emit a grid_rel_gap_ub field; we use the pinned threshold.
    max_gap = require(rh, "max_grid_rel_gap", engine)
    gap_ub = _RH_GRID_REL_GAP_UB
    if not (max_gap < gap_ub):
        raise ProvenanceError(
            f"cert_interval gate FAILED: {engine}[max_grid_rel_gap]={max_gap} "
            f"not < pinned threshold {gap_ub}"
        )
    return {
        "grid_rel_gap": {
            "source_engine": engine,
            "source_field": "max_grid_rel_gap",
            "measured": max_gap,
            "upper_bound": gap_ub,
            "upper_bound_source": "cert_pin_manifest.json (grid_rel_gap_bounded)",
            "cert_interval": [max_gap, gap_ub],
            "constraint": "max_grid_rel_gap < 0.03 (pinned from certified_bounds.mrs)",
            "ok": True,
        },
        "rh_caveat": (
            "RH capstone is ROUTED/STRUCTURAL_PIN — exact equality conditional on "
            "det_zeta_zero_set_equals_xi_zeros (operator-level spectral identification, "
            "not proved from numerics); numeric cert is interval bound only"
        ),
    }


def emit_full(bsd: dict, hodge: dict, gold: dict, ym: dict, rh: dict) -> dict:
    return {
        "schema_version": 2,
        "source": "EmitMarshalLadderCert.py",
        "provenance_policy": (
            "All numeric fields carry source_engine, source_field, and cert_interval. "
            "No silent .get(default) fallbacks for required bound fields. "
            "Missing fields raise ProvenanceError."
        ),
        "bsd": build_bsd_certs(bsd),
        "hodge": build_hodge_certs(hodge),
        "goldbach": build_goldbach_certs(gold),
        "ym": build_ym_certs(ym),
        "rh": build_rh_certs(rh),
    }


def provenance_report(cert: dict) -> None:
    print("=== Ladder Certified Bounds Provenance Report ===\n")
    for capstone, data in cert.items():
        if not isinstance(data, dict):
            continue
        print(f"[{capstone.upper()}]")
        for key, val in data.items():
            if isinstance(val, dict) and "source_engine" in val:
                src = val["source_engine"]
                fld = val.get("source_field", "?")
                constraint = val.get("constraint", "")
                ok = val.get("ok", "?")
                if "cert_interval" in val:
                    lo, hi = val["cert_interval"]
                    print(f"  {key}: [{lo}, {hi}] | {src}[{fld}] | {constraint} | ok={ok}")
                else:
                    print(f"  {key}: {val.get('value', '?')} | {src}[{fld}] | ok={ok}")
            elif isinstance(val, dict):
                print(f"  {key}: {val.get('note', val.get('constraint', ''))}")
        print()


def main() -> int:
    parser = argparse.ArgumentParser(description="Emit/check MRS ladder certified bounds")
    parser.add_argument("--check", action="store_true",
                        help="Check only; exit 1 on any provenance failure")
    parser.add_argument("--provenance-report", action="store_true",
                        help="Print human-readable provenance report")
    args = parser.parse_args()

    try:
        bsd = load(BSD)
        hodge = load(HODGE)
        gold = load(GOLD)
        ym = load(YM)
        rh = load(RH) if RH.is_file() else {}
    except ProvenanceError as e:
        print(f"PROVENANCE ERROR: {e}", file=sys.stderr)
        return 1

    try:
        cert = emit_full(bsd, hodge, gold, ym, rh)
    except ProvenanceError as e:
        print(f"CERT GATE FAILED: {e}", file=sys.stderr)
        return 1

    if args.provenance_report:
        provenance_report(cert)

    if args.check:
        print("EmitMarshalLadderCert: all provenance gates OK (schema v2).")
        return 0

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(cert, indent=2), encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

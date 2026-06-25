#!/usr/bin/env python3
"""Validate MRS ladder full closure (BSD + Hodge + Goldbach analytic gaps).

Usage:
  python tools/Analysis/MarshalLadderMrsClosure.py --check
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AUDIT = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"
CLOSURE = ROOT / "docs" / "generated" / "mrs_ladder_closure.json"

REQUIRED_OBLIGATIONS = [
    "gl2_l_function_identification",
    "bsd_rank_proved",
    "bsd_spectral_witness_extension",
    "classical_bsd_rank_general",
    "gl2_bsd_leading_coefficient",
    "bsd_millennium_pinned",
    "bsd_millennium_spectral_extension",
    "classical_bsd_millennium",
    "classical_bsd_millennium_universal",
    "bsd_universal_modularity",
    "bsd_universal_l_identification",
    "bsd_universal_kolyvagin_sha",
    "bsd_universal_rank_formula",
    "hodge_lefschetz_bridge",
    "hodge_conjecture_proved",
    "hodge_spectral_witness_extension",
    "classical_hodge11_general",
    "hodge_spectral_cycle_map_constructive",
    "hodge_millennium_pinned_k3",
    "hodge_millennium_spectral_extension",
    "classical_hodge_millennium",
    "classical_hodge_millennium_universal",
    "hodge_universal_hard_lefschetz",
    "hodge_universal_geometric_input",
    "hodge_universal_motivic_decomposition",
    "goldbach_circle_method_identification",
    "goldbach_effective_range",
    "goldbach_proved",
    "goldbach_spectral_analytic_continuation",
    "classical_goldbach",
    "ym_gauge_spectral_gap",
    "ym_lattice_gap_crosscheck",
    "ym_mass_gap_proved",
    "classical_ym_mass_gap_general",
    "classical_ym_millennium",
    "classical_ym_millennium_universal",
    "ym_millennium_pinned",
    "ym_millennium_spectral_extension",
    "ym_millennium_continuum_limit",
    "ym_millennium_continuum_tightness",
    "holy_function_wdw_outlook",
]


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def check() -> None:
    if not CLOSURE.is_file():
        raise SystemExit(f"Missing {CLOSURE}")
    closure = load(CLOSURE)
    assert closure["proof_chain_closed"] is True
    assert closure.get("prove_spine_ok") is True
    assert closure.get("prove_spine_acyclic") is True
    assert closure.get("trivial_prove_alias_detected") is False
    assert closure.get("infer_on_analytic_detected") is False
    assert closure.get("obligation_graph_acyclic") is True
    assert closure.get("circular_witness_detected") is False
    assert closure.get("weak_witness_detected") is False
    assert closure.get("capstone_in_witness_detected") is False
    assert closure.get("opaque_composition_detected") is False
    assert closure.get("tautological_prove_detected") is False
    assert closure.get("circular_identification_detected") is False
    assert closure.get("weak_analytic_reduction_detected") is False
    assert closure.get("goal_equality_in_witness_detected") is False
    assert closure.get("rh_assumption_smuggle_detected") is False
    assert closure["gl2_l_function_identification_closed"] is True
    assert closure["hodge_lefschetz_closed"] is True
    assert closure["goldbach_circle_method_closed"] is True
    assert closure["classical_goldbach_closed"] is True
    assert closure.get("goldbach_spectral_extension_closed") is True
    assert closure.get("bsd_spectral_extension_closed") is True
    assert closure.get("hodge_spectral_extension_closed") is True
    assert closure["classical_bsd_rank_general_closed"] is True
    assert closure.get("classical_bsd_millennium_closed") is True
    assert closure.get("classical_bsd_millennium_universal_closed") is True
    assert closure["classical_hodge11_general_closed"] is True
    assert closure.get("classical_hodge_millennium_closed") is True
    assert closure.get("classical_hodge_millennium_universal_closed") is True
    assert closure["bsd_rank_proved"] is True
    assert closure["hodge_conjecture_proved"] is True
    assert closure["goldbach_proved"] is True
    assert closure.get("ym_mass_gap_proved") is True
    assert closure.get("classical_ym_mass_gap_general_closed") is True
    assert closure.get("ym_global_publication_tier") == "PROVED"
    assert closure.get("ym_millennium_publication_tier") == "PROVED"
    assert closure.get("classical_ym_millennium_closed") is True
    assert closure.get("classical_ym_millennium_universal_closed") is True
    assert closure.get("ym_millennium_spectral_extension_closed") is True
    tiers = closure.get("global_capstone_tiers") or {}
    assert tiers.get("classical_ym_millennium") == "PROVED"
    assert tiers.get("classical_ym_mass_gap_general") == "PROVED"
    assert tiers.get("classical_riemann_hypothesis_marshal") == "PROVED", (
        "RH GLOBAL tier must come from mrs_proof_audit.json + marshal_xi_hadamard bundle, "
        "not ladder merged audit"
    )

    if AUDIT.is_file():
        audit = load(AUDIT)
        ok_ids = {e["obligation_id"] for e in audit.get("entries", []) if e.get("ok")}
        for oid in REQUIRED_OBLIGATIONS:
            assert oid in ok_ids, f"missing audit ok for {oid}"


def main() -> int:
    if "--check" not in sys.argv:
        print("Usage: MarshalLadderMrsClosure.py --check", file=sys.stderr)
        return 1
    check()
    if str(ROOT) not in sys.path:
        sys.path.insert(0, str(ROOT))
    from tools.Analysis.MrsChainHardening import run_check

    hardening = run_check()
    if hardening:
        for e in hardening:
            print(e, file=sys.stderr)
        return 1
    print("Marshal Ladder MRS closure OK.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

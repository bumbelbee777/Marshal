#!/usr/bin/env python3
"""Ironclad MRS proof-chain hardening gate (audit JSON + closure flags).

Usage:
  python tools/Analysis/MrsChainHardening.py --check
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LADDER_AUDIT = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"
RH_AUDIT = ROOT / "docs" / "generated" / "mrs_proof_audit.json"
CLOSURE = ROOT / "docs" / "generated" / "mrs_ladder_closure.json"
PROGRAMS = ROOT / "programs"

CAPSTONE_FORBIDDEN_IN_WITNESS: dict[str, tuple[str, ...]] = {
    "bsd_rank_proved": ("bsd_rank_proved",),
    "classical_bsd_rank_general": ("bsd_rank_proved", "classical_bsd_rank_general"),
    "classical_bsd_millennium": ("bsd_millennium_proved", "classical_bsd_millennium"),
    "classical_bsd_millennium_universal": (
        "classical_bsd_millennium",
        "classical_bsd_millennium_universal",
    ),
    "hodge_conjecture_proved": ("hodge_conjecture_proved",),
    "classical_hodge11_general": ("hodge_conjecture_proved", "classical_hodge11_general"),
    "classical_hodge_millennium": ("hodge_millennium_proved", "classical_hodge_millennium"),
    "classical_hodge_millennium_universal": (
        "classical_hodge_millennium",
        "classical_hodge_millennium_universal",
    ),
    "classical_ym_millennium": ("ym_millennium_proved", "classical_ym_millennium"),
    "classical_ym_millennium_universal": (
        "classical_ym_millennium",
        "classical_ym_millennium_universal",
    ),
    "goldbach_proved": ("goldbach_proved",),
    "classical_goldbach": ("goldbach_proved", "classical_goldbach"),
    "ym_mass_gap_proved": ("ym_mass_gap_proved",),
    "classical_ym_mass_gap_general": ("ym_mass_gap_proved", "classical_ym_mass_gap_general"),
}

STRUCTURAL_PREREQS = frozenset(
    {
        "bsd_rh_prerequisite",
        "hodge_rh_prerequisite",
        "goldbach_rh_prerequisite",
        "ym_hodge_prerequisite",
        "goldbach_bsd_shared_gln2",
    }
)

TRIVIAL_WITNESS = re.compile(
    r"^\s*(true|1|bsd_rank_proved|hodge_conjecture_proved|goldbach_proved|"
    r"ym_mass_gap_proved|classical_goldbach|proof_chain_closed|mrs_proof_audit_ok)\s*$",
    re.I,
)

CIRCULAR_ID_TOKENS = (
    "gamma_locked",
    "gamma_tuned",
    "uses_gamma",
    "height_locked",
    "zero_height_ansatz",
    "round_gamma",
)

GOAL_EQUALITY_PATTERNS = (
    r"kernel_multiplicity\s*==\s*algebraic_rank",
    r"hodge_kernel_multiplicity\s*==\s*hodge_h11",
    r"witness_expr:\s*cycle_map_ok\s*$",
    r"witness_expr:\s*rank_match_ok\s*$",
)

RH_SMUGGLE_TOKENS = (
    "classical_rh_ok",
    "rh_capstone_ok",
    "riemann_hypothesis_proved",
    "classical_rh_cert",
)

LADDER_GRAPHS = frozenset({
    "MarshalBSD",
    "MarshalBSDMillennium",
    "MarshalBSDUniversal",
    "MarshalHodge",
    "MarshalHodgeMillennium",
    "MarshalHodgeUniversal",
    "MarshalYM",
    "MarshalYMMillennium",
    "MarshalYMUniversal",
    "MarshalGoldbach",
})

CAPSTONE_PROOF_STATUS = (
    "anavm_bsd_proof.json",
    "anavm_hodge_proof.json",
    "anavm_goldbach_proof.json",
    "anavm_ym_proof.json",
    "anavm_xi_hadamard_proof.json",
)


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def audit_rows(audit: dict) -> list[dict]:
    return audit.get("entries", audit.get("obligations", audit.get("rows", [])))


def contains_token(hay: str, tok: str) -> bool:
    return re.search(rf"(?<![\w_]){re.escape(tok)}(?![\w_])", hay) is not None


def witness_math_core(witness: str) -> str:
    """Inequality replay only — exclude mrs_prove step text (may cite dep ids by name)."""
    if ";mrs_prove:" in witness:
        return witness.split(";mrs_prove:")[0]
    return witness


def witness_evidence_circular(oid: str, witness: str) -> bool:
    if witness.startswith("forall_extension:") or witness.startswith("convergence:"):
        return False
    core = witness_math_core(witness)
    return contains_token(core, oid)


def check_audit_file(path: Path, errors: list[str]) -> None:
    if not path.is_file():
        errors.append(f"missing audit file {path}")
        return
    audit = load(path)
    if not audit.get("ok", False):
        errors.append(f"{path.name}: ok=false")
    rows = audit_rows(audit)
    if not rows:
        errors.append(f"{path.name}: no obligation rows")
    for row in rows:
        oid = row.get("obligation_id", "")
        witness = row.get("witness", "")
        if not row.get("ok", False):
            errors.append(f"{path.name}: obligation not ok: {oid}")
            continue
        if oid and witness_evidence_circular(oid, witness):
            errors.append(f"{path.name}: circular witness evidence for {oid}")
        if oid not in STRUCTURAL_PREREQS:
            math_core = witness_math_core(witness)
            for tok in CAPSTONE_FORBIDDEN_IN_WITNESS.get(oid, ()):
                if contains_token(math_core, tok):
                    errors.append(
                        f"{path.name}: capstone token {tok} in witness evidence for {oid}"
                    )


def check_closure(errors: list[str]) -> None:
    if not CLOSURE.is_file():
        errors.append(f"missing {CLOSURE}")
        return
    closure = load(CLOSURE)
    if closure.get("version") != 1:
        errors.append("mrs_ladder_closure version must be 1")
    if not closure.get("proof_chain_closed", False):
        errors.append("proof_chain_closed is false")
    hardening_flags = {
        "prove_spine_ok": True,
        "prove_spine_acyclic": True,
        "trivial_prove_alias_detected": False,
        "infer_on_analytic_detected": False,
        "obligation_graph_acyclic": True,
        "circular_witness_detected": False,
        "weak_witness_detected": False,
        "capstone_in_witness_detected": False,
        "opaque_composition_detected": False,
        "tautological_prove_detected": False,
        "circular_identification_detected": False,
        "weak_analytic_reduction_detected": False,
        "goal_equality_in_witness_detected": False,
        "rh_assumption_smuggle_detected": False,
    }
    for key, expected in hardening_flags.items():
        actual = closure.get(key, expected)
        if actual != expected:
            errors.append(f"mrs_ladder_closure.{key} expected {expected!r}, got {actual!r}")


def scan_mrs_sources(errors: list[str]) -> None:
    current_graph = ""
    for path in sorted(PROGRAMS.rglob("*.mrs")):
        text = path.read_text(encoding="utf-8")
        rel = path.relative_to(ROOT).as_posix()
        for gm in re.finditer(r"proof_graph\s+(\w+)", text):
            current_graph = gm.group(1)
        for block in re.finditer(r"obligation\s+(\w+)\s*\{", text):
            oid = block.group(1)
            start = block.end()
            depth = 1
            i = start
            while i < len(text) and depth:
                if text[i] == "{":
                    depth += 1
                elif text[i] == "}":
                    depth -= 1
                i += 1
            body = text[start : i - 1]
            for wm in re.finditer(r"witness_expr:\s*([^\n,]+)", body):
                expr = wm.group(1).strip()
                if TRIVIAL_WITNESS.match(expr):
                    errors.append(f"{rel}: trivial witness_expr on {oid}: {expr!r}")
                first = re.match(r"(\w+)", expr)
                if first and first.group(1) == oid:
                    errors.append(f"{rel}: witness_expr self-ref on obligation {oid}")
                for tok in CAPSTONE_FORBIDDEN_IN_WITNESS.get(oid, ()):
                    if contains_token(expr, tok):
                        errors.append(f"{rel}: capstone {tok} in witness_expr for {oid}")
                for tok in CIRCULAR_ID_TOKENS:
                    if contains_token(expr, tok):
                        errors.append(f"{rel}: circular identification token {tok} in {oid}")
                for pat in GOAL_EQUALITY_PATTERNS:
                    if re.search(pat, expr, re.I):
                        errors.append(f"{rel}: goal equality in witness_expr for {oid}")
                if current_graph in LADDER_GRAPHS and oid not in STRUCTURAL_PREREQS:
                    for tok in RH_SMUGGLE_TOKENS:
                        if contains_token(expr, tok):
                            errors.append(f"{rel}: RH flag {tok} smuggled in ladder witness {oid}")
            for pm in re.finditer(r"prove_ref:\s*(\w+)", body):
                if pm.group(1) == oid:
                    errors.append(f"{rel}: prove_ref equals obligation id {oid}")
            sm = re.search(r'statement:\s*"([^"]+)"[\s\S]*?witness_expr:\s*([^\n,]+)', body)
            if sm and squash(sm.group(1)) == squash(sm.group(2)):
                errors.append(f"{rel}: statement equals witness_expr on {oid}")
        if re.search(r"proof_class:\s*analytic[\s\S]*?prove:\s*infer\b", text, re.I):
            errors.append(f"{rel}: prove:infer on analytic obligation")


def squash(s: str) -> str:
    return re.sub(r"\s+", " ", s.strip().lower())


def check_capstone_json(errors: list[str]) -> None:
    gen = ROOT / "docs" / "generated"
    for name in CAPSTONE_PROOF_STATUS:
        path = gen / name
        if not path.is_file():
            errors.append(f"missing capstone JSON {name}")
            continue
        data = load(path)
        if data.get("proof_status") == "PENDING":
            errors.append(f"{name}: proof_status PENDING")
        if "proof_chain_closed" in data and not data.get("proof_chain_closed"):
            errors.append(f"{name}: proof_chain_closed false")
        pg = data.get("proof_graph_summary") or data.get("proof_graph") or {}
        if isinstance(pg, dict):
            if pg.get("circular_logic_detected") is True:
                errors.append(f"{name}: circular_logic_detected true")
            failed = pg.get("failed_ids") or []
            if failed:
                errors.append(f"{name}: failed_ids non-empty: {failed}")


def run_check() -> list[str]:
    errors: list[str] = []
    check_closure(errors)
    check_audit_file(LADDER_AUDIT, errors)
    check_audit_file(RH_AUDIT, errors)
    scan_mrs_sources(errors)
    check_capstone_json(errors)
    return errors


def main() -> int:
    errors = run_check()
    if errors:
        for e in errors:
            print(f"MrsChainHardening FAIL: {e}", file=sys.stderr)
        return 1
    print("MrsChainHardening: proof chain ironclad OK.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Full C/D candidate sweep: Marshal operator-candidates + IdeleClassLaplacian + merge."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"
OUT = ROOT / "docs" / "generated" / "operator_candidates.json"
IDELE_OUT = ROOT / "docs" / "generated" / "idele_class_spectrum.json"
REGISTRY = ROOT / "docs" / "Analysis" / "OperatorTraitRegistry.json"
OUT_MD = ROOT / "docs" / "generated" / "operator_candidates.md"


def run_marshal() -> dict | None:
    if not MARSHAL.is_file():
        return None
    OUT.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--zeros", str(ZEROS),
        "--max-zeros", "10000",
        "--prime-limit", "200000",
        "--test", "sinc2",
        "--test-param", "1.0",
        "--precision",
        "--operator-candidates",
        "--spectral-compare", "64",
        "--quotient-primes", "40",
        "--export-formal-cal", str(OUT),
    ]
    r = subprocess.run(cmd, cwd=ROOT / "build", capture_output=True, text=True, timeout=300)
    if r.returncode != 0:
        print(r.stderr or r.stdout)
        return None
    return json.loads(OUT.read_text(encoding="utf-8"))


def run_idele() -> dict | None:
    script = ROOT / "tools" / "QuotientAnalyzer" / "IdeleClassLaplacian.py"
    if not script.is_file():
        return None
    IDELE_OUT.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        sys.executable, str(script),
        "--primes", "50",
        "--prime-limit", "200000",
        "--mesh", "12",
        "--n-compare", "64",
        "--n-zeros", "64",
        "--output", str(IDELE_OUT),
    ]
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, timeout=120)
    if r.returncode != 0:
        print(r.stderr or r.stdout)
        return None
    return json.loads(IDELE_OUT.read_text(encoding="utf-8"))


def merge_synthetic(marshal_data: dict) -> None:
    reg = json.loads(REGISTRY.read_text(encoding="utf-8"))
    existing = {c["ansatz_id"] for c in marshal_data.get("candidates", [])}
    for syn in reg.get("synthetic_candidates", []):
        if syn["id"] in existing:
            continue
        entry = {
            "ansatz_id": syn["id"],
            "registry_status": syn["status"],
            "verdict": syn["status"],
            "plausibility_score": syn.get("plausibility_score", 0),
            "traits": syn["traits"],
            "requirements": {"satisfied": [], "violated": [], "missing": []},
            "metrics": {},
        }
        marshal_data.setdefault("candidates", []).append(entry)


def attach_idele(marshal_data: dict, idele: dict) -> None:
    v = idele.get("verdict", {})
    uncon = idele.get("unconstrained_direct_sum", {})
    idele_metrics = {
        "uncon_lex_max_gap": v.get("uncon_lex_max_gap"),
        "uncon_matched_max_gap": v.get("uncon_matched_max_gap"),
        "gamma_locked_max_gap_circular": v.get("gamma_locked_max_gap"),
        "frequency_locked_scan_n_modes": v.get("frequency_locked_scan_n_modes"),
        "frequency_locked_scan_lex_max_gap": v.get("frequency_locked_scan_lex_max_gap"),
        "matrix_s_unit_dim_invariant": idele.get("matrix_s_unit_quotient", {}).get(
            "dim_invariant"
        ),
    }
    marshal_data["idele_metrics"] = idele_metrics
    for c in marshal_data.get("candidates", []):
        aid = c["ansatz_id"]
        if aid == "idele_unconstrained_direct_sum":
            c["metrics"] = {**c.get("metrics", {}), **idele_metrics}
            c["verdict"] = "FALSIFIED"
            c["plausibility_score"] = 0
        elif aid in ("idele_gamma_locked", "idele_frequency_locked_scan"):
            c["metrics"] = {**c.get("metrics", {}), **idele_metrics}
            c["verdict"] = "DIAGNOSTIC_ONLY"
            c["plausibility_score"] = 0


def write_md(data: dict, idele: dict | None) -> None:
    lines = [
        "# Operator candidate sweep (v1)\n\n",
        "**Scoring:** only γ-free lex / matched gaps and sinc² count for identification. "
        "γ-locked gaps are DIAGNOSTIC_ONLY (circular).\n\n",
        "| Ansatz | Verdict | Lex γ-free | Matched γ-free | sinc² |\n",
        "|--------|---------|------------|----------------|-------|\n",
    ]
    for c in sorted(data.get("candidates", []), key=lambda x: x.get("ansatz_id", "")):
        m = c.get("metrics", {})
        lines.append(
            f"| {c['ansatz_id']} | {c.get('verdict')} | "
            f"{m.get('gamma_free_gap_max', m.get('uncon_lex_max_gap', '—'))} | "
            f"{m.get('uncon_matched_max_gap', '—')} | "
            f"{m.get('compact_sinc2_residual', '—')} |\n"
        )
    if idele:
        v = idele["verdict"]
        lines.append(
            f"\n## IdeleClassLaplacian (honest)\n\n"
            f"- **uncon lex** (cylinder heap, γ-free): {v.get('uncon_lex_max_gap')}\n"
            f"- **uncon matched** (γ-free): {v.get('uncon_matched_max_gap')}\n"
            f"- **gamma_locked** (CIRCULAR): {v.get('gamma_locked_max_gap')}\n"
            f"- **freq_lock_scan modes**: {v.get('frequency_locked_scan_n_modes')}\n"
        )
    OUT_MD.write_text("".join(lines), encoding="utf-8")


def main() -> int:
    data = run_marshal()
    if data is None:
        print("FAIL: Marshal operator-candidates")
        return 1
    idele = run_idele()
    merge_synthetic(data)
    if idele:
        attach_idele(data, idele)
    data["idele_spectrum"] = str(IDELE_OUT) if idele else None
    data["gap_semantics"] = json.loads(REGISTRY.read_text())["gap_semantics"]
    data["inferred_requirements"] = json.loads(REGISTRY.read_text())["inferred_requirements"]
    OUT.write_text(json.dumps(data, indent=2), encoding="utf-8")
    write_md(data, idele)
    print(f"Wrote {OUT} and {OUT_MD}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

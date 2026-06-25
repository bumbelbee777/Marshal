#!/usr/bin/env python3
"""One-shot migration: LemmaManifest lean -> mrs_obligation + proof_script."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "docs" / "Analysis" / "LemmaManifest.json"

MRS_MAP = {
    "classical_rh": ("classical_riemann_hypothesis_marshal", "programs/lib/marshal_hadamard_proof.mrs"),
    "marshal_hadamard": ("MarshalHadamardWeierstrassIdentification", "programs/lib/marshal_hadamard_proof.mrs"),
    "bsd_rank": ("bsd_rank_proved", "programs/lib/marshal_bsd_proof.mrs"),
    "hodge_conjecture": ("hodge_conjecture_proved", "programs/lib/marshal_hodge_proof.mrs"),
    "goldbach": ("classical_goldbach", "programs/lib/marshal_goldbach_proof.mrs"),
    "ym_mass_gap": ("ym_mass_gap_proved", "programs/lib/marshal_ym_proof.mrs"),
}


def infer_mrs(lemma_id: str, lean: str | None) -> tuple[str | None, str | None]:
    for key, pair in MRS_MAP.items():
        if key in lemma_id or (lean and key in lean):
            return pair
    if lean:
        if "Hadamard" in lean or "Xi" in lean or "ClassicalRiemann" in lean:
            return "MarshalHadamardWeierstrassIdentification", "programs/lib/marshal_hadamard_proof.mrs"
        if "GlobalOperator" in lean or "GlobalConnes" in lean:
            return "rooted_dag_limit_contract", "programs/lib/gln_spectral_triple.mrs"
        if "TheoremA" in lean:
            return "theorem_a_pure_scaling", "programs/lib/marshal_theorem_a_proof.mrs"
        if "TheoremB" in lean or "Fortress" in lean:
            return "marshal_theorem_b_closed", "programs/lib/marshal_theorem_a_proof.mrs"
        if "BSD" in lean or "Bsd" in lean:
            return "bsd_rank_proved", "programs/lib/marshal_bsd_proof.mrs"
        if "Hodge" in lean:
            return "hodge_conjecture_proved", "programs/lib/marshal_hodge_proof.mrs"
        if "Goldbach" in lean:
            return "classical_goldbach", "programs/lib/marshal_goldbach_proof.mrs"
        if "YM" in lean or "YMMass" in lean:
            return "ym_mass_gap_proved", "programs/lib/marshal_ym_proof.mrs"
    return None, None


def main() -> int:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    for lem in data.get("lemmas", []):
        lean = lem.pop("lean", None)
        ob, script = infer_mrs(lem.get("id", ""), lean)
        lem["mrs_obligation"] = ob
        lem["proof_script"] = script
    MANIFEST.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    remaining = sum(1 for lem in data["lemmas"] if "lean" in lem)
    print(f"Migrated {len(data['lemmas'])} lemmas; lean keys remaining: {remaining}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

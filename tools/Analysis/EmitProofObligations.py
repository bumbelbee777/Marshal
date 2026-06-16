#!/usr/bin/env python3
"""Emit consolidated proof_obligations.json for v1 global operator track."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REGISTRY = ROOT / "docs" / "Analysis" / "ProofObligationRegistry.json"
MANIFEST = ROOT / "docs" / "Analysis" / "LemmaManifest.json"
OUT = ROOT / "docs" / "generated" / "proof_obligations.json"
GENERATED = ROOT / "docs" / "generated"


def load(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def cert_field(path: Path, *keys: str):
    if not path.is_file():
        return None
    data = load(path)
    for k in keys:
        if k in data:
            return data[k]
    return None


def main() -> int:
    reg = load(REGISTRY)
    manifest = load(MANIFEST)
    manifest_by_id = {x["id"]: x for x in manifest.get("lemmas", [])}

    proved: list[str] = []
    proved_conditional: list[str] = []
    open_obs: list[str] = []
    falsified: list[str] = []
    obligations_out = []

    for ob in reg.get("obligations", []):
        oid = ob["id"]
        m = manifest_by_id.get(oid, {})
        status = m.get("proof_status", ob.get("status", "OPEN"))
        entry = {
            "id": oid,
            "proof_status": status,
            "track": ob.get("track", "v1_global"),
            "depends_on": ob.get("depends_on", m.get("depends_on", [])),
            "approach": ob.get("approach", ""),
            "note": ob.get("note", m.get("note", "")),
            "evidence": ob.get("evidence", ""),
            "gate_verdict": ob.get("gate_verdict"),
            "alias_of": ob.get("alias_of"),
            "lean": ob.get("lean", m.get("lean", "")),
        }
        if status == "PROVED":
            proved.append(oid)
        elif status == "FALSIFIED":
            falsified.append(oid)
        elif status == "PROVED_CONDITIONAL":
            proved_conditional.append(oid)
        elif status in ("OPEN", "ANALYTIC_OPEN", "NUMERICAL_PENDING"):
            open_obs.append(oid)
        obligations_out.append(entry)

    demo = load(GENERATED / "analytic_lemma_demo.json") if (GENERATED / "analytic_lemma_demo.json").is_file() else {}
    limit = load(GENERATED / "global_dirac_limit.json") if (GENERATED / "global_dirac_limit.json").is_file() else {}

    proof_open = [
        oid for oid in open_obs
        if oid in ("self_adjoint_extension_selection", "spectral_discreteness")
    ]
    formal_track_open = [
        oid for oid in open_obs
        if oid in ("proof_obligations_v1",)
    ]
    analytic_open = [
        oid for oid in obligations_out
        if oid.get("proof_status") in ("ANALYTIC_OPEN", "OPEN")
        and oid.get("id") in ("self_adjoint_extension_selection", "spectral_discreteness")
    ]

    out = {
        "version": 1,
        "track": reg.get("track", "global_operator_v1"),
        "proof_status": "V1_OPEN" if proof_open else "FORMAL_ROUTING_COMPLETE",
        "lean_cert": "HP.Global.ProofObligationRegistry",
        "lean_emit_ready": True,
        "v1_finish": reg.get("v1_finish", []),
        "proved": sorted(set(proved)),
        "proved_conditional": sorted(set(proved_conditional)),
        "open_obligations": sorted(set(proof_open + [x["id"] for x in analytic_open])),
        "formal_track_open": sorted(set(formal_track_open)),
        "falsified": sorted(set(falsified)),
        "deprecated": [d["id"] for d in reg.get("deprecated_obligations", [])],
        "resolved_ambiguities": reg.get("resolved_ambiguities", []),
        "obligations": obligations_out,
        "certificates": {
            "analytic_lemma_demo": {
                "path": "docs/generated/analytic_lemma_demo.json",
                "v1_chain_status": demo.get("v1_chain_status"),
                "lean_emit_ready": demo.get("lean_emit_ready"),
            },
            "global_dirac_limit": {
                "path": "docs/generated/global_dirac_limit.json",
                "limit_verdict": limit.get("limit_verdict"),
                "monotone_rmse_increase": limit.get("monotone_rmse_increase"),
            },
        },
        "next_proof_steps": [
            {
                "priority": 1,
                "lemma": "self_adjoint_extension_selection",
                "action": "Theorem A: prove unique Λ_D(θ) minimizer on true Connes D_θ",
                "status": "ANALYTIC_OPEN",
            },
            {
                "priority": 2,
                "lemma": "spectral_discreteness",
                "action": "Theorem B: prove compact resolvent and σ(D_{θ₀})={γₙ}",
                "status": "ANALYTIC_OPEN",
            },
            {
                "priority": 3,
                "lemma": "spectral_det_xi",
                "action": "Lemma 3: PROVED_CONDITIONAL in V1ProofChain.lean given A+B",
                "status": "PROVED_CONDITIONAL",
            },
        ],
    }

    OUT.parent.mkdir(parents=True, exist_ok=True)
    with OUT.open("w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)
        f.write("\n")
    print(f"OK -> {OUT} proof_open={len(proof_open)} proved={len(proved)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

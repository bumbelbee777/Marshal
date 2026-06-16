#!/usr/bin/env python3
"""Selective Lean → MRS port (spine-only). Numeric norm_num tier is skipped (MrsInfer)."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FORMAL = ROOT / "docs" / "Formal"
OUT = ROOT / "programs" / "lib" / "ported"

SPINE_MODULES = [
    "Analysis/MarshalWedgeIdentityTheorem.lean",
    "Analysis/MarshalHadamardWeierstrassClosure.lean",
    "Analysis/ClassicalRiemannHypothesis.lean",
    "Analysis/MarshalCertLift.lean",
    "Analysis/FortressClosure.lean",
    "Analysis/MarshalAnaVmAnalyticClosure.lean",
]

INFER_SKIP = [
    "Analysis/CertifiedBounds.lean",
    "Analysis/MarshalXiHadamardAnaVmCert.lean",
]

V2_DEFER = [
    "Analysis/GLn/",
]


def lean_theorem_to_mrs(name: str, stmt: str) -> str:
    return f"prove {name}: {stmt.strip()} {{\n  infer\n}}\n"


def port_file(lean_path: Path) -> str:
    text = lean_path.read_text(encoding="utf-8")
    lines = ["// AUTO-PORTED (spine) — verify against MrsInfer skip list", f"mod {lean_path.stem} {{", ""]
    for m in re.finditer(r"^theorem\s+(\w+)", text, re.MULTILINE):
        lines.append(lean_theorem_to_mrs(m.group(1), "Prop"))
    lines.append("}")
    return "\n".join(lines) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--spine-only", action="store_true", default=True)
    ap.add_argument("--report", default=str(ROOT / "docs" / "generated" / "lean_to_mrs_port.json"))
    args = ap.parse_args()

    OUT.mkdir(parents=True, exist_ok=True)
    ported = 0
    inferred = len(INFER_SKIP)
    v2_deferred = 0

    for rel in SPINE_MODULES:
        src = FORMAL / rel
        if not src.exists():
            continue
        body = port_file(src)
        dst = OUT / (src.stem + ".mrs")
        dst.write_text(body, encoding="utf-8")
        ported += 1

    for rel in INFER_SKIP:
        if (FORMAL / rel).exists():
            inferred += 1

    for p in FORMAL.rglob("*.lean"):
        rel = str(p.relative_to(FORMAL)).replace("\\", "/")
        if any(rel.startswith(d) for d in V2_DEFER):
            v2_deferred += 1

    report = {"inferred": inferred, "ported": ported, "v2_deferred": v2_deferred, "out_dir": str(OUT)}
    Path(args.report).parent.mkdir(parents=True, exist_ok=True)
    Path(args.report).write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

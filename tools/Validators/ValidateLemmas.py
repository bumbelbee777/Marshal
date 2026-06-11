#!/usr/bin/env python3
"""Cross-check cert proof_status fields against docs/Analysis/LemmaManifest.json."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "docs" / "Analysis" / "LemmaManifest.json"


def load_manifest() -> dict:
    with MANIFEST.open(encoding="utf-8") as f:
        return json.load(f)


def check_generated_docs(manifest: dict) -> list[str]:
    errors: list[str] = []
    gen = ROOT / "docs" / "Generated"
    if not gen.exists():
        return errors
    open_ids = {e["id"] for e in manifest["lemmas"] if e["proof_status"] == "OPEN"}
    for md in gen.glob("*.md"):
        text = md.read_text(encoding="utf-8", errors="replace")
        for line in text.splitlines():
            low = line.lower()
            if "theorem" not in low or "no theorem" in low:
                continue
            for lemma_id in open_ids:
                phrase = lemma_id.replace("_", " ").lower()
                if phrase in low:
                    errors.append(
                        f"{md.name}: contains Theorem-like text for OPEN lemma {lemma_id}"
                    )
                    break
    return errors


def main() -> int:
    if not MANIFEST.exists():
        print(f"FAIL: missing {MANIFEST}")
        return 1
    manifest = load_manifest()
    if manifest.get("version") != 1:
        print(f"FAIL: LemmaManifest version must be 1 (got {manifest.get('version')})")
        return 1
    errors: list[str] = []
    valid_status = {"PROVED", "OPEN", "NUMERICAL", "FALSIFIED", "IMPOSSIBLE", "DISPROVED"}
    for entry in manifest["lemmas"]:
        st = entry.get("proof_status", "")
        if st not in valid_status:
            errors.append(f"lemma {entry['id']} invalid proof_status {st!r}")
        if entry["id"] == "frequency_lock" and st != "IMPOSSIBLE":
            errors.append("frequency_lock must be IMPOSSIBLE")
        if st == "PROVED":
            doc = ROOT / entry["doc"]
            if not doc.exists():
                errors.append(f"PROVED lemma {entry['id']} missing doc {doc}")
        if st in ("FALSIFIED", "IMPOSSIBLE", "DISPROVED"):
            doc = ROOT / entry["doc"]
            if not doc.exists():
                errors.append(f"{st} lemma {entry['id']} missing doc {doc}")
        heat = entry.get("heat")
        if heat and not (ROOT / heat).exists():
            errors.append(f"lemma {entry['id']} missing heat doc {heat}")
    errors.extend(check_generated_docs(manifest))
    if errors:
        for e in errors:
            print(f"FAIL: {e}")
        return 1
    print(f"validate-lemmas OK ({len(manifest['lemmas'])} lemmas)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Cross-check Marshal cert exports against docs/Analysis + docs/Heat (alpha v1)."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "docs" / "Analysis" / "LemmaManifest.json"
HEAT_README = ROOT / "docs" / "Heat" / "README.md"


def fail(msg: str) -> None:
    print(f"FAIL: {msg}")
    raise SystemExit(1)


def load_manifest() -> dict:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if data.get("version") != 1:
        fail(f"LemmaManifest version must be 1 (got {data.get('version')})")
    return data


def check_heat_docs(manifest: dict) -> None:
    if not HEAT_README.exists():
        fail("missing docs/Heat/README.md")
    for entry in manifest["lemmas"]:
        heat = entry.get("heat")
        if heat and not (ROOT / heat).exists():
            fail(f"lemma {entry['id']} missing heat doc {heat}")


def check_cert(cert_path: Path) -> None:
    if not cert_path.exists():
        print(f"SKIP cert: {cert_path} not found")
        return
    cert = json.loads(cert_path.read_text(encoding="utf-8"))
    conv = cert.get("phase_convergence", cert.get("tier5", {}))
    lemmas = conv.get("lemmas", {})
    if lemmas.get("resolvent_limit_status") == "PROVED":
        fail("resolvent_limit_status must stay OPEN in alpha")
    if cert.get("verdict") == "HP_PROVED":
        fail("HP_PROVED forbidden in alpha discipline")
    trace = cert.get("phase_trace_identity", cert.get("tier4", {}))
    if trace.get("lhs_underflow") and cert.get("verdict") not in (
        "INVALID_SPECTRAL_UNDERFLOW",
        "CONTROLLED_WEIL",
        "GEOMETRIC_PASS",
        "GEOMETRIC_PASS_SPECTRAL_FAIL",
    ):
        fail(f"lhs_underflow with unexpected verdict {cert.get('verdict')}")
    spec = cert.get("phase_spectrum_diagnostic", {})
    if spec.get("legacy_frequency_lock"):
        fail("legacy_frequency_lock must be false")
    print(
        f"analytical workload OK: verdict={cert.get('verdict')}, "
        f"tail_status={lemmas.get('tail_bound_status')}, "
        f"resolvent={lemmas.get('resolvent_limit_status', 'OPEN')}"
    )


def main() -> int:
    manifest = load_manifest()
    check_heat_docs(manifest)
    cert = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "build" / "cert" / "demo_cert.json"
    if not cert.is_absolute():
        cert = ROOT / cert
    check_cert(cert)
    print(f"validate-analytical OK ({len(manifest['lemmas'])} lemmas, manifest v1)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

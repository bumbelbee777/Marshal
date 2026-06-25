#!/usr/bin/env python3
"""Zip canonical witness/cert JSON + emit SHA256 manifest for CI artifacts.

Usage:
  python tools/CI/PackageWitnessBundle.py
  python tools/CI/PackageWitnessBundle.py --check
  python tools/CI/PackageWitnessBundle.py --out-dir build/ci-artifacts
"""

from __future__ import annotations

import hashlib
import json
import os
import subprocess
import sys
import zipfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WITNESS_MANIFEST = ROOT / "tools" / "CI" / "ci_witness_manifest.json"
DEFAULT_OUT = ROOT / "build" / "ci-artifacts"


def git_sha() -> str:
    try:
        out = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=ROOT,
            text=True,
            stderr=subprocess.DEVNULL,
        )
        return out.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return os.environ.get("GITHUB_SHA", "unknown")[:40]


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def load_members() -> list[str]:
    spec = json.loads(WITNESS_MANIFEST.read_text(encoding="utf-8"))
    return list(spec.get("bundle_members", []))


def build_bundle(out_dir: Path) -> dict:
    out_dir.mkdir(parents=True, exist_ok=True)
    members = load_members()
    missing: list[str] = []
    entries: list[dict] = []

    for rel in members:
        path = ROOT / rel
        if not path.is_file():
            missing.append(rel)
            continue
        digest = sha256_file(path)
        entries.append(
            {
                "path": rel.replace("\\", "/"),
                "bytes": path.stat().st_size,
                "sha256": digest,
            }
        )

    if missing:
        raise SystemExit(f"PackageWitnessBundle: missing bundle members: {missing}")

    sha = git_sha()[:12]
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    zip_name = f"marshal-witness-bundle-{sha}-{stamp}.zip"
    zip_path = out_dir / zip_name

    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for e in entries:
            zf.write(ROOT / e["path"], arcname=e["path"])

    sums_path = out_dir / "SHA256SUMS.txt"
    with sums_path.open("w", encoding="utf-8") as f:
        f.write(f"{sha256_file(zip_path)}  {zip_name}\n")
        for e in sorted(entries, key=lambda x: x["path"]):
            f.write(f"{e['sha256']}  {e['path']}\n")

    meta = {
        "schema_version": 1,
        "git_sha": git_sha(),
        "created_utc": stamp,
        "zip_file": zip_name,
        "zip_sha256": sha256_file(zip_path),
        "member_count": len(entries),
        "members": entries,
    }
    meta_path = out_dir / "witness_bundle_meta.json"
    meta_path.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")

    return {
        "zip_path": zip_path,
        "sums_path": sums_path,
        "meta_path": meta_path,
        "meta": meta,
    }


def check_bundle(out_dir: Path) -> int:
    meta_path = out_dir / "witness_bundle_meta.json"
    if not meta_path.is_file():
        print(f"MISSING: {meta_path}", file=sys.stderr)
        return 1
    meta = json.loads(meta_path.read_text(encoding="utf-8"))
    zip_path = out_dir / meta["zip_file"]
    if not zip_path.is_file():
        print(f"MISSING: {zip_path}", file=sys.stderr)
        return 1
    if sha256_file(zip_path) != meta["zip_sha256"]:
        print("FAIL: zip sha256 drift", file=sys.stderr)
        return 1
    print(f"PackageWitnessBundle OK: {zip_path.name} ({meta['member_count']} members)")
    return 0


def main() -> int:
    out_dir = DEFAULT_OUT
    if "--out-dir" in sys.argv:
        i = sys.argv.index("--out-dir")
        out_dir = Path(sys.argv[i + 1])

    if "--check" in sys.argv:
        return check_bundle(out_dir)

    result = build_bundle(out_dir)
    print(f"Wrote {result['zip_path']}")
    print(f"Wrote {result['sums_path']}")
    print(f"Wrote {result['meta_path']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

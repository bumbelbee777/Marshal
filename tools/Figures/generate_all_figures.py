#!/usr/bin/env python3
"""Generate all publication figures (PDF + PNG) and manifest."""
from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime, timezone
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.Figures.loaders import GENERATED, cert_path  # noqa: E402
from tools.Figures.meta import FigureMeta  # noqa: E402
from tools.Figures.sections import ALL_RENDERERS  # noqa: E402
from tools.Figures.style import PDF_DIR, PNG_DIR, apply_publication_style  # noqa: E402

REGISTRY = Path(__file__).resolve().parent / "registry.yaml"
MANIFEST = ROOT / "docs" / "figures" / "manifest.json"
FIGURES_TEX = ROOT / "docs" / "figures" / "figures.tex"


def load_registry() -> list[dict]:
    with REGISTRY.open(encoding="utf-8") as f:
        return yaml.safe_load(f)["figures"]


def check_certs(entries: list[dict]) -> list[str]:
    missing = []
    for e in entries:
        for c in e.get("certs", []):
            if c and not cert_path(c).is_file():
                missing.append(f"{e['id']}: {c}")
    return missing


def write_manifest(metas: list[FigureMeta]) -> None:
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "figures": [
            {
                "id": m.figure_id,
                "caption": m.caption,
                "tier": m.tier,
                "sources": m.sources,
                "pdf": str(PDF_DIR / f"fig_{m.figure_id}.pdf"),
                "png": str(PNG_DIR / f"fig_{m.figure_id}.png"),
            }
            for m in metas
        ],
    }
    MANIFEST.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")

    lines = ["% Auto-generated — do not edit", ""]
    for m in metas:
        stem = m.figure_id
        macro = "fig" + stem.replace("_", "")
        lines.append(
            f"\\newcommand{{\\{macro}}}{{\\includegraphics[width=\\linewidth]{{figures/pdf/fig_{stem}.pdf}}}}"
        )
    FIGURES_TEX.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="verify required certs exist")
    parser.add_argument("--list", action="store_true", help="list figure ids")
    parser.add_argument("--only", action="append", help="render subset (e.g. S9)")
    args = parser.parse_args()

    entries = load_registry()
    if args.list:
        for e in entries:
            print(e["id"])
        return 0

    if args.check:
        missing = check_certs(entries)
        if missing:
            for m in missing:
                print(f"MISSING: {m}", file=sys.stderr)
            return 1
        print(f"All required certs present under {GENERATED}")
        return 0

    apply_publication_style()
    ids = {e["id"] for e in entries}
    if args.only:
        want = set()
        for o in args.only:
            want.update(x for x in ids if x.startswith(o) or x == o)
        ids = want

    metas: list[FigureMeta] = []
    for fid in sorted(ids):
        fn = ALL_RENDERERS.get(fid)
        if not fn:
            print(f"No renderer for {fid}", file=sys.stderr)
            return 1
        print(f"Rendering {fid}...")
        metas.append(fn())

    write_manifest(metas)
    print(f"Wrote {MANIFEST} ({len(metas)} figures)")
    print(f"Wrote {FIGURES_TEX}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

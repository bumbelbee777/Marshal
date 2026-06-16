#!/usr/bin/env python3
"""Unit tests for investigation analyzers (synthetic certs)."""
from __future__ import annotations

import json
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

import tools.Analysis.analyzers  # noqa: F401
from tools.Analysis.analyzers.base import REGISTRY


def write_cert(root: Path, name: str, payload: dict) -> None:
    (root / f"{name}.json").write_text(json.dumps(payload), encoding="utf-8")


def test_curvature() -> None:
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        write_cert(
            root,
            "spectral_action_curvature",
            {
                "fixed_theta": 5.76,
                "series": [
                    {"x": 5.5, "y": 2.0, "flag": True},
                    {"x": 5.76, "y": 1.0, "flag": True},
                    {"x": 6.0, "y": 2.1, "flag": True},
                ],
            },
        )
        rep = REGISTRY["spectral_action_curvature"].analyze({}, root)
        assert rep.diagnostic_id == "spectral_action_curvature"
        assert len(rep.gates) == 3


def test_registry() -> None:
    assert len(REGISTRY) == 5


def main() -> int:
    test_registry()
    test_curvature()
    print("analyzer tests OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())

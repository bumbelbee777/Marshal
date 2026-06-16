from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
GENERATED = ROOT / "docs" / "generated"
TRACES = ROOT / "traces"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"

PINNED_DEFAULTS = {
    "theta0": 5.759586531581287,
    "t1_gap": 9.97e-18,
    "moment_l2": 0.0007040364592606541,
    "variational_gap": 0.062143,
}


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def cert_path(name: str) -> Path:
    return GENERATED / name


def load_cert(name: str) -> dict[str, Any]:
    path = cert_path(name)
    if not path.is_file():
        raise FileNotFoundError(path)
    return load_json(path)


def try_cert(name: str) -> dict[str, Any] | None:
    path = cert_path(name)
    return load_json(path) if path.is_file() else None


def pinned_constants() -> dict[str, float]:
    out = dict(PINNED_DEFAULTS)
    demo = try_cert("analytic_lemma_demo.json")
    if demo:
        out["theta0"] = float(demo.get("selected_theta", out["theta0"]))
        out["t1_gap"] = float(demo.get("log_prime_t1_gap", out["t1_gap"]))
        out["moment_l2"] = float(demo.get("moment_l2_distance", out["moment_l2"]))
    bcert = try_cert("marshal_theorem_b_cert.json")
    if bcert:
        b13 = bcert.get("B1_3_variational_gap", bcert.get("variational_gap"))
        if b13 is not None:
            out["variational_gap"] = float(b13)
    return out


def load_zero_heights(max_n: int = 500) -> list[float]:
    if ZEROS.is_file():
        heights: list[float] = []
        with ZEROS.open(encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                heights.append(float(line.split()[0]))
                if len(heights) >= max_n:
                    break
        return heights
    zt = try_cert("marshal_zero_asymptotics.json")
    if zt and "initial_heights" in zt:
        return [float(x) for x in zt["initial_heights"][:max_n]]
    return [14.134725, 21.022040, 25.010858, 30.424876, 32.935062, 37.586178, 40.918719]

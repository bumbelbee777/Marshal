#!/usr/bin/env python3
"""Weil convergence ladders at T=8 (kappa=1) and T=gamma1 (kappa=60); fit b_zeros, b_primes."""
from __future__ import annotations

import json
import math
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
OUT_DIR = ROOT / "docs" / "generated"
T_GAMMA1 = "14.134725142"
T_OPT = "8.0"
PRIME_LIMIT = "8200000"
MAX_ZEROS = "500000"


def fit_power_law(points: list[dict], n_key: str, res_key: str = "weil_residual") -> dict:
    xs, ys = [], []
    for pt in points:
        n = pt.get(n_key, 0)
        r = pt.get(res_key, 0)
        if n > 0 and r > 0:
            xs.append(math.log(float(n)))
            ys.append(math.log(float(r)))
    if len(xs) < 2:
        return {"b": None, "a": None, "n": len(xs)}
    n = len(xs)
    sx = sum(xs)
    sy = sum(ys)
    sxx = sum(x * x for x in xs)
    sxy = sum(x * y for x, y in zip(xs, ys))
    denom = n * sxx - sx * sx
    if abs(denom) < 1e-30:
        return {"b": None, "a": None, "n": n}
    b = -(n * sxy - sx * sy) / denom
    log_a = (sy + b * sx) / n
    return {"b": b, "a": math.exp(log_a), "n": n}


def annotate_fit(path: Path) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
    zfit = fit_power_law(data.get("zero_ladder", []), "n_zeros")
    pfit = fit_power_law(data.get("prime_ladder", []), "n_primes")
    data["fit"] = {"b_zeros": zfit, "b_primes": pfit}
    path.write_text(json.dumps(data, indent=2), encoding="utf-8")
    return data["fit"]


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def marshal_base(test_param: str, kappa: str | None = None) -> list[str]:
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    cmd = [
        str(MARSHAL),
        "--zeros",
        str(zeros),
        "--max-zeros",
        MAX_ZEROS,
        "--prime-limit",
        PRIME_LIMIT,
        "--test",
        "sinc2",
        "--test-param",
        test_param,
        "--precision",
        "--kmax",
        "40",
    ]
    if kappa is not None:
        cmd += ["--sinc2-kappa", kappa]
    return cmd


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    scenarios = [
        ("weil_convergence_T8_kappa1.json", T_OPT, "1"),
        ("weil_convergence_gamma1_kappa60.json", T_GAMMA1, "60"),
    ]

    run(
        marshal_base(T_OPT)
        + [
            "--arch-sinc2-converge",
            "--arch-target",
            "1e-12",
            "--export-arch-sinc2",
            str(OUT_DIR / "arch_sinc2_audit_T8.json"),
        ]
    )

    run(
        marshal_base(T_OPT, "1")
        + [
            "--weil-convergence-study",
            "--export-weil-convergence",
            str(OUT_DIR / "weil_convergence_T8_kappa1_500k.json"),
        ]
    )

    summary: dict = {"scenarios": []}
    for out_name, t_param, kappa in scenarios:
        out_path = OUT_DIR / out_name
        run(
            marshal_base(t_param, kappa)
            + [
                "--weil-convergence-study",
                "--export-weil-convergence",
                str(out_path),
            ]
        )
        fit = annotate_fit(out_path)
        summary["scenarios"].append(
            {
                "file": out_name,
                "T": float(t_param),
                "kappa": float(kappa),
                "b_zeros": fit["b_zeros"].get("b"),
                "b_primes": fit["b_primes"].get("b"),
            }
        )
        print(
            f"{out_name}: b_zeros={fit['b_zeros'].get('b')}  "
            f"b_primes={fit['b_primes'].get('b')}"
        )

    summary_path = OUT_DIR / "weil_convergence_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(f"wrote {summary_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

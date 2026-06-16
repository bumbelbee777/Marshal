#!/usr/bin/env python3
"""Run H_log weighted trace validation suite (T1-T6) + test-function catalog."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "log_prime_validation.json"
CONNES = ROOT / "docs" / "generated" / "connes_crossed_product_study.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
T_GAMMA1 = "14.134725142"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    OUT.parent.mkdir(parents=True, exist_ok=True)
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    max_zeros = "500000" if zeros.suffix == ".zerocache" else "100000"
    cmd = [
        str(MARSHAL),
        "--log-prime-validation",
        "--zeros",
        str(zeros),
        "--max-zeros",
        max_zeros,
        "--prime-limit",
        "8200000",
        "--test",
        "sinc2",
        "--test-param",
        T_GAMMA1,
        "--sinc2-kappa",
        "60",
        "--log-prime-cap",
        "0",
        "--kmax",
        "40",
        "--precision",
        "--counting-window",
        "100",
        "--export-log-prime",
        str(OUT),
        "--export-connes-study",
        str(CONNES),
    ]
    print("+", " ".join(cmd))
    proc = subprocess.run(cmd, cwd=ROOT)
    if proc.returncode != 0:
        print("FAIL: log-prime validation failed")
        return proc.returncode
    if OUT.is_file():
        data = json.loads(OUT.read_text(encoding="utf-8"))
        t1 = data.get("tests", {}).get("T1_weil_vs_marshal", {})
        tf = data.get("tests", {}).get("T_full_weil_identity_sinc2", {})
        tg = data.get("tests", {}).get("T_full_weil_identity_gauss", {})
        td = data.get("tests", {}).get("T_duality_diagnostic", {})
        catalog = data.get("test_function_catalog", [])
        print(f"T1 pass={t1.get('pass')} gap={t1.get('gap')}")
        ts = data.get("tests", {}).get("T_sinc2_sweep", {})
        print(f"Weil identity sinc2 @T={T_GAMMA1} residual={tf.get('residual')}")
        print(
            f"T-sweep best_T={ts.get('best_T')} best_kappa={ts.get('best_kappa')} "
            f"residual={ts.get('best_residual')} pass={ts.get('pass')}"
        )
        print(f"Weil identity Gauss pass={tg.get('pass')} residual={tg.get('residual')}")
        print(f"Spectral diag weil_sinc2_vs_zeros={td.get('weil_prime_sinc2_vs_zeros')}")
        print(f"T5 drift={data.get('tests', {}).get('T5_sinc2_drift_halving')}")
        for row in catalog:
            print(
                f"  catalog {row.get('name')}: t1={row.get('t1_pass')} "
                f"weil={row.get('weil_pass')} residual={row.get('weil_residual')} "
                f"compact_hhat={row.get('has_compact_hhat')}"
            )
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Run H_log weighted trace validation suite (T1-T5)."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "log_prime_validation.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    OUT.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--log-prime-validation",
        "--zeros",
        str(ZEROS),
        "--max-zeros",
        "100000",
        "--prime-limit",
        "500000",
        "--test",
        "sinc2",
        "--test-param",
        "1.0",
        "--log-prime-cap",
        "0",
        "--kmax",
        "20",
        "--precision",
        "--counting-window",
        "100",
        "--export-log-prime",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    proc = subprocess.run(cmd, cwd=ROOT)
    if proc.returncode != 0:
        print("FAIL: T1 weighted trace verification failed")
        return proc.returncode
    if OUT.is_file():
        data = json.loads(OUT.read_text(encoding="utf-8"))
        t1 = data.get("tests", {}).get("T1_weil_vs_marshal", {})
        tf = data.get("tests", {}).get("T_full_weil_identity_sinc2", {})
        tg = data.get("tests", {}).get("T_full_weil_identity_gauss", {})
        td = data.get("tests", {}).get("T_duality_diagnostic", {})
        print(f"T1 pass={t1.get('pass')} gap={t1.get('gap')}")
        ts = data.get("tests", {}).get("T_sinc2_sweep", {})
        print(f"Weil identity sinc2 @fixed T residual={tf.get('residual')}")
        print(
            f"T-sweep best_T={ts.get('best_T')} residual={ts.get('best_residual')} "
            f"pass={ts.get('pass')}"
        )
        print(f"Weil identity Gauss pass={tg.get('pass')} residual={tg.get('residual')}")
        print(f"Spectral diag weil_sinc2_vs_zeros={td.get('weil_prime_sinc2_vs_zeros')}")
        print(f"Spectral diag p_weight_sinc2_vs_zeros={td.get('p_weight_sinc2_vs_zeros')} (misweighted)")
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

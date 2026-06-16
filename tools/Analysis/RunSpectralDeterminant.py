#!/usr/bin/env python3
"""Spectral determinant: Laplace h(t)=exp(-a|t|) + arch boundary sweep + mpmath xi audit."""
from __future__ import annotations

import json
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
OUT = ROOT / "docs" / "generated" / "spectral_determinant.json"
MRS = ROOT / "programs" / "adelic_cauchy_completion.mrs"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def load_gammas(cache: Path, n: int) -> list[float]:
    with cache.open("rb") as f:
        f.read(16)
        count = struct.unpack("Q", f.read(8))[0]
        want = min(n, count)
        return list(struct.unpack(f"{want}d", f.read(want * 8)))


def mpmath_log_xi_offset(gammas: list[float], offset: float = 0.5) -> list[dict]:
    import mpmath as mp

    mp.mp.dps = 80
    out = []
    for g in gammas[:20]:
        t = g + offset
        s = mp.mpc("0.5", str(t))
        xi = mp.mpf("0.5") * s * (s - 1) * mp.power(mp.pi, -s / 2) * mp.gamma(s / 2) * mp.zeta(s)
        out.append({"gamma": float(g), "t_im": float(t), "log_abs_xi": float(mp.log(mp.fabs(xi)))})
    return out


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    OUT.parent.mkdir(parents=True, exist_ok=True)
    sweep = "--spectral-det-sweep" in sys.argv
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(zeros),
        "--max-zeros",
        "50000",
        "--prime-limit",
        "500000",
        "--duality-a",
        "1.0",
        "--fast",
        "--skip-archimedean-sweep",
        "--spectral-det-sweep" if sweep else "--spectral-determinant",
        "--export-spectral-det",
        str(OUT),
    ]
    print("+", " ".join(cmd))
    rc = subprocess.run(cmd, cwd=ROOT).returncode
    if rc != 0:
        return rc
    if not OUT.is_file():
        print("FAIL: missing output JSON")
        return 1
    rep = json.loads(OUT.read_text(encoding="utf-8"))
    print(
        f"boundary={rep.get('boundary')}  laplace_weil_res={rep.get('laplace_weil_residual')}  "
        f"log_det_gap={rep.get('log_det_gap')}  xi_det_gap={rep.get('xi_det_gap')}"
    )
    if rep.get("boundary_sweep"):
        for row in rep["boundary_sweep"]:
            print(f"  {row['boundary']}: xi_gap={row['xi_det_gap']:.4g}  weil={row['laplace_weil_residual']:.4g}")
    if ZEROS.is_file():
        try:
            gammas = load_gammas(ZEROS, 20)
            audit = mpmath_log_xi_offset(gammas)
            audit_path = OUT.with_name("spectral_determinant_mpmath_audit.json")
            audit_path.write_text(json.dumps({"xi_samples": audit}, indent=2), encoding="utf-8")
            print(f"mpmath audit: {audit_path}")
        except ImportError:
            print("WARN: mpmath not installed")
    return 0


if __name__ == "__main__":
    sys.exit(main())

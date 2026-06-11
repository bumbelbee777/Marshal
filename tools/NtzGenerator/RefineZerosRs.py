#!/usr/bin/env python3
"""Riemann-Siegel zero refinement: C++ mmap batches + mpmath Newton + validation surrogate."""
from __future__ import annotations

import argparse
import json
import mmap
import random
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TRACES = ROOT / "build" / "cert"
MARSHAL = ROOT / "build" / "Marshal.exe"


def build() -> None:
    cmd = [
        "g++", "-std=c++20", "-O2", "-fopenmp",
        "-I", str(ROOT),
        str(ROOT / "weil_zeros_refine.cxx"),
        "-o", str(EXE),
    ]
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True, cwd=ROOT)


def mmap_gammas(path: Path, n: int, offset: int = 0) -> list[float]:
    """Zero-copy mmap scan; parse only requested window."""
    out: list[float] = []
    with path.open("rb") as f:
        mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
        idx = 0
        start = 0
        for i in range(mm.size()):
            if mm[i] == ord("\n"):
                if idx >= offset:
                    line = mm[start:i].strip()
                    if line:
                        out.append(float(line))
                        if len(out) >= n:
                            break
                idx += 1
                start = i + 1
        mm.close()
    return out


def _refine_worker(gamma: float) -> tuple[float, float, float, float]:
    import mpmath as mp

    mp.mp.dps = 55
    t = mp.mpf(repr(gamma))
    z0 = mp.siegelz(t)
    for _ in range(12):
        z = mp.siegelz(t)
        if abs(z) < mp.mpf("1e-40"):
            break
        h = mp.mpf("1e-8") * (mp.mpf("1") + abs(t))
        zp = (mp.siegelz(t + h) - mp.siegelz(t - h)) / (2 * h)
        if abs(zp) < mp.mpf("1e-50"):
            break
        t -= z / zp
    zf = mp.siegelz(t)
    return float(gamma), float(t), float(abs(z0)), float(abs(zf))


def refine_python_pool(gammas: list[float], workers: int, batch: int) -> list[float]:
    refined = [0.0] * len(gammas)
    with ProcessPoolExecutor(max_workers=workers) as pool:
        futs = {pool.submit(_refine_worker, g): i for i, g in enumerate(gammas)}
        for fut in as_completed(futs):
            i = futs[fut]
            _, t, _, _ = fut.result()
            refined[i] = t
    return refined


def run_cpp_refine(**kw) -> dict:
    batch_script = ROOT / "scripts" / "rs_batch.py"
    cmd = [
        str(EXE),
        "--input", str(kw["input"]),
        "--output", str(kw["output"]),
        "--report", str(TRACES / "zeros_refine_report.json"),
        "--max-zeros", str(kw["max_zeros"]),
        "--offset", str(kw.get("offset", 0)),
        "--batch", str(kw.get("batch", 64)),
        "--threads", str(kw.get("threads", 0)),
        "--root", str(ROOT),
        "--python", sys.executable,
        "--batch-script", str(batch_script.relative_to(ROOT)),
    ]
    print("+", " ".join(cmd))
    t0 = time.time()
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    print(proc.stdout)
    if proc.stderr:
        print(proc.stderr, file=sys.stderr)
    proc.check_returncode()
    report = json.loads((TRACES / "zeros_refine_report.json").read_text())
    report["python_wall_s"] = time.time() - t0
    return report


def mpmath_validate(refined_path: Path, coarse_path: Path, sample: int) -> dict:
    import mpmath as mp

    mp.mp.dps = 60
    refined = mmap_gammas(refined_path, sample)
    coarse = mmap_gammas(coarse_path, sample)
    z_vals, shifts = [], []
    for i, (gr, gc) in enumerate(zip(refined, coarse)):
        g = mp.mpf(str(gr))
        z = mp.siegelz(g)
        z_vals.append(float(abs(z)))
        shifts.append(gr - gc)
        if i < 5:
            print(f"  n={i+1} gamma={gr:.21f} |Z|={z_vals[-1]:.3e} shift={shifts[-1]:.3e}")
    return {
        "sample_n": len(refined),
        "max_abs_siegelz": max(z_vals) if z_vals else None,
        "mean_abs_siegelz": sum(z_vals) / max(len(z_vals), 1),
        "max_gamma_shift": max(abs(s) for s in shifts) if shifts else 0,
        "mpmath_pass": max(z_vals) < 1e-12 if z_vals else False,
    }


def weil_residual(zeros_path: Path, max_zeros: int, sigma: float) -> dict | None:
    if not WEIL.exists():
        return None
    cmd = [
        str(WEIL),
        "--zeros", str(zeros_path.relative_to(ROOT)),
        "--max-zeros", str(max_zeros),
        "--prime-limit", "10000000",
        "--sigma", str(sigma),
        "--precision", "--sign-check", "--deterministic", "--no-cache", "--simd", "scalar",
    ]
    print("+", " ".join(cmd))
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    residual = None
    for line in proc.stdout.splitlines():
        if "Residual" in line:
            try:
                residual = float(line.split()[-1])
            except ValueError:
                pass
    return {"sigma": sigma, "residual": residual}


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--max-zeros", type=int, default=2000)
    ap.add_argument("--offset", type=int, default=0)
    ap.add_argument("--batch", type=int, default=64)
    ap.add_argument("--threads", type=int, default=4)
    ap.add_argument("--workers", type=int, default=4, help="Python pool workers (pure-python mode)")
    ap.add_argument("--mode", choices=["cpp", "python", "both"], default="cpp")
    ap.add_argument("--input", default=str(ROOT / "odlyzko_zeros2m.txt"))
    ap.add_argument("--output", default=str(TRACES / "zeros_refined.txt"))
    ap.add_argument("--sample", type=int, default=32)
    ap.add_argument("--sigma", type=float, default=5.0)
    ap.add_argument("--skip-build", action="store_true")
    ap.add_argument("--skip-weil", action="store_true")
    args = ap.parse_args()

    TRACES.mkdir(exist_ok=True)
    input_path = Path(args.input)
    output_path = Path(args.output)

    if not args.skip_build and args.mode in ("cpp", "both"):
        build()

    summary: dict = {}

    if args.mode in ("cpp", "both"):
        summary["cpp"] = run_cpp_refine(
            input=input_path,
            output=output_path,
            max_zeros=args.max_zeros,
            offset=args.offset,
            batch=args.batch,
            threads=args.threads,
        )

    if args.mode == "python":
        print("Python pool refine (mmap read)...")
        t0 = time.time()
        gammas = mmap_gammas(input_path, args.max_zeros, args.offset)
        refined = refine_python_pool(gammas, args.workers, args.batch)
        with output_path.open("w", encoding="utf-8") as out:
            for g in refined:
                out.write(f"{g:.21f}\n")
        summary["python"] = {
            "n_refined": len(refined),
            "wall_s": time.time() - t0,
            "engine": "python_mpmath_pool",
        }
        print(f"Wrote {output_path} ({len(refined)} zeros, {summary['python']['wall_s']:.1f}s)")

    print("\n=== mpmath validation surrogate ===")
    val = mpmath_validate(output_path, input_path, args.sample)
    print(json.dumps(val, indent=2))
    summary["mpmath_validation"] = val

    if not args.skip_weil:
        n = min(args.max_zeros, 100_000)
        summary["weil_coarse"] = weil_residual(input_path, n, args.sigma)
        summary["weil_refined"] = weil_residual(output_path, n, args.sigma)

    out_json = TRACES / "zeros_refine_validation.json"
    out_json.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(f"Wrote {out_json}")
    sys.exit(0 if val.get("mpmath_pass") else 1)


if __name__ == "__main__":
    main()

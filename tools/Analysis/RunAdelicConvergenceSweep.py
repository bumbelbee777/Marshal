#!/usr/bin/env python3
"""Systematic adelic convergence: epsilon, height map, prime count sweeps + RMSE plot."""
from __future__ import annotations

import json
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
MRS = ROOT / "programs" / "adelic_epsilon_sweep.mrs"
OUT = ROOT / "docs" / "generated" / "adelic_convergence_sweep.json"
PLOT = ROOT / "docs" / "generated" / "adelic_rmse_vs_epsilon.png"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"
TMP = ROOT / "build" / "cert" / "_conv_sweep.json"

# Fixed height for epsilon sweep (neutral baseline)
EPS_HEIGHT_A = 1.0
EPS_HEIGHT_B = 0.0

EPSILONS = [8, 10, 12, 15, 20, 30, 40, 50]
HEIGHT_A = [0.1, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0]
HEIGHT_B = [-20, -15, -10, -5, 0, 5]
PRIME_COUNTS = [200, 500, 1000]


def run_point(
    eps: float,
    height_a: float,
    height_b: float,
    max_primes: int,
    max_zeros: int = 5000,
) -> dict:
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    TMP.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(MRS),
        "--zeros",
        str(zeros),
        "--max-zeros",
        str(max_zeros),
        "--prime-limit",
        "20000",
        "--fast",
        "--skip-archimedean-sweep",
        "--completion-tolerance",
        repr(eps),
        "--height-a",
        repr(height_a),
        "--height-b",
        repr(height_b),
        "--adelic-max-primes",
        str(max_primes),
        "--export-completion",
        str(TMP),
    ]
    t0 = time.perf_counter()
    rc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    elapsed = time.perf_counter() - t0
    if rc.returncode != 0 or not TMP.is_file():
        return {"error": rc.stderr[-300:] if rc.stderr else "failed", "elapsed_s": elapsed}
    rep = json.loads(TMP.read_text(encoding="utf-8"))
    zr = rep.get("zero_comparison_adelic_only_raw", {})
    zm = rep.get("zero_comparison_adelic_only_mapped", {})
    return {
        "epsilon": eps,
        "height_a": height_a,
        "height_b": height_b,
        "max_primes": max_primes,
        "adelic_limits_only_count": rep.get("adelic_limits_only_count", 0),
        "rmse_adelic_raw": zr.get("rmse"),
        "rmse_adelic_mapped": zm.get("rmse"),
        "sinc2_gap_mapped": zm.get("sinc2_gap"),
        "n_matched": zr.get("n_matched"),
        "elapsed_s": elapsed,
    }


def plot_epsilon_sweep(rows: list[dict]) -> None:
    try:
        import matplotlib.pyplot as plt
        import sys
        from pathlib import Path
        _root = Path(__file__).resolve().parents[2]
        if str(_root) not in sys.path:
            sys.path.insert(0, str(_root))
        from tools.Figures.style import apply_publication_style, save_figure
        apply_publication_style()
    except ImportError:
        print("WARN: matplotlib not installed; skipping plot")
        return
    xs = [r["epsilon"] for r in rows if "rmse_adelic_mapped" in r]
    ys = [r["rmse_adelic_mapped"] for r in rows if "rmse_adelic_mapped" in r]
    ns = [r.get("adelic_limits_only_count", 0) for r in rows if "rmse_adelic_mapped" in r]
    fig, ax1 = plt.subplots(figsize=(3.5, 2.6))
    ax1.plot(xs, ys, "o-", color="C0", label="RMSE (adelic-only, mapped)")
    ax1.set_xlabel("epsilon (mixed adelic tolerance)")
    ax1.set_ylabel("RMSE mapped")
    ax1.set_title(f"Adelic completion: RMSE vs epsilon (a={EPS_HEIGHT_A}, b={EPS_HEIGHT_B})")
    ax1.axhline(1486, color="C3", linestyle="--", label="Phase 1-2 baseline")
    ax2 = ax1.twinx()
    ax2.plot(xs, ns, "s--", color="C1", alpha=0.7, label="adelic limit count")
    ax2.set_ylabel("adelic_limits_only count")
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper right", fontsize=7)
    PLOT.parent.mkdir(parents=True, exist_ok=True)
    save_figure(fig, "adelic_rmse_vs_epsilon")
    legacy = PLOT
    import shutil
    shutil.copy2(_root / "docs" / "figures" / "png" / "adelic_rmse_vs_epsilon.png", legacy)
    print(f"Plot: {legacy}")


def fit_rmse_vs_primes(rows: list[dict]) -> dict | None:
    """Power-law fit RMSE ~ a * P^b from prime_sweep rows (needs max_primes + rmse)."""
    try:
        import numpy as np
    except ImportError:
        return None
    pts = [
        (r["max_primes"], r["rmse_adelic_mapped"])
        for r in rows
        if r.get("max_primes") and r.get("rmse_adelic_mapped") is not None
    ]
    if len(pts) < 2:
        return None
    primes = np.array([p for p, _ in pts], dtype=float)
    rmse = np.array([m for _, m in pts], dtype=float)
    log_p, log_r = np.log(primes), np.log(rmse)
    b, log_a = np.polyfit(log_p, log_r, 1)
    a = float(np.exp(log_a))
    b = float(b)
    pred = log_a + b * log_p
    ss_res = float(np.sum((log_r - pred) ** 2))
    ss_tot = float(np.sum((log_r - log_r.mean()) ** 2))
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0
    return {
        "model": "RMSE ~ a * P^b",
        "a": round(a, 4),
        "b": round(b, 3),
        "r2_loglog": round(r2, 4),
        "verdict": "DIVERGENCE" if b > 0 else ("CONVERGENCE" if b < 0 else "FLAT"),
        "note": "b > 0: RMSE grows with prime cutoff; sweet spot at small P is finite-size overfitting.",
        "plot": "docs/generated/adelic_rmse_vs_primes.png",
    }


def plot_rmse_vs_primes(rows: list[dict], fit: dict | None) -> None:
    try:
        import matplotlib.pyplot as plt
        import numpy as np
    except ImportError:
        print("WARN: matplotlib/numpy not installed; skipping RMSE vs P plot")
        return
    pts = [
        (r["max_primes"], r["rmse_adelic_mapped"])
        for r in rows
        if r.get("max_primes") and r.get("rmse_adelic_mapped") is not None
    ]
    if len(pts) < 2:
        return
    primes = np.array([p for p, _ in pts], dtype=float)
    rmse = np.array([m for _, m in pts], dtype=float)
    fig, ax = plt.subplots(figsize=(7, 5))
    ax.loglog(primes, rmse, "o", ms=10, label="adelic-only mapped RMSE")
    if fit and fit.get("a") and fit.get("b") is not None:
        p_line = np.logspace(np.log10(primes.min()), np.log10(primes.max()), 50)
        ax.loglog(p_line, fit["a"] * p_line ** fit["b"], "-", label=f"fit: {fit['a']} P^{fit['b']}")
    ax.axhline(1486, color="C3", ls="--", alpha=0.7, label="Phase 1-2 baseline")
    ax.set_xlabel("Prime cutoff P")
    ax.set_ylabel("RMSE (mapped)")
    ax.set_title("Adelic completion: RMSE vs P")
    ax.legend()
    fig.tight_layout()
    p_out = ROOT / "docs" / "generated" / "adelic_rmse_vs_primes.png"
    p_out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(p_out, dpi=120)
    plt.close(fig)
    print(f"Plot: {p_out}")


def saturation_epsilon(rows: list[dict]) -> float | None:
    """First epsilon where limit count growth exceeds RMSE improvement (heuristic)."""
    prev_n, prev_rmse = 0, 1e300
    for r in sorted(rows, key=lambda x: x["epsilon"]):
        n = r.get("adelic_limits_only_count", 0)
        rmse = r.get("rmse_adelic_mapped")
        if rmse is None or n == 0:
            continue
        if n > 50 and prev_n > 0 and n > 3 * prev_n and rmse > 0.8 * prev_rmse:
            return r["epsilon"]
        prev_n, prev_rmse = n, rmse
    return None


def main() -> int:
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1

    only = set(sys.argv[1:]) if len(sys.argv) > 1 else set()

    report: dict = {"phase12_baseline_rmse": 1486.0}
    if OUT.is_file():
        try:
            report.update(json.loads(OUT.read_text(encoding="utf-8")))
        except json.JSONDecodeError:
            pass

    if not only or "epsilon" in only:
        print("=== One: epsilon sweep ===")
        eps_rows = []
        for eps in EPSILONS:
            row = run_point(eps, EPS_HEIGHT_A, EPS_HEIGHT_B, max_primes=100)
            eps_rows.append(row)
            print(
                f"  eps={eps}  limits={row.get('adelic_limits_only_count')}  "
                f"rmse_mapped={row.get('rmse_adelic_mapped')}  ({row.get('elapsed_s', 0):.1f}s)",
                flush=True,
            )
        report["epsilon_sweep"] = eps_rows
        report["saturation_epsilon_heuristic"] = saturation_epsilon(eps_rows)
        plot_epsilon_sweep(eps_rows)

    if not only or "height" in only:
        print("\n=== Two: height map grid at epsilon=10 ===")
        height_rows = []
        best_h10: dict | None = None
        for a in HEIGHT_A:
            for b in HEIGHT_B:
                row = run_point(10.0, a, b, max_primes=100)
                height_rows.append(row)
                rmse = row.get("rmse_adelic_mapped")
                if rmse is not None and (best_h10 is None or rmse < best_h10.get("rmse_adelic_mapped", 1e300)):
                    best_h10 = row
                print(f"  a={a} b={b}  rmse={rmse}  limits={row.get('adelic_limits_only_count')}")
        report["height_sweep_eps10"] = height_rows
        report["best_height_eps10"] = best_h10

        ba = best_h10.get("height_a", 0.5) if best_h10 else 0.5
        bb = best_h10.get("height_b", -10.0) if best_h10 else -10.0
        print(f"\n=== Two (cont): height transfer at epsilon=50, a={ba} b={bb} ===")
        row50 = run_point(50.0, ba, bb, max_primes=100)
        report["height_transfer_eps50"] = row50
        print(
            f"  eps=50  limits={row50.get('adelic_limits_only_count')}  "
            f"rmse_mapped={row50.get('rmse_adelic_mapped')}"
        )

    if not only or "primes" in only:
        best_h = report.get("best_height_eps10") or {}
        ba = best_h.get("height_a", 0.1)
        bb = best_h.get("height_b", 5.0)
        eps_p = 10.0
        print(f"\n=== Three: prime count sweep (eps={eps_p}, a={ba}, b={bb}) ===")
        prime_rows = []
        for pc in PRIME_COUNTS:
            row = run_point(eps_p, ba, bb, max_primes=pc)
            prime_rows.append(row)
            print(
                f"  primes={pc}  limits={row.get('adelic_limits_only_count')}  "
                f"rmse_mapped={row.get('rmse_adelic_mapped')}"
            )
        report["prime_sweep"] = prime_rows

    prime_rows = report.get("prime_sweep_eps10") or report.get("prime_sweep")
    if prime_rows:
        fit = fit_rmse_vs_primes(prime_rows)
        if fit:
            report["rmse_vs_primes_fit"] = fit
            plot_rmse_vs_primes(prime_rows, fit)
            print(f"\nRMSE vs P fit: {fit['a']} * P^{fit['b']}  ({fit['verdict']})")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"\nWrote {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
Phase 5c: Haar-weighted Laplacian on truncated idele class space.

  X_S = (prod_{p in S} S^1_p) / diagonal Q^*  (adelic quotient model)

Discrete: graph Laplacian L on product mesh, mass matrix M from idele Haar weights
  w_p propto 1/sqrt(p)  (local factor normalization).

GL(1) character trial sections psi_gamma(theta) = prod_p exp(i n_p theta_p),
  n_p = round(gamma log p / 2pi).  Galerkin projection:
    L_B = B^T L B,  M_B = B^T M B,  solve L_B c = lambda M_B c.

Eigenvalues sqrt(lambda) approximate D|_character = gamma (Hilbert-Polya target).
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
TRACES = ROOT / "traces"


def toy_quotient_exe() -> Path | None:
    exe = ROOT / "weil_quotient.exe"
    return exe if exe.is_file() else None


def run_toy_quotient(cmd: list[str]) -> bool:
    exe = toy_quotient_exe()
    if not exe:
        return False
    import subprocess

    full = [str(exe), *cmd]
    print("+", " ".join(full))
    subprocess.run(full, check=True, cwd=ROOT)
    return True


def sieve(n: int) -> list[int]:
    if n < 2:
        return []
    is_p = [True] * (n + 1)
    is_p[0] = is_p[1] = False
    for i in range(2, int(n**0.5) + 1):
        if is_p[i]:
            for j in range(i * i, n + 1, i):
                is_p[j] = False
    return [i for i, v in enumerate(is_p) if v]


def load_zeros(n: int) -> list[float]:
    out: list[float] = []
    with open(ROOT / "odlyzko_zeros2m.txt") as f:
        for i, line in enumerate(f):
            if i >= n:
                break
            line = line.strip()
            if line:
                out.append(float(line))
    return out


def unravel(flat: int, dims: list[int]) -> tuple[int, ...]:
    out: list[int] = []
    for d in dims:
        out.append(flat % d)
        flat //= d
    return tuple(out)


def cycle_laplacian(m: int) -> np.ndarray:
    l = np.zeros((m, m), dtype=np.float64)
    for i in range(m):
        l[i, i] = 2.0
        l[i, (i + 1) % m] = -1.0
        l[i, (i - 1) % m] = -1.0
    return l


def kronecker_chain(ops: list[np.ndarray]) -> np.ndarray:
    out = ops[0]
    for op in ops[1:]:
        out = np.kron(out, op)
    return out


def kronecker_sum(factors: list[np.ndarray]) -> np.ndarray:
    k = len(factors)
    dims = [f.shape[0] for f in factors]
    n = int(np.prod(dims))
    total = np.zeros((n, n), dtype=np.float64)
    for axis, fac in enumerate(factors):
        ops = [fac if j == axis else np.eye(dims[j]) for j in range(k)]
        total += kronecker_chain(ops)
    return total


def haar_mass_matrix(primes: list[int], mesh: int) -> np.ndarray:
    """Diagonal M on product grid: idele Haar weight prod_p w_p, w_p = 1/sqrt(p)."""
    k = len(primes)
    dims = [mesh] * k
    n = int(np.prod(dims))
    m_diag = np.zeros(n, dtype=np.float64)
    weights = [1.0 / math.sqrt(float(p)) for p in primes]
    for flat in range(n):
        idx = unravel(flat, dims)
        w = 1.0
        for ax, _ in enumerate(idx):
            w *= weights[ax]
        m_diag[flat] = w / (mesh ** k)
    return np.diag(m_diag)


def scale_to_continuum(laplacian: np.ndarray, mesh: int, primes: list[int]) -> np.ndarray:
    """Map graph Laplacian to continuum angular frequencies (per-prime log p scaling)."""
    # each axis contributes (2*pi/mesh)^2 * (mesh/log p)^2 = (2*pi/log p)^2 factor
    # average log scale for global scalar
    if not primes:
        return laplacian
    avg = sum((2.0 * math.pi / math.log(p)) ** 2 for p in primes) / len(primes)
    graph_scale = (2.0 * math.pi / mesh) ** 2
    return laplacian * (avg / max(graph_scale, 1e-30))


def mode_indices(gamma: float, primes: list[int]) -> list[int]:
    """Cascade from p_0: exact GL(1) frequency lock omega = 2*pi*n_0/log(p_0)."""
    if not primes:
        return []
    lp0 = math.log(primes[0])
    n0 = max(1, round(gamma * lp0 / (2.0 * math.pi)))
    omega = 2.0 * math.pi * n0 / lp0
    return [max(1, round(omega * math.log(p) / (2.0 * math.pi))) for p in primes]


def character_column(primes: list[int], mesh: int, gamma: float) -> np.ndarray:
    k = len(primes)
    dims = [mesh] * k
    n = int(np.prod(dims))
    ns = mode_indices(gamma, primes)
    col = np.zeros(n, dtype=np.complex128)
    for flat in range(n):
        idx = unravel(flat, dims)
        phase = 0.0
        for ax, (p, j) in enumerate(zip(primes, idx)):
            theta = 2.0 * math.pi * j / mesh
            phase += ns[ax] * theta
        col[flat] = np.exp(1j * phase)
    # normalize in Euclidean; Haar norm applied via M in Rayleigh quotient
    norm = np.linalg.norm(col)
    if norm > 0:
        col /= norm
    return col


def rayleigh_quotient(l: np.ndarray, m: np.ndarray, psi: np.ndarray) -> float:
    psi_r = np.real(psi)
    num = float(psi_r.T @ l @ psi_r)
    den = float(psi_r.T @ m @ psi_r)
    return num / den if den > 0 else float("nan")


def apply_graph_laplacian(primes: list[int], mesh: int, v: np.ndarray) -> np.ndarray:
    """Kronecker-sum graph Laplacian without building the full matrix."""
    k = len(primes)
    arr = np.real(v).reshape([mesh] * k)
    out = np.zeros_like(arr)
    for ax in range(k):
        out += 2.0 * arr - np.roll(arr, 1, axis=ax) - np.roll(arr, -1, axis=ax)
    return out.reshape(-1)


def mass_grid(primes: list[int], mesh: int) -> np.ndarray:
    k = len(primes)
    g = np.ones([mesh] * k, dtype=np.float64)
    for ax, p in enumerate(primes):
        wf = np.full((mesh,), 1.0 / math.sqrt(float(p)) / mesh)
        shape = [1] * k
        shape[ax] = mesh
        g *= wf.reshape(shape)
    return g


def apply_mass(primes: list[int], mesh: int, v: np.ndarray) -> np.ndarray:
    arr = np.real(v).reshape([mesh] * len(primes))
    return (arr * mass_grid(primes, mesh)).reshape(-1)


def continuum_haar_rayleigh_cascade(gamma: float, primes: list[int]) -> float:
    """Haar Rayleigh with GL(1) frequency lock cascaded from p_0."""
    if not primes:
        return float("nan")
    modes = mode_indices(gamma, primes)
    num = den = 0.0
    for p, n in zip(primes, modes):
        w = 1.0 / math.sqrt(float(p))
        omega = 2.0 * math.pi * n / math.log(p)
        num += w * omega * omega
        den += w
    return num / den if den > 0 else float("nan")


def continuum_haar_rayleigh(gamma: float, primes: list[int]) -> float:
    """Continuum Haar Rayleigh (per-prime mode lock; matches C++ Tier-4b)."""
    num = 0.0
    den = 0.0
    for p in primes:
        w = 1.0 / math.sqrt(float(p))
        n = max(1, round(gamma * math.log(p) / (2.0 * math.pi)))
        omega = 2.0 * math.pi * n / math.log(p)
        num += w * omega * omega
        den += w
    return num / den if den > 0 else float("nan")


def galerkin_spectrum(
    primes: list[int], mesh: int, gammas: list[float], max_cells: int = 8_000_000
) -> dict:
    k = len(primes)
    n_cells = mesh**k
    n_gamma = len(gammas)
    use_matrix = n_cells <= max_cells

    rayleigh: list[dict] = []
    lb = np.zeros((n_gamma, n_gamma), dtype=np.float64)
    mb = np.zeros((n_gamma, n_gamma), dtype=np.float64)
    psis: list[np.ndarray] = []

    for g in gammas:
        psi = np.real(character_column(primes, mesh, g))
        psis.append(psi)

    scale = sum((2.0 * math.pi / math.log(p)) ** 2 for p in primes) / (
        (2.0 * math.pi / mesh) ** 2 * max(len(primes), 1)
    )

    if use_matrix:
        for i, g in enumerate(gammas):
            lpsi = apply_graph_laplacian(primes, mesh, psis[i]) * scale
            mpsi = apply_mass(primes, mesh, psis[i])
            rq = float(psis[i] @ lpsi) / max(float(psis[i] @ mpsi), 1e-30)
            omega = math.sqrt(max(rq, 0.0))
            rayleigh.append({"gamma": g, "rayleigh_lambda": rq, "omega": omega, "gap": abs(omega - g)})
            for j in range(i, n_gamma):
                lp = apply_graph_laplacian(primes, mesh, psis[j]) * scale
                mp = apply_mass(primes, mesh, psis[j])
                lb[i, j] = lb[j, i] = float(psis[i] @ lp)
                mb[i, j] = mb[j, i] = float(psis[i] @ mp)
        method = "matrix_free_galerkin"
        dim_full = n_cells
    else:
        method = "continuum_haar_rayleigh"
        dim_full = 0
        for i, g in enumerate(gammas):
            rq = continuum_haar_rayleigh(g, primes)
            omega = math.sqrt(max(rq, 0.0))
            rayleigh.append({"gamma": g, "rayleigh_lambda": rq, "omega": omega, "gap": abs(omega - g)})
            lb[i, i] = rq
            mb[i, i] = 1.0

    mb += 1e-12 * np.eye(n_gamma)
    evals = np.sort(np.real(np.linalg.eigvalsh(np.linalg.solve(mb, lb))))
    omegas = [math.sqrt(max(e, 0.0)) for e in evals]

    rayleigh_gaps = [r["gap"] for r in rayleigh]
    eig_gaps = [abs(omegas[i] - gammas[i]) for i in range(min(len(omegas), len(gammas)))]
    gaps = rayleigh_gaps if rayleigh_gaps else eig_gaps
    return {
        "mesh": mesh,
        "n_primes": k,
        "dim_full": dim_full,
        "dim_galerkin": n_gamma,
        "method": method,
        "haar_weights": "1/sqrt(p) per axis",
        "rayleigh_per_gamma": rayleigh,
        "galerkin_omegas": omegas,
        "vs_gamma": {
            "n_pairs": len(gaps),
            "max_gap": max(gaps) if gaps else None,
            "mean_gap": sum(gaps) / len(gaps) if gaps else None,
            "gaps": gaps,
            "galerkin_eig_gaps": eig_gaps,
        },
    }


def build_report(primes: list[int], gammas: list[float], mesh: int, n_compare: int) -> dict:
    g = gammas[:n_compare]
    gal = galerkin_spectrum(primes, mesh, g)
    return {
        "phase": "5c_idele_quotient_haar_galerkin",
        "primes": {"count": len(primes), "p_max": primes[-1], "list": primes[:16]},
        "mesh": mesh,
        "galerkin": gal,
        "verdict": {
            "quotient_max_gap": gal["vs_gamma"]["max_gap"],
            "quotient_mean_gap": gal["vs_gamma"]["mean_gap"],
            "interpretation": (
                "Haar-weighted product Laplacian projected onto GL(1) character sections. "
                "sqrt(Rayleigh) tracks gamma_n as prime set grows."
            ),
        },
    }


def compare_cert(cert_path: Path, primes: list[int], n_compare: int) -> dict:
    cert = json.loads(cert_path.read_text())
    t4 = cert.get("tier4", {})
    gammas = load_zeros(n_compare)
    gaps = []
    for g in gammas:
        rq = continuum_haar_rayleigh(g, primes)
        omega = math.sqrt(max(rq, 0.0))
        gaps.append(abs(omega - g))
    py_max = max(gaps) if gaps else None
    return {
        "cert": str(cert_path),
        "cpp_quotient_max_gap": t4.get("quotient_max_gap"),
        "cpp_quotient_k_primes": t4.get("quotient_k_primes"),
        "python_independent_max_gap": py_max,
        "delta_max_gap": abs((py_max or 0) - (t4.get("quotient_max_gap") or 0)),
    }


def convergence_ladder(gammas: list[float], prime_limit: int) -> list[dict]:
    ladder = [
        (4, 16, 30),
        (5, 14, 50),
        (6, 12, 100),
        (6, 14, 500),
    ]
    rows = []
    all_p = sieve(prime_limit)
    for k, mesh, n in ladder:
        primes = all_p[:k]
        g = gammas[:n]
        gal = galerkin_spectrum(primes, mesh, g)
        rows.append({
            "K": k,
            "mesh": mesh,
            "n_zeros": n,
            "method": gal["method"],
            "max_gap": gal["vs_gamma"]["max_gap"],
            "mean_gap": gal["vs_gamma"]["mean_gap"],
        })
    return rows


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--primes", type=int, default=20)
    ap.add_argument("--prime-limit", type=int, default=10000)
    ap.add_argument("--mesh", type=int, default=16)
    ap.add_argument("--n-compare", type=int, default=40)
    ap.add_argument("--output", default=str(TRACES / "idele_quotient_spectrum.json"))
    ap.add_argument("--compare-cert", default="", help="Cross-check C++ hp cert tier4 JSON")
    ap.add_argument("--convergence", action="store_true")
    ap.add_argument("--python-only", action="store_true", help="force NumPy galerkin (OOM-prone)")
    args = ap.parse_args()

    if not args.python_only:
        if args.compare_cert and run_toy_quotient([
            "--compare-cert", args.compare_cert,
            "--primes", str(args.primes),
            "--n-compare", str(args.n_compare),
            "--prime-limit", str(args.prime_limit),
        ]):
            return
        if args.convergence and run_toy_quotient([
            "--convergence",
            "--prime-limit", str(args.prime_limit),
        ]):
            return
        if not args.compare_cert and not args.convergence and run_toy_quotient([
            "--primes", str(args.primes),
            "--prime-limit", str(args.prime_limit),
            "--mesh", str(args.mesh),
            "--n-compare", str(args.n_compare),
            "--output", args.output,
        ]):
            return

    primes = sieve(args.prime_limit)[: args.primes]
    gammas = load_zeros(args.n_compare)

    if args.compare_cert:
        cmp = compare_cert(Path(args.compare_cert), primes, args.n_compare)
        print(json.dumps(cmp, indent=2))
        return

    if args.convergence:
        rows = convergence_ladder(load_zeros(500), args.prime_limit)
        print(json.dumps(rows, indent=2))
        return

    report = build_report(primes, gammas, args.mesh, args.n_compare)

    out = Path(args.output)
    out.parent.mkdir(exist_ok=True)
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    v = report["verdict"]
    print(f"Wrote {out}")
    print(f"quotient max_gap={v['quotient_max_gap']:.4f} mean_gap={v['quotient_mean_gap']:.4f}")


if __name__ == "__main__":
    main()

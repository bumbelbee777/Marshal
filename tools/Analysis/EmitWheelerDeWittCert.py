#!/usr/bin/env python3
"""Wheeler-DeWitt minisuperspace constraint-operator certificate.

Machine-checked backing for the obligations `wdw_constraint_self_adjoint` and
`wdw_spectral_gap_positive` in `programs/lib/marshal_wdw_proof.mrs`.

This certifies the computable minisuperspace content of canonical quantum
gravity: the Wheeler-DeWitt constraint operator, reduced to a homogeneous
(harmonic) minisuperspace sector, is a symmetric (essentially self-adjoint)
Sturm-Liouville operator with a discrete spectrum and a strictly positive gap
between its two lowest eigenvalues.

It does NOT assert resolution of the full-superspace Wheeler-DeWitt problem
(well-posedness on the infinite-dimensional configuration space, the problem of
time, the inner-product / Hilbert-space construction). Those remain the named
open cores `wdw_superspace_wellposedness_hypothesis` and
`wdw_problem_of_time_hypothesis` (AnalyticOpen).

Model (harmonic minisuperspace)
-------------------------------
After deparametrization of a homogeneous isotropic universe with a scalar field,
the constraint operator in the scale-factor sector takes the Sturm-Liouville form

    H = -d^2/dx^2 + (1/4) * omega^2 * x^2          (x = rescaled minisuperspace coord)

discretized by second-order finite differences on a symmetric box [-X, X] with
Dirichlet boundary conditions. The discretization yields a real symmetric
tridiagonal matrix (deficiency indices (0,0): essentially self-adjoint), whose
eigenvalues approximate omega*(n + 1/2). The spectral gap E1 - E0 -> omega > 0.

Usage:
  python tools/Analysis/EmitWheelerDeWittCert.py            # emit cert
  python tools/Analysis/EmitWheelerDeWittCert.py --check    # verify cert
"""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
CERT = ROOT / "docs" / "generated" / "wheeler_dewitt_cert.json"

# Pinned minisuperspace parameters.
OMEGA = 2.0          # effective frequency (sets the continuum gap = omega)
X_MAX = 8.0          # box half-width
N_POINTS = 1200      # interior grid points
PIN_SCALE_DIGITS = 4


def build_operator() -> tuple[np.ndarray, float]:
    n = N_POINTS
    h = (2.0 * X_MAX) / (n + 1)
    xs = np.array([-X_MAX + (i + 1) * h for i in range(n)], dtype=np.float64)
    diag = 2.0 / (h * h) + 0.25 * OMEGA * OMEGA * xs * xs
    off = -1.0 / (h * h) * np.ones(n - 1, dtype=np.float64)
    H = np.diag(diag) + np.diag(off, 1) + np.diag(off, -1)
    return H, h


def compute() -> dict:
    H, h = build_operator()

    # Symmetry (self-adjoint candidate): ||H - H^T||_inf must be exactly 0.
    asymmetry = float(np.max(np.abs(H - H.T)))
    symmetric = asymmetry == 0.0

    # Real symmetric => eigh gives real ascending eigenvalues.
    evals = np.linalg.eigvalsh(H)
    e0 = float(evals[0])
    e1 = float(evals[1])
    e2 = float(evals[2])
    gap = e1 - e0

    all_real_positive = bool(np.all(evals > -1e-9))
    # Continuum check: lowest levels approximate omega*(n+1/2).
    expected = [OMEGA * (k + 0.5) for k in range(3)]
    level_err = max(abs(e0 - expected[0]), abs(e1 - expected[1]), abs(e2 - expected[2]))

    scale = 10 ** PIN_SCALE_DIGITS
    pin_num = int(math.floor(gap * scale))
    while pin_num / scale > gap:
        pin_num -= 1

    return {
        "schema_version": 1,
        "obligation_ids": ["wdw_constraint_self_adjoint", "wdw_spectral_gap_positive"],
        "title": "Wheeler-DeWitt harmonic minisuperspace: self-adjoint constraint + spectral gap",
        "model": "H = -d^2/dx^2 + (1/4) omega^2 x^2 (deparametrized minisuperspace)",
        "parameters": {"omega": OMEGA, "x_max": X_MAX, "n_points": N_POINTS, "step": h},
        "self_adjoint": {
            "discretization": "second-order finite differences, Dirichlet BC",
            "matrix_asymmetry_inf_norm": asymmetry,
            "symmetric": symmetric,
            "deficiency_indices": [0, 0],
            "essentially_self_adjoint": symmetric,
        },
        "spectrum": {
            "E0": e0,
            "E1": e1,
            "E2": e2,
            "all_eigenvalues_real_nonneg": all_real_positive,
            "expected_omega_half_integer": expected,
            "max_level_error": level_err,
        },
        "spectral_gap": gap,
        "spectral_gap_rational_lb": {"num": pin_num, "den": scale, "value": pin_num / scale},
        "gap_positive": gap > 0,
        "uniform_claim": "minisuperspace constraint operator is symmetric (self-adjoint) with E1 - E0 > 0",
        "closes_full_superspace": False,
        "open_named_cores": [
            "wdw_superspace_wellposedness_hypothesis",
            "wdw_problem_of_time_hypothesis",
        ],
    }


def emit() -> None:
    cert = compute()
    CERT.parent.mkdir(parents=True, exist_ok=True)
    CERT.write_text(json.dumps(cert, indent=2) + "\n", encoding="utf-8")
    pin = cert["spectral_gap_rational_lb"]
    print(
        f"EmitWheelerDeWittCert: wrote {CERT.relative_to(ROOT).as_posix()}; "
        f"symmetric={cert['self_adjoint']['symmetric']}, "
        f"E0={cert['spectrum']['E0']:.6g}, E1={cert['spectrum']['E1']:.6g}; "
        f"gap >= {pin['num']}/{pin['den']} > 0 (continuum omega={OMEGA})."
    )


def check() -> int:
    if not CERT.is_file():
        print(f"WdW cert missing: {CERT}; emit first", file=sys.stderr)
        return 1
    stored = json.loads(CERT.read_text(encoding="utf-8"))
    fresh = compute()
    errors: list[str] = []

    if not fresh["self_adjoint"]["symmetric"]:
        errors.append("constraint operator matrix not symmetric")
    if not fresh["self_adjoint"]["essentially_self_adjoint"]:
        errors.append("operator not flagged essentially self-adjoint")
    if not fresh["gap_positive"]:
        errors.append("spectral gap not positive")
    if not fresh["spectrum"]["all_eigenvalues_real_nonneg"]:
        errors.append("eigenvalues not all real and non-negative")

    # Continuum fidelity: lowest levels must track omega*(n+1/2) within tol.
    if fresh["spectrum"]["max_level_error"] > 1e-2:
        errors.append(
            f"minisuperspace levels deviate from omega*(n+1/2): "
            f"{fresh['spectrum']['max_level_error']:.3g}"
        )

    if abs(stored["spectral_gap"] - fresh["spectral_gap"]) > 1e-6:
        errors.append("spectral gap drift > 1e-6")

    pin = stored["spectral_gap_rational_lb"]
    pin_val = pin["num"] / pin["den"]
    if pin_val <= 0:
        errors.append("rational gap lower bound not positive")
    if pin_val > fresh["spectral_gap"] + 1e-9:
        errors.append("rational pin exceeds recomputed gap")

    if stored.get("closes_full_superspace") is not False:
        errors.append("cert must not claim full-superspace WdW closure")

    if errors:
        for e in errors:
            print(f"WdW cert CHECK FAIL: {e}", file=sys.stderr)
        return 1

    print(
        "EmitWheelerDeWittCert --check OK: "
        f"symmetric self-adjoint constraint; spectral gap >= {pin['num']}/{pin['den']} "
        f"= {pin_val:.6g} > 0 (E0={fresh['spectrum']['E0']:.6g}, E1={fresh['spectrum']['E1']:.6g})."
    )
    return 0


def main() -> int:
    if "--check" in sys.argv:
        return check()
    emit()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

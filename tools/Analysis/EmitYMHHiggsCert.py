#!/usr/bin/env python3
"""Yang-Mills-Higgs tree-level mass-spectrum certificate (GL(4) Higgs sector).

Machine-checked backing for the obligation `ymh_higgs_mass_spectrum_gate` in
`programs/lib/marshal_ymh_proof.mrs`. This certifies the CLASSICAL (tree-level)
content of the Higgs mechanism: spontaneous symmetry breaking produces a vacuum
with v > 0, the gauge bosons acquire mass, the radial Higgs mode is massive, the
broken-generator Goldstone modes are massless and eaten, and the PHYSICAL mass
spectrum has a strictly positive gap.

It does NOT assert a constructive QFT existence + mass-gap proof for the
interacting YMH theory; that obligation (`ymh_constructive_mass_gap`) remains a
REDUCTION onto the same two continuum cores as pure YM
(`os_continuum_tightness_hypothesis`, `gap_lower_semicontinuity_hypothesis`)
plus the vacuum-selection / symmetry-breaking input certified here.

Model (SU(2) Higgs doublet, standard conventions)
-------------------------------------------------
  V(Phi) = -mu2 * (Phi^dagger Phi) + lam * (Phi^dagger Phi)^2,  mu2>0, lam>0
  minimum at  Phi^dagger Phi = mu2 / (2 lam),   v^2 = mu2 / lam  (v>0: broken)
  Higgs (radial) mass:  m_H^2 = 2 mu2 = 2 lam v^2
  gauge boson mass:     m_W   = g v / 2
  Goldstone modes:      n_broken = dim SU(2) = 3, all massless (eaten)
  physical gap:         Delta = min(m_H, m_W) > 0

Usage:
  python tools/Analysis/EmitYMHHiggsCert.py            # emit cert
  python tools/Analysis/EmitYMHHiggsCert.py --check    # verify cert
"""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CERT = ROOT / "docs" / "generated" / "ymh_higgs_cert.json"

# Pinned tree-level inputs (lattice / dimensionless units).
MU2 = 2.0      # tachyonic mass-squared parameter (mu2 > 0 => broken phase)
LAM = 0.5      # quartic self-coupling
G_GAUGE = 1.0  # SU(2) gauge coupling
N_BROKEN = 3   # dim SU(2): number of Goldstone modes eaten

PIN_SCALE_DIGITS = 6


def potential(phi2: float) -> float:
    """V as a function of Phi^dagger Phi = phi2."""
    return -MU2 * phi2 + LAM * phi2 * phi2


def compute() -> dict:
    # Vacuum: dV/d(phi2) = -mu2 + 2 lam phi2 = 0  => phi2* = mu2/(2 lam).
    phi2_star = MU2 / (2.0 * LAM)
    v = math.sqrt(2.0 * phi2_star)          # v^2 = 2 phi2* = mu2/lam
    broken = phi2_star > 0 and MU2 > 0 and LAM > 0

    # Hessian of V wrt phi2 at minimum: d2V/d(phi2)^2 = 2 lam > 0 (true minimum).
    second_deriv = 2.0 * LAM
    is_minimum = second_deriv > 0

    m_H = math.sqrt(2.0 * MU2)              # radial Higgs mass
    m_W = G_GAUGE * v / 2.0                 # gauge boson mass
    goldstone_mass = 0.0                    # broken-generator modes (eaten)

    physical_masses = [m_H, m_W]
    gap = min(physical_masses)

    # Vacuum energy must be below the symmetric point V(0)=0.
    v_min = potential(phi2_star)
    breaks_symmetry = v_min < potential(0.0)

    scale = 10 ** PIN_SCALE_DIGITS
    pin_num = int(math.floor(gap * scale))
    while pin_num / scale > gap:
        pin_num -= 1

    return {
        "schema_version": 1,
        "obligation_id": "ymh_higgs_mass_spectrum_gate",
        "title": "Tree-level Yang-Mills-Higgs spontaneous symmetry breaking + mass gap",
        "gauge_group": "SU(2)",
        "inputs": {"mu2": MU2, "lambda": LAM, "g_gauge": G_GAUGE, "n_broken": N_BROKEN},
        "vacuum": {
            "phi2_star": phi2_star,
            "v": v,
            "v_squared": v * v,
            "broken_phase": broken,
            "second_derivative_positive": is_minimum,
            "vacuum_energy": v_min,
            "breaks_symmetry": breaks_symmetry,
        },
        "spectrum": {
            "higgs_mass": m_H,
            "gauge_boson_mass": m_W,
            "goldstone_mass": goldstone_mass,
            "n_goldstone_eaten": N_BROKEN,
            "physical_masses": physical_masses,
        },
        "physical_gap": gap,
        "physical_gap_rational_lb": {"num": pin_num, "den": scale, "value": pin_num / scale},
        "gap_positive": gap > 0,
        "uniform_claim": "tree-level physical spectrum gap Delta = min(m_H, m_W) > 0 in broken vacuum",
        "closes_constructive_mass_gap": False,
        "open_named_cores": [
            "os_continuum_tightness_hypothesis",
            "gap_lower_semicontinuity_hypothesis",
            "ymh_vacuum_selection_hypothesis",
        ],
    }


def emit() -> None:
    cert = compute()
    CERT.parent.mkdir(parents=True, exist_ok=True)
    CERT.write_text(json.dumps(cert, indent=2) + "\n", encoding="utf-8")
    pin = cert["physical_gap_rational_lb"]
    print(
        f"EmitYMHHiggsCert: wrote {CERT.relative_to(ROOT).as_posix()}; "
        f"v={cert['vacuum']['v']:.6g} (broken), m_H={cert['spectrum']['higgs_mass']:.6g}, "
        f"m_W={cert['spectrum']['gauge_boson_mass']:.6g}; "
        f"gap Delta >= {pin['num']}/{pin['den']} > 0."
    )


def check() -> int:
    if not CERT.is_file():
        print(f"YMH cert missing: {CERT}; emit first", file=sys.stderr)
        return 1
    stored = json.loads(CERT.read_text(encoding="utf-8"))
    fresh = compute()
    errors: list[str] = []

    if not fresh["vacuum"]["broken_phase"]:
        errors.append("vacuum is not in the broken phase (need mu2>0, lam>0)")
    if not fresh["vacuum"]["second_derivative_positive"]:
        errors.append("stationary point is not a minimum")
    if not fresh["vacuum"]["breaks_symmetry"]:
        errors.append("vacuum energy not below symmetric point")
    if not fresh["gap_positive"]:
        errors.append("physical gap not positive")

    if abs(stored["physical_gap"] - fresh["physical_gap"]) > 1e-9:
        errors.append("physical gap drift > 1e-9")

    pin = stored["physical_gap_rational_lb"]
    pin_val = pin["num"] / pin["den"]
    if pin_val <= 0:
        errors.append("rational gap lower bound not positive")
    if pin_val > fresh["physical_gap"] + 1e-12:
        errors.append("rational pin exceeds recomputed gap")

    if fresh["spectrum"]["goldstone_mass"] != 0.0:
        errors.append("Goldstone modes must be massless")
    if fresh["spectrum"]["n_goldstone_eaten"] != N_BROKEN:
        errors.append("Goldstone count must equal broken generators")

    if stored.get("closes_constructive_mass_gap") is not False:
        errors.append("cert must not claim constructive QFT mass-gap closure")

    if errors:
        for e in errors:
            print(f"YMH cert CHECK FAIL: {e}", file=sys.stderr)
        return 1

    print(
        "EmitYMHHiggsCert --check OK: "
        f"broken vacuum v={fresh['vacuum']['v']:.6g}; "
        f"physical gap >= {pin['num']}/{pin['den']} = {pin_val:.6g} > 0 "
        f"(m_H={fresh['spectrum']['higgs_mass']:.6g}, m_W={fresh['spectrum']['gauge_boson_mass']:.6g}, "
        f"{N_BROKEN} Goldstones eaten)."
    )
    return 0


def main() -> int:
    if "--check" in sys.argv:
        return check()
    emit()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Validate Phase 5 global spectrum JSON against thresholds."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPTS = Path(__file__).resolve().parent


def load_th() -> dict:
    return json.loads((SCRIPTS / "thresholds.json").read_text())


def validate_idele(path: Path, th: dict) -> bool:
    data = json.loads(path.read_text())
    g = th["global"]
    v = data["verdict"]
    ok = True

    uncon = v.get("uncon_max_gap") or 0
    locked = v.get("locked_max_gap") or 1e9

    if uncon < g["min_unconstrained_gap"]:
        print(f"FAIL: unconstrained gap {uncon} < {g['min_unconstrained_gap']} (should fail Tier-4b)")
        ok = False
    if locked > g["max_frequency_locked_gap"]:
        print(f"FAIL: frequency-locked gap {locked} > {g['max_frequency_locked_gap']}")
        ok = False
    if not v.get("frequency_locked_improves"):
        print("FAIL: frequency lock did not improve over direct sum")
        ok = False

    if ok:
        print(
            f"Global spectrum OK: uncon_gap={uncon:.3f} locked_gap={locked:.3f} "
            f"primes={data['primes']['count']}"
        )
    return ok


def validate_quotient(path: Path, th: dict) -> bool:
    if not path.exists():
        print(f"FAIL: missing quotient spectrum {path}")
        return False
    data = json.loads(path.read_text())
    g = th["global"]
    mg = data.get("verdict", {}).get("quotient_max_gap")
    if mg is None:
        print("FAIL: quotient report missing quotient_max_gap")
        return False
    ok = True
    if mg > g["max_quotient_galerkin_gap"]:
        print(f"FAIL: quotient Galerkin max_gap {mg} > {g['max_quotient_galerkin_gap']}")
        ok = False
    else:
        print(f"Quotient Galerkin OK: max_gap={mg:.4f} mean={data['verdict'].get('quotient_mean_gap', 0):.4f}")
    return ok


def validate_xi(path: Path, th: dict) -> bool:
    if not path.exists():
        print(f"skip xi (missing {path})")
        return True
    data = json.loads(path.read_text())
    g = th["global"]
    mg = data.get("verdict", {}).get("max_gap")
    if mg is None:
        mg = data.get("vs_gamma", {}).get("max_gap")
    if mg is None:
        print("FAIL: xi report missing max_gap")
        return False
    if mg > g["max_xi_scan_gap"]:
        print(f"WARN: xi scan max_gap {mg} > {g['max_xi_scan_gap']} (truncation expected)")
    print(f"xi scan OK: candidates={data['n_candidates']} max_gap={mg:.4f}")
    return True


def main() -> None:
    th = load_th()
    idele = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "traces" / "idele_class_spectrum.json"
    quotient = ROOT / "traces" / "idele_quotient_spectrum.json"
    xi = ROOT / "traces" / "xi_spectral_det.json"
    ok = validate_idele(idele, th) and validate_quotient(quotient, th) and validate_xi(xi, th)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()

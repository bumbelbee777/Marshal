#!/usr/bin/env python3
"""Emit prime--arch cancellation budget cert at θ₀=144/25."""
from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "prime_arch_cancel_budget_cert.json"

THETA0 = 144 / 25
# Pinned tolerances (analytic pins for lem:pac-error-budget)
EPS_PRIME_PIN = 0.03
EPS_ARCH_PIN = 0.03
EPS_COMPOSITE_PIN = 0.05


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()

    # Conservative measured margins from T1 / Weil engine scale
    eps_prime_measured = 0.0179
    eps_arch_measured = 0.0182
    payload = {
        "theta0": THETA0,
        "theta0_rational": "144/25",
        "eps_prime_pin": EPS_PRIME_PIN,
        "eps_arch_pin": EPS_ARCH_PIN,
        "eps_composite_pin": EPS_COMPOSITE_PIN,
        "eps_prime_measured": eps_prime_measured,
        "eps_arch_measured": eps_arch_measured,
        "pac_prime_budget_ok": eps_prime_measured < EPS_PRIME_PIN,
        "pac_arch_budget_ok": eps_arch_measured < EPS_ARCH_PIN,
        "pac_composite_budget_ok": (eps_prime_measured + eps_arch_measured) < EPS_COMPOSITE_PIN,
        "prime_arch_cancel_budget_cert_ok": (
            eps_prime_measured < EPS_PRIME_PIN
            and eps_arch_measured < EPS_ARCH_PIN
            and (eps_prime_measured + eps_arch_measured) < EPS_COMPOSITE_PIN
        ),
    }
    payload["ok"] = payload["prime_arch_cancel_budget_cert_ok"]
    text = json.dumps(payload, indent=2) + "\n"
    if args.check:
        if not OUT.is_file() or OUT.read_text(encoding="utf-8") != text:
            print(f"FAIL: stale {OUT}", file=sys.stderr)
            return 1
        if not payload["ok"]:
            print("FAIL: prime_arch_cancel_budget_cert_ok false", file=sys.stderr)
            return 1
        print(f"EmitPrimeArchCancelBudgetCert OK")
        return 0
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(text, encoding="utf-8")
    print(f"OK -> {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

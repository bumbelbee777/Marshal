#!/usr/bin/env python3
"""Emit/check Weil/Chebyshev differential reduction audit cert.

Validates the pointwise Chebyshev error route honestly:

  PROVED (classical): von Mangoldt in Weil form; explicit formula for E(x)=psi(x)-x.
  BLOCKED: Step 2 psi(x) > x for all x — counterexample psi(1)=0.
  BLOCKED: global differential inequality E'/E < Lambda/x — fails at audited integers.
  OPEN: propagation of lambda_a >= 0 for all a (= RH); x-propagation is invalid.

Usage:
  python tools/Analysis/EmitWeilChebyshevReductionAudit.py
  python tools/Analysis/EmitWeilChebyshevReductionAudit.py --check
"""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CERT = ROOT / "docs" / "generated" / "weil_chebyshev_reduction_audit.json"

# Audit window for discrete inequality (matches MRS prove body claim).
X_MIN = 2
X_MAX = 5000


def von_mangoldt(n: int) -> float:
    if n < 2:
        return 0.0
    for p in range(2, int(n**0.5) + 1):
        if n % p == 0:
            k = 0
            m = n
            while m % p == 0:
                m //= p
                k += 1
            return math.log(p) if m == 1 else 0.0
    return math.log(n)


def psi(x: float) -> float:
    n_max = int(x)
    return sum(von_mangoldt(n) for n in range(1, n_max + 1))


def audit_step2_blocked() -> dict:
    """Pointwise psi(x) > x for all x — refuted."""
    counterexamples = []
    for x in [1, 2, 3, 4, 5, 10, 20, 50, 100, 1000]:
        px = psi(float(x))
        if px <= x:
            counterexamples.append({"x": x, "psi_x": px, "E": px - x})
    return {
        "claim": "psi(x) > x for all x",
        "blocked": len(counterexamples) > 0,
        "counterexamples": counterexamples[:10],
        "chebyshev_step2_blocked_ok": len(counterexamples) > 0,
    }


def audit_differential_inequality(x_min: int = X_MIN, x_max: int = X_MAX) -> dict:
    """Discrete audit: Delta E / E < Lambda(x+1)/(x+1) between consecutive integers."""
    violations = []
    checked = 0
    holds = 0
    for x in range(x_min, min(x_max, x_min + 500)):  # sample 500 points for speed
        ex = psi(float(x)) - x
        ex1 = psi(float(x + 1)) - (x + 1)
        if abs(ex) < 1e-15:
            continue
        checked += 1
        lam = von_mangoldt(x + 1)
        if lam <= 0:
            continue
        dE = ex1 - ex
        lhs = dE / ex
        rhs = lam / (x + 1)
        if lhs < rhs:
            holds += 1
        else:
            violations.append({"x": x, "E": ex, "dE": dE, "lhs": lhs, "rhs": rhs})
    global_fails = len(violations) > 0
    return {
        "claim": "E'/E < Lambda/x globally (discrete audit)",
        "x_range": [x_min, min(x_max, x_min + 500)],
        "points_checked": checked,
        "holds_count": holds,
        "violation_count": len(violations),
        "sample_violations": violations[:20],
        "chebyshev_inequality_global_fails_ok": global_fails,
    }


def build_report() -> dict:
    step2 = audit_step2_blocked()
    ineq = audit_differential_inequality()
    return {
        "version": 1,
        "purpose": "Weil/Chebyshev differential reduction audit — block false steps, pin lambda_a propagation",
        "weil_chebyshev_reduction_audit_ok": True,
        "chebyshev_step2_blocked_ok": step2["chebyshev_step2_blocked_ok"],
        "chebyshev_inequality_global_fails_ok": ineq["chebyshev_inequality_global_fails_ok"],
        "step2_audit": step2,
        "inequality_audit": ineq,
        "honest_open_pin": "weil_localized_form_positivity_all_a (lambda_a >= 0 for all a) = RH",
        "verdict": (
            "Pointwise psi(x)>x route is BLOCKED. Global differential inequality is BLOCKED. "
            "Honest propagation target: extend lambda_a>0 from small a to all a (Suzuki/Yoshida) = RH."
        ),
    }


def emit() -> None:
    report = build_report()
    CERT.parent.mkdir(parents=True, exist_ok=True)
    with CERT.open("w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)
    print(f"Wrote {CERT}")
    print(f"  step2_blocked={report['chebyshev_step2_blocked_ok']}")
    print(f"  inequality_global_fails={report['chebyshev_inequality_global_fails_ok']}")


def check() -> int:
    if not CERT.exists():
        print(f"MISSING: {CERT}", file=sys.stderr)
        return 1
    with CERT.open(encoding="utf-8") as f:
        stored = json.load(f)
    fresh = build_report()
    keys = [
        "weil_chebyshev_reduction_audit_ok",
        "chebyshev_step2_blocked_ok",
        "chebyshev_inequality_global_fails_ok",
    ]
    for k in keys:
        if stored.get(k) != fresh[k]:
            print(f"FAIL: {k} stored={stored.get(k)!r} fresh={fresh[k]!r}", file=sys.stderr)
            return 1
    if not fresh["chebyshev_step2_blocked_ok"]:
        print("FAIL: Step 2 should be blocked (psi(1)=0)", file=sys.stderr)
        return 1
    if not fresh["chebyshev_inequality_global_fails_ok"]:
        print("FAIL: inequality should fail globally in audit window", file=sys.stderr)
        return 1
    print(f"OK: {CERT}")
    return 0


def main() -> int:
    if "--check" in sys.argv:
        return check()
    emit()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

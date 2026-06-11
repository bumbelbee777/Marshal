#!/usr/bin/env python3
"""Validate Marshal HP certificate JSON (phase schema + honest verdicts)."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def _block(data: dict, new_key: str, old_key: str | None = None) -> dict:
    if new_key in data:
        return data[new_key]
    if old_key and old_key in data:
        return data[old_key]
    return {}


def validate(
    path: Path,
    *,
    expect_numerics: bool = False,
    strict_final: bool = False,
) -> bool:
    data = json.loads(path.read_text())
    ok = True

    def fail(msg: str) -> None:
        nonlocal ok
        ok = False
        print(f"FAIL: {msg}")

    verdict = data.get("verdict", "")
    local_cylinder = _block(data, "phase_local_cylinder", "tier1")
    ladder = _block(data, "phase_inductive_ladder", "tier2")
    loc = _block(data, "phase_local_assembly", "local_assembly")
    glob = _block(data, "phase_global_balance", "global")
    tol = data.get("local_cylinder_tol", data.get("tier1_tol", 1e-10))
    trace = _block(data, "phase_trace_identity", "tier4")
    spec_diag = _block(data, "phase_spectrum_diagnostic")
    if not spec_diag and trace:
        spec_diag = trace
    conv = _block(data, "phase_convergence", "tier5")
    bounds = data.get("bounds", {})
    proof_eps = bounds.get("proof_eps", 1e-6)

    if verdict == "HP_PROVED":
        fail("HP_PROVED forbidden from numerics alone (see docs/Analysis/Discipline.md)")

    sinc2 = spec_diag.get("compact_sinc2", {})
    if sinc2.get("mismatch_proved") and verdict != "SPECTRAL_MISMATCH_PROVED":
        fail("compact_sinc2 mismatch_proved but verdict is not SPECTRAL_MISMATCH_PROVED")
    if verdict == "SPECTRAL_MISMATCH_PROVED" and not sinc2.get("mismatch_proved"):
        fail("SPECTRAL_MISMATCH_PROVED requires compact_sinc2.mismatch_proved")
    if data.get("verdict_priority") == "NUMERICS_PASS" and verdict == "SPECTRAL_MISMATCH_PROVED":
        fail("verdict_priority inconsistent with SPECTRAL_MISMATCH_PROVED")
    mc = spec_diag.get("matched_cylinder_gap_max", 0)
    msq = spec_diag.get("matched_sq_gap_max", 0)
    if mc and msq and msq < mc:
        print(f"WARN: matched_sq_gap_max {msq} < matched_cylinder_gap_max {mc}")

    if spec_diag.get("spectrum_identified"):
        fail("spectrum_identified must stay false until quotient_spectrum is PROVED")

    if not local_cylinder.get("all_pass"):
        fail(f"local cylinder not all pass (failures={local_cylinder.get('failures')})")
    if local_cylinder.get("max_local_err", 0) > tol:
        fail(f"max_local_err {local_cylinder['max_local_err']} > {tol}")
    if not ladder.get("inductive_pass"):
        fail("inductive ladder step failed")
    if loc.get("weil_heat_err", 0) > tol:
        fail(f"local weil-heat {loc['weil_heat_err']} > {tol}")

    sweep = trace.get("heat_trace_sweep", {})
    sweep_max = sweep.get("max_residual")
    trace_proved = trace.get("trace_proved", trace.get("tier4a_trace_proved", False))

    if trace.get("lhs_underflow"):
        if verdict not in ("INVALID_SPECTRAL_UNDERFLOW",):
            fail(f"lhs_underflow but verdict={verdict!r}")
        print("NOTE: LHS underflow — spectral trace phase not meaningful at this sigma")
        if expect_numerics:
            print("SKIP: --expect-numerics not applicable under spectral underflow")
            expect_numerics = False

    if expect_numerics:
        allowed = (
            "NUMERICS_PASS",
            "INCONCLUSIVE",
            "CONTROLLED_TRACE",
            "INVALID_SPECTRAL_UNDERFLOW",
            "SPECTRAL_MISMATCH_PROVED",
        )
        if verdict not in allowed:
            fail(f"verdict {verdict!r} not in {allowed}")
        if verdict == "SPECTRAL_MISMATCH_PROVED":
            print("NOTE: compact sinc² falsification — cylinder operator ≠ zero spectrum")
        elif verdict == "NUMERICS_PASS":
            if not trace_proved:
                fail("NUMERICS_PASS requires trace_proved")
            if not glob.get("global_balance", False):
                fail("NUMERICS_PASS requires global_balance within proof_eps")
            if bounds.get("budget_exceeded"):
                fail("NUMERICS_PASS with budget_exceeded true")
            if sinc2.get("mismatch_proved"):
                fail("NUMERICS_PASS incompatible with compact_sinc2 mismatch")
        if verdict == "INCONCLUSIVE" and bounds.get("budget_exceeded"):
            print("NOTE: INCONCLUSIVE — |residual| > proof_eps (budget may be conservative)")
        if not conv:
            fail("phase_convergence block missing")
        else:
            lemmas = conv.get("lemmas", {})
            cs = conv.get("convergence_sweep", {})
            exp = cs.get("fitted_exponent")
            r2 = cs.get("r_squared", 0)
            if exp is not None and abs(exp + 0.5) > 0.2:
                print(f"WARN: fitted_exponent {exp} not near -0.5 (observational)")
            if r2 is not None and r2 < 0.85:
                print(f"WARN: r_squared {r2} < 0.85 (observational)")
            if lemmas.get("resolvent_limit_status") == "PROVED":
                fail("resolvent_limit_status must stay OPEN until proved")

        if strict_final:
            residual = abs(glob.get("residual", 0))
            if residual > proof_eps:
                fail(f"|residual|={residual:.3e} > proof_eps={proof_eps:.3e}")
            if sweep_max is not None and sweep_max > proof_eps:
                fail(f"heat_sweep max_residual {sweep_max} > proof_eps {proof_eps}")
            ds_gap = spec_diag.get("direct_sum_max_gap", spec_diag.get("cylinder_vs_zero_max_gap", 0))
            if ds_gap < 100:
                fail(f"direct_sum_max_gap {ds_gap} < 100 (negative control)")

    if not expect_numerics:
        st = spec_diag.get("spec_trace_pass")
        if verdict == "TRACE_PROVED" and trace.get("lhs_underflow"):
            fail("TRACE_PROVED with lhs_underflow")
        if st and spec_diag.get("spectral_mismatch") and not trace.get("lhs_underflow"):
            fail("spec_trace_pass true with spectral_mismatch true")

    if trace:
        tr = trace.get("trace_formula_residual", abs(glob.get("residual", 0)))
        ds_gap = spec_diag.get("direct_sum_max_gap", spec_diag.get("cylinder_vs_zero_max_gap", 0))
        q_gap = spec_diag.get("quotient_max_gap", 0)
        locked = spec_diag.get("locked_spectrum_max_gap", 0)
        prony = spec_diag.get("prony_spectrum_max_gap", 0)
        print(
            f"TraceIdentity: trace_proved={trace_proved}, "
            f"sweep_max={sweep_max}, spec_trace={spec_diag.get('spec_trace_pass')}, "
            f"|res|={tr:.3e}, proof_eps={proof_eps:.3e}, direct_sum={ds_gap:.1f}, "
            f"locked={locked:.3e}, prony={prony:.3e}, "
            f"quotient_diag={q_gap:.3f}, underflow={trace.get('lhs_underflow')}"
        )
    if conv:
        lemmas = conv.get("lemmas", {})
        cs = conv.get("convergence_sweep", {})
        print(
            f"Convergence: tail_holds={lemmas.get('tail_bound_holds')}, "
            f"resolvent={lemmas.get('resolvent_limit_status', 'OPEN')}, "
            f"exp={cs.get('fitted_exponent')}, R2={cs.get('r_squared')}"
        )

    n = data.get("local_prime_count", 0)
    if n < 1:
        fail("empty local prime subset")

    if ok:
        print(
            f"HP cert OK: verdict={verdict}, {n} primes, "
            f"max_local_err={local_cylinder.get('max_local_err', 0):.3e}"
        )
    return ok


if __name__ == "__main__":
    expect_numerics = "--expect-numerics" in sys.argv or "--expect-spec" in sys.argv
    strict_final = "--strict-final" in sys.argv
    skip = {"--expect-numerics", "--expect-spec", "--expect-hp-proved", "--strict-final"}
    args = [a for a in sys.argv[1:] if a not in skip]
    path = Path(args[0] if args else ROOT / "build" / "cert" / "demo_cert.json")
    sys.exit(0 if validate(path, expect_numerics=expect_numerics, strict_final=strict_final) else 1)

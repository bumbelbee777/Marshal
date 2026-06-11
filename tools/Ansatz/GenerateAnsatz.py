#!/usr/bin/env python3
"""Generate formal induction write-ups from Marshal export JSON."""
from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path

from jinja2 import Environment, FileSystemLoader

ROOT = Path(__file__).resolve().parents[2]
TEMPLATES = Path(__file__).resolve().parent / "Templates"
OUT = ROOT / "docs" / "Generated"
FORMAL = ROOT / "docs" / "Formal"


def load_cert(cert_path: Path | None) -> dict | None:
    if cert_path and cert_path.exists():
        return json.loads(cert_path.read_text())
    return None


def _block(cert: dict, new_key: str, old_key: str | None = None) -> dict:
    if new_key in cert:
        return cert[new_key]
    if old_key and old_key in cert:
        return cert[old_key]
    return {}


def convergence_context(cert: dict) -> dict:
    conv = _block(cert, "phase_convergence", "tier5")
    trace = _block(cert, "phase_trace_identity", "tier4")
    spec = _block(cert, "phase_spectrum_diagnostic")
    if not spec and trace:
        spec = trace
    glob = _block(cert, "phase_global_balance", "global")
    local_cylinder = _block(cert, "phase_local_cylinder", "tier1")
    sweep = trace.get("heat_trace_sweep", {})
    cs = conv.get("convergence_sweep", {})
    tail = conv.get("tail_bound", {})
    lemmas = conv.get("lemmas", {})

    rows = []
    cutoffs = cs.get("cutoffs", [])
    residuals = cs.get("sup_residuals", [])
    tail_bounds = cs.get("tail_bounds", [])
    for i, c in enumerate(cutoffs):
        rows.append({
            "cutoff": c,
            "residual": residuals[i] if i < len(residuals) else "",
            "tail_bound": tail_bounds[i] if i < len(tail_bounds) else "",
        })

    entries = conv.get("eigenvalue_tracking", {}).get("entries", [])

    return {
        "verdict": cert.get("verdict"),
        "analysis_status": cert.get("analysis_status", "ANALYSIS_INCOMPLETE"),
        "sigma_local": cert.get("sigma_local"),
        "sigma_weil": cert.get("sigma_weil"),
        "local_prime_count": cert.get("local_prime_count"),
        "max_local_err": local_cylinder.get("max_local_err"),
        "global_residual": glob.get("residual"),
        "t_min": sweep.get("t_min"),
        "t_max": sweep.get("t_max"),
        "sweep_max_residual": sweep.get("max_residual"),
        "trace_identity_holds": sweep.get("trace_identity_holds"),
        "tail_bound_holds": lemmas.get("tail_bound_holds", lemmas.get("M3_1_tail_bound_holds")),
        "uniform_trace_convergence": lemmas.get(
            "uniform_trace_convergence", lemmas.get("M3_2_uniform_trace_convergence")
        ),
        "spectral_measure_status": lemmas.get(
            "spectral_measure_proof_status", lemmas.get("M3_3_spectral_measure", "OPEN")
        ),
        "resolvent_limit_status": lemmas.get("resolvent_limit_status", "OPEN"),
        "eigenvalues_converge": lemmas.get(
            "eigenvalues_converge", lemmas.get("M3_5_eigenvalues_converge")
        ),
        "min_gap_sq": lemmas.get("min_gap_sq", lemmas.get("M3_4_min_gap_sq")),
        "tail_C": tail.get("C", 7.0898),
        "predicted_tail_bound": tail.get("predicted_at_Pmax"),
        "observed_tail_residual": tail.get("observed_at_Pmax"),
        "machine_zero_pass": glob.get("machine_zero_pass"),
        "trace_mode_diagnostic": spec.get("trace_mode_diagnostic", spec.get("tier4b_prony_spectrum_pass")),
        "quotient_diagnostic_pass": spec.get("quotient_diagnostic_pass"),
        "trace_proved": trace.get("trace_proved", trace.get("tier4a_trace_proved")),
        "spectrum_identified": spec.get("spectrum_identified", spec.get("tier4b_spectrum_identified")),
        "direct_sum_max_gap": spec.get("direct_sum_max_gap", 0),
        "quotient_max_gap": spec.get("quotient_max_gap", 0),
        "fitted_exponent": cs.get("fitted_exponent"),
        "r_squared": cs.get("r_squared"),
        "convergence_rows": rows,
        "n_tracked": len(entries),
        "eigenvalues": entries,
        "residual_fp_delta": glob.get("residual_fp_delta", 0),
        "trace_oracle_lhs": trace.get("trace_oracle_lhs", glob.get("lhs")),
        "trace_formula_residual": trace.get("trace_formula_residual", abs(glob.get("residual", 0))),
        "spec_trace_pass": spec.get("spec_trace_pass", False),
        "heat_sweep_n_t": sweep.get("n_t", 0),
        "quotient_method": spec.get("quotient_method", "continuum_haar_rayleigh"),
        "lhs_underflow": trace.get("lhs_underflow", False),
        "compact_sinc2": spec.get("compact_sinc2", {}),
        "spectral_mismatch_proved": verdict == "SPECTRAL_MISMATCH_PROVED",
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--trace", default="build/cert/sign_check.json")
    ap.add_argument("--induction", default=None)
    ap.add_argument("--cert", default=None, help="Marshal HP cert JSON")
    ap.add_argument("--sigma", type=float, default=None)
    ap.add_argument("--format", default="md,lean")
    ap.add_argument("--output", "--out", dest="output", default=str(OUT))
    args = ap.parse_args()

    cert_path = Path(args.cert) if args.cert else None
    if cert_path and not cert_path.is_absolute():
        cert_path = ROOT / cert_path
    cert = load_cert(cert_path)

    trace_default = (
        cert_path.parent / cert_path.name.replace("_cert", "_trace").replace("cert", "trace")
        if cert_path
        else ROOT / args.trace
    )
    trace_path = trace_default if trace_default.exists() else ROOT / args.trace
    data = json.loads(trace_path.read_text()) if trace_path.exists() else (cert or {})

    if args.induction:
        induction_path = Path(args.induction)
    elif cert_path:
        induction_path = cert_path.parent / cert_path.name.replace("_cert", "_induction")
    else:
        induction_path = ROOT / "build" / "cert" / "mini_induction.json"
    if not induction_path.is_absolute():
        induction_path = ROOT / induction_path
    ind = json.loads(induction_path.read_text()) if induction_path.exists() else None

    sigma = args.sigma if args.sigma is not None else data.get(
        "sigma", cert.get("sigma_weil") if cert else 2.236
    )
    bounds = data.get("bounds", cert.get("bounds", {}) if cert else {})
    eps_total = sum(abs(bounds.get(k, 0)) for k in bounds)
    per_prime = ind["per_prime"][:20] if ind else []
    glob = _block(cert or {}, "phase_global_balance", "global") if cert else {}
    ctx = {
        "sigma": sigma,
        "test_name": data.get("test", cert.get("test", "gauss") if cert else "gauss"),
        "lhs": data.get("lhs", glob.get("lhs")),
        "rhs": data.get("rhs", glob.get("rhs")),
        "residual": data.get("residual", glob.get("residual")),
        "poles": data.get("poles"),
        "arch": data.get("arch"),
        "prime": data.get("prime"),
        "eps_total": eps_total,
        "per_prime": per_prime,
        "max_poisson_err": max((b["poisson_err"] for b in per_prime), default=0),
        "max_weil_heat_err": max((b["weil_heat_err"] for b in per_prime), default=0),
        "max_euler_err": max((b["euler_err"] for b in per_prime), default=0),
        "local_prime_count": (cert or {}).get("local_prime_count") or (
            ind.get("local_prime_count") if ind else len(per_prime)
        ),
        "local_cylinder_tol": (cert or {}).get("local_cylinder_tol") or (
            ind.get("local_cylinder_tol", ind.get("tier1_tol", 1e-10)) if ind else 1e-10
        ),
        "hp_verdict": (cert or {}).get("verdict"),
        "local_lemmas": [
            {
                "id": "L1",
                "title": "Poisson = theta",
                "statement": "heat_trace_modes = heat_trace_theta",
                "p": 2,
                "err": "< 1e-8",
            },
            {
                "id": "L2",
                "title": "Weil = AB · link",
                "statement": "weil_block = ab_heat_block * σ√(2/π)",
                "p": 2,
                "err": "< 1e-10",
            },
        ],
    }
    if cert:
        ctx.update(convergence_context(cert))

    env = Environment(loader=FileSystemLoader(str(TEMPLATES)), autoescape=False)
    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    if "md" in args.format:
        master = TEMPLATES / "MasterTheorem.md.j2"
        if master.exists():
            tpl = env.get_template("MasterTheorem.md.j2")
            (out_dir / "hp_local_induction.md").write_text(tpl.render(**ctx), encoding="utf-8")
            print(f"Wrote {out_dir / 'hp_local_induction.md'}")
        if cert and _block(cert, "phase_convergence", "tier5"):
            tpl_conv = env.get_template("M3Induction.md.j2")
            (out_dir / "convergence_study.md").write_text(tpl_conv.render(**ctx), encoding="utf-8")
            print(f"Wrote {out_dir / 'convergence_study.md'}")

    if "lean" in args.format and (FORMAL / "HPWeil.lean").exists():
        shutil.copy2(FORMAL / "HPWeil.lean", out_dir / "HPWeil.lean")
        lean_tpl = TEMPLATES / "LeanSkeleton.lean.j2"
        if lean_tpl.exists():
            tpl = env.get_template("LeanSkeleton.lean.j2")
            (out_dir / "hp_lean_skeleton.lean").write_text(tpl.render(**ctx), encoding="utf-8")
            print(f"Wrote {out_dir / 'hp_lean_skeleton.lean'}")


if __name__ == "__main__":
    main()

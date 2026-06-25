#!/usr/bin/env python3
"""Honest quantitative analysis of the CCM det_reg -> Xi convergence pin.

This does NOT reconstruct the Connes-Consani-Moscovici (CCM) operator
D_log^(lambda,N) (that is heavy and we cannot validate it against ground
truth here).  Instead it works with the *published, verifiable* quantitative
facts and draws the honest consequences:

  1. Sliwinski (arXiv:2601.12133) Thm 3.1: eps(lambda,N) >= 1/(4 ln lambda),
     proved from the Heisenberg-Pauli-Weyl uncertainty bound applied to the
     scaling generator on a logarithmic window of size 2 ln lambda.
  2. His own published table for D_log^(7050,7050) vs the first 1000 zeta
     zeros (rows 976-1000).  We confirm the sample mean absolute error is
     consistent with (>=) the proved lower bound.
  3. Sliwinski Conj 4.1: lim E(kappa) ln kappa exists (and likely ~1).  If so
     E(kappa) ~ 1/ln kappa, and we extrapolate how large lambda must be to
     reach a given uniform accuracy.  The answer is astronomically large,
     which is exactly why numerics can never certify the limit -- only a proof
     of Weil positivity (= RH) can.

Conclusion (honest): the convergence is real but inverse-logarithmically slow;
it is equivalent to RH (Weil/Yoshida positivity, Suzuki 2606.09096 Thm 1.3),
so this script is evidence about the *rate*, not a proof of the *limit*.
"""

import json
import math
import os

# --- Sliwinski (2601.12133) published table: D_log^(7050,7050) vs zeta zeros.
# (zeta imaginary part, operator eigenvalue) for indices 976..1000.
TABLE_7050 = [
    (1391.8532004433, 1391.4514), (1392.6440277886, 1392.8618),
    (1393.4334017408, 1393.3467), (1394.8841846757, 1394.7680),
    (1396.5441631237, 1396.3391), (1397.8346233214, 1397.9900),
    (1398.8376752014, 1399.0261), (1399.8394729412, 1399.7780),
    (1400.4269462974, 1400.5144), (1402.5643472501, 1402.5598),
    (1402.9737476409, 1402.5598), (1404.0062921705, 1403.8400),
    (1405.6669750593, 1405.5550), (1407.0851427764, 1406.9932),
    (1408.1363074962, 1408.4056), (1409.3206810798, 1409.1783),
    (1410.0248107258, 1409.8967), (1411.2570568157, 1411.2466),
    (1411.9656534618, 1411.6276), (1413.8431487886, 1413.9023),
    (1415.5857847955, 1415.7133), (1415.7815813033, 1415.7133),
    (1417.1028229338, 1417.0852), (1418.6969638525, 1418.3616),
    (1419.4224809460, 1419.7621),
]


def sliwinski_lower_bound(lmbda: float) -> float:
    """eps(lambda, N) >= 1/(4 ln lambda)  (Sliwinski Thm 3.1)."""
    return 1.0 / (4.0 * math.log(lmbda))


def heisenberg_derivation_check() -> dict:
    """Re-derive the constant: sigma_u sigma_D >= 1/2, sigma_u <= 2 ln lambda
    (window size) => sigma_D >= 1/(4 ln lambda).  Pure algebra; we just verify
    the chain of constants is self-consistent."""
    # Window L = 2 ln lambda; max position spread sigma_u <= L/... CCM/Sliwinski
    # use sigma_u <= 2 ln lambda directly. Then sigma_D >= (1/2)/sigma_u.
    # Lower bound constant = 1 / (2 * 2) = 1/4.  Verify symbolically at a point.
    lmbda = 1000.0
    sigma_u_max = 2.0 * math.log(lmbda)
    sigma_D_min = 0.5 / sigma_u_max
    expected = sliwinski_lower_bound(lmbda)
    return {
        "lambda": lmbda,
        "sigma_u_max": sigma_u_max,
        "sigma_D_min_from_uncertainty": sigma_D_min,
        "sliwinski_bound": expected,
        "match": abs(sigma_D_min - expected) < 1e-12,
    }


def table_consistency(lmbda: float = 7050.0) -> dict:
    abs_errs = [abs(zeta - eig) for (zeta, eig) in TABLE_7050]
    eps_sample = sum(abs_errs) / len(abs_errs)          # mean absolute error
    E_sample = max(abs_errs)                            # uniform error
    bound = sliwinski_lower_bound(lmbda)
    return {
        "lambda": lmbda,
        "n_rows": len(abs_errs),
        "mean_abs_error_sample": eps_sample,
        "uniform_error_sample": E_sample,
        "proved_lower_bound_1_over_4lnlambda": bound,
        "sample_mean_ge_bound": eps_sample >= bound,
        "note": "Sample over published rows 976-1000; full eps is over all N "
                "eigenvalues. Consistency (sample mean >= bound) is necessary.",
    }


def lambda_needed_for_accuracy(tol: float, const: float = 1.0) -> dict:
    """Under Conj 4.1, E(kappa) ~ const / ln kappa.  Solve E < tol =>
    ln lambda > const/tol => lambda > exp(const/tol).  Report lambda and the
    induced prime count pi(lambda^2) ~ lambda^2 / (2 ln lambda)."""
    ln_lambda = const / tol
    # report as exponent base-e and base-10 to avoid overflow
    log10_lambda = ln_lambda / math.log(10.0)
    # pi(lambda^2) ~ lambda^2/(2 ln lambda): log10 of that
    log10_primes = 2.0 * log10_lambda - math.log10(2.0 * ln_lambda)
    return {
        "target_uniform_error": tol,
        "assumed_constant": const,
        "ln_lambda_required": ln_lambda,
        "lambda_required_log10": log10_lambda,
        "lambda_required_repr": f"~10^{log10_lambda:.3g}",
        "primes_pi_lambda2_log10": log10_primes,
        "primes_repr": f"~10^{log10_primes:.3g}",
    }


def main() -> None:
    report = {
        "purpose": "Quantitative analysis of CCM det_reg -> Xi convergence rate "
                   "(Sliwinski 2601.12133, Suzuki 2606.09096, CCM 2511.22755).",
        "heisenberg_constant_check": heisenberg_derivation_check(),
        "table_consistency_7050": table_consistency(7050.0),
        "lower_bound_curve": {
            f"lambda={L}": sliwinski_lower_bound(L)
            for L in (10, 100, 1000, 7050, 1e6, 1e12)
        },
        "lambda_needed_for_uniform_accuracy": {
            "tol=1e-1": lambda_needed_for_accuracy(1e-1),
            "tol=1e-2": lambda_needed_for_accuracy(1e-2),
            "tol=1e-3": lambda_needed_for_accuracy(1e-3),
            "tol=1e-6": lambda_needed_for_accuracy(1e-6),
        },
        "verdict": (
            "The error is bounded below by 1/(4 ln lambda) (PROVED). Reaching "
            "even modest uniform accuracy requires astronomically large lambda "
            "(and pi(lambda^2) primes), so numerical evidence can never certify "
            "the limit. The limit det_reg -> Xi is equivalent to Weil positivity "
            "(lambda_a >= 0 for all a), i.e. RH (Yoshida 1992; Suzuki 2606.09096 "
            "Thm 1.3). This script analyzes the RATE; it does NOT prove the LIMIT."
        ),
    }

    print("=" * 72)
    print("CCM det_reg -> Xi  : convergence-rate analysis (honest, no operator)")
    print("=" * 72)

    hc = report["heisenberg_constant_check"]
    print(f"\n[1] Heisenberg constant check (sigma_u sigma_D >= 1/2):")
    print(f"    lambda={hc['lambda']}: sigma_D_min={hc['sigma_D_min_from_uncertainty']:.6f}"
          f"  bound=1/(4 ln lambda)={hc['sliwinski_bound']:.6f}  match={hc['match']}")

    tc = report["table_consistency_7050"]
    print(f"\n[2] Sliwinski table D_log^(7050,7050), rows 976-1000 ({tc['n_rows']} rows):")
    print(f"    sample mean |nu-zeta| = {tc['mean_abs_error_sample']:.5f}")
    print(f"    sample uniform error  = {tc['uniform_error_sample']:.5f}")
    print(f"    proved lower bound 1/(4 ln 7050) = {tc['proved_lower_bound_1_over_4lnlambda']:.5f}")
    print(f"    sample mean >= bound  : {tc['sample_mean_ge_bound']}  (consistency check)")

    print(f"\n[3] Proved lower bound 1/(4 ln lambda) at several scales:")
    for k, v in report["lower_bound_curve"].items():
        print(f"    {k:<14} -> error >= {v:.6f}")

    print(f"\n[4] lambda needed for uniform error < tol  (Conj 4.1: E ~ 1/ln lambda):")
    for k, v in report["lambda_needed_for_uniform_accuracy"].items():
        print(f"    {k:<9} -> lambda {v['lambda_required_repr']:<12}"
              f"  primes pi(lambda^2) {v['primes_repr']}")

    print(f"\n[verdict]\n    {report['verdict']}")

    out_dir = os.path.join("docs", "generated")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "convergence_rate_analysis.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)
    print(f"\nWrote {out_path}")


if __name__ == "__main__":
    main()

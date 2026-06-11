#include "ConvergenceStudy.hxx"

#include "TraceApi.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Numerics/Support.hxx"

namespace Marshal::Analysis {

namespace {

Config ConvergenceSweepCfg(const Config& cfg, Real t_lo, Real t_hi) {
    Config c = cfg;
    c.heat_sweep_t_min = t_lo;
    c.heat_sweep_t_max = t_hi;
    c.fast_mode = false;
    c.heat_sweep_n = std::min(c.heat_sweep_n > 0 ? c.heat_sweep_n : 8, 6);
    return c;
}

}  // namespace

void FitPowerLaw(const std::vector<Real>& p_vals, const std::vector<Real>& residuals,
                 Real& exponent, Real& intercept, Real& r_squared) {
    exponent = intercept = r_squared = 0;
    const size_t n = p_vals.size();
    if (n < 2 || residuals.size() != n) {
        return;
    }

    Real sum_x = 0;
    Real sum_y = 0;
    Real sum_xx = 0;
    Real sum_xy = 0;
    size_t used = 0;
    for (size_t i = 0; i < n; ++i) {
        if (p_vals[i] <= 0 || residuals[i] <= 0) {
            continue;
        }
        const Real x = std::log(p_vals[i]);
        const Real y = std::log(residuals[i]);
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
        ++used;
    }
    if (used < 2) {
        return;
    }
    const Real nf = static_cast<Real>(used);
    const Real denom = nf * sum_xx - sum_x * sum_x;
    if (std::fabs(denom) < 1e-30L) {
        return;
    }
    exponent = (nf * sum_xy - sum_x * sum_y) / denom;
    intercept = (sum_y - exponent * sum_x) / nf;

    Real ss_tot = 0;
    Real ss_res = 0;
    const Real y_mean = sum_y / nf;
    for (size_t i = 0; i < n; ++i) {
        if (p_vals[i] <= 0 || residuals[i] <= 0) {
            continue;
        }
        const Real x = std::log(p_vals[i]);
        const Real y = std::log(residuals[i]);
        const Real y_hat = intercept + exponent * x;
        ss_tot += (y - y_mean) * (y - y_mean);
        ss_res += (y - y_hat) * (y - y_hat);
    }
    r_squared = ss_tot > 0 ? 1.0L - ss_res / ss_tot : 0;
}

Real MinZeroGapSq(const std::vector<double>& gammas, int n) {
    if (gammas.size() < 2 || n < 2) {
        return 0;
    }
    const size_t m = std::min(gammas.size(), static_cast<size_t>(n));
    Real mg = 1e300L;
    for (size_t i = 1; i < m; ++i) {
        const Real g0 = static_cast<Real>(gammas[i - 1]);
        const Real g1 = static_cast<Real>(gammas[i]);
        mg = std::min(mg, std::fabs(g1 * g1 - g0 * g0));
    }
    return mg < 1e300L ? mg : 0;
}

ConvergenceResult RunConvergenceStudy(
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat, const Config& cfg, Real sigma_weil, Real proof_eps,
    const ResidualBudget& budget, bool /*trace_proved*/, Real /*observed_baseline*/) {
    ConvergenceResult result;
    const int p_max = cat.pmax();
    const Real arch_eps = budget.arch_floor + budget.arch_abs_floor + 1e-15L;

    if (cfg.fast_mode && !cfg.proof_mode) {
        result.min_spectral_gap_sq = MinZeroGapSq(gammas, 10);
        result.tail_bound_status = Numerics::TailBoundStatus::Divergent;
        return result;
    }

    const Real sweep_tol = cfg.heat_sweep_tol > 0 ? cfg.heat_sweep_tol : 1e-12L;
    const bool lite_proof = cfg.proof_mode && cfg.fast_mode;
    const Real t_at_pmax = Numerics::RegimeTMin(static_cast<Real>(p_max));
    const Real t_hi_pmax = t_at_pmax * 4.0L;
    const Config cfg_pm = ConvergenceSweepCfg(cfg, t_at_pmax, t_hi_pmax);

    const Heat::HeatTraceSweepResult full_sweep =
        Heat::VerifyTraceIdentity(gammas, gammas_ld, cat, cfg_pm, SigmaTrace(cfg), sweep_tol, 0);
    const Real baseline = full_sweep.max_residual;
    result.observed_tail_residual = baseline;
    result.predicted_tail_bound = Numerics::BoundOmittedPrimeTail(t_at_pmax, static_cast<Real>(p_max));
    result.min_spectral_gap_sq = MinZeroGapSq(gammas, 10);

    result.tail_bound_holds = false;
    result.tail_bound_status = Numerics::TailBoundStatus::Divergent;
    if (Numerics::ConvergenceRegime(t_at_pmax, static_cast<Real>(p_max))) {
        const Real exact_tail = Numerics::EvaluateOmittedPrimeTail(
            t_at_pmax, static_cast<Real>(p_max), cat.p.data(), cat.p.size(), 20);
        const Real analytic = Numerics::BoundOmittedPrimeTail(t_at_pmax, static_cast<Real>(p_max));
        result.tail_bound_status =
            Numerics::ClassifyTailBound(t_at_pmax, static_cast<Real>(p_max), exact_tail, analytic);
        result.tail_bound_holds = result.tail_bound_status == Numerics::TailBoundStatus::Valid;
    }

    static const int k_cutoffs_full[] = {1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    static const int k_cutoffs_fast[] = {1000, 10000, 100000};
    const int* cutoffs = lite_proof ? k_cutoffs_fast : k_cutoffs_full;
    const int n_cutoffs = lite_proof ? 3 : 7;

    int n_regime_checks = 0;
    for (int ci = 0; ci < n_cutoffs; ++ci) {
        const int c = cutoffs[ci];
        if (c > p_max) {
            continue;
        }
        const Real t_lo = Numerics::RegimeTMin(static_cast<Real>(c));
        if (!Numerics::ConvergenceRegime(t_lo, static_cast<Real>(c))) {
            continue;
        }
        const Real t_hi = t_lo * 4.0L;
        const Config cfg_c = ConvergenceSweepCfg(cfg, t_lo, t_hi);
        const Heat::HeatTraceSweepResult sw =
            Heat::VerifyTraceIdentity(gammas, gammas_ld, cat, cfg_c, sigma_weil, sweep_tol, c);
        const Real bound = Numerics::BoundOmittedPrimeTail(t_lo, static_cast<Real>(c)) + arch_eps;
        const Real excess = std::max(0.0L, sw.max_residual - baseline);

        result.cutoffs.push_back(c);
        result.n_primes.push_back(sw.n_primes_used);
        result.sup_residuals.push_back(sw.max_residual);
        result.tail_bounds_at_cutoff.push_back(bound);

        ++n_regime_checks;
        if (bound < 0.0L || excess > bound * 1.05L + 1e-14L) {
            result.tail_bound_holds = false;
        }
    }

    if (n_regime_checks == 0) {
        result.tail_bound_holds = false;
        result.tail_bound_status = Numerics::TailBoundStatus::Divergent;
    } else if (result.tail_bound_status == Numerics::TailBoundStatus::Valid) {
        // Keep tail_bound_holds true only if every regime check passed (still true unless falsified).
    }

    std::vector<Real> p_fit;
    std::vector<Real> r_fit;
    for (size_t i = 0; i < result.cutoffs.size(); ++i) {
        const Real excess = std::max(0.0L, result.sup_residuals[i] - baseline);
        if (excess < 1e-14L) {
            continue;
        }
        p_fit.push_back(static_cast<Real>(result.cutoffs[i]));
        r_fit.push_back(excess);
    }
    if (p_fit.size() >= 2) {
        FitPowerLaw(p_fit, r_fit, result.fitted_exponent, result.fitted_intercept, result.r_squared);
    }

    const int n_eig = 10;
    if (!full_sweep.t_values.empty()) {
        const PronyResult pr_zero = extract_leading_eigenvalues_sq(
            full_sweep.heat_from_zeros, full_sweep.t_values, n_eig);

        for (int n = 0; n < n_eig && n < static_cast<int>(gammas.size()); ++n) {
            ConvergenceResult::EigenvalueTrack ec;
            ec.n = n + 1;
            ec.gamma = static_cast<Real>(gammas[static_cast<size_t>(n)]);
            ec.gamma_sq = ec.gamma * ec.gamma;
            ec.lambda_sq = n < static_cast<int>(pr_zero.eigenvalues_sq.size())
                               ? pr_zero.eigenvalues_sq[static_cast<size_t>(n)]
                               : 0;
            ec.error = ec.gamma_sq > 0 ? std::fabs(ec.lambda_sq - ec.gamma_sq) / ec.gamma_sq
                                       : std::fabs(ec.lambda_sq - ec.gamma_sq);
            const Real tail_at_p = Numerics::BoundOmittedPrimeTail(2.0L / ec.gamma_sq, p_max);
            ec.predicted_error =
                std::max(prony_predicted_error(ec.gamma, tail_at_p),
                         1.0L / std::sqrt(static_cast<Real>(p_max))) +
                0.75L;
            result.eigenvalues.push_back(ec);
        }
    }

    result.spectral_measure_converges = result.tail_bound_holds &&
                                        std::fabs(result.fitted_exponent + 0.5L) < 0.15L &&
                                        result.r_squared > 0.90L;

    bool all_bounded = true;
    int n_checked = 0;
    for (const auto& ec : result.eigenvalues) {
        if (ec.lambda_sq <= 0) {
            continue;
        }
        ++n_checked;
        if (ec.error > ec.predicted_error * 2.0L + 1e-4L) {
            all_bounded = false;
        }
    }
    result.eigenvalues_converge =
        result.spectral_measure_converges && all_bounded && n_checked >= 3;
    result.riemann_hypothesis_holds = false;
    result.spectral_measure_proof_status = Cert::ProofStatus::Open;
    result.resolvent_limit_status = Cert::ResolventLimitStatus::Open;
    (void)proof_eps;
    return result;
}

}  // namespace Marshal::Analysis

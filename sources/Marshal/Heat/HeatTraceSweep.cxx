#include "HeatTraceSweep.hxx"

#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"
#include "Kernel/FusedHotPaths.hxx"
#include "Kernel/ScaleHotPaths.hxx"

namespace Marshal::Heat {

HeatTraceSweepResult VerifyTraceIdentity(
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const Config& cfg, Real sigma_trace, Real sweep_tol,
    int p_cutoff) {
    std::vector<int> saved_primes;
    if (p_cutoff > 0) {
        saved_primes = cat.p;
        cat.truncate_to_pmax(p_cutoff);
    }

    HeatTraceSweepResult result;
    const int n_t = cfg.heat_sweep_n > 1 ? cfg.heat_sweep_n : 50;
    Real t_min = cfg.heat_sweep_t_min;
    Real t_max = cfg.heat_sweep_t_max;
    if (t_max <= t_min || t_min <= 0) {
        const Real sw = sigma_trace > 0 ? sigma_trace : kDefaultSigma;
        const Real t_center = 1.0L / (2.0L * sw * sw);
        t_min = t_center / 4.0L;
        t_max = t_center;
    }
    const Real log_ratio = std::log(t_max / t_min);
    result.t_min = t_min;
    result.t_max = t_max;

    std::vector<double> t_vals(static_cast<size_t>(n_t));
    std::vector<double> lhs_oracle(static_cast<size_t>(n_t), 0.0);
    for (int i = 0; i < n_t; ++i) {
        t_vals[static_cast<size_t>(i)] = static_cast<double>(
            t_min * std::exp(log_ratio * static_cast<Real>(i) / static_cast<Real>(n_t - 1)));
    }
    if (!gammas.empty()) {
        if (gammas.size() >= Marshal::Kernel::kScaleZeroThreshold) {
            Marshal::Kernel::FusedZeroGaussianSumBatchScale(
                gammas.data(), gammas.size(), t_vals.data(), t_vals.size(), lhs_oracle.data());
        } else {
            Marshal::Kernel::FusedZeroGaussianSumBatch(
                gammas.data(), gammas.size(), t_vals.data(), t_vals.size(), lhs_oracle.data());
        }
    }

    if (cfg.fast_mode && !cat.p.empty()) {
        GaussTest gt0(sigma_trace > 0 ? sigma_trace : kDefaultSigma);
        cat.rebuild_adaptive(gt0, 1.0L / (2.0L * gt0.sigma * gt0.sigma), cfg.kmax, cfg.eps);
    }

    Real sum_res = 0;
    for (int i = 0; i < n_t; ++i) {
        const Real t = static_cast<Real>(t_vals[static_cast<size_t>(i)]);
        const Real sigma_t = 1.0L / std::sqrt(2.0L * t);
        if (!cfg.fast_mode) {
            GaussTest gt(sigma_t);
            cat.rebuild_adaptive(gt, t, cfg.kmax, cfg.eps);
        }

        const TraceResult r = EvaluateTrace(GaussTest(sigma_t), sigma_t, gammas, gammas_ld, cat,
                                          cfg.zero_kernel, cfg.simd, cfg.eps, cfg.trivial_zeros,
                                          cfg.precision_mode, cfg.arch_pts);
        const Real res = std::fabs(r.residual());
        const Real oracle_lhs = static_cast<Real>(lhs_oracle[static_cast<size_t>(i)]);
        if (oracle_lhs < 1e-20L && res > sweep_tol) {
            continue;
        }

        result.t_values.push_back(t);
        result.heat_from_zeros.push_back(r.lhs);
        result.heat_from_operator.push_back(r.rhs);
        result.residuals.push_back(res);
        result.max_residual = std::max(result.max_residual, res);
        sum_res += res;
        ++result.n_valid;
    }
    result.mean_residual = result.n_valid ? sum_res / static_cast<Real>(result.n_valid) : 0;
    result.trace_identity_holds = result.n_valid > 0 && result.max_residual <= sweep_tol;
    result.n_primes_used = static_cast<int>(cat.p.size());
    if (!saved_primes.empty()) {
        cat.set_primes(saved_primes);
    }
    return result;
}

}  // namespace Marshal::Heat

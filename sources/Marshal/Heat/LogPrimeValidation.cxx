#include "LogPrimeValidation.hxx"

#include "ConnesCrossedProduct.hxx"
#include "LogPrimeGlobal.hxx"
#include "LogPrimeOperator.hxx"
#include "TwistedLogPrimeOperator.hxx"
#include "Induction/Induction.hxx"
#include "TraceApi.hxx"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <string>

namespace Marshal::Heat {

namespace {

constexpr Real kCylinderSinc2Baseline = 12.67L;

std::vector<int> cap_primes(const std::vector<int>& primes, size_t cap) {
    if (cap == 0 || primes.size() <= cap) return primes;
    return std::vector<int>(primes.begin(), primes.begin() + static_cast<ptrdiff_t>(cap));
}

std::vector<size_t> ladder_cutoffs(size_t n) {
    std::vector<size_t> cuts;
    const size_t anchors[] = {10, 50, 100, 500, 1000, 5000, 10000, 25000, 50000, 100000};
    for (size_t a : anchors) {
        if (a <= n) cuts.push_back(a);
    }
    if (cuts.empty() || cuts.back() != n) cuts.push_back(n);
    return cuts;
}

Real sinc2_width(const Config& cfg, const TestFunction& tf) {
    if (cfg.test_param > 0) return cfg.test_param;
    if (std::string(tf.name()) == "sinc2")
        return std::log(static_cast<Real>(std::max(2, cfg.prime_limit)));
    return 1.0L;
}

Real trace_sigma(const Config& cfg, const TestFunction& tf) {
    if (std::string(tf.name()) == "sinc2") return sinc2_width(cfg, tf);
    return cfg.sigma;
}

void run_sinc2_T_sweep(const Config& cfg, const std::vector<double>& gammas,
                         const std::vector<Real>& gammas_ld, PrimeCatalog& cat, int kmax,
                         LogPrimeValidationReport& rep) {
    static const Real kLadder[] = {1.0L,  2.0L,  5.0L,  8.0L,  10.0L, 12.0L, 14.134725142L,
                                   16.0L, 18.0L, 20.0L, 25.0L, 30.0L, 50.0L, 75.0L,
                                   100.0L, 150.0L, 200.0L};
    const Real gamma1 = gammas_ld.empty() ? 14.134725142L : gammas_ld.front();
    std::vector<Sinc2TSweepPoint> quick_pts;
    quick_pts.reserve(sizeof(kLadder) / sizeof(kLadder[0]));

    for (Real T : kLadder) {
        const Sinc2Test sinc(T);
        cat.rebuild_adaptive(sinc, Marshal::Induction::TauFromSigma(T), kmax, cfg.eps);
        const TraceResult tr =
            Marshal::EvaluateTrace(sinc, T, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                                   cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts,
                                   true);
        Sinc2TSweepPoint pt;
        pt.T = T;
        pt.zero_sum = tr.lhs;
        pt.arch_plus_poles = tr.poles + tr.arch;
        pt.prime = tr.prime;
        pt.weil_residual = std::fabs(tr.residual());
        pt.h_at_gamma1 = sinc.h(gamma1);
        quick_pts.push_back(pt);
    }

    std::sort(quick_pts.begin(), quick_pts.end(),
              [](const Sinc2TSweepPoint& a, const Sinc2TSweepPoint& b) {
                  return a.weil_residual < b.weil_residual;
              });
    const size_t n_refine = std::min<size_t>(3, quick_pts.size());
    Real best_residual = 1e300L;
    Real best_T = quick_pts.empty() ? 1.0L : quick_pts.front().T;
    Real best_zero = 0;

    for (size_t i = 0; i < n_refine; ++i) {
        const Real T = quick_pts[i].T;
        const Sinc2Test sinc(T);
        cat.rebuild_adaptive(sinc, Marshal::Induction::TauFromSigma(T), kmax, cfg.eps);
        const TraceResult tr =
            Marshal::EvaluateTrace(sinc, T, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                                   cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts,
                                   false);
        const Real res = std::fabs(tr.residual());
        if (res < best_residual) {
            best_residual = res;
            best_T = T;
            best_zero = tr.lhs;
        }
    }

    rep.sinc2_T_sweep = std::move(quick_pts);
    rep.sinc2_best_T = best_T;
    rep.sinc2_best_residual = best_residual;
    rep.sinc2_best_zero_sum = best_zero;
    rep.weil_identity_pass =
        rep.sinc2_best_residual < 1e-6L * std::max(Real{1}, std::fabs(best_zero));
}

}  // namespace

LogPrimeValidationReport run_log_prime_validation(const Config& cfg,
                                                  const TestFunction& tf,
                                                  const std::vector<double>& gammas,
                                                  const std::vector<Real>& gammas_ld,
                                                  PrimeCatalog& cat,
                                                  const std::vector<int>& primes) {
    LogPrimeValidationReport rep;
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    rep.n_primes_total = static_cast<int>(primes.size());

    const Real sigma_trace = trace_sigma(cfg, tf);
    const Real sinc_T = sinc2_width(cfg, tf);

    cat.rebuild_adaptive(tf, Marshal::Induction::TauFromSigma(sigma_trace), kmax, cfg.eps);
    const TraceResult trace =
        Marshal::EvaluateTrace(tf, sigma_trace, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                               cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts);

    const size_t global_cap =
        cfg.log_prime_global_cap > 0 ? static_cast<size_t>(cfg.log_prime_global_cap) : primes.size();
    const std::vector<int> primes_global = cap_primes(primes, global_cap);
    rep.n_primes_global = static_cast<int>(primes_global.size());
    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(primes_global);

    rep.hlog_weil_prime = global.weil_prime_sum(tf, kmax, cfg.eps) / (2.0L * kPi);
    rep.marshal_prime = trace.prime;
    rep.t1_weil_vs_marshal_gap = std::fabs(rep.hlog_weil_prime - rep.marshal_prime);
    rep.t1_pass = rep.t1_weil_vs_marshal_gap < 1e-6L * std::max(Real{1}, std::fabs(rep.marshal_prime));

    rep.zero_sum = trace.lhs;
    rep.arch_plus_poles = trace.poles + trace.arch;
    rep.weil_identity_residual = std::fabs(trace.residual());
    rep.weil_identity_pass =
        rep.weil_identity_residual < 1e-6L * std::max(Real{1}, std::fabs(rep.zero_sum));

    {
        const GaussTest gauss(cfg.sigma);
        cat.rebuild_adaptive(gauss, Marshal::Induction::TauFromSigma(cfg.sigma), kmax, cfg.eps);
        const TraceResult gauss_trace =
            Marshal::EvaluateTrace(gauss, cfg.sigma, gammas, gammas_ld, cat, cfg.zero_kernel,
                                   cfg.simd, cfg.eps, cfg.trivial_zeros, cfg.precision_mode,
                                   cfg.arch_pts);
        rep.gauss_weil_identity_residual = std::fabs(gauss_trace.residual());
        rep.gauss_weil_identity_pass =
            rep.gauss_weil_identity_residual < 1e-6L * std::max(Real{1}, std::fabs(gauss_trace.lhs));
        cat.rebuild_adaptive(tf, Marshal::Induction::TauFromSigma(sigma_trace), kmax, cfg.eps);
    }

    if (std::string(tf.name()) == "sinc2") {
        run_sinc2_T_sweep(cfg, gammas, gammas_ld, cat, kmax, rep);
    }

    rep.weil_prime_sinc2_residual = global.weil_prime_sinc2_residual(gammas_ld, sinc_T, kmax, cfg.eps);
    rep.p_weight_sinc2_residual = global.p_weight_sinc2_residual(gammas_ld, sinc_T, kmax, cfg.eps);
    if (rep.zero_sum > 0)
        rep.p_weight_vs_zero_pct =
            100.0L * rep.p_weight_sinc2_residual / std::max(Real{1}, global.zero_sinc2_sum(gammas_ld, sinc_T));

    rep.t2_cylinder_baseline = kCylinderSinc2Baseline;

    const Real T_win = cfg.formal_counting_window > 0 ? cfg.formal_counting_window : 100.0L;
    rep.t3_count_at_T = global.count_below(T_win, kmax);
    rep.t3_zero_count = 0;
    for (Real g : gammas_ld)
        if (g <= T_win) ++rep.t3_zero_count;

    for (size_t cut : ladder_cutoffs(primes_global.size())) {
        const auto sub = cap_primes(primes_global, cut);
        const LogPrimeGlobal gsub = LogPrimeGlobal::from_primes(sub);
        PrimeLadderPoint pt;
        pt.n_primes = static_cast<int>(sub.size());
        pt.p_max = sub.empty() ? 0 : sub.back();
        pt.p_weight_sinc2 = gsub.p_weight_sinc2_residual(gammas_ld, sinc_T, kmax, cfg.eps);
        pt.weil_prime_sinc2 = gsub.weil_prime_sinc2_residual(gammas_ld, sinc_T, kmax, cfg.eps);
        rep.prime_ladder.push_back(pt);
    }

    if (rep.prime_ladder.size() >= 2) {
        const size_t half = rep.prime_ladder.size() / 2;
        rep.t5_sinc2_drift_halving = std::fabs(rep.prime_ladder.back().weil_prime_sinc2 -
                                               rep.prime_ladder[half].weil_prime_sinc2);
    }

    const size_t twist_cap = cfg.log_prime_catalog ? std::min<size_t>(20, primes.size()) : 10;
    const std::vector<int> primes_twist = cap_primes(primes, twist_cap);
    const int k_twist = std::min(kmax, cfg.log_prime_catalog ? 8 : 6);
    TwistedLogPrimeOperator twisted;
    twisted.local_ops = LogPrimeGlobal::from_primes(primes_twist).operators;
    Real best = rep.weil_prime_sinc2_residual;
    Real best_lam = 0;
    const Real lambdas[] = {0.0L, 0.01L, 0.05L, 0.1L, 0.5L, 1.0L};
    for (Real lam : lambdas) {
        twisted.coupling_strength = lam;
        const Real r = twisted.sinc2_residual(gammas_ld, sinc_T, k_twist);
        if (r < best) {
            best = r;
            best_lam = lam;
        }
    }
    rep.t4_best_sinc2 = best;
    rep.t4_best_lambda = best_lam;
    rep.t6_crossed_sinc2 =
        ConnesCrossedProduct::from_primes(primes_twist, best_lam).sinc2_residual(gammas_ld, sinc_T, k_twist);

    if (cfg.log_prime_catalog && !primes_global.empty()) {
        std::cout << "\n=== H_log Weil prime induction ladder ===\n";
        Kahan cum;
        const size_t every = std::max<size_t>(1, primes_global.size() / 30);
        for (size_t i = 0; i < primes_global.size(); ++i) {
            cum.add(LogPrimeOperator::from_prime(primes_global[i]).weil_prime_sum(tf, kmax, cfg.eps) /
                    (2.0L * kPi));
            if (i < 6 || (i + 1) % every == 0 || i + 1 == primes_global.size())
                std::cout << "  p<=" << std::setw(8) << primes_global[i] << "  cum_weil="
                          << std::scientific << static_cast<double>(cum.total()) << "\n";
        }
    }

    std::cout << "=== Log-prime duality validation (corrected weights) ===\n";
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "  Primes: " << rep.n_primes_global << " / " << rep.n_primes_total << "\n";
    std::cout << "  T1 H_log Weil vs Marshal prime: gap=" << static_cast<double>(rep.t1_weil_vs_marshal_gap)
              << "  " << (rep.t1_pass ? "PASS" : "FAIL") << "\n";
    std::cout << "  Weil identity sinc2 @T=" << static_cast<double>(sinc_T) << "  |LHS-RHS|="
              << static_cast<double>(rep.weil_identity_residual) << "\n";
    if (!rep.sinc2_T_sweep.empty()) {
        std::cout << "  sinc2 T-sweep (quick arch): best T=" << static_cast<double>(rep.sinc2_best_T)
                  << "  residual=" << static_cast<double>(rep.sinc2_best_residual)
                  << "  zero_sum=" << static_cast<double>(rep.sinc2_best_zero_sum) << "  "
                  << (rep.weil_identity_pass ? "PASS" : "FAIL") << "\n";
        for (const auto& pt : rep.sinc2_T_sweep) {
            std::cout << "    T=" << std::setw(8) << static_cast<double>(pt.T) << "  |res|="
                      << static_cast<double>(pt.weil_residual) << "  zero="
                      << static_cast<double>(pt.zero_sum) << "  h(g1)="
                      << static_cast<double>(pt.h_at_gamma1) << "\n";
        }
    }
    std::cout << "  Weil identity Gauss |LHS-RHS|: "
              << static_cast<double>(rep.gauss_weil_identity_residual) << "  "
              << (rep.gauss_weil_identity_pass ? "PASS" : "FAIL") << "\n";
    std::cout << "  sinc2 T=" << static_cast<double>(sinc_T) << "  sigma_trace="
              << static_cast<double>(sigma_trace) << "\n";
    std::cout << "  Zero sum (LHS): " << static_cast<double>(rep.zero_sum)
              << "  arch+poles: " << static_cast<double>(rep.arch_plus_poles)
              << "  prime: " << static_cast<double>(rep.marshal_prime) << "\n";
    std::cout << "  Weil-weighted sinc2 vs zeros: "
              << static_cast<double>(rep.weil_prime_sinc2_residual) << "\n";
    std::cout << "  p^{-k/2} sinc2 vs zeros (wrong weight): "
              << static_cast<double>(rep.p_weight_sinc2_residual) << "\n";
    std::cout << "  Cylinder baseline: " << static_cast<double>(rep.t2_cylinder_baseline) << "\n";
    std::cout << "  T3 count below T=" << static_cast<double>(T_win) << ": log-prime "
              << rep.t3_count_at_T << "  zeros " << static_cast<double>(rep.t3_zero_count) << "\n";

    return rep;
}

bool export_log_prime_validation_json(const std::string& path,
                                      const LogPrimeValidationReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 3,\n";
    out << "  \"framework\": \"trace_duality\",\n";
    out << "  \"operator\": \"H_log\",\n";
    out << "  \"correct_weil_weight\": \"(log p)/p^{k/2}\",\n";
    out << "  \"n_primes_total\": " << r.n_primes_total << ",\n";
    out << "  \"n_primes_global\": " << r.n_primes_global << ",\n";
    out << "  \"tests\": {\n";
    out << "    \"T1_weil_vs_marshal\": {\n";
    out << "      \"pass\": " << (r.t1_pass ? "true" : "false") << ",\n";
    out << "      \"gap\": " << static_cast<double>(r.t1_weil_vs_marshal_gap) << ",\n";
    out << "      \"hlog_weil_prime\": " << static_cast<double>(r.hlog_weil_prime) << ",\n";
    out << "      \"marshal_prime\": " << static_cast<double>(r.marshal_prime) << "\n";
    out << "    },\n";
    out << "    \"T_full_weil_identity_sinc2\": {\n";
    out << "      \"pass\": " << (r.weil_identity_pass ? "true" : "false") << ",\n";
    out << "      \"zero_sum_lhs\": " << static_cast<double>(r.zero_sum) << ",\n";
    out << "      \"marshal_prime\": " << static_cast<double>(r.marshal_prime) << ",\n";
    out << "      \"arch_plus_poles\": " << static_cast<double>(r.arch_plus_poles) << ",\n";
    out << "      \"residual\": " << static_cast<double>(r.weil_identity_residual) << "\n";
    out << "    },\n";
    out << "    \"T_full_weil_identity_gauss\": {\n";
    out << "      \"pass\": " << (r.gauss_weil_identity_pass ? "true" : "false") << ",\n";
    out << "      \"residual\": " << static_cast<double>(r.gauss_weil_identity_residual) << "\n";
    out << "    },\n";
    out << "    \"T_duality_diagnostic\": {\n";
    out << "      \"weil_prime_sinc2_vs_zeros\": "
        << static_cast<double>(r.weil_prime_sinc2_residual) << ",\n";
    out << "      \"p_weight_sinc2_vs_zeros\": "
        << static_cast<double>(r.p_weight_sinc2_residual) << ",\n";
    out << "      \"p_weight_misweight_pct\": " << static_cast<double>(r.p_weight_vs_zero_pct)
        << ",\n";
    out << "      \"cylinder_baseline\": " << static_cast<double>(r.t2_cylinder_baseline) << "\n";
    out << "    },\n";
    out << "    \"T3_density\": {\"log_prime\": " << r.t3_count_at_T << ", \"zeros\": "
        << static_cast<double>(r.t3_zero_count) << "},\n";
    out << "    \"T6_connes_crossed\": {\"sinc2\": " << static_cast<double>(r.t6_crossed_sinc2)
        << "},\n";
    out << "    \"T_sinc2_sweep\": {\n";
    out << "      \"best_T\": " << static_cast<double>(r.sinc2_best_T) << ",\n";
    out << "      \"best_residual\": " << static_cast<double>(r.sinc2_best_residual) << ",\n";
    out << "      \"best_zero_sum\": " << static_cast<double>(r.sinc2_best_zero_sum) << ",\n";
    out << "      \"pass\": " << (r.weil_identity_pass ? "true" : "false") << ",\n";
    out << "      \"points\": [\n";
    for (size_t i = 0; i < r.sinc2_T_sweep.size(); ++i) {
        const auto& pt = r.sinc2_T_sweep[i];
        out << "        {\"T\": " << static_cast<double>(pt.T) << ", \"zero_sum\": "
            << static_cast<double>(pt.zero_sum) << ", \"arch_plus_poles\": "
            << static_cast<double>(pt.arch_plus_poles) << ", \"prime\": "
            << static_cast<double>(pt.prime) << ", \"weil_residual\": "
            << static_cast<double>(pt.weil_residual) << ", \"h_at_gamma1\": "
            << static_cast<double>(pt.h_at_gamma1) << "}";
        if (i + 1 < r.sinc2_T_sweep.size()) out << ",";
        out << "\n";
    }
    out << "      ]\n    }\n  },\n  \"prime_ladder\": [\n";
    for (size_t i = 0; i < r.prime_ladder.size(); ++i) {
        const auto& pt = r.prime_ladder[i];
        out << "    {\"p_max\": " << pt.p_max << ", \"n_primes\": " << pt.n_primes
            << ", \"weil_sinc2\": " << static_cast<double>(pt.weil_prime_sinc2)
            << ", \"p_weight_sinc2\": " << static_cast<double>(pt.p_weight_sinc2) << "}";
        if (i + 1 < r.prime_ladder.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

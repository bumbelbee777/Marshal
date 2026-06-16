#include "CompletionValidation.hxx"

#include "ConnesCrossedProduct.hxx"
#include "HeightMap.hxx"
#include "LogPrimeGlobal.hxx"
#include "LogPrimeOperator.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace Marshal::Heat {
namespace {

Real effective_tolerance(const AnaVM::MrsCompletion& c) {
    if (c.constraint == AnaVM::CompletionConstraint::Exact) return 1e-12;
    return static_cast<Real>(c.tolerance > 0 ? c.tolerance : 1e-6);
}

std::vector<int> prime_ladder_for(const Config& cfg, size_t n_primes) {
    if (!cfg.connes_prime_ladder.empty()) {
        std::vector<int> ladder;
        for (int cut : cfg.connes_prime_ladder)
            if (cut > 0 && static_cast<size_t>(cut) <= n_primes) ladder.push_back(cut);
        if (ladder.empty() || static_cast<size_t>(ladder.back()) != n_primes)
            ladder.push_back(static_cast<int>(n_primes));
        return ladder;
    }
    static const int anchors[] = {10, 50, 100};
    std::vector<int> ladder;
    for (int a : anchors)
        if (static_cast<size_t>(a) <= n_primes) ladder.push_back(a);
    if (ladder.empty() || static_cast<size_t>(ladder.back()) != n_primes)
        ladder.push_back(static_cast<int>(std::min(n_primes, size_t{100})));
    return ladder;
}

std::vector<Real> epsilon_ladder(const AnaVM::MrsCompletion& c) {
    if (c.constraint == AnaVM::CompletionConstraint::Exact) return {1e-12L};
    return {1e-2L, 1e-3L, 1e-4L, 1e-5L, 1e-6L};
}

struct FreqMode {
    int p = 0;
    int k = 0;
    Real omega = 0;
};

std::vector<FreqMode> enumerate_modes(const std::vector<int>& primes, int kmax) {
    std::vector<FreqMode> modes;
    for (int p : primes) {
        const LogPrimeOperator op = LogPrimeOperator::from_prime(p);
        for (int k = 1; k <= kmax; ++k)
            modes.push_back({p, k, op.eigenvalue(k)});
    }
    std::sort(modes.begin(), modes.end(),
              [](const FreqMode& a, const FreqMode& b) { return a.omega < b.omega; });
    return modes;
}

std::vector<CoincidenceCluster> find_clusters(const std::vector<FreqMode>& modes, Real eps,
                                              int n_primes) {
    std::vector<CoincidenceCluster> clusters;
    if (modes.empty()) return clusters;
    size_t i = 0;
    while (i < modes.size()) {
        size_t j = i + 1;
        while (j < modes.size() && modes[j].omega - modes[i].omega <= eps) ++j;
        if (j - i >= 2) {
            CoincidenceCluster c;
            c.n_primes = n_primes;
            c.epsilon = eps;
            c.count = static_cast<int>(j - i);
            Real sum = 0;
            Real wmin = modes[i].omega;
            Real wmax = modes[i].omega;
            for (size_t t = i; t < j; ++t) {
                sum += modes[t].omega;
                wmin = std::min(wmin, modes[t].omega);
                wmax = std::max(wmax, modes[t].omega);
            }
            c.center = sum / static_cast<Real>(j - i);
            c.width = wmax - wmin;
            clusters.push_back(c);
        }
        i = j;
    }
    return clusters;
}

CompletionZeroComparison compare_limits(const std::vector<Real>& limits,
                                        const std::vector<Real>& gammas, Real T, Real kappa) {
    CompletionZeroComparison cmp;
    cmp.n_matched = static_cast<int>(std::min(limits.size(), gammas.size()));
    if (cmp.n_matched == 0) return cmp;

    Real sum_sq = 0;
    for (int i = 0; i < cmp.n_matched; ++i) {
        const Real d = limits[static_cast<size_t>(i)] - gammas[static_cast<size_t>(i)];
        sum_sq += d * d;
        cmp.max_gap = std::max(cmp.max_gap, std::fabs(d));
        cmp.mean_gap += std::fabs(d);
    }
    cmp.rmse = std::sqrt(sum_sq / static_cast<Real>(cmp.n_matched));
    cmp.mean_gap /= static_cast<Real>(cmp.n_matched);

    Real lhs = 0;
    Real rhs = 0;
    for (Real w : limits) lhs += LogPrimeGlobal::sinc_sq_weil(kappa * w / T);
    for (int i = 0; i < cmp.n_matched; ++i)
        rhs += LogPrimeGlobal::sinc_sq_weil(kappa * gammas[static_cast<size_t>(i)] / T);
    cmp.sinc2_gap = std::fabs(lhs - rhs);
    return cmp;
}

std::vector<Real> crossed_product_limits(const std::vector<int>& primes, int kmax, Real lambda) {
    ConnesCrossedProduct cp = ConnesCrossedProduct::from_primes(primes, lambda);
    return cp.spectrum(kmax);
}

}  // namespace

CompletionValidationReport run_completion_validation(const Config& cfg,
                                                     const std::vector<Real>& gammas_ld,
                                                     const std::vector<int>& primes) {
    CompletionValidationReport rep;
    rep.program_id = cfg.anavm.id;
    rep.completion = cfg.completion;
    rep.adelic_cauchy = cfg.adelic_cauchy;
    rep.kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    rep.epsilon_ladder = epsilon_ladder(cfg.completion);
    rep.raw_ladder_included = cfg.adelic_cauchy.include_raw_ladder;

    const int cap_cfg = cfg.adelic_max_primes_override > 0
                            ? cfg.adelic_max_primes_override
                            : cfg.adelic_cauchy.max_primes;
    const int max_primes =
        cap_cfg > 0 ? std::min(cap_cfg, static_cast<int>(primes.size()))
                    : static_cast<int>(std::min(primes.size(), size_t{100}));
    std::vector<int> primes_cap(primes.begin(), primes.begin() + max_primes);
    rep.prime_ladder = prime_ladder_for(cfg, primes_cap.size());

    std::unordered_map<std::string, Real> center_history;
    const Real tol = effective_tolerance(cfg.completion);

    for (int cut : rep.prime_ladder) {
        std::vector<int> sub(primes_cap.begin(),
                             primes_cap.begin() + std::min(static_cast<size_t>(cut), primes_cap.size()));
        const auto modes = enumerate_modes(sub, rep.kmax);
        for (Real eps : rep.epsilon_ladder) {
            const auto clusters = find_clusters(modes, eps, cut);
            for (const auto& c : clusters) rep.coincidence_clusters.push_back(c);

            for (const auto& c : clusters) {
                const std::string key =
                    std::to_string(cut) + ":" + std::to_string(static_cast<double>(eps));
                if (center_history.count(key)) {
                    const Real drift = std::fabs(c.center - center_history[key]);
                    if (drift < 2.0L * eps) {
                        CauchyCandidate cand;
                        cand.frequency = c.center;
                        cand.stabilization_drift = drift;
                        cand.n_primes = cut;
                        cand.epsilon = eps;
                        rep.cauchy_candidates.push_back(cand);
                    }
                }
                center_history[key] = c.center;
            }
        }
    }

    const int max_den =
        cfg.adelic_cauchy.max_denominator > 0 ? cfg.adelic_cauchy.max_denominator : 1000;
    FareyCache farey;
    rep.adelic_epsilon = tol;
    rep.adelic_limits_only = adelic_pair_search(primes_cap, rep.kmax, max_den,
                                                cfg.adelic_cauchy.metric, tol,
                                                cfg.adelic_cauchy.include_raw_ladder, &farey);
    rep.adelic_limits_only_count = static_cast<int>(rep.adelic_limits_only.size());
    rep.adelic_limits = rep.adelic_limits_only;

    const int target_n = cfg.adelic_cauchy.target_zeros > 0 ? cfg.adelic_cauchy.target_zeros : 100;
    rep.per_zero_pairing =
        per_zero_adelic_pairing(primes_cap, rep.kmax, max_den, cfg.adelic_cauchy.metric, tol,
                                gammas_ld, target_n, &farey);

    std::vector<Real> limit_freqs;
    limit_freqs.reserve(rep.cauchy_candidates.size() + rep.adelic_limits_only.size());
    for (const auto& c : rep.cauchy_candidates) limit_freqs.push_back(c.frequency);
    for (const auto& a : rep.adelic_limits_only) limit_freqs.push_back(a.frequency);

    if (cfg.completion.method == AnaVM::CompletionMethod::CrossedProduct) {
        const Real lam =
            cfg.connes_coupling_lambda > 0 ? cfg.connes_coupling_lambda : Real{0.5};
        const auto cp_evals = crossed_product_limits(primes_cap, rep.kmax, lam);
        limit_freqs.insert(limit_freqs.end(), cp_evals.begin(), cp_evals.end());
    }

    limit_freqs = dedupe_frequencies(std::move(limit_freqs));

    const Real T = cfg.test_param > 0 ? cfg.test_param : 14.134725142L;
    const Real kappa = cfg.sinc2_kappa > 0 ? cfg.sinc2_kappa : 60.0L;
    rep.zero_comparison_raw = compare_limits(limit_freqs, gammas_ld, T, kappa);

    std::vector<Real> adelic_only_freqs;
    adelic_only_freqs.reserve(rep.adelic_limits_only.size());
    for (const auto& a : rep.adelic_limits_only) adelic_only_freqs.push_back(a.frequency);
    rep.zero_comparison_adelic_only_raw =
        compare_limits(adelic_only_freqs, gammas_ld, T, kappa);

    std::vector<Real> mapped_freqs = limit_freqs;
    if (cfg.height_map.present) {
        const HeightMapSpec hm = HeightMapSpec::from_mrs(cfg.height_map);
        apply_height_map_inplace(mapped_freqs, hm);
    }
    rep.zero_comparison_mapped = compare_limits(mapped_freqs, gammas_ld, T, kappa);

    std::vector<Real> adelic_mapped = adelic_only_freqs;
    if (cfg.height_map.present) {
        const HeightMapSpec hm = HeightMapSpec::from_mrs(cfg.height_map);
        apply_height_map_inplace(adelic_mapped, hm);
    }
    rep.zero_comparison_adelic_only_mapped =
        compare_limits(adelic_mapped, gammas_ld, T, kappa);

    rep.zero_comparison = cfg.height_map.present ? rep.zero_comparison_mapped : rep.zero_comparison_raw;

    const Real rmse_ref =
        cfg.height_map.present ? rep.zero_comparison_mapped.rmse : rep.zero_comparison_raw.rmse;
    if (rmse_ref < 0.5L && rep.zero_comparison_mapped.max_gap < 0.5L)
        rep.verdict = "ADELIC_LIMIT_MATCHES_ZEROS";
    else if (rmse_ref > 5.0L)
        rep.verdict = "ADELIC_LIMIT_MISMATCH";
    else
        rep.verdict = "INCONCLUSIVE";

    rep.notes = "Phase 3 adelic-only limits; height map optional for mapped comparison.";

    std::cout << "=== Completion / adelic Cauchy validation ===\n";
    std::cout << "  clusters=" << rep.coincidence_clusters.size()
              << "  cauchy_candidates=" << rep.cauchy_candidates.size()
              << "  adelic_limits_only=" << rep.adelic_limits_only.size()
              << "  raw_ladder=" << (rep.raw_ladder_included ? "true" : "false") << "\n";
    std::cout << std::setprecision(6);
    std::cout << "  rmse_raw=" << static_cast<double>(rep.zero_comparison_raw.rmse)
              << "  rmse_mapped=" << static_cast<double>(rep.zero_comparison_mapped.rmse)
              << "  sinc2_gap=" << static_cast<double>(rep.zero_comparison_mapped.sinc2_gap)
              << "  verdict=" << rep.verdict << "\n";
    return rep;
}

bool export_completion_validation_json(const std::string& path,
                                       const CompletionValidationReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    auto write_cmp = [&](const char* name, const CompletionZeroComparison& zc) {
        out << "  \"" << name << "\": {\n";
        out << "    \"rmse\": " << static_cast<double>(zc.rmse) << ",\n";
        out << "    \"max_gap\": " << static_cast<double>(zc.max_gap) << ",\n";
        out << "    \"mean_gap\": " << static_cast<double>(zc.mean_gap) << ",\n";
        out << "    \"sinc2_gap\": " << static_cast<double>(zc.sinc2_gap) << ",\n";
        out << "    \"n_matched\": " << zc.n_matched << "\n  }";
    };
    out << "{\n  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n  \"notes\": \"" << r.notes << "\",\n";
    out << "  \"kmax\": " << r.kmax << ",\n";
    out << "  \"adelic_epsilon\": " << static_cast<double>(r.adelic_epsilon) << ",\n";
    out << "  \"adelic_limits_only_count\": " << r.adelic_limits_only_count << ",\n";
    out << "  \"raw_ladder_included\": " << (r.raw_ladder_included ? "true" : "false") << ",\n";
    write_cmp("zero_comparison", r.zero_comparison);
    out << ",\n";
    write_cmp("zero_comparison_raw", r.zero_comparison_raw);
    out << ",\n";
    write_cmp("zero_comparison_mapped", r.zero_comparison_mapped);
    out << ",\n";
    write_cmp("zero_comparison_adelic_only_raw", r.zero_comparison_adelic_only_raw);
    out << ",\n";
    write_cmp("zero_comparison_adelic_only_mapped", r.zero_comparison_adelic_only_mapped);
    out << ",\n  \"prime_ladder\": [";
    for (size_t i = 0; i < r.prime_ladder.size(); ++i) {
        if (i) out << ", ";
        out << r.prime_ladder[i];
    }
    out << "],\n  \"epsilon_ladder\": [";
    for (size_t i = 0; i < r.epsilon_ladder.size(); ++i) {
        if (i) out << ", ";
        out << static_cast<double>(r.epsilon_ladder[i]);
    }
    out << "],\n  \"coincidence_clusters\": [\n";
    for (size_t i = 0; i < r.coincidence_clusters.size(); ++i) {
        const auto& c = r.coincidence_clusters[i];
        out << "    { \"center\": " << static_cast<double>(c.center) << ", \"width\": "
            << static_cast<double>(c.width) << ", \"count\": " << c.count << ", \"n_primes\": "
            << c.n_primes << ", \"epsilon\": " << static_cast<double>(c.epsilon) << " }";
        if (i + 1 < r.coincidence_clusters.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"cauchy_candidates\": [\n";
    for (size_t i = 0; i < r.cauchy_candidates.size(); ++i) {
        const auto& c = r.cauchy_candidates[i];
        out << "    { \"frequency\": " << static_cast<double>(c.frequency)
            << ", \"stabilization_drift\": " << static_cast<double>(c.stabilization_drift)
            << ", \"n_primes\": " << c.n_primes << ", \"epsilon\": "
            << static_cast<double>(c.epsilon) << " }";
        if (i + 1 < r.cauchy_candidates.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"adelic_limits_only\": [\n";
    for (size_t i = 0; i < r.adelic_limits_only.size(); ++i) {
        const auto& a = r.adelic_limits_only[i];
        out << "    { \"frequency\": " << static_cast<double>(a.frequency) << ", \"delta_real\": "
            << static_cast<double>(a.delta_real) << ", \"eta_multiplicative\": "
            << static_cast<double>(a.eta_multiplicative) << ", \"p\": " << a.p << ", \"k\": "
            << a.k << ", \"q\": " << a.q << ", \"l\": " << a.l << " }";
        if (i + 1 < r.adelic_limits_only.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"adelic_limits\": [\n";
    for (size_t i = 0; i < r.adelic_limits.size(); ++i) {
        const auto& a = r.adelic_limits[i];
        out << "    { \"frequency\": " << static_cast<double>(a.frequency) << ", \"delta_real\": "
            << static_cast<double>(a.delta_real) << ", \"eta_multiplicative\": "
            << static_cast<double>(a.eta_multiplicative) << ", \"p\": " << a.p << ", \"k\": "
            << a.k << ", \"q\": " << a.q << ", \"l\": " << a.l << " }";
        if (i + 1 < r.adelic_limits.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"per_zero_pairing\": [\n";
    for (size_t i = 0; i < r.per_zero_pairing.size(); ++i) {
        const auto& p = r.per_zero_pairing[i];
        out << "    { \"gamma\": " << static_cast<double>(p.gamma) << ", \"frequency\": "
            << static_cast<double>(p.frequency) << ", \"gap\": " << static_cast<double>(p.gap)
            << ", \"p\": " << p.p << ", \"k\": " << p.k << ", \"q\": " << p.q << ", \"l\": "
            << p.l << ", \"adelic_admissible\": " << (p.adelic_admissible ? "true" : "false")
            << " }";
        if (i + 1 < r.per_zero_pairing.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

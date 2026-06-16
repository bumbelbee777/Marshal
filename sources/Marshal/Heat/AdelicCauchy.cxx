#include "AdelicCauchy.hxx"

#include <algorithm>
#include <cmath>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Heat {
namespace {

Real adelic_score(Real delta, Real eta, AnaVM::AdelicMetric metric) {
    if (metric == AnaVM::AdelicMetric::Adelic) return eta;
    if (metric == AnaVM::AdelicMetric::Mixed) return std::max(delta, eta);
    return delta;
}

bool mode_has_adelic_partner(Real log_p, int p, const std::vector<int>& primes,
                             const std::vector<Real>& log_lut, int max_den,
                             AnaVM::AdelicMetric metric, Real eps_adelic, FareyCache& cache,
                             int& out_q, int& out_l) {
    for (size_t qi = 0; qi < primes.size(); ++qi) {
        const int q = primes[qi];
        if (q == p) continue;
        const Real log_q = log_lut[qi];
        const auto fr = cache.best_ratio(log_p / log_q, max_den);
        if (fr.first < 1) continue;
        const Real delta =
            std::fabs(static_cast<Real>(fr.first) * log_p - static_cast<Real>(fr.second) * log_q);
        const Real eta =
            std::fabs(std::exp(static_cast<Real>(fr.first) * log_p -
                               static_cast<Real>(fr.second) * log_q) -
                      1.0L);
        if (adelic_score(delta, eta, metric) <= eps_adelic) {
            out_q = q;
            out_l = fr.second;
            return true;
        }
    }
    return false;
}

}  // namespace

std::pair<int, int> FareyCache::best_ratio(Real target_ratio, int max_den) {
    const long long key = static_cast<long long>(std::llround(target_ratio * 1e12)) ^ max_den;
    const auto it = memo.find(key);
    if (it != memo.end()) return it->second;
    int best_k = 0;
    int best_l = 0;
    Real best_delta = 1e300L;
    for (int l = 1; l <= max_den; ++l) {
        const int k = static_cast<int>(std::lround(target_ratio * l));
        if (k < 1 || k > max_den) continue;
        const Real approx = static_cast<Real>(k) / static_cast<Real>(l);
        const Real d = std::fabs(approx - target_ratio);
        if (d < best_delta) {
            best_delta = d;
            best_k = k;
            best_l = l;
        }
    }
    memo[key] = {best_k, best_l};
    return {best_k, best_l};
}

std::vector<AdelicModeIndex> build_adelic_mode_index(const std::vector<int>& primes, int kmax,
                                                     int max_den, AnaVM::AdelicMetric metric,
                                                     Real eps_adelic, FareyCache* cache) {
    FareyCache local;
    if (!cache) cache = &local;
    std::vector<Real> log_lut;
    log_lut.reserve(primes.size());
    for (int p : primes) log_lut.push_back(std::log(static_cast<Real>(p)));

    std::vector<AdelicModeIndex> modes;
    modes.reserve(primes.size() * static_cast<size_t>(kmax));
    for (size_t pi = 0; pi < primes.size(); ++pi) {
        const int p = primes[pi];
        const Real log_p = log_lut[pi];
        for (int k = 1; k <= kmax; ++k) {
            AdelicModeIndex m;
            m.p = p;
            m.k = k;
            m.omega = static_cast<Real>(k) * log_p;
            m.adelic_admissible =
                mode_has_adelic_partner(log_p, p, primes, log_lut, max_den, metric, eps_adelic,
                                        *cache, m.partner_q, m.partner_l);
            modes.push_back(m);
        }
    }
    std::sort(modes.begin(), modes.end(),
              [](const AdelicModeIndex& a, const AdelicModeIndex& b) { return a.omega < b.omega; });
    return modes;
}

std::vector<AdelicLimitPoint> adelic_pair_search(const std::vector<int>& primes, int kmax,
                                                 int max_den, AnaVM::AdelicMetric metric,
                                                 Real eps_adelic, bool include_raw_ladder,
                                                 FareyCache* cache) {
    FareyCache local;
    if (!cache) cache = &local;
    std::vector<AdelicLimitPoint> out;
    const int n = static_cast<int>(primes.size());
    std::vector<Real> log_lut(n);
    for (int i = 0; i < n; ++i) log_lut[static_cast<size_t>(i)] = std::log(static_cast<Real>(primes[static_cast<size_t>(i)]));

#ifdef _OPENMP
#pragma omp parallel
    {
        FareyCache thread_cache;
        std::vector<AdelicLimitPoint> local_out;
#pragma omp for schedule(dynamic, 4)
        for (int pi = 0; pi < n; ++pi) {
            const int p = primes[static_cast<size_t>(pi)];
            const Real log_p = log_lut[static_cast<size_t>(pi)];
            for (int qi = pi + 1; qi < n; ++qi) {
                const int q = primes[static_cast<size_t>(qi)];
                const Real log_q = log_lut[static_cast<size_t>(qi)];
                const auto fr = thread_cache.best_ratio(log_p / log_q, max_den);
                if (fr.first < 1 || fr.second < 1) continue;
                const Real delta =
                    std::fabs(static_cast<Real>(fr.first) * log_p - static_cast<Real>(fr.second) * log_q);
                const Real eta = std::fabs(
                    std::exp(static_cast<Real>(fr.first) * log_p - static_cast<Real>(fr.second) * log_q) -
                    1.0L);
                const Real score = adelic_score(delta, eta, metric);
                if (score > eps_adelic) continue;
                AdelicLimitPoint pt;
                pt.frequency = static_cast<Real>(fr.first) * log_p;
                pt.delta_real = delta;
                pt.eta_multiplicative = eta;
                pt.p = p;
                pt.k = fr.first;
                pt.q = q;
                pt.l = fr.second;
                local_out.push_back(pt);
            }
        }
#pragma omp critical
        { out.insert(out.end(), local_out.begin(), local_out.end()); }
    }
#else
    for (int pi = 0; pi < n; ++pi) {
        const int p = primes[static_cast<size_t>(pi)];
        const Real log_p = log_lut[static_cast<size_t>(pi)];
        for (int qi = pi + 1; qi < n; ++qi) {
            const int q = primes[static_cast<size_t>(qi)];
            const Real log_q = log_lut[static_cast<size_t>(qi)];
            const auto fr = cache->best_ratio(log_p / log_q, max_den);
            if (fr.first < 1 || fr.second < 1) continue;
            const Real delta =
                std::fabs(static_cast<Real>(fr.first) * log_p - static_cast<Real>(fr.second) * log_q);
            const Real eta = std::fabs(
                std::exp(static_cast<Real>(fr.first) * log_p - static_cast<Real>(fr.second) * log_q) -
                1.0L);
            const Real score = adelic_score(delta, eta, metric);
            if (score > eps_adelic) continue;
            AdelicLimitPoint pt;
            pt.frequency = static_cast<Real>(fr.first) * log_p;
            pt.delta_real = delta;
            pt.eta_multiplicative = eta;
            pt.p = p;
            pt.k = fr.first;
            pt.q = q;
            pt.l = fr.second;
            out.push_back(pt);
        }
    }
#endif

    if (include_raw_ladder) {
        for (size_t pi = 0; pi < primes.size(); ++pi) {
            const Real log_p = log_lut[pi];
            const int p = primes[pi];
            for (int k = 1; k <= kmax; ++k) {
                AdelicLimitPoint pt;
                pt.frequency = static_cast<Real>(k) * log_p;
                pt.p = p;
                pt.k = k;
                out.push_back(pt);
            }
        }
    }

    std::sort(out.begin(), out.end(),
              [](const AdelicLimitPoint& a, const AdelicLimitPoint& b) {
                  return a.frequency < b.frequency;
              });
    std::vector<AdelicLimitPoint> dedup;
    for (const auto& pt : out) {
        if (dedup.empty() || std::fabs(pt.frequency - dedup.back().frequency) > 1e-4L)
            dedup.push_back(pt);
    }
    return dedup;
}

std::vector<PerZeroPairing> per_zero_adelic_pairing(const std::vector<int>& primes, int kmax,
                                                    int max_den, AnaVM::AdelicMetric metric,
                                                    Real eps_adelic,
                                                    const std::vector<Real>& gammas, int target_n,
                                                    FareyCache* cache) {
    const auto modes = build_adelic_mode_index(primes, kmax, max_den, metric, eps_adelic, cache);
    std::vector<PerZeroPairing> out;
    const int nz = static_cast<int>(std::min(gammas.size(), static_cast<size_t>(target_n)));
    out.reserve(static_cast<size_t>(nz));

#ifdef _OPENMP
#pragma omp parallel
    {
        std::vector<PerZeroPairing> local;
#pragma omp for schedule(static)
        for (int ni = 0; ni < nz; ++ni) {
            const Real gamma = gammas[static_cast<size_t>(ni)];
            PerZeroPairing best;
            best.gamma = gamma;
            best.gap = 1e300L;
            for (const auto& m : modes) {
                const Real gap = std::fabs(m.omega - gamma);
                if (gap < best.gap) {
                    best.frequency = m.omega;
                    best.gap = gap;
                    best.p = m.p;
                    best.k = m.k;
                    best.q = m.partner_q;
                    best.l = m.partner_l;
                    best.adelic_admissible = m.adelic_admissible;
                }
            }
            local.push_back(best);
        }
#pragma omp critical
        { out.insert(out.end(), local.begin(), local.end()); }
    }
    std::sort(out.begin(), out.end(),
              [](const PerZeroPairing& a, const PerZeroPairing& b) { return a.gamma < b.gamma; });
#else
    for (int ni = 0; ni < nz; ++ni) {
        const Real gamma = gammas[static_cast<size_t>(ni)];
        PerZeroPairing best;
        best.gamma = gamma;
        best.gap = 1e300L;
        for (const auto& m : modes) {
            const Real gap = std::fabs(m.omega - gamma);
            if (gap < best.gap) {
                best.frequency = m.omega;
                best.gap = gap;
                best.p = m.p;
                best.k = m.k;
                best.q = m.partner_q;
                best.l = m.partner_l;
                best.adelic_admissible = m.adelic_admissible;
            }
        }
        out.push_back(best);
    }
#endif
    return out;
}

std::vector<Real> dedupe_frequencies(std::vector<Real> freqs, Real tol) {
    std::sort(freqs.begin(), freqs.end());
    if (freqs.empty()) return freqs;
    std::vector<Real> dedup;
    dedup.push_back(freqs.front());
    for (size_t i = 1; i < freqs.size(); ++i) {
        if (std::fabs(freqs[i] - dedup.back()) > tol) dedup.push_back(freqs[i]);
    }
    return dedup;
}

}  // namespace Marshal::Heat

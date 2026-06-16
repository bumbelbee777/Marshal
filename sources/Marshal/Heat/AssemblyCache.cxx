#include "AssemblyCache.hxx"

#include <algorithm>

namespace Marshal::Heat {

void AssemblyCache::build(const std::vector<int>& all_primes, int cut, int km,
                          const HeightMapSpec& hm) {
    kmax = km;
    prime_cut = std::min(cut, static_cast<int>(all_primes.size()));
    primes.assign(all_primes.begin(), all_primes.begin() + prime_cut);
    height_spec = hm;
    modes.clear();
    modes.reserve(primes.size() * static_cast<size_t>(kmax));
    for (int p : primes) {
        const LogPrimeOperator op = LogPrimeOperator::from_prime(p);
        for (int k = 1; k <= kmax; ++k) modes.push_back({p, k, op.eigenvalue(k)});
    }
    std::sort(modes.begin(), modes.end(),
              [](const ModeIndexEntry& a, const ModeIndexEntry& b) { return a.omega < b.omega; });
    height_lut = Marshal::Heat::height_lut(primes, kmax, hm);
    farey.memo.clear();
}

std::vector<Real> AssemblyCache::mapped_limits(const AnaVM::AdelicMetric metric, Real eps,
                                               bool include_raw_ladder) const {
    const int max_den = 1000;
    const auto adelic =
        adelic_pair_search(primes, kmax, max_den, metric, eps, include_raw_ladder,
                           const_cast<FareyCache*>(&farey));
    std::vector<Real> freqs;
    freqs.reserve(adelic.size());
    for (const auto& pt : adelic) freqs.push_back(apply_height_map(pt.frequency, height_spec));
    return dedupe_frequencies(std::move(freqs));
}

}  // namespace Marshal::Heat

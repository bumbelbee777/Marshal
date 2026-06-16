#pragma once

#include <unordered_map>
#include <vector>

#include "AnaVM/MrsTypes.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct AdelicLimitPoint {
    Real frequency = 0;
    Real delta_real = 0;
    Real eta_multiplicative = 0;
    int p = 0;
    int k = 0;
    int q = 0;
    int l = 0;
};

struct PerZeroPairing {
    Real gamma = 0;
    Real frequency = 0;
    Real gap = 0;
    int p = 0;
    int k = 0;
    int q = 0;
    int l = 0;
    bool adelic_admissible = false;
};

struct AdelicCauchyResult {
    std::vector<AdelicLimitPoint> adelic_limits_only;
    std::vector<PerZeroPairing> per_zero_pairing;
    bool raw_ladder_included = false;
};

struct FareyCache {
    std::unordered_map<long long, std::pair<int, int>> memo;

    std::pair<int, int> best_ratio(Real target_ratio, int max_den);
};

std::vector<AdelicLimitPoint> adelic_pair_search(const std::vector<int>& primes, int kmax,
                                                 int max_den, AnaVM::AdelicMetric metric,
                                                 Real eps_adelic, bool include_raw_ladder,
                                                 FareyCache* cache = nullptr);

struct AdelicModeIndex {
    int p = 0;
    int k = 0;
    Real omega = 0;
    bool adelic_admissible = false;
    int partner_q = 0;
    int partner_l = 0;
};

std::vector<AdelicModeIndex> build_adelic_mode_index(const std::vector<int>& primes, int kmax,
                                                     int max_den, AnaVM::AdelicMetric metric,
                                                     Real eps_adelic, FareyCache* cache = nullptr);

std::vector<PerZeroPairing> per_zero_adelic_pairing(const std::vector<int>& primes, int kmax,
                                                    int max_den, AnaVM::AdelicMetric metric,
                                                    Real eps_adelic,
                                                    const std::vector<Real>& gammas, int target_n,
                                                    FareyCache* cache = nullptr);

std::vector<Real> dedupe_frequencies(std::vector<Real> freqs, Real tol = 1e-4L);

}  // namespace Marshal::Heat

#pragma once

#include <vector>

#include "AdelicCauchy.hxx"
#include "HeightMap.hxx"
#include "LogPrimeOperator.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct ModeIndexEntry {
    int p = 0;
    int k = 0;
    Real omega = 0;
};

struct AssemblyCache {
    int prime_cut = 0;
    int kmax = 0;
    std::vector<int> primes;
    std::vector<ModeIndexEntry> modes;
    FareyCache farey;
    HeightMapSpec height_spec;
    std::vector<Real> height_lut;

    void build(const std::vector<int>& all_primes, int cut, int km, const HeightMapSpec& hm);

    std::vector<Real> mapped_limits(const AnaVM::AdelicMetric metric, Real eps,
                                    bool include_raw_ladder) const;
};

}  // namespace Marshal::Heat

#pragma once

#include <string>
#include <vector>

#include "AdelicCauchy.hxx"
#include "AnaVM/MrsTypes.hxx"
#include "Config.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct CoincidenceCluster {
    Real center = 0;
    Real width = 0;
    int count = 0;
    int n_primes = 0;
    Real epsilon = 0;
};

struct CauchyCandidate {
    Real frequency = 0;
    Real stabilization_drift = 0;
    int n_primes = 0;
    Real epsilon = 0;
};

struct CompletionZeroComparison {
    Real rmse = 0;
    Real max_gap = 0;
    Real mean_gap = 0;
    Real sinc2_gap = 0;
    int n_matched = 0;
};

struct CompletionValidationReport {
    std::string program_id;
    AnaVM::MrsCompletion completion;
    AnaVM::MrsAdelicCauchy adelic_cauchy;
    std::vector<int> prime_ladder;
    std::vector<Real> epsilon_ladder;
    int kmax = 0;
    Real adelic_epsilon = 0;
    int adelic_limits_only_count = 0;
    std::vector<CoincidenceCluster> coincidence_clusters;
    std::vector<CauchyCandidate> cauchy_candidates;
    std::vector<AdelicLimitPoint> adelic_limits;
    std::vector<AdelicLimitPoint> adelic_limits_only;
    std::vector<PerZeroPairing> per_zero_pairing;
    bool raw_ladder_included = false;
    CompletionZeroComparison zero_comparison;
    CompletionZeroComparison zero_comparison_raw;
    CompletionZeroComparison zero_comparison_mapped;
    CompletionZeroComparison zero_comparison_adelic_only_raw;
    CompletionZeroComparison zero_comparison_adelic_only_mapped;
    std::string verdict;
    std::string notes;
};

CompletionValidationReport run_completion_validation(const Config& cfg,
                                                     const std::vector<Real>& gammas_ld,
                                                     const std::vector<int>& primes);

bool export_completion_validation_json(const std::string& path,
                                       const CompletionValidationReport& r);

}  // namespace Marshal::Heat

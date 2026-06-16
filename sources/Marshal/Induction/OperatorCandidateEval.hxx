#pragma once
#include <string>
#include <vector>
#include "AnaVM/OperatorTraits.hxx"
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

struct CandidateMetrics {
    Real gamma_free_gap_max = 0;
    Real gamma_free_gap_mean = 0;
    Real matched_sq_gap_max = 0;
    Real quotient_sq_gap_max = 0;
    Real fixed_quotient_gap_max = 0;
    Real compact_sinc2_residual = 0;
    bool compact_sinc2_mismatch_proved = false;
    bool weil_poisson_compatible = true;
    int n_spectrum_pairs = 0;
};

struct OperatorCandidateResult {
    std::string ansatz_id;
    std::string registry_status;
    std::string verdict;
    std::string elimination_reason;
    AnaVM::OperatorTraits traits;
    CandidateMetrics metrics;
    double plausibility_score = 0;
};

std::string elimination_reason_for(const AnaVM::OperatorTraits& traits);

std::vector<Real> CollectLogpRescaledSpectrum(const Heat::PrimeCatalog& cat, int N);

CandidateMetrics evaluate_candidate_metrics(const AnaVM::OperatorTraits& traits,
                                            const std::vector<double>& gammas,
                                            const std::vector<Real>& gammas_ld,
                                            Heat::PrimeCatalog& cat, const Config& cfg,
                                            int n_pairs);

double score_plausibility(const AnaVM::OperatorTraits& traits, const CandidateMetrics& m);

}  // namespace Marshal::Induction

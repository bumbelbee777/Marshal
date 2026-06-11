#pragma once
#include <vector>
#include "Cert/Verdict.hxx"
#include "Config.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/Support.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Heat/PrimeCatalog.hxx"

namespace Marshal::Analysis {

struct ResidualBudget {
    Real zero_tail_naive = 0;
    Real zero_tail_effective = 0;
    Real prime_tail_naive = 0;
    Real prime_tail_effective = 0;
    Real arch_floor = 0;
    Real arch_abs_floor = 0;
    Real float_floor = 0;
    size_t n_zeros_effective = 0;
    size_t n_primes_effective = 0;
};

struct ConvergenceResult {
    Real predicted_tail_bound = 0;
    Real observed_tail_residual = 0;
    bool tail_bound_holds = false;
    Numerics::TailBoundStatus tail_bound_status = Numerics::TailBoundStatus::Unproved;
    Cert::ProofStatus spectral_measure_proof_status = Cert::ProofStatus::Open;
    Cert::ResolventLimitStatus resolvent_limit_status = Cert::ResolventLimitStatus::Open;
    Real fitted_exponent = 0;
    Real fitted_intercept = 0;
    Real r_squared = 0;
    Real min_spectral_gap_sq = 0;
    bool spectral_measure_converges = false;
    bool eigenvalues_converge = false;
    bool riemann_hypothesis_holds = false;
    std::vector<int> cutoffs;
    std::vector<int> n_primes;
    std::vector<Real> sup_residuals;
    std::vector<Real> tail_bounds_at_cutoff;

    struct EigenvalueTrack {
        int n = 0;
        Real gamma = 0;
        Real gamma_sq = 0;
        Real lambda_sq = 0;
        Real error = 0;
        Real predicted_error = 0;
    };
    std::vector<EigenvalueTrack> eigenvalues;
};

void FitPowerLaw(const std::vector<Real>& p_vals, const std::vector<Real>& residuals,
                 Real& exponent, Real& intercept, Real& r_squared);
Real MinZeroGapSq(const std::vector<double>& gammas, int n);

ConvergenceResult RunConvergenceStudy(
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat, const Config& cfg, Real sigma_weil, Real proof_eps,
    const ResidualBudget& budget, bool trace_proved, Real observed_baseline = -1);

}  // namespace Marshal::Analysis

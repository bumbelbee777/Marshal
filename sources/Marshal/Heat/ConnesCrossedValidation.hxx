#pragma once

#include <string>
#include <vector>

#include "Config.hxx"
#include "ConnesCouplingMode.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct ConnesSpectrumMetrics {
    Real rmse = 0;
    Real max_gap = 0;
    Real mean_gap = 0;
    int n_modes = 0;
    int n_zeros_matched = 0;
};

struct ConnesCrossedPoint {
    int n_primes = 0;
    int p_max = 0;
    Real lambda = 0;
    int k_twist = 0;
    ConnesCouplingMode coupling = ConnesCouplingMode::LogLadder;
    ConnesSpectrumMetrics spectrum;
    Real sinc2_spectral_gap = 0;
    Real weil_prime_vs_zeros = 0;
};

struct ConnesCrossedValidationReport {
    ConnesCouplingMode coupling = ConnesCouplingMode::LogLadder;
    Real test_T = 0;
    Real test_kappa = 1;
    Real spectrum_rmse_target = 1.0L;
    Real spectrum_max_gap_target = 1.0L;
    Real best_rmse = 0;
    Real best_max_gap = 0;
    Real best_lambda = 0;
    int best_n_primes = 0;
    bool spectrum_identified = false;
    std::string verdict;
    std::vector<ConnesCrossedPoint> ladder;
    std::vector<Real> lambda_sweep;
};

ConnesCrossedValidationReport run_connes_crossed_validation(
    const Config& cfg, const std::vector<Real>& gammas_ld, const std::vector<int>& primes);

bool export_connes_crossed_validation_json(const std::string& path,
                                           const ConnesCrossedValidationReport& r);

}  // namespace Marshal::Heat

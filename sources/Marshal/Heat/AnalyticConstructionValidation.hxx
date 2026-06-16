#pragma once

#include "Config.hxx"
#include "SelfAdjointExtensionSweep.hxx"
#include "SpectralDeterminant.hxx"
#include "TraceFormulaGate.hxx"

#include <string>

namespace Marshal::Heat {

struct AnalyticConstructionStep {
    std::string step;
    std::string verdict;
    bool pass = false;
};

struct AnalyticConstructionReport {
    std::string program_id;
    std::string rule_id;
    std::vector<AnalyticConstructionStep> steps;
    SelfAdjointExtensionSweepReport extension_sweep;
    TraceFormulaGateReport trace_gate;
    SpectralDeterminantReport spectral_det;
    Real spectrum_rmse = 0;
    Real spectrum_rmse_raw = 0;
    Real spectrum_max_gap = 0;
    Real best_theta = 0;
    Real log_prime_t1_gap = 0;
    Real xi_det_gap = 0;
    bool height_map_applied = false;
    bool continuous_spectrum_present = true;
    std::string analytic_shape_verdict;
    int prime_limit = 0;
    std::string overall_verdict;
};

AnalyticConstructionReport run_analytic_construction_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes);

bool export_analytic_construction_json(const std::string& path, const AnalyticConstructionReport& r);

}  // namespace Marshal::Heat

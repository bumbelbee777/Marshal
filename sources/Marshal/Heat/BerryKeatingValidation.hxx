#pragma once

#include "BerryKeatingOperator.hxx"
#include "Config.hxx"
#include "SelfAdjointExtensionSweep.hxx"

#include <string>

namespace Marshal::Heat {

struct BerryKeatingValidationReport {
    std::string program_id;
    SelfAdjointExtensionSweepReport extension_sweep;
    BerryKeatingSpectrumMetrics best_metrics;
    BerryKeatingSpectrumMetrics best_metrics_raw;
    Real best_theta = 0;
    bool height_map_applied = false;
    std::string verdict;
};

BerryKeatingValidationReport run_berry_keating_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes);

bool export_berry_keating_validation_json(const std::string& path,
                                          const BerryKeatingValidationReport& r);

}  // namespace Marshal::Heat

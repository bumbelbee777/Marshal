#pragma once

#include "AnaVM/MrsTypes.hxx"
#include "ArchimedeanBoundary.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "BerryKeatingOperator.hxx"
#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct ExtensionSweepRow {
    Real theta = 0;
    std::string boundary_name;
    AnaVM::ArchimedeanBoundary boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
    Real spectrum_rmse = 0;
    Real spectrum_max_gap = 0;
    Real laplace_weil_residual = 0;
    Real xi_det_gap = 0;
    Real score = 0;
    bool admissible = false;
};

struct SelfAdjointExtensionSweepReport {
    std::string program_id;
    std::string rule_id;
    int sweep_steps = 24;
    Real best_theta = 0;
    Real best_score = 0;
    Real best_rmse = 0;
    Real best_rmse_raw = 0;
    bool height_map_applied = false;
    std::vector<ExtensionSweepRow> rows;
    std::string verdict;
};

SelfAdjointExtensionSweepReport run_self_adjoint_extension_sweep(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes);

bool export_self_adjoint_extension_sweep_json(const std::string& path,
                                              const SelfAdjointExtensionSweepReport& r);

}  // namespace Marshal::Heat

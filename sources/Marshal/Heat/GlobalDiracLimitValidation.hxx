#pragma once

#include "AnaVM/MrsTypes.hxx"
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct GlobalDiracLimitPoint {
    int combined_cap = 0;
    int n_modes = 0;
    Real spectrum_rmse = 0;
    double elapsed_ms = 0;
};

struct GlobalDiracLimitReport {
    std::string program_id;
    std::string rule_id;
    std::string kind = "global_connes_dirac";
    std::string limit_target = "riemann_zero_heights";
    std::string metric = "spectrum_rmse";
    std::string formal_status = "open";
    std::string formal_lemma;
    std::string formal_approach;
    std::string proof_status = "FORMAL_LIMIT_OPEN";
    std::string limit_verdict;
    bool monotone_rmse_increase = false;
    bool lean_emit_ready = false;
    double total_elapsed_ms = 0;
    std::vector<GlobalDiracLimitPoint> points;
};

GlobalDiracLimitReport run_global_dirac_limit_validation(
    const Config& cfg, const std::vector<Real>& gammas_ld, const std::vector<int>& primes);

bool export_global_dirac_limit_json(const std::string& path, const GlobalDiracLimitReport& r);

}  // namespace Marshal::Heat

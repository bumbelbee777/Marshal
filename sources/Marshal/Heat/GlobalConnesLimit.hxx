#pragma once

#include "CombinedConnesDirac.hxx"
#include "Quotient/RootedCausalDAG.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct GlobalConnesLimitPoint {
    int combined_cap = 0;
    int mesh = 0;
    int n_primes = 0;
    Real spectrum_rmse = 0;
    Real resolvent_gap = 0;
    Real trace_extraction_rmse = 0;
    int n_modes = 0;
};

struct GlobalConnesLimitReport {
    std::string proof_status = "FORMAL_LIMIT_OPEN";
    std::string limit_verdict;
    std::string limit_target = "riemann_zero_heights";
    bool monotone_rmse_decrease = false;
    bool lean_emit_ready = false;
    Real final_spectrum_rmse = 0;
    Real final_resolvent_gap = 0;
    Real final_trace_extraction_rmse = 0;
    std::vector<GlobalConnesLimitPoint> points;
};

/// Full global limit ladder: DAG blend + crossed-product Dirac at increasing resolution.
GlobalConnesLimitReport run_global_connes_limit(const CombinedConnesDiracSpec& dirac_spec,
                                                const std::vector<int>& primes,
                                                const std::vector<Real>& gammas);

bool export_global_connes_limit_json(const std::string& path, const GlobalConnesLimitReport& r);

}  // namespace Marshal::Heat

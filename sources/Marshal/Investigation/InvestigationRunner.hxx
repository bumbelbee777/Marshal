#pragma once

#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "InvestigationTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::Investigation {

struct InvestigationRunResult {
    std::string investigation_id;
    std::string cert_root;
    bool ok = false;
    std::vector<std::pair<std::string, bool>> diagnostic_results;
};

InvestigationSpec spec_from_config(const Config& cfg);

InvestigationRunResult run_investigation(const Config& cfg,
                                         const std::vector<double>& gammas,
                                         const std::vector<Real>& gammas_ld,
                                         Marshal::Heat::PrimeCatalog& cat,
                                         const std::vector<int>& primes);

DiagnosticReport run_diagnostic(DiagnosticId id, const InvestigationSpec& spec,
                                const Config& cfg, const std::vector<double>& gammas,
                                const std::vector<Real>& gammas_ld, Heat::PrimeCatalog& cat,
                                const std::vector<int>& primes);

}  // namespace Marshal::Investigation

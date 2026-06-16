#pragma once

#include "InvestigationTypes.hxx"

#include <string>

namespace Marshal::Investigation {

bool export_diagnostic_json(const std::string& path, const DiagnosticReport& r);
bool export_investigation_manifest(const std::string& path,
                                   const std::string& investigation_id,
                                   const std::vector<std::pair<std::string, bool>>& entries);

}  // namespace Marshal::Investigation

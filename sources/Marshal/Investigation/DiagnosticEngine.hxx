#pragma once

#include "InvestigationRunner.hxx"
#include "InvestigationTypes.hxx"

#include <vector>

namespace Marshal::Investigation {

struct DiagnosticRoute {
    DiagnosticId id;
    const char* mrs_key;
    const char* export_flag;
    const char* default_subpath;
};

const std::vector<DiagnosticRoute>& diagnostic_routes();

}  // namespace Marshal::Investigation

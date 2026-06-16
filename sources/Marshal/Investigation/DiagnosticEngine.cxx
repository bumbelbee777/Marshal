#include "DiagnosticEngine.hxx"

namespace Marshal::Investigation {

const std::vector<DiagnosticRoute>& diagnostic_routes() {
    static const std::vector<DiagnosticRoute> kRoutes = {
        {DiagnosticId::SpectralActionCurvature, "spectral_action_curvature",
         "--diag spectral_action_curvature", "spectral_action_curvature.json"},
        {DiagnosticId::T1AdmissibleTopology, "t1_admissible_topology",
         "--diag t1_admissible_topology", "t1_admissible_topology.json"},
        {DiagnosticId::HeatTraceAtTheta, "heat_trace_at_theta", "--diag heat_trace_at_theta",
         "heat_trace_at_theta.json"},
        {DiagnosticId::SpectralSpacing, "spectral_spacing", "--diag spectral_spacing",
         "spectral_spacing.json"},
        {DiagnosticId::ContinuumPersistence, "continuum_persistence",
         "--diag continuum_persistence", "continuum_persistence.json"},
    };
    return kRoutes;
}

}  // namespace Marshal::Investigation

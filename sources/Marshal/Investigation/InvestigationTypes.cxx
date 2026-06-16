#include "InvestigationTypes.hxx"

#include <algorithm>
#include <cmath>

namespace Marshal::Investigation {
namespace {

constexpr Real kTwoPi = 6.28318530717958647692L;

}  // namespace

const char* diagnostic_id_string(DiagnosticId id) {
    switch (id) {
        case DiagnosticId::SpectralActionCurvature:
            return "spectral_action_curvature";
        case DiagnosticId::T1AdmissibleTopology:
            return "t1_admissible_topology";
        case DiagnosticId::HeatTraceAtTheta:
            return "heat_trace_at_theta";
        case DiagnosticId::SpectralSpacing:
            return "spectral_spacing";
        case DiagnosticId::ContinuumPersistence:
            return "continuum_persistence";
    }
    return "unknown";
}

std::vector<Real> build_theta_grid(const ThetaSweepSpec& spec) {
    const int n = std::max(1, spec.steps);
    std::vector<Real> grid;
    grid.reserve(static_cast<size_t>(n));

    if (spec.mode == ThetaSweepMode::ExplicitList && !spec.explicit_values.empty()) {
        return spec.explicit_values;
    }

    if (spec.mode == ThetaSweepMode::Centered) {
        const Real half = spec.range * 0.5L;
        const Real lo = spec.center - half;
        const Real hi = spec.center + half;
        if (n == 1) {
            grid.push_back(spec.center);
            return grid;
        }
        const Real step = (hi - lo) / static_cast<Real>(n - 1);
        for (int i = 0; i < n; ++i) grid.push_back(lo + step * static_cast<Real>(i));
        return grid;
    }

    const Real lo = (spec.mode == ThetaSweepMode::Uniform && spec.max_theta > spec.min_theta)
                        ? spec.min_theta
                        : 0;
    const Real hi = (spec.mode == ThetaSweepMode::Uniform && spec.max_theta > spec.min_theta)
                        ? spec.max_theta
                        : kTwoPi;
    if (n == 1) {
        grid.push_back(lo);
        return grid;
    }
    const Real step = (hi - lo) / static_cast<Real>(n - 1);
    for (int i = 0; i < n; ++i) grid.push_back(lo + step * static_cast<Real>(i));
    return grid;
}

void apply_quick_preset(InvestigationSpec& spec) {
    if (spec.curvature_sweep.steps > 40) spec.curvature_sweep.steps = 40;
    if (spec.topology_sweep.steps > 80) spec.topology_sweep.steps = 80;
    if (spec.prime_limit_ladder.values.size() > 4) {
        spec.prime_limit_ladder.values = {10, 100, 1000, 10000};
    }
    if (spec.heat_t_ladder.values.size() > 3) {
        spec.heat_t_ladder.values = {0.01L, 0.1L, 1.0L};
    }
}

}  // namespace Marshal::Investigation

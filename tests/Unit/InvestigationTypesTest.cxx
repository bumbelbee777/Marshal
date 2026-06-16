#include "Investigation/InvestigationTypes.hxx"

#include <cmath>
#include <iostream>

using Marshal::Investigation::InvestigationSpec;
using Marshal::Investigation::ThetaSweepMode;
using Marshal::Investigation::ThetaSweepSpec;
using Marshal::Investigation::apply_quick_preset;
using Marshal::Investigation::build_theta_grid;

int main() {
    int fails = 0;
    auto fail = [&](const char* msg) {
        std::cerr << "FAIL: " << msg << "\n";
        ++fails;
    };

    ThetaSweepSpec centered;
    centered.mode = ThetaSweepMode::Centered;
    centered.center = 5.76L;
    centered.range = 1.0L;
    centered.steps = 5;
    const auto grid = build_theta_grid(centered);
    if (grid.size() != 5) fail("centered grid size");
    if (std::fabs(static_cast<double>(grid.front() - 5.26L)) > 1e-9) fail("centered grid lo");
    if (std::fabs(static_cast<double>(grid.back() - 6.26L)) > 1e-9) fail("centered grid hi");

    InvestigationSpec spec;
    spec.curvature_sweep.steps = 200;
    spec.topology_sweep.steps = 400;
    spec.prime_limit_ladder.values = {10, 50, 100, 500, 1000};
    apply_quick_preset(spec);
    if (spec.curvature_sweep.steps != 40) fail("quick curvature steps");
    if (spec.prime_limit_ladder.values.size() != 4) fail("quick prime ladder");

    return fails == 0 ? 0 : 1;
}

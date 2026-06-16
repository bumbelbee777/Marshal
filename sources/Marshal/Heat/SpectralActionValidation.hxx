#pragma once

#include "AnaVM/MrsTypes.hxx"
#include "ArchimedeanBoundary.hxx"
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct SpectralActionGateRow {
    std::string id;
    std::string gate_class;
    bool pass = false;
    std::string note;
};

struct SpectralActionCandidateRow {
    Real theta = 0;
    std::string boundary_name;
    AnaVM::ArchimedeanBoundary boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
    Real combined_spectral_action = 0;
    Real heat_action_proxy = 0;
    int n_combined_modes = 0;
    Real t1_gap = 0;
    Real laplace_weil_residual = 0;
    Real spectrum_rmse = 0;
    Real gue_spacing_l2 = 0;
    bool admissible = false;
    bool selected = false;
};

struct SpectralActionConvergenceRow {
    int combined_cap = 0;
    int n_modes = 0;
    Real spectrum_rmse = 0;
    Real combined_action = 0;
    double elapsed_ms = 0;
};

struct SpectralActionReport {
    std::string program_id;
    std::string rule_id;
    std::string selection_rule;
    std::string action_proxy = "combined_crossed_product";
    std::string proof_status = "EXPERIMENTAL_NOT_PROVED";
    bool lean_emit_ready = false;
    Real selected_theta = 0;
    std::string selected_boundary;
    Real selected_heat_action = 0;
    Real selected_t1_gap = 0;
    Real second_best_action = 0;
    Real action_gap = 0;
    bool strict_minimum = false;
    bool unique_minimum = false;
    int minimizer_count_at_minimum = 0;
    Real selected_spectrum_rmse = 0;
    Real selected_gue_spacing_l2 = 0;
    Real zeros_gue_spacing_l2 = 0;
    Real cylinder_gue_spacing_l2 = 0;
    int combined_cap = 400;
    int n_combined_modes = 0;
    double sweep_elapsed_ms = 0;
    std::string simd_backend = "scalar";
    std::vector<SpectralActionConvergenceRow> convergence_probe;
    int admissible_count = 0;
    std::string verdict;
    std::vector<SpectralActionGateRow> gates;
    std::vector<SpectralActionCandidateRow> candidates;
};

SpectralActionReport run_spectral_action_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes);

bool export_spectral_action_validation_json(const std::string& path, const SpectralActionReport& r);

}  // namespace Marshal::Heat

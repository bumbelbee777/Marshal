#pragma once

#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Investigation {

enum class ThetaSweepMode { Uniform, Centered, ExplicitList };

struct ThetaSweepSpec {
    ThetaSweepMode mode = ThetaSweepMode::Uniform;
    Real center = 0;
    Real range = 1;
    int steps = 24;
    Real min_theta = 0;
    Real max_theta = 0;
    std::vector<Real> explicit_values;
};

struct LadderSpec {
    std::string param;
    std::vector<Real> values;
};

struct InvestigationSpec {
    std::string id = "theorem_ab";
    std::string cert_root = "build/cert/investigations/theorem_ab";
    bool quick = false;
    Real fixed_theta = 5.76L;
    std::string fixed_boundary = "periodic";
    Real t1_tolerance = 1e-6L;
    Real heat_scale_base = 1.0L;
    int heat_scales = 3;
    Real coupling_lambda = 0.5L;
    int combined_cap = 400;
    int arch_cap = 80;
    int kmax = 12;
    int sweep_steps = 24;

    ThetaSweepSpec curvature_sweep;
    ThetaSweepSpec topology_sweep;
    LadderSpec heat_t_ladder;
    LadderSpec prime_limit_ladder;

    bool run_curvature = true;
    bool run_topology = true;
    bool run_heat_trace = true;
    bool run_spacing = true;
    bool run_continuum = true;
};

enum class DiagnosticId {
    SpectralActionCurvature,
    T1AdmissibleTopology,
    HeatTraceAtTheta,
    SpectralSpacing,
    ContinuumPersistence,
};

const char* diagnostic_id_string(DiagnosticId id);

struct DiagnosticGate {
    std::string id;
    std::string gate_class;
    bool pass = false;
    std::string note;
};

struct DiagnosticSeriesPoint {
    Real x = 0;
    Real y = 0;
    std::string label;
    bool flag = false;
    Real aux = 0;
};

struct DiagnosticReport {
    int version = 1;
    std::string investigation_id;
    std::string diagnostic_id;
    std::string proof_status = "NUMERICAL";
    std::string analysis_status = "ANALYSIS_INCOMPLETE";
    std::string epistemic_note =
        "Numeric investigation evidence only; not analytic proof of Theorems A/B.";
    std::string analytic_obligation;
    Real fixed_theta = 0;
    int prime_limit = 0;
    std::vector<DiagnosticGate> gates;
    std::vector<DiagnosticSeriesPoint> series;
    std::string note;
};

std::vector<Real> build_theta_grid(const ThetaSweepSpec& spec);

void apply_quick_preset(InvestigationSpec& spec);

}  // namespace Marshal::Investigation

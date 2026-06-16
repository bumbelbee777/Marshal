#pragma once

#include <string>
#include <vector>

#include "AnaVM/MrsTypes.hxx"
#include "Config.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct AssemblyPoint {
    Real height_a = 1.0L;
    Real height_b = 0.0L;
    Real connes_lambda = 0.5L;
    AnaVM::AdelicMetric adelic_metric = AnaVM::AdelicMetric::Mixed;
    AnaVM::ArchimedeanBoundary arch_boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
    AnaVM::CompletionMethod completion_method = AnaVM::CompletionMethod::Cauchy;
};

struct AssemblyPointResult {
    AssemblyPoint point;
    Real score = 0;
    Real max_weil = 0;
    Real rmse_mapped = 0;
    Real rmse_raw = 0;
    Real sinc2_gap = 0;
    Real xi_det_gap = 0;
    std::vector<Real> weil_residuals;
};

struct AssemblySearchReport {
    std::string program_id;
    std::string tier;
    int grid_points = 0;
    std::vector<AssemblyPointResult> ranked;
    std::string verdict;
};

bool parse_assembly_point_file(const std::string& path, AssemblyPoint& out);

AssemblyPointResult evaluate_assembly_point(const Config& cfg, const AssemblyPoint& pt,
                                            const std::vector<double>& gammas,
                                            const std::vector<Real>& gammas_ld,
                                            const std::vector<int>& primes, int zero_cap,
                                            int prime_cap);

AssemblySearchReport run_assembly_search_validation(const Config& cfg,
                                                    const std::vector<double>& gammas,
                                                    const std::vector<Real>& gammas_ld,
                                                    const std::vector<int>& primes);

bool export_assembly_search_json(const std::string& path, const AssemblySearchReport& r);

}  // namespace Marshal::Heat

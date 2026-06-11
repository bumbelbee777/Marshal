#pragma once
#include "FormalCalibration.hxx"
#include "MrsTypes.hxx"
#include "OperatorTraits.hxx"
#include "Numerics/Real.hxx"
#include <string>
#include <vector>

namespace Marshal::Analysis {
struct PairCorrelationReport;
}

namespace Marshal::AnaVM {

struct CountingAnalytics {
    Real window_t = 100;
    Real chebyshev_theta = 0;
    Real predicted_cylinder_count = 0;
    int riemann_count = 0;
    bool counting_diverges = true;
};

struct FormalAnalyticsResult {
    std::string ansatz_id;
    OperatorTraits traits;
    CountingAnalytics counting;
    bool density_slopes_incompatible = true;
    bool pair_correlation_separates = false;
    Real gue_spacing_l2_cylinder = 0;
    Real gue_spacing_l2_zero = 0;
    Real montgomery_r2_l2 = 0;
    bool cylinder_class_excluded = false;
    bool lean_emit_ready = false;
    std::vector<std::string> proved_gates;
    std::vector<std::string> diagnostic_gates;
    std::vector<std::string> failed_gates;
};

FormalAnalyticsResult run_formal_analytics(const MrsProgram& prog, const OperatorTraits& traits,
                                           const CountingAnalytics& counting,
                                           const Analysis::PairCorrelationReport* pair);

void export_formal_analytics_json(const std::string& path, const FormalAnalyticsResult& r,
                                  const FormalCalibration& cal);

CountingAnalytics compute_counting_analytics(const std::vector<int>& primes, Real T,
                                             int riemann_count_in_window);

}  // namespace Marshal::AnaVM

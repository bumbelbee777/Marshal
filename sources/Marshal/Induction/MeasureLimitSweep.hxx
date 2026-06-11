#pragma once
#include <string>
#include <vector>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Induction {

struct MeasureLimitPoint {
    int prime_limit = 0;
    Real sinc2_residual = 0;
    bool mismatch_proved = false;
};

struct MeasureLimitSweepResult {
    std::vector<MeasureLimitPoint> points;
    bool residual_stable = false;
    Real max_deviation = 0;
    Real reference_residual = 0;
};

MeasureLimitSweepResult RunMeasureLimitSweep(const Config& cfg, const TestFunction& tf,
                                             const std::vector<double>& gammas,
                                             const std::vector<Real>& gammas_ld,
                                             Heat::PrimeCatalog& cat,
                                             const std::vector<int>& primes);

void ExportMeasureLimitJson(const std::string& path, const MeasureLimitSweepResult& r);

}  // namespace Marshal::Induction

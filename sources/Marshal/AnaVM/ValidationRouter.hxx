#pragma once

#include "Config.hxx"
#include "MrsTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

enum class ValidationKind {
    LogPrime,
    ConnesCrossed,
    Completion,
    SpectralDeterminant,
    ArchimedeanSweep,
    AssemblySearch,
    SelfAdjointExtensionSweep,
    TraceFormulaGate,
    BerryKeatingValidation,
    AnalyticConstruction,
    SpectralActionSelection,
    GlobalDiracLimit,
    AnalyticLemmaDemo,
    PairCorrelation,
    FormalAnalytics,
    MarshalWedgeAnalytic,
    XiHadamardProof,
    Investigation,
};

struct ValidationJob {
    ValidationKind kind;
    std::string diagnostic_id;
    std::string export_flag;
    std::string default_path;
};

std::vector<ValidationJob> route_validation(const MrsProgram& prog);
void apply_validation_jobs(Config& cfg, const std::vector<ValidationJob>& jobs);

}  // namespace Marshal::AnaVM

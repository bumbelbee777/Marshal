#pragma once
#include <string>
#include "../Numerics/Support.hxx"

namespace Marshal::Cert {

enum class MarshalVerdict {
    AnalysisIncomplete,
    NumericsPass,
    DiagnosticSuggestive,
    AnalysisComplete
};

enum class ProofStatus { Proved, Numerical, Open, Disproved, Falsified, Impossible };

enum class ResolventLimitStatus { Open, Defined, Proved };

enum class VerdictPriority : int {
    Invalid = 0,
    SpectralMismatch = 1,
    Inconclusive = 2,
    NumericsPass = 3,
    ControlledTrace = 4,
};

struct HpVerdictResult {
    std::string label;
    VerdictPriority priority = VerdictPriority::Inconclusive;
};

inline const char* VerdictPriorityString(VerdictPriority p) {
    switch (p) {
        case VerdictPriority::Invalid: return "INVALID";
        case VerdictPriority::SpectralMismatch: return "SPECTRAL_MISMATCH";
        case VerdictPriority::NumericsPass: return "NUMERICS_PASS";
        case VerdictPriority::ControlledTrace: return "CONTROLLED_TRACE";
        default: return "INCONCLUSIVE";
    }
}

inline const char* VerdictString(MarshalVerdict v) {
    switch (v) {
        case MarshalVerdict::NumericsPass: return "NUMERICS_PASS";
        case MarshalVerdict::DiagnosticSuggestive: return "DIAGNOSTIC_SUGGESTIVE";
        case MarshalVerdict::AnalysisComplete: return "ANALYSIS_COMPLETE";
        default: return "ANALYSIS_INCOMPLETE";
    }
}

inline const char* ProofStatusString(ProofStatus s) {
    switch (s) {
        case ProofStatus::Proved: return "PROVED";
        case ProofStatus::Numerical: return "NUMERICAL";
        case ProofStatus::Disproved: return "DISPROVED";
        case ProofStatus::Falsified: return "FALSIFIED";
        case ProofStatus::Impossible: return "IMPOSSIBLE";
        default: return "OPEN";
    }
}

inline const char* TailBoundStatusString(Numerics::TailBoundStatus s) {
    using Numerics::TailBoundStatus;
    switch (s) {
        case TailBoundStatus::Valid: return "VALID";
        case TailBoundStatus::Divergent: return "DIVERGENT";
        default: return "UNPROVED";
    }
}

}  // namespace Marshal::Cert

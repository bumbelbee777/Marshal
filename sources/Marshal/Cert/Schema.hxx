#pragma once
// Marshal certificate JSON field names (no legacy tier/milestone wording).

namespace Marshal::Cert::Schema {

inline constexpr const char* kEngine = "Marshal";
inline constexpr const char* kVerdict = "verdict";
inline constexpr const char* kAnalysisStatus = "analysis_status";

inline constexpr const char* kPhaseLocalCylinder = "phase_local_cylinder";
inline constexpr const char* kPhaseInductiveLadder = "phase_inductive_ladder";
inline constexpr const char* kPhaseLocalAssembly = "phase_local_assembly";
inline constexpr const char* kPhaseGlobalBalance = "phase_global_balance";
inline constexpr const char* kPhaseTraceIdentity = "phase_trace_identity";
inline constexpr const char* kPhaseSpectrumDiagnostic = "phase_spectrum_diagnostic";
inline constexpr const char* kPhaseConvergence = "phase_convergence";

}  // namespace Marshal::Cert::Schema

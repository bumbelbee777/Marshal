#pragma once

#include <string>

#include "Heat/XiHadamardEngine.hxx"

namespace Marshal::AnaVM {

enum class XiHadamardLeanEmitRefusal {
    None,
    ProofChainOpen,
    CircularGraph,
    BoundsExceedTolerance,
};

struct XiHadamardLeanEmitOptions {
    bool quiet_refusal = false;
};

/// Pre-flight gate: why emission would be refused (no logging).
XiHadamardLeanEmitRefusal xi_hadamard_lean_emit_refusal(const Heat::XiHadamardReport& report);

/// Emit AnaVM XiHadamard Lean artifacts when the acyclic proof chain closes.
/// Writes cert, canonical, RH closure, genus analytic bridge, and genus bounds siblings.
bool emit_xi_hadamard_lean_artifacts(const std::string& cert_lean_path,
                                     const std::string& canonical_lean_path,
                                     const std::string& rh_closure_lean_path,
                                     const Heat::XiHadamardReport& report,
                                     XiHadamardLeanEmitOptions opts = {});

}  // namespace Marshal::AnaVM

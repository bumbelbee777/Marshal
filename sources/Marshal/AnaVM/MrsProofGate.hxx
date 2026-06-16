#pragma once

#include "Heat/XiHadamardEngine.hxx"

#include <string>

namespace Marshal::AnaVM {

/// Why the Marshal XiHadamard MRS proof chain is not closed.
enum class XiHadamardMrsProofRefusal {
    None,
    ProofChainOpen,
    CircularGraph,
    BoundsExceedTolerance,
    StructuralStubRemaining,
    UnconditionalRhOpen,
    MrsProofAuditFailed,
};

/// Numeric audits within MRS / engine tolerances (primary closure gate — not Lean).
bool xi_hadamard_report_bounds_ok(const Heat::XiHadamardReport& report);

/// Pre-flight: why MRS proof closure would fail (no logging).
XiHadamardMrsProofRefusal xi_hadamard_mrs_proof_refusal(const Heat::XiHadamardReport& report);

/// True when acyclic proof graph closes and audit bounds sit inside MRS tolerance.
bool xi_hadamard_mrs_proof_ok(const Heat::XiHadamardReport& report);

const char* xi_hadamard_mrs_proof_refusal_message(XiHadamardMrsProofRefusal refusal);

}  // namespace Marshal::AnaVM

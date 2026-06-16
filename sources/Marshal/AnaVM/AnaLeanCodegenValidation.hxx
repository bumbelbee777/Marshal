#pragma once

#include "MrsProofGate.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct LeanCodegenIssue {
    std::string role;
    std::string message;
};

struct LeanCodegenValidationReport {
    bool ok = true;
    std::vector<LeanCodegenIssue> issues;

    void fail(const std::string& role, const std::string& message);
};

LeanCodegenValidationReport validate_xi_hadamard_cert_lean(const std::string& body,
                                                           const Heat::XiHadamardReport& report);

LeanCodegenValidationReport validate_xi_hadamard_canonical_lean(const std::string& body);

LeanCodegenValidationReport validate_xi_hadamard_rh_closure_lean(const std::string& body);

LeanCodegenValidationReport validate_xi_hadamard_lean_bundle(
    const std::string& cert_body, const std::string& canonical_body,
    const std::string& rh_body, const std::string& genus_bridge_body,
    const std::string& genus_bounds_body, const Heat::XiHadamardReport& report);

LeanCodegenValidationReport validate_genus_one_log_analytic_bridge_lean(const std::string& body);

LeanCodegenValidationReport validate_marshal_genus_one_log_bounds_lean(const std::string& body);

/// Reject sabotaged head-envelope cert proofs (hand-edited coarse bounds).
bool genus_one_head_envelope_cert_proof_ok(const std::string& cert_body);

/// True when emitted RH capstone has no open analytic hypothesis parameters.
bool rh_closure_lean_is_parameterless(const std::string& rh_body);

}  // namespace Marshal::AnaVM

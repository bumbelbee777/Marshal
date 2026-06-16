#pragma once

#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat::GLn {

struct GL3HodgeProofReport {
    int rank = 3;
    Real theta = 0;
    int predicted_hodge_multiplicity = 20;
    int kernel_multiplicity = 0;
    Real kernel_tolerance = 1e-6L;
    Real smallest_eigenvalue_abs = 0;
    bool theta_stable = false;
    bool rank3_contract_ok = false;
    bool rh_prerequisite_ok = false;
    bool hodge_match_ok = false;
    bool bounds_ok = false;
    bool hodge_conjecture_proved = false;
    bool mrs_proof_audit_ok = false;
    std::string proof_status = "PENDING";
};

GL3HodgeProofReport run_gl3_hodge_proof_engine(const Config& cfg, const std::vector<int>& primes);

bool export_gl3_hodge_proof_json(const std::string& path, const GL3HodgeProofReport& r);

}  // namespace Marshal::Heat::GLn

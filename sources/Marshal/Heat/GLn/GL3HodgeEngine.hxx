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
    int cycle_map_cokernel_dim = 0;
    int hitchin_divisor_count = 0;
    int cycle_constructive_span = 0;
    int hodge_pp_2_0 = 1;
    int hodge_pp_1_1 = 20;
    int hodge_pp_0_2 = 1;
    int hodge_millennium_pp_match = 22;
    int hodge_millennium_pp_target = 22;
    Real kernel_tolerance = 1e-6L;
    Real smallest_eigenvalue_abs = 0;
    bool theta_stable = false;
    bool rank3_contract_ok = false;
    bool rh_prerequisite_ok = false;
    bool hodge_match_ok = false;
    bool cycle_constructive_ok = false;
    bool cycle_map_ok = false;
    bool hodge_pp_ok = false;
    bool hodge_millennium_ok = false;
    bool bounds_ok = false;
    bool hodge_conjecture_proved = false;
    bool hodge_millennium_proved = false;
    bool mrs_proof_audit_ok = false;
    std::string proof_status = "PENDING";
};

GL3HodgeProofReport run_gl3_hodge_proof_engine(const Config& cfg, const std::vector<int>& primes);

bool export_gl3_hodge_proof_json(const std::string& path, const GL3HodgeProofReport& r);

}  // namespace Marshal::Heat::GLn

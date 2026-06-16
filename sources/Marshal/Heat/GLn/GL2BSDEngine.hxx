#pragma once

#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat::GLn {

struct GL2BSDProofReport {
    int rank = 2;
    std::string curve_label = "37a";
    Real theta = 0;
    int algebraic_rank = 1;
    int kernel_multiplicity = 0;
    Real l_function_grid_rel_gap = 0;
    Real l_function_grid_rel_gap_ub = 0.03L;
    Real sha_resolvent_gap = 0;
    Real sha_resolvent_gap_ub = 0.05L;
    bool rh_prerequisite_ok = false;
    bool rank_match_ok = false;
    bool l_grid_ok = false;
    bool sha_gap_ok = false;
    bool bounds_ok = false;
    bool bsd_rank_proved = false;
    bool mrs_proof_audit_ok = false;
    std::string proof_status = "PENDING";
};

GL2BSDProofReport run_gl2_bsd_proof_engine(const Config& cfg, const std::vector<int>& primes);

bool export_gl2_bsd_proof_json(const std::string& path, const GL2BSDProofReport& r);

}  // namespace Marshal::Heat::GLn

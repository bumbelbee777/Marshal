#pragma once

#include "MarshalGLnDirac.hxx"

#include "Config.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat::GLn {

struct GLnLadderRankEntry {
    int rank = 1;
    std::string arch_preset;
    Real theta = 0;
    std::vector<Real> eigenvalues;
    Real spectral_action_heat = 0;
    int kernel_multiplicity = 0;
    int predicted_hodge_multiplicity = 0;
    Real kernel_tolerance = 0;
    bool hodge_match = false;
    bool theta_stable = false;
    bool rank3_contract_ok = false;
    bool rank4_contract_ok = false;
    Real smallest_eigenvalue_abs = 0;
    std::string proof_status;
};

struct GLnLadderReport {
    std::vector<GLnLadderRankEntry> ranks;
    Real pinned_theta = 0;
};

GLnLadderReport run_gln_ladder_validation(const Config& cfg, const std::vector<int>& primes);

bool export_gln_ladder_json(const std::string& path, const GLnLadderReport& r);

bool export_hodge_k3_demo_json(const std::string& path, const GLnLadderRankEntry& rank3);

}  // namespace Marshal::Heat::GLn

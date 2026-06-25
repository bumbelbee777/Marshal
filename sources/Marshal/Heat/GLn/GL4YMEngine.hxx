#pragma once

#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat::GLn {

struct GL4YMProofReport {
    int rank = 4;
    Real theta = 0;
    Real gauge_smallest_positive_eigenvalue = 0;
    Real lattice_gap_estimate = 0;
    Real ym_mass_gap_lb = 2.0;
    Real kernel_tolerance = 1e-6L;
    Real smallest_eigenvalue_abs = 0;
    bool theta_stable = false;
    bool rank4_contract_ok = false;
    bool hodge_prerequisite_ok = false;
    bool outlook_ok = false;
    bool self_adjoint_ok = false;
    bool os_axioms_ok = false;
    bool mass_gap_ok = false;
    bool lattice_gap_ok = false;
    bool su3_reduction_ok = false;
    bool bounds_ok = false;
    Real gauge_over_gravity = 0;
    Real rooted_blended_rmse = 0;
    Real os_reflection_residual = 0;
    bool os_reflection_ok = false;
    bool ym_mass_gap_proved = false;
    bool mrs_proof_audit_ok = false;
    std::string proof_status = "PENDING";
    std::vector<Real> gauge_block_eigenvalues;
};

GL4YMProofReport run_gl4_ym_proof_engine(const Config& cfg, const std::vector<int>& primes,
                                         bool hodge_capstone_ok, bool outlook_ok);

bool export_gl4_ym_proof_json(const std::string& path, const GL4YMProofReport& r);

}  // namespace Marshal::Heat::GLn

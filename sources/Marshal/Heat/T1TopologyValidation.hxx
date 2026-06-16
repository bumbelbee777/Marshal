#pragma once

#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat {

struct T1GapAtTheta {
    Real theta = 0;
    Real prime_gap = 0;
    Real arch_gap = 0;
    Real total_gap = 0;
    bool admissible = false;
};

struct T1TopologyValidationReport {
    std::string program_id;
    Real fixed_theta = 0;
    Real t1_tolerance = 1e-6;
    Real prime_gap_uniform = 0;
    std::vector<T1GapAtTheta> sweep;
};

Real t1_gap_at_theta(const Config& cfg, Real theta, const std::vector<Real>& gammas_ld,
                     PrimeCatalog& cat, const std::vector<int>& primes, Real* prime_gap_out,
                     Real* arch_gap_out);

T1TopologyValidationReport run_t1_topology_validation(
    const Config& cfg, const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
    const std::vector<int>& primes, const std::vector<Real>& theta_grid, Real t1_tol);

void export_t1_topology_validation_json(const std::string& path,
                                        const T1TopologyValidationReport& rep);

}  // namespace Marshal::Heat

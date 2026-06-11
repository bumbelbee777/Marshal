#pragma once
#include <vector>
#include "Config.hxx"
#include "Numerics/Real.hxx"
#include "PrimeCatalog.hxx"

namespace Marshal::Heat {

struct HeatTraceSweepResult {
    std::vector<Real> t_values;
    std::vector<Real> heat_from_zeros;
    std::vector<Real> heat_from_operator;
    std::vector<Real> residuals;
    Real t_min = 0;
    Real t_max = 0;
    Real max_residual = 0;
    Real mean_residual = 0;
    int n_valid = 0;
    int n_primes_used = 0;
    bool trace_identity_holds = false;
};

HeatTraceSweepResult VerifyTraceIdentity(
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const Config& cfg, Real sigma_trace, Real sweep_tol,
    int p_cutoff = 0);

}  // namespace Marshal::Heat

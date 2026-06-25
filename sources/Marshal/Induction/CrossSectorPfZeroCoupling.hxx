#pragma once

#include <vector>

#include "CrossSectorStep5Bounds.hxx"
#include "CrossSectorWeilOperator.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

/// Pf+zero+arch coupling audit at the Rayleigh-min sin mode (Suzuki Phase 5.6).
struct PfZeroCouplingBreakdown {
    Real q_pf = 0;
    Real q_zero = 0;
    Real q_prime = 0;
    Real q_arch_lower = 0;  // unconditional 2A lower bound −A(a) on arch sector
    Real pf_zero_sum = 0;
    Real pf_zero_arch_sum = 0;
    Real lambda_proxy = 0;
    Real cross_balance = 0;
    /// λ = cross_balance + 2 q_prime (exact at audited mode)
    Real coupling_identity_residual = 0;
    /// RH pin at audited mode: q_pf + q_zero + q_prime >= 0
    Real bare_lambda_at_mode = 0;
    /// Diagnostic arch shift: bare_lambda - A(a); NOT equivalent to RH when A > 0
    Real suzuki_coupling_margin = 0;
    /// Sharp Pf+zero vs prime: q_pf + q_zero - |q_prime| when q_prime <= 0
    Real pf_zero_prime_pin_margin = 0;
    int mode_index = 0;
};

PfZeroCouplingBreakdown pf_zero_coupling_at_rayleigh(
    Real a, Real A_arch, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat,
    Real log_cutoff, Real zero_T, Real eps, const WeilOperatorRayleighOpts& opts = {});

}  // namespace Marshal::Induction

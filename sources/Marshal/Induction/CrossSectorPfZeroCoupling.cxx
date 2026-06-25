#include "CrossSectorPfZeroCoupling.hxx"

#include <cmath>

namespace Marshal::Induction {

PfZeroCouplingBreakdown pf_zero_coupling_at_rayleigh(
    Real a, Real A_arch, const std::vector<double>& gammas, const Heat::PrimeCatalog& cat,
    Real log_cutoff, Real zero_T, Real eps, const WeilOperatorRayleighOpts& opts) {
    PfZeroCouplingBreakdown out;
    if (a <= 0) return out;

    const WeilQuadraticSectorBreakdown sectors =
        weil_rayleigh_sector_breakdown(a, gammas, cat, log_cutoff, zero_T, eps, opts);

    out.q_pf = sectors.q_pf;
    out.q_zero = sectors.q_zero;
    out.q_prime = sectors.q_prime;
    out.q_arch_lower = -A_arch;
    out.pf_zero_sum = out.q_pf + out.q_zero;
    out.pf_zero_arch_sum = out.pf_zero_sum + out.q_arch_lower;
    out.lambda_proxy = sectors.rayleigh;
    out.cross_balance = out.pf_zero_sum - out.q_prime;
    out.coupling_identity_residual =
        out.lambda_proxy - (out.cross_balance + 2.0L * out.q_prime);
    out.bare_lambda_at_mode = out.lambda_proxy;
    out.suzuki_coupling_margin = out.bare_lambda_at_mode - A_arch;
    out.pf_zero_prime_pin_margin = out.bare_lambda_at_mode;
    out.mode_index = sectors.mode_index;
    return out;
}

}  // namespace Marshal::Induction

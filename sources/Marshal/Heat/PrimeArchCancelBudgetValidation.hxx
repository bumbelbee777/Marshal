#pragma once

#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct PrimeArchCancelBudgetReport {
    Real theta0 = 144.0L / 25.0L;
    Real eps_prime_pin = 0.03L;
    Real eps_arch_pin = 0.03L;
    Real eps_composite_pin = 0.05L;
    Real eps_prime_measured = 0;
    Real eps_arch_measured = 0;
    bool pac_prime_budget_ok = false;
    bool pac_arch_budget_ok = false;
    bool pac_composite_budget_ok = false;
    bool prime_arch_cancel_budget_cert_ok = false;
};

PrimeArchCancelBudgetReport run_prime_arch_cancel_budget_validation(
    const Config& cfg, const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
    const std::vector<int>& primes);

bool export_prime_arch_cancel_budget_json(const std::string& path,
                                          const PrimeArchCancelBudgetReport& rep);

}  // namespace Marshal::Heat

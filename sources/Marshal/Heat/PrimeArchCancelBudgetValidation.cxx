#include "PrimeArchCancelBudgetValidation.hxx"

#include "T1TopologyValidation.hxx"

#include <fstream>
#include <iomanip>

namespace Marshal::Heat {

PrimeArchCancelBudgetReport run_prime_arch_cancel_budget_validation(
    const Config& cfg, const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
    const std::vector<int>& primes) {
    PrimeArchCancelBudgetReport rep;
    rep.theta0 = 144.0L / 25.0L;
    Real prime_gap = 0;
    Real arch_gap = 0;
    (void)t1_gap_at_theta(cfg, rep.theta0, gammas_ld, cat, primes, &prime_gap, &arch_gap);
    rep.eps_prime_measured = prime_gap;
    rep.eps_arch_measured = arch_gap;
    rep.pac_prime_budget_ok = prime_gap < rep.eps_prime_pin;
    rep.pac_arch_budget_ok = arch_gap < rep.eps_arch_pin;
    rep.pac_composite_budget_ok =
        (prime_gap + arch_gap) < rep.eps_composite_pin;
    rep.prime_arch_cancel_budget_cert_ok =
        rep.pac_prime_budget_ok && rep.pac_arch_budget_ok && rep.pac_composite_budget_ok;
    return rep;
}

bool export_prime_arch_cancel_budget_json(const std::string& path,
                                          const PrimeArchCancelBudgetReport& rep) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"theta0\": " << static_cast<double>(rep.theta0) << ",\n";
    out << "  \"theta0_rational\": \"144/25\",\n";
    out << "  \"eps_prime_pin\": " << static_cast<double>(rep.eps_prime_pin) << ",\n";
    out << "  \"eps_arch_pin\": " << static_cast<double>(rep.eps_arch_pin) << ",\n";
    out << "  \"eps_composite_pin\": " << static_cast<double>(rep.eps_composite_pin) << ",\n";
    out << "  \"eps_prime_measured\": " << static_cast<double>(rep.eps_prime_measured) << ",\n";
    out << "  \"eps_arch_measured\": " << static_cast<double>(rep.eps_arch_measured) << ",\n";
    out << "  \"pac_prime_budget_ok\": "
        << (rep.pac_prime_budget_ok ? "true" : "false") << ",\n";
    out << "  \"pac_arch_budget_ok\": "
        << (rep.pac_arch_budget_ok ? "true" : "false") << ",\n";
    out << "  \"pac_composite_budget_ok\": "
        << (rep.pac_composite_budget_ok ? "true" : "false") << ",\n";
    out << "  \"prime_arch_cancel_budget_cert_ok\": "
        << (rep.prime_arch_cancel_budget_cert_ok ? "true" : "false") << ",\n";
    out << "  \"ok\": " << (rep.prime_arch_cancel_budget_cert_ok ? "true" : "false") << "\n";
    out << "}\n";
    return true;
}

}  // namespace Marshal::Heat

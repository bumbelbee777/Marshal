#include "ConnesCrossedValidation.hxx"

#include "ConnesBasisCap.hxx"
#include "ConnesCrossedProduct.hxx"
#include "LogPrimeValidation.hxx"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

namespace Marshal::Heat {

namespace {

std::vector<Real> default_lambdas() {
    return {0.0L, 0.01L, 0.05L, 0.1L, 0.25L, 0.5L, 1.0L, 2.0L, 4.0L};
}

std::vector<size_t> prime_cuts_for_validation(const Config& cfg, size_t n_primes) {
    if (!cfg.connes_prime_ladder.empty()) {
        std::vector<size_t> cuts;
        for (int cut : cfg.connes_prime_ladder) {
            if (cut > 0 && static_cast<size_t>(cut) <= n_primes) cuts.push_back(static_cast<size_t>(cut));
        }
        if (cuts.empty() || cuts.back() != n_primes) cuts.push_back(n_primes);
        return cuts;
    }
    static const size_t anchors[] = {10, 50, 100, 500, 1000, 5000, 10000};
    std::vector<size_t> cuts;
    for (size_t a : anchors) {
        if (a <= n_primes) cuts.push_back(a);
    }
    if (cuts.empty() || cuts.back() != n_primes) cuts.push_back(n_primes);
    return cuts;
}

std::vector<int> cap_primes(const std::vector<int>& primes, size_t cap) {
    if (cap == 0 || primes.size() <= cap) return primes;
    return std::vector<int>(primes.begin(), primes.begin() + static_cast<ptrdiff_t>(cap));
}

}  // namespace

ConnesCrossedValidationReport run_connes_crossed_validation(const Config& cfg,
                                                            const std::vector<Real>& gammas_ld,
                                                            const std::vector<int>& primes) {
    ConnesCrossedValidationReport rep;
    rep.coupling = cfg.connes_coupling_mode;
    rep.test_T = cfg.test_param > 0 ? cfg.test_param : 14.134725142L;
    rep.test_kappa = cfg.sinc2_kappa > 0 ? cfg.sinc2_kappa : 60.0L;
    rep.spectrum_rmse_target =
        cfg.connes_spectrum_rmse_max > 0 ? cfg.connes_spectrum_rmse_max : 1.0L;
    rep.spectrum_max_gap_target =
        cfg.connes_spectrum_max_gap_max > 0 ? cfg.connes_spectrum_max_gap_max : 1.0L;
    rep.lambda_sweep = cfg.connes_lambda_sweep.empty() ? default_lambdas() : cfg.connes_lambda_sweep;

    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const auto prime_cuts = prime_cuts_for_validation(cfg, primes.size());

    Real best_score = 1e300L;
    std::vector<std::string> seen;
    for (size_t cut : prime_cuts) {
        const auto sub = cap_primes(primes, cut);
        const auto plan = plan_connes_basis(static_cast<int>(sub.size()), kmax);
        if (plan.k_twist < 1 || plan.n_modes < 1) continue;
        const auto sub_eff = cap_primes(sub, static_cast<size_t>(plan.n_primes));
        for (Real lam : rep.lambda_sweep) {
            const std::string key = std::to_string(sub_eff.size()) + ":" + std::to_string(plan.k_twist) +
                                      ":" + std::to_string(static_cast<double>(lam));
            if (std::find(seen.begin(), seen.end(), key) != seen.end()) continue;
            seen.push_back(key);
            auto cp = ConnesCrossedProduct::from_primes(sub_eff, lam, rep.coupling);
            ConnesCrossedPoint pt;
            pt.n_primes = static_cast<int>(sub_eff.size());
            pt.p_max = sub_eff.empty() ? 0 : sub_eff.back();
            pt.lambda = lam;
            pt.k_twist = plan.k_twist;
            pt.coupling = rep.coupling;
            pt.spectrum = cp.spectrum_metrics(gammas_ld, plan.k_twist);
            pt.sinc2_spectral_gap = cp.sinc2_residual(gammas_ld, rep.test_T, plan.k_twist, rep.test_kappa);
            pt.weil_prime_vs_zeros =
                cp.weil_sinc2_residual(gammas_ld, rep.test_T, plan.k_twist, rep.test_kappa, cfg.eps);
            rep.ladder.push_back(pt);

            const Real score = pt.spectrum.rmse + 0.25L * pt.spectrum.max_gap;
            if (score < best_score) {
                best_score = score;
                rep.best_rmse = pt.spectrum.rmse;
                rep.best_max_gap = pt.spectrum.max_gap;
                rep.best_lambda = lam;
                rep.best_n_primes = pt.n_primes;
            }
        }
    }

    rep.spectrum_identified = rep.best_rmse <= rep.spectrum_rmse_target &&
                              rep.best_max_gap <= rep.spectrum_max_gap_target;
    if (rep.spectrum_identified)
        rep.verdict = "SPECTRUM_IDENTIFIED_NUMERIC";
    else if (rep.best_rmse < 5.0L)
        rep.verdict = "INCONCLUSIVE_APPROACHING";
    else
        rep.verdict = "SPECTRUM_MISMATCH";

    std::cout << "=== Connes crossed-product spectrum validation ===\n";
    std::cout << "  coupling: " << connes_coupling_mode_name(rep.coupling) << "\n";
    std::cout << "  basis_cap: " << kConnesBasisCap << " modes\n";
    std::cout << "  T=" << static_cast<double>(rep.test_T)
              << "  kappa=" << static_cast<double>(rep.test_kappa) << "\n";
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "  best RMSE=" << static_cast<double>(rep.best_rmse)
              << "  max_gap=" << static_cast<double>(rep.best_max_gap)
              << "  lambda=" << static_cast<double>(rep.best_lambda)
              << "  n_primes=" << rep.best_n_primes << "\n";
    std::cout << "  targets: rmse<" << static_cast<double>(rep.spectrum_rmse_target)
              << " max_gap<" << static_cast<double>(rep.spectrum_max_gap_target) << "\n";
    std::cout << "  verdict: " << rep.verdict << "  spectrum_identified="
              << (rep.spectrum_identified ? "yes" : "no") << "\n";
    return rep;
}

bool export_connes_crossed_validation_json(const std::string& path,
                                           const ConnesCrossedValidationReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"goal\": \"spectrum_equals_gamma_n\",\n";
    out << "  \"coupling_mode\": \"" << connes_coupling_mode_name(r.coupling) << "\",\n";
    out << "  \"basis_cap_modes\": " << kConnesBasisCap << ",\n";
    out << "  \"test_T\": " << static_cast<double>(r.test_T) << ",\n";
    out << "  \"test_kappa\": " << static_cast<double>(r.test_kappa) << ",\n";
    out << "  \"spectrum_rmse_target\": " << static_cast<double>(r.spectrum_rmse_target) << ",\n";
    out << "  \"spectrum_max_gap_target\": " << static_cast<double>(r.spectrum_max_gap_target)
        << ",\n";
    out << "  \"best_rmse\": " << static_cast<double>(r.best_rmse) << ",\n";
    out << "  \"best_max_gap\": " << static_cast<double>(r.best_max_gap) << ",\n";
    out << "  \"best_lambda\": " << static_cast<double>(r.best_lambda) << ",\n";
    out << "  \"best_n_primes\": " << r.best_n_primes << ",\n";
    out << "  \"spectrum_identified\": " << (r.spectrum_identified ? "true" : "false") << ",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n";
    out << "  \"lemma\": \"connes_crossed_product_assembly\",\n";
    out << "  \"note\": \"numeric evidence only; not a proof that sigma(H_cross)={gamma_n}\",\n";
    out << "  \"ladder\": [\n";
    for (size_t i = 0; i < r.ladder.size(); ++i) {
        const auto& pt = r.ladder[i];
        out << "    {\"n_primes\": " << pt.n_primes << ", \"p_max\": " << pt.p_max
            << ", \"lambda\": " << static_cast<double>(pt.lambda) << ", \"k_twist\": "
            << pt.k_twist << ", \"coupling\": \"" << connes_coupling_mode_name(pt.coupling)
            << "\", \"spectrum_rmse\": " << static_cast<double>(pt.spectrum.rmse)
            << ", \"spectrum_max_gap\": " << static_cast<double>(pt.spectrum.max_gap)
            << ", \"spectrum_mean_gap\": " << static_cast<double>(pt.spectrum.mean_gap)
            << ", \"n_modes\": " << pt.spectrum.n_modes << ", \"sinc2_spectral_gap\": "
            << static_cast<double>(pt.sinc2_spectral_gap) << ", \"weil_prime_vs_zeros\": "
            << static_cast<double>(pt.weil_prime_vs_zeros) << "}";
        if (i + 1 < r.ladder.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

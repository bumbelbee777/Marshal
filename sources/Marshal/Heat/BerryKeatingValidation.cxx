#include "BerryKeatingValidation.hxx"

#include <fstream>
#include <iomanip>

namespace Marshal::Heat {

BerryKeatingValidationReport run_berry_keating_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes) {
    BerryKeatingValidationReport rep;
    rep.program_id = cfg.anavm.id;
    rep.height_map_applied = spec_from_config(cfg).apply_log_height;
    rep.extension_sweep =
        run_self_adjoint_extension_sweep(cfg, gammas, gammas_ld, cat, primes);
    rep.best_theta = rep.extension_sweep.best_theta;
    rep.best_metrics.rmse = rep.extension_sweep.best_rmse;
    rep.best_metrics_raw.rmse = rep.extension_sweep.best_rmse_raw;
    rep.best_metrics.n_matched = static_cast<int>(gammas_ld.size());

    const Real rmse_target =
        cfg.diagnostics_spectrum_rmse_max > 0 ? cfg.diagnostics_spectrum_rmse_max : 50.0L;
    if (rep.best_metrics.rmse < rmse_target)
        rep.verdict = "BK_SPECTRUM_APPROACHING";
    else if (rep.best_metrics.rmse < 10.0L)
        rep.verdict = "BK_SPECTRUM_VIABLE_ARCHIMEDIAN";
    else
        rep.verdict = "BK_SPECTRUM_MISMATCH";
    return rep;
}

bool export_berry_keating_validation_json(const std::string& path,
                                          const BerryKeatingValidationReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"best_theta\": " << static_cast<double>(r.best_theta) << ",\n";
    out << "  \"spectrum_rmse\": " << static_cast<double>(r.best_metrics.rmse) << ",\n";
    out << "  \"spectrum_rmse_raw\": " << static_cast<double>(r.best_metrics_raw.rmse) << ",\n";
    out << "  \"height_map_applied\": " << (r.height_map_applied ? "true" : "false") << ",\n";
    out << "  \"height_map_formula\": \"gamma_bk * log(n) / (2*pi)\",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n";
    out << "  \"extension_sweep_verdict\": \"" << r.extension_sweep.verdict << "\"\n";
    out << "}\n";
    return true;
}

}  // namespace Marshal::Heat

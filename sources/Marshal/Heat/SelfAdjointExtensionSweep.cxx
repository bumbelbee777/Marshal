#include "SelfAdjointExtensionSweep.hxx"

#include "Heat/Common.hxx"
#include "Heat/LogPrimeGlobal.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Induction/Induction.hxx"
#include "Numerics/TestFunctions.hxx"
#include "SpectralDeterminant.hxx"
#include "TraceApi.hxx"

#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {
namespace {

const char* boundary_name(AnaVM::ArchimedeanBoundary b) {
    switch (b) {
        case AnaVM::ArchimedeanBoundary::Dirichlet:
            return "dirichlet";
        case AnaVM::ArchimedeanBoundary::Neumann:
            return "neumann";
        case AnaVM::ArchimedeanBoundary::Periodic:
            return "periodic";
        default:
            return "berry_keating";
    }
}

Real evaluate_extension_score(const Config& cfg, Real theta, const BerryKeatingSpec& bspec,
                              const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
                              const std::vector<int>& primes, AnaVM::ArchimedeanBoundary boundary,
                              ExtensionSweepRow& row) {
    BerryKeatingSpec spec = bspec;
    spec.theta = theta;
    const auto ladder = bk_wkb_ladder(spec);
    const auto ladder_raw = bk_wkb_ladder(spec, false);
    const auto metrics = compare_to_zeros(ladder, gammas_ld);
    (void)compare_to_zeros(ladder_raw, gammas_ld);
    row.theta = theta;
    row.boundary = boundary;
    row.boundary_name = boundary_name(boundary);
    row.spectrum_rmse = metrics.rmse;
    row.spectrum_max_gap = metrics.max_gap;

    ArchimedeanBoundarySpec arch;
    arch.boundary = boundary;
    arch.type = AnaVM::ArchimedeanType::RealLine;
    arch.cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;

    const Real laplace_a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;
    const LaplaceTest laplace(laplace_a);
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const size_t nz = std::min(gammas_ld.size(), size_t{50000});
    cat.rebuild_adaptive(laplace, 1.0L / (2.0L * laplace_a * laplace_a), kmax, cfg.eps);
    const TraceResult tr =
        EvaluateTracePrefix(laplace, laplace_a, nullptr, nz, gammas_ld.data(), gammas_ld.size(),
                            cat, cfg.zero_kernel, cfg.simd, cfg.eps, cfg.trivial_zeros,
                            cfg.precision_mode, cfg.arch_pts, false, &arch);

    const int max_primes = static_cast<int>(std::min(primes.size(), size_t{5000}));
    std::vector<int> sub(primes.begin(), primes.begin() + max_primes);
    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(sub);
    const Real hlog = global.weil_prime_sum(laplace, kmax, cfg.eps) / (2.0L * kPi);

    row.laplace_weil_residual = std::fabs(tr.residual());
    (void)hlog;
    row.xi_det_gap = std::fabs(std::log(std::max(std::fabs(tr.lhs), Real{1e-300})) -
                               std::log(std::max(std::fabs(tr.poles + tr.arch - tr.prime), Real{1e-300})));
    row.score = row.laplace_weil_residual + 0.1L * row.spectrum_rmse + 0.05L * row.xi_det_gap;
    row.admissible = row.laplace_weil_residual < 1.0L;
    return row.score;
}

}  // namespace

SelfAdjointExtensionSweepReport run_self_adjoint_extension_sweep(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes) {
    SelfAdjointExtensionSweepReport rep;
    rep.program_id = cfg.anavm.id;
    rep.rule_id = cfg.anavm.rule_id;
    rep.sweep_steps = cfg.self_adjoint_extension.present && cfg.self_adjoint_extension.sweep_steps > 0
                          ? cfg.self_adjoint_extension.sweep_steps
                          : 24;

    const BerryKeatingSpec bspec = spec_from_config(cfg);
    rep.height_map_applied = bspec.apply_log_height;
    const AnaVM::ArchimedeanBoundary bounds[] = {
        AnaVM::ArchimedeanBoundary::BerryKeating, AnaVM::ArchimedeanBoundary::Dirichlet,
        AnaVM::ArchimedeanBoundary::Neumann, AnaVM::ArchimedeanBoundary::Periodic};

    Real best = 1e300L;
    for (int si = 0; si < rep.sweep_steps; ++si) {
        const Real theta = 2.0L * kPi * static_cast<Real>(si) / static_cast<Real>(rep.sweep_steps);
        for (auto bd : bounds) {
            ExtensionSweepRow row;
            const Real score =
                evaluate_extension_score(cfg, theta, bspec, gammas_ld, cat, primes, bd, row);
            rep.rows.push_back(row);
            if (score < best) {
                best = score;
                rep.best_theta = theta;
                rep.best_score = score;
                rep.best_rmse = row.spectrum_rmse;
                BerryKeatingSpec raw_spec = bspec;
                raw_spec.theta = theta;
                rep.best_rmse_raw =
                    compare_to_zeros(bk_wkb_ladder(raw_spec, false), gammas_ld).rmse;
            }
        }
    }

    const Real rmse_target = cfg.connes_spectrum_rmse_max > 0 ? cfg.connes_spectrum_rmse_max : 50.0L;
    rep.verdict = rep.best_rmse < rmse_target ? "EXTENSION_ADMISSIBLE" : "EXTENSION_INCONCLUSIVE";
  (void)gammas;
    return rep;
}

bool export_self_adjoint_extension_sweep_json(const std::string& path,
                                              const SelfAdjointExtensionSweepReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"rule_id\": \"" << r.rule_id << "\",\n";
    out << "  \"sweep_steps\": " << r.sweep_steps << ",\n";
    out << "  \"best_theta\": " << static_cast<double>(r.best_theta) << ",\n";
    out << "  \"best_score\": " << static_cast<double>(r.best_score) << ",\n";
    out << "  \"best_rmse\": " << static_cast<double>(r.best_rmse) << ",\n";
    out << "  \"best_rmse_raw\": " << static_cast<double>(r.best_rmse_raw) << ",\n";
    out << "  \"height_map_applied\": " << (r.height_map_applied ? "true" : "false") << ",\n";
    out << "  \"height_map_formula\": \"gamma_bk * log(n) / (2*pi)\",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n";
    out << "  \"admissible_extensions\": [\n";
    for (size_t i = 0; i < r.rows.size(); ++i) {
        const auto& row = r.rows[i];
        out << "    {\"theta\": " << static_cast<double>(row.theta) << ", \"boundary\": \""
            << row.boundary_name << "\", \"spectrum_rmse\": "
            << static_cast<double>(row.spectrum_rmse) << ", \"laplace_weil_residual\": "
            << static_cast<double>(row.laplace_weil_residual) << ", \"xi_det_gap\": "
            << static_cast<double>(row.xi_det_gap) << ", \"admissible\": "
            << (row.admissible ? "true" : "false") << "}";
        if (i + 1 < r.rows.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

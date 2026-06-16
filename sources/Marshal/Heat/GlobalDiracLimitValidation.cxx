#include "GlobalDiracLimitValidation.hxx"

#include "BerryKeatingOperator.hxx"

#include "Kernel/CombinedDiracFast.hxx"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {

GlobalDiracLimitReport run_global_dirac_limit_validation(const Config& cfg,
                                                           const std::vector<Real>& gammas_ld,
                                                           const std::vector<int>& primes) {
    GlobalDiracLimitReport rep;
    rep.program_id = cfg.anavm.id;
    rep.rule_id = cfg.anavm.rule_id;
    if (cfg.discretization_limit.present) {
        rep.kind = cfg.discretization_limit.kind;
        rep.limit_target = cfg.discretization_limit.limit_target;
        rep.metric = cfg.discretization_limit.metric;
        rep.formal_status = cfg.discretization_limit.formal_status;
    }
    if (cfg.formal_target.present) {
        rep.formal_lemma = cfg.formal_target.lemma;
        rep.formal_approach = cfg.formal_target.approach;
    }

    std::vector<int> caps = cfg.discretization_limit.present && !cfg.discretization_limit.caps.empty()
                                ? cfg.discretization_limit.caps
                                : std::vector<int>{120, 240, 400};
    const int theta_idx = 0;
    const int boundary_idx = 0;
    ArchimedeanBoundarySpec arch;
    arch.boundary = cfg.archimedean.present ? cfg.archimedean.boundary
                                           : AnaVM::ArchimedeanBoundary::BerryKeating;
    arch.type = AnaVM::ArchimedeanType::RealLine;
    arch.cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;

    const int n_scales = cfg.spectral_action.heat_scales > 0 ? cfg.spectral_action.heat_scales : 3;
    const Real scale_base =
        cfg.spectral_action.heat_scale_base > 0 ? static_cast<Real>(cfg.spectral_action.heat_scale_base)
                                                : 1.0L;

    Marshal::Kernel::CombinedDiracFastSpec fspec;
    fspec.bk = spec_from_config(cfg);
    fspec.coupling_lambda = cfg.connes_coupling_lambda > 0
                                ? cfg.connes_coupling_lambda
                                : static_cast<Real>(cfg.spectral_action.coupling_lambda);
    fspec.coupling_mode = cfg.connes_coupling_mode;
    fspec.kmax = cfg.kmax > 0 ? cfg.kmax : 12;
    fspec.arch_cap = 80;
    fspec.sweep_steps = 1;

    const auto t0 = std::chrono::steady_clock::now();
    Real prev_rmse = -1;
    bool monotone_inc = true;
    for (int cap : caps) {
        const auto row = Marshal::Kernel::probe_combined_cap(fspec, primes, cap, theta_idx,
                                                             boundary_idx, arch, scale_base,
                                                             n_scales, gammas_ld);
        GlobalDiracLimitPoint pt;
        pt.combined_cap = cap;
        pt.n_modes = row.n_modes;
        pt.spectrum_rmse = row.spectrum_rmse;
        pt.elapsed_ms = row.elapsed_ms;
        rep.points.push_back(pt);
        if (prev_rmse >= 0 && row.spectrum_rmse <= prev_rmse) monotone_inc = false;
        prev_rmse = row.spectrum_rmse;
    }
    rep.total_elapsed_ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    rep.monotone_rmse_increase = rep.points.size() >= 2 && monotone_inc;

    if (rep.points.size() < 2) {
        rep.limit_verdict = "LIMIT_INCONCLUSIVE";
    } else if (rep.monotone_rmse_increase) {
        rep.limit_verdict = "DISCRETIZATION_IDENTIFICATION_FAILS";
    } else {
        rep.limit_verdict = "LIMIT_OPEN_RMSE_NOT_MONOTONE";
    }

    rep.lean_emit_ready = rep.points.size() >= 2;
    rep.proof_status = "FORMAL_LIMIT_OPEN";

    std::cout << "Global Dirac limit: " << rep.limit_verdict << " kind=" << rep.kind
              << " points=" << rep.points.size() << " monotone_rmse_inc="
              << (rep.monotone_rmse_increase ? "true" : "false")
              << " lean_emit_ready=" << (rep.lean_emit_ready ? "true" : "false")
              << " formal=" << rep.proof_status << "\n";
    return rep;
}

bool export_global_dirac_limit_json(const std::string& path, const GlobalDiracLimitReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"rule_id\": \"" << r.rule_id << "\",\n";
    out << "  \"proof_status\": \"" << r.proof_status << "\",\n";
    out << "  \"kind\": \"" << r.kind << "\",\n";
    out << "  \"limit_target\": \"" << r.limit_target << "\",\n";
    out << "  \"metric\": \"" << r.metric << "\",\n";
    out << "  \"formal_status\": \"" << r.formal_status << "\",\n";
    out << "  \"formal_lemma\": \"" << r.formal_lemma << "\",\n";
    out << "  \"formal_approach\": \"" << r.formal_approach << "\",\n";
    out << "  \"limit_verdict\": \"" << r.limit_verdict << "\",\n";
    out << "  \"monotone_rmse_increase\": " << (r.monotone_rmse_increase ? "true" : "false")
        << ",\n";
    out << "  \"lean_emit_ready\": " << (r.lean_emit_ready ? "true" : "false") << ",\n";
    out << "  \"total_elapsed_ms\": " << r.total_elapsed_ms << ",\n";
    out << "  \"points\": [\n";
    for (size_t i = 0; i < r.points.size(); ++i) {
        const auto& p = r.points[i];
        out << "    {\"combined_cap\": " << p.combined_cap << ", \"n_modes\": " << p.n_modes
            << ", \"spectrum_rmse\": " << static_cast<double>(p.spectrum_rmse)
            << ", \"elapsed_ms\": " << p.elapsed_ms << "}";
        if (i + 1 < r.points.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

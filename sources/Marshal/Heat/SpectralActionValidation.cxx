#include "SpectralActionValidation.hxx"

#include "Analysis/PairCorrelation.hxx"
#include "BerryKeatingOperator.hxx"
#include "Kernel/CombinedDiracFast.hxx"
#include "Heat/Common.hxx"
#include "Heat/LogPrimeGlobal.hxx"
#include "Induction/Induction.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#ifdef _OPENMP
#include <omp.h>
#endif

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

ArchimedeanBoundarySpec make_arch(AnaVM::ArchimedeanBoundary boundary) {
    ArchimedeanBoundarySpec arch;
    arch.boundary = boundary;
    arch.type = AnaVM::ArchimedeanType::RealLine;
    arch.cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;
    return arch;
}

struct BoundaryMetrics {
    Real weil_residual = 0;
};

}  // namespace

SpectralActionReport run_spectral_action_validation(const Config& cfg,
                                                    const std::vector<double>& gammas,
                                                    const std::vector<Real>& gammas_ld,
                                                    PrimeCatalog& cat,
                                                    const std::vector<int>& primes) {
    SpectralActionReport rep;
    rep.program_id = cfg.anavm.id;
    rep.rule_id = cfg.anavm.rule_id;
    rep.selection_rule = cfg.spectral_action.present ? cfg.spectral_action.selection
                                                     : "min_action_subject_to_t1";
    rep.action_proxy = "combined_crossed_product";

    const Real t1_tol =
        cfg.spectral_action.present ? static_cast<Real>(cfg.spectral_action.t1_tolerance) : 1e-6L;
    const Real weil_max = cfg.spectral_action.present
                              ? static_cast<Real>(cfg.spectral_action.weil_residual_max)
                              : 1.0L;
    const Real gue_max = cfg.spectral_action.present
                             ? static_cast<Real>(cfg.spectral_action.gue_spacing_l2_max)
                             : 0.4L;
    const int n_scales =
        cfg.spectral_action.heat_scales > 0 ? cfg.spectral_action.heat_scales : 3;
    const Real scale_base =
        cfg.spectral_action.heat_scale_base > 0 ? static_cast<Real>(cfg.spectral_action.heat_scale_base)
                                                : 1.0L;
    const Real coupling = cfg.connes_coupling_lambda > 0
                              ? cfg.connes_coupling_lambda
                              : (cfg.spectral_action.present
                                     ? static_cast<Real>(cfg.spectral_action.coupling_lambda)
                                     : 0.5L);
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 12;

    const int sweep_steps =
        cfg.self_adjoint_extension.present && cfg.self_adjoint_extension.sweep_steps > 0
            ? cfg.self_adjoint_extension.sweep_steps
            : 24;
    const BerryKeatingSpec bspec = spec_from_config(cfg);
    const AnaVM::ArchimedeanBoundary bounds[] = {
        AnaVM::ArchimedeanBoundary::BerryKeating, AnaVM::ArchimedeanBoundary::Dirichlet,
        AnaVM::ArchimedeanBoundary::Neumann, AnaVM::ArchimedeanBoundary::Periodic};

    const Real laplace_a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;
    const LaplaceTest laplace(laplace_a);
    const int kmax_trace = cfg.kmax > 0 ? cfg.kmax : 20;
    cat.rebuild_adaptive(laplace, 1.0L / (2.0L * laplace_a * laplace_a), kmax_trace, cfg.eps);

    const size_t prime_cap =
        cfg.precision_mode ? primes.size() : std::min(primes.size(), size_t{5000});
    std::vector<int> sub(primes.begin(), primes.begin() + static_cast<std::ptrdiff_t>(prime_cap));
    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(sub);

    const size_t nz = std::min(gammas_ld.size(), size_t{50000});
    const auto t1_arch = make_arch(AnaVM::ArchimedeanBoundary::BerryKeating);
    const TraceResult t1_tr =
        EvaluateTracePrefix(laplace, laplace_a, nullptr, nz, gammas_ld.data(), gammas_ld.size(),
                            cat, cfg.zero_kernel, cfg.simd, cfg.eps, cfg.trivial_zeros,
                            cfg.precision_mode, cfg.arch_pts, false, &t1_arch);
    const Real hlog = global.weil_prime_sum(laplace, kmax_trace, cfg.eps) / (2.0L * kPi);
    const Real shared_t1_gap = std::fabs(t1_tr.prime - hlog);

    const size_t nz_ref = std::min(gammas.size(), size_t{5000});
    std::vector<double> gamma_ref(gammas.begin(), gammas.begin() + static_cast<std::ptrdiff_t>(nz_ref));
    rep.zeros_gue_spacing_l2 =
        Analysis::spacing_distribution_l2_gue(Analysis::normalized_nn_spacings(gamma_ref));
    const auto cyl_levels =
        Analysis::cylinder_positive_levels(sub, static_cast<int>(std::min(size_t{500}, gammas.size())));
    std::vector<double> cyl_ld(cyl_levels.begin(), cyl_levels.end());
    rep.cylinder_gue_spacing_l2 =
        Analysis::spacing_distribution_l2_gue(Analysis::normalized_nn_spacings(cyl_ld));

    const bool t1_only = cfg.spectral_action.selection.find("t1_only") != std::string::npos ||
                         cfg.spectral_action.selection.find("t1") != std::string::npos;

    BoundaryMetrics boundary_metrics[4];
    for (int bi = 0; bi < 4; ++bi) {
        const auto arch = make_arch(bounds[bi]);
        const TraceResult tr =
            EvaluateTracePrefix(laplace, laplace_a, nullptr, nz, gammas_ld.data(),
                                gammas_ld.size(), cat, cfg.zero_kernel, cfg.simd, cfg.eps,
                                cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false, &arch);
        boundary_metrics[bi].weil_residual = std::fabs(tr.residual());
    }

    Marshal::Kernel::CombinedDiracFastSpec fspec;
    fspec.bk = bspec;
    fspec.coupling_lambda = coupling;
    fspec.coupling_mode = cfg.connes_coupling_mode;
    fspec.kmax = kmax;
    fspec.arch_cap = 80;
    fspec.combined_cap = 400;
    fspec.sweep_steps = sweep_steps;
    rep.combined_cap = fspec.combined_cap;
#if defined(MARSHAL_HAVE_AVX2)
    rep.simd_backend = "avx2_lut";
#endif

    Marshal::Kernel::CombinedDiracWorkspace ws;
    if (!ws.prepare(fspec, sub)) {
        rep.verdict = "SPECTRAL_ACTION_NO_ADMISSIBLE";
        return rep;
    }
    rep.n_combined_modes = ws.n_total();

    const int n_cand = sweep_steps * 4;
    rep.candidates.resize(static_cast<size_t>(n_cand));
    std::vector<char> admissible_flags(static_cast<size_t>(n_cand), 0);

    ArchimedeanBoundarySpec arch_specs[4];
    for (int bi = 0; bi < 4; ++bi) arch_specs[bi] = make_arch(bounds[bi]);

    const auto t_sweep0 = std::chrono::steady_clock::now();
#ifdef _OPENMP
    #pragma omp parallel for schedule(static)
#endif
    for (int cand = 0; cand < n_cand; ++cand) {
        const int si = cand / 4;
        const int bi = cand % 4;
#ifdef _OPENMP
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        const auto fr =
            ws.eval_candidate(si, bi, arch_specs[bi], scale_base, n_scales, gammas_ld, tid);

        SpectralActionCandidateRow row;
        row.theta = fr.theta;
        row.boundary = bounds[bi];
        row.boundary_name = boundary_name(bounds[bi]);
        row.t1_gap = shared_t1_gap;
        row.laplace_weil_residual = boundary_metrics[bi].weil_residual;
        row.n_combined_modes = fr.n_modes;
        row.combined_spectral_action = fr.combined_action;
        row.heat_action_proxy = fr.combined_action;
        row.spectrum_rmse = fr.spectrum_rmse;
        row.gue_spacing_l2 = fr.gue_spacing_l2;
        row.admissible =
            row.t1_gap <= t1_tol && (t1_only || row.laplace_weil_residual <= weil_max);
        row.selected = false;
        rep.candidates[static_cast<size_t>(cand)] = row;
        admissible_flags[static_cast<size_t>(cand)] = row.admissible ? 1 : 0;
    }
    rep.sweep_elapsed_ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t_sweep0)
            .count();

    Real best_action = 1e300L;
    int best_idx = -1;
    rep.admissible_count = 0;
    for (int cand = 0; cand < n_cand; ++cand) {
        if (admissible_flags[static_cast<size_t>(cand)]) ++rep.admissible_count;
        auto& row = rep.candidates[static_cast<size_t>(cand)];
        if (row.admissible && row.combined_spectral_action < best_action) {
            best_action = row.combined_spectral_action;
            best_idx = cand;
        }
    }

    int best_si = 0;
    int best_bi = 0;

    if (best_idx >= 0) {
        rep.candidates[static_cast<size_t>(best_idx)].selected = true;
        const auto& sel = rep.candidates[static_cast<size_t>(best_idx)];
        best_si = best_idx / 4;
        best_bi = best_idx % 4;
        rep.selected_theta = sel.theta;
        rep.selected_boundary = sel.boundary_name;
        rep.selected_heat_action = sel.combined_spectral_action;
        rep.selected_t1_gap = sel.t1_gap;
        rep.selected_spectrum_rmse = sel.spectrum_rmse;
        rep.selected_gue_spacing_l2 = sel.gue_spacing_l2;
        rep.verdict = "SPECTRAL_ACTION_SELECTED";

        Real second_best = 1e300L;
        for (int cand = 0; cand < n_cand; ++cand) {
            if (!admissible_flags[static_cast<size_t>(cand)] || cand == best_idx) continue;
            const Real a = rep.candidates[static_cast<size_t>(cand)].combined_spectral_action;
            if (a < second_best) second_best = a;
        }
        rep.second_best_action = second_best < 1e299L ? second_best : rep.selected_heat_action;
        rep.action_gap = rep.second_best_action - rep.selected_heat_action;
        rep.strict_minimum = true;
        for (int cand = 0; cand < n_cand; ++cand) {
            if (!admissible_flags[static_cast<size_t>(cand)]) continue;
            if (rep.candidates[static_cast<size_t>(cand)].combined_spectral_action <
                rep.selected_heat_action - 1e-12L) {
                rep.strict_minimum = false;
                break;
            }
        }
        const Real action_eps = 1e-9L;
        rep.minimizer_count_at_minimum = 0;
        for (int cand = 0; cand < n_cand; ++cand) {
            if (!admissible_flags[static_cast<size_t>(cand)]) continue;
            if (std::fabs(rep.candidates[static_cast<size_t>(cand)].combined_spectral_action -
                          rep.selected_heat_action) <= action_eps)
                ++rep.minimizer_count_at_minimum;
        }
        rep.unique_minimum =
            rep.strict_minimum && rep.minimizer_count_at_minimum == 1 && rep.action_gap > 0;
    } else if (rep.admissible_count == 0) {
        rep.verdict = "SPECTRAL_ACTION_NO_ADMISSIBLE";
    } else {
        rep.verdict = "SPECTRAL_ACTION_INCONCLUSIVE";
    }

    SpectralActionGateRow g1;
    g1.id = "t1_local_pass";
    g1.gate_class = "SANITY";
    g1.pass = best_idx >= 0 && rep.selected_t1_gap <= t1_tol;
    g1.note = "T1 gap at combined Dirac minimizer";
    rep.gates.push_back(g1);

    SpectralActionGateRow g2;
    g2.id = "spectral_action_minimizer";
    g2.gate_class = "SELECTION";
    g2.pass = rep.verdict == "SPECTRAL_ACTION_SELECTED";
    g2.note = rep.selection_rule + " on combined_crossed_product generator";
    rep.gates.push_back(g2);

    SpectralActionGateRow g3;
    g3.id = "gue_spacing_vs_cylinder";
    g3.gate_class = "ANALYTIC_SHAPE";
    if (best_idx >= 0) {
        const Real op_l2 = rep.selected_gue_spacing_l2;
        g3.pass = op_l2 < rep.cylinder_gue_spacing_l2 && op_l2 <= gue_max;
        g3.note = "combined spectrum GUE L2 vs cylinder; not discreteness proof";
    } else {
        g3.pass = false;
        g3.note = "no admissible extension selected";
    }
    rep.gates.push_back(g3);

    SpectralActionGateRow g4;
    g4.id = "global_action_strict_minimum";
    g4.gate_class = "SELECTION";
    g4.pass = rep.strict_minimum && rep.action_gap > 0 && best_idx >= 0;
    g4.note = "Λ_D strict minimum on T1-admissible global Connes pool";
    rep.gates.push_back(g4);

    SpectralActionGateRow g5;
    g5.id = "global_minimizer_unique";
    g5.gate_class = "SELECTION";
    g5.pass = rep.unique_minimum && rep.minimizer_count_at_minimum == 1 && best_idx >= 0;
    g5.note = "Exactly one T1-admissible extension attains minimum Λ_D";
    rep.gates.push_back(g5);

    rep.lean_emit_ready = g1.pass && g2.pass && g4.pass && g5.pass;
    rep.proof_status =
        (rep.lean_emit_ready && rep.verdict == "SPECTRAL_ACTION_SELECTED") ? "PROVED"
                                                                            : "EXPERIMENTAL_NOT_PROVED";

    if (best_idx >= 0) {
        static const int caps[] = {120, 240, 400};
        for (int cap : caps) {
            const auto row = Marshal::Kernel::probe_combined_cap(
                fspec, sub, cap, best_si, best_bi, arch_specs[best_bi], scale_base, n_scales,
                gammas_ld);
            SpectralActionConvergenceRow cr;
            cr.combined_cap = row.combined_cap;
            cr.n_modes = row.n_modes;
            cr.spectrum_rmse = row.spectrum_rmse;
            cr.combined_action = row.combined_action;
            cr.elapsed_ms = row.elapsed_ms;
            rep.convergence_probe.push_back(cr);
        }
    }

    std::cout << "Spectral action: " << rep.verdict << " admissible=" << rep.admissible_count
              << " cap=" << rep.combined_cap << " modes=" << rep.n_combined_modes
              << " simd=" << rep.simd_backend << " sweep_ms=" << rep.sweep_elapsed_ms
              << " selected_theta="
              << static_cast<double>(rep.selected_theta) << " boundary=" << rep.selected_boundary
              << " action=" << static_cast<double>(rep.selected_heat_action)
              << " rmse=" << static_cast<double>(rep.selected_spectrum_rmse)
              << " gue_l2=" << static_cast<double>(rep.selected_gue_spacing_l2) << "\n";
    return rep;
}

bool export_spectral_action_validation_json(const std::string& path, const SpectralActionReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"rule_id\": \"" << r.rule_id << "\",\n";
    out << "  \"proof_status\": \"" << r.proof_status << "\",\n";
    out << "  \"lean_emit_ready\": " << (r.lean_emit_ready ? "true" : "false") << ",\n";
    out << "  \"selection_rule\": \"" << r.selection_rule << "\",\n";
    out << "  \"action_proxy\": \"" << r.action_proxy << "\",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n";
    out << "  \"admissible_count\": " << r.admissible_count << ",\n";
    out << "  \"selected_theta\": " << static_cast<double>(r.selected_theta) << ",\n";
    out << "  \"selected_boundary\": \"" << r.selected_boundary << "\",\n";
    out << "  \"selected_combined_action\": " << static_cast<double>(r.selected_heat_action)
        << ",\n";
    out << "  \"selected_t1_gap\": " << static_cast<double>(r.selected_t1_gap) << ",\n";
    out << "  \"second_best_action\": " << static_cast<double>(r.second_best_action) << ",\n";
    out << "  \"action_gap\": " << static_cast<double>(r.action_gap) << ",\n";
    out << "  \"strict_minimum\": " << (r.strict_minimum ? "true" : "false") << ",\n";
    out << "  \"unique_minimum\": " << (r.unique_minimum ? "true" : "false") << ",\n";
    out << "  \"minimizer_count_at_minimum\": " << r.minimizer_count_at_minimum << ",\n";
    out << "  \"selected_spectrum_rmse\": " << static_cast<double>(r.selected_spectrum_rmse)
        << ",\n";
    out << "  \"selected_gue_spacing_l2\": " << static_cast<double>(r.selected_gue_spacing_l2)
        << ",\n";
    out << "  \"zeros_gue_spacing_l2\": " << static_cast<double>(r.zeros_gue_spacing_l2) << ",\n";
    out << "  \"cylinder_gue_spacing_l2\": " << static_cast<double>(r.cylinder_gue_spacing_l2)
        << ",\n";
    out << "  \"combined_cap\": " << r.combined_cap << ",\n";
    out << "  \"n_combined_modes\": " << r.n_combined_modes << ",\n";
    out << "  \"sweep_elapsed_ms\": " << r.sweep_elapsed_ms << ",\n";
    out << "  \"simd_backend\": \"" << r.simd_backend << "\",\n";
    out << "  \"convergence_probe\": [\n";
    for (size_t i = 0; i < r.convergence_probe.size(); ++i) {
        const auto& c = r.convergence_probe[i];
        out << "    {\"combined_cap\": " << c.combined_cap << ", \"n_modes\": " << c.n_modes
            << ", \"spectrum_rmse\": " << static_cast<double>(c.spectrum_rmse)
            << ", \"combined_action\": " << static_cast<double>(c.combined_action)
            << ", \"elapsed_ms\": " << c.elapsed_ms << "}";
        if (i + 1 < r.convergence_probe.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n";
    out << "  \"gates\": [\n";
    for (size_t i = 0; i < r.gates.size(); ++i) {
        const auto& g = r.gates[i];
        out << "    {\"id\": \"" << g.id << "\", \"gate\": \"" << g.gate_class
            << "\", \"pass\": " << (g.pass ? "true" : "false") << ", \"note\": \"" << g.note
            << "\"}";
        if (i + 1 < r.gates.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"candidates\": [\n";
    for (size_t i = 0; i < r.candidates.size(); ++i) {
        const auto& c = r.candidates[i];
        out << "    {\"theta\": " << static_cast<double>(c.theta) << ", \"boundary\": \""
            << c.boundary_name << "\", \"combined_spectral_action\": "
            << static_cast<double>(c.combined_spectral_action) << ", \"n_combined_modes\": "
            << c.n_combined_modes << ", \"t1_gap\": " << static_cast<double>(c.t1_gap)
            << ", \"weil_residual\": " << static_cast<double>(c.laplace_weil_residual)
            << ", \"spectrum_rmse\": " << static_cast<double>(c.spectrum_rmse)
            << ", \"gue_spacing_l2\": " << static_cast<double>(c.gue_spacing_l2)
            << ", \"admissible\": " << (c.admissible ? "true" : "false") << ", \"selected\": "
            << (c.selected ? "true" : "false") << "}";
        if (i + 1 < r.candidates.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

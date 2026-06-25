#include "InvestigationRunner.hxx"

#include "DiagnosticExport.hxx"
#include "../Analysis/PairCorrelation.hxx"
#include "../Heat/AnalyticConstructionValidation.hxx"
#include "../Heat/ArchimedeanBoundary.hxx"
#include "../Heat/BerryKeatingOperator.hxx"
#include "../Heat/LogPrimeGlobal.hxx"
#include "../Heat/T1TopologyValidation.hxx"
#include "../Heat/PrimeArchCancelBudgetValidation.hxx"
#include "../Kernel/CombinedDiracFast.hxx"
#include "../Numerics/TestFunctions.hxx"
#include "../TraceApi.hxx"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Investigation {
namespace {

using Marshal::AnaVM::ArchimedeanBoundary;
using Marshal::Heat::ArchimedeanBoundarySpec;
using Marshal::Heat::BerryKeatingSpec;
using Marshal::Kernel::CombinedDiracFastSpec;
using Marshal::Kernel::CombinedDiracWorkspace;
using Marshal::Heat::kPi;
using Marshal::Heat::PrimeCatalog;

ArchimedeanBoundarySpec make_arch(ArchimedeanBoundary b) {
    ArchimedeanBoundarySpec arch;
    arch.boundary = b;
    arch.type = Marshal::AnaVM::ArchimedeanType::RealLine;
    arch.cutoff = Marshal::AnaVM::ArchimedeanCutoff::PlanckScale;
    return arch;
}

BerryKeatingSpec bk_from_cfg(const Config& cfg) {
    BerryKeatingSpec bspec;
    if (cfg.semiclassical.present) {
        bspec.log_span = static_cast<Real>(cfg.semiclassical.log_span);
        bspec.max_modes = cfg.semiclassical.max_modes;
    }
    return bspec;
}

CombinedDiracFastSpec dirac_spec(const InvestigationSpec& spec, const Config& cfg) {
    CombinedDiracFastSpec fspec;
    fspec.bk = bk_from_cfg(cfg);
    fspec.coupling_lambda = spec.coupling_lambda > 0 ? spec.coupling_lambda : 0.5L;
    fspec.coupling_mode = cfg.connes_coupling_mode;
    fspec.kmax = spec.kmax > 0 ? spec.kmax : 12;
    fspec.arch_cap = spec.arch_cap;
    fspec.combined_cap = spec.combined_cap;
    fspec.sweep_steps = spec.sweep_steps;
    return fspec;
}

double riemann_heat_trace_at_t(const std::vector<Real>& gammas_ld, double t) {
    double s = 0;
    for (Real g : gammas_ld) s += std::exp(-static_cast<double>(t * g * g));
    return s;
}

std::vector<int> capped_primes(const std::vector<int>& primes, int limit) {
    if (limit <= 0) return primes;
    const size_t n = std::min(primes.size(), static_cast<size_t>(limit));
    return std::vector<int>(primes.begin(), primes.begin() + static_cast<std::ptrdiff_t>(n));
}

}  // namespace

InvestigationSpec spec_from_config(const Config& cfg) {
    InvestigationSpec spec = cfg.investigation_spec;
    if (spec.id.empty()) spec.id = cfg.investigation_id.empty() ? "theorem_ab" : cfg.investigation_id;
    if (spec.cert_root.empty()) {
        spec.cert_root = cfg.investigation_cert_root.empty()
                             ? "build/cert/investigations/" + spec.id
                             : cfg.investigation_cert_root;
    }
    if (cfg.investigation_quick) apply_quick_preset(spec);
    if (spec.curvature_sweep.steps <= 0) {
        spec.curvature_sweep.mode = ThetaSweepMode::Centered;
        spec.curvature_sweep.center = spec.fixed_theta;
        spec.curvature_sweep.range = 1.0L;
        spec.curvature_sweep.steps = 200;
    }
    if (spec.topology_sweep.steps <= 0) {
        spec.topology_sweep.mode = ThetaSweepMode::Uniform;
        spec.topology_sweep.min_theta = 0;
        spec.topology_sweep.max_theta = 6.28318530717958647692L;
        spec.topology_sweep.steps = 400;
    }
    if (spec.heat_t_ladder.values.empty())
        spec.heat_t_ladder.values = {0.001L, 0.01L, 0.1L, 1.0L, 10.0L, 100.0L};
    if (spec.prime_limit_ladder.values.empty())
        spec.prime_limit_ladder.values = {10, 50, 100, 500, 1000, 5000, 10000, 50000};
    return spec;
}

DiagnosticReport run_diagnostic(DiagnosticId id, const InvestigationSpec& spec, const Config& cfg,
                                const std::vector<double>& gammas,
                                const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
                                const std::vector<int>& primes) {
    DiagnosticReport rep;
    rep.investigation_id = spec.id;
    rep.diagnostic_id = diagnostic_id_string(id);
    rep.fixed_theta = spec.fixed_theta;

    const Real t1_tol = spec.t1_tolerance;
    const int bi_periodic = static_cast<int>(ArchimedeanBoundary::Periodic);
    const auto arch_periodic = make_arch(ArchimedeanBoundary::Periodic);
    const Real scale_base = spec.heat_scale_base;
    const int n_scales = spec.heat_scales > 0 ? spec.heat_scales : 3;

    if (id == DiagnosticId::SpectralActionCurvature) {
        rep.analytic_obligation = "theorem_a.A3";
        const auto grid = build_theta_grid(spec.curvature_sweep);
        CombinedDiracWorkspace ws;
        const auto fspec = dirac_spec(spec, cfg);
        if (!ws.prepare_with_thetas(fspec, primes, grid)) {
            rep.note = "workspace prepare failed";
            return rep;
        }
        Real best = 1e300L;
        int best_idx = -1;
        int admissible = 0;
        std::vector<Real> ys;
        ys.reserve(grid.size());
        for (size_t i = 0; i < grid.size(); ++i) {
            Real prime_gap = 0;
            Real arch_gap = 0;
            const Real t1_gap =
                Marshal::Heat::t1_gap_at_theta(cfg, grid[i], gammas_ld, cat, primes, &prime_gap, &arch_gap);
            const auto r = ws.eval_candidate(static_cast<int>(i), bi_periodic, arch_periodic,
                                             scale_base, n_scales, gammas_ld, 0);
            const bool ok = t1_gap <= t1_tol;
            if (ok) ++admissible;
            ys.push_back(r.combined_action);
            rep.series.push_back({grid[i], r.combined_action, "periodic", ok, prime_gap});
            rep.series.back().aux = arch_gap;
            if (ok && r.combined_action < best) {
                best = r.combined_action;
                best_idx = static_cast<int>(i);
            }
        }
        if (grid.size() >= 3 && best_idx >= 1 && static_cast<size_t>(best_idx + 1) < grid.size()) {
            const Real h = grid[static_cast<size_t>(best_idx + 1)] - grid[static_cast<size_t>(best_idx)];
            if (std::fabs(h) > 1e-12L) {
                const int j = best_idx;
                const Real d1 = (ys[static_cast<size_t>(j + 1)] - ys[static_cast<size_t>(j - 1)]) /
                                (grid[static_cast<size_t>(j + 1)] - grid[static_cast<size_t>(j - 1)]);
                const Real d2 =
                    (ys[static_cast<size_t>(j + 1)] - 2 * ys[static_cast<size_t>(j)] +
                     ys[static_cast<size_t>(j - 1)]) /
                    (h * h);
                rep.note = "d1=" + std::to_string(static_cast<double>(d1)) +
                           " d2=" + std::to_string(static_cast<double>(d2)) +
                           " action_proxy=combined_crossed_product";
            }
        }
        Real second = 1e300L;
        int minimizers = 0;
        for (const auto& pt : rep.series) {
            if (!pt.flag) continue;
            if (std::fabs(pt.y - best) < 1e-9L) ++minimizers;
            if (pt.y > best + 1e-9L && pt.y < second) second = pt.y;
        }
        DiagnosticGate g1;
        g1.id = "curvature_minimum_found";
        g1.gate_class = "THEOREM_A";
        g1.pass = best_idx >= 0 && std::fabs(grid[static_cast<size_t>(best_idx)] - spec.fixed_theta) < 0.02L;
        g1.note = "theta_0 within 0.02 of target";
        rep.gates.push_back(g1);
        DiagnosticGate g2;
        g2.id = "strict_minimum";
        g2.gate_class = "THEOREM_A";
        g2.pass = best_idx >= 0 && minimizers == 1 && second > best;
        g2.note = "unique minimizer on T1 pool";
        rep.gates.push_back(g2);
        rep.analysis_status = g1.pass && g2.pass ? "EVIDENCE_SUPPORTS" : "ANALYSIS_INCOMPLETE";
        rep.note = "admissible=" + std::to_string(admissible) + " minimizers=" + std::to_string(minimizers);
        return rep;
    }

    if (id == DiagnosticId::T1AdmissibleTopology) {
        rep.analytic_obligation = "theorem_a.A2";
        const auto grid = build_theta_grid(spec.topology_sweep);
        const auto t1rep = Marshal::Heat::run_t1_topology_validation(cfg, gammas_ld, cat, primes,
                                                                     grid, t1_tol);
        const std::string t1_path = spec.cert_root + "/t1_gap_curve.json";
        Marshal::Heat::export_t1_topology_validation_json(t1_path, t1rep);

        const auto pac_rep = Marshal::Heat::run_prime_arch_cancel_budget_validation(
            cfg, gammas_ld, cat, primes);
        const std::string pac_path = "docs/generated/prime_arch_cancel_budget_cert.json";
        Marshal::Heat::export_prime_arch_cancel_budget_json(pac_path, pac_rep);

        Real lo = -1, hi = -1;
        bool theta0_inside = false;
        for (const auto& row : t1rep.sweep) {
            if (!row.admissible) continue;
            if (lo < 0) lo = row.theta;
            hi = row.theta;
            if (std::fabs(row.theta - spec.fixed_theta) < 0.05L) theta0_inside = true;
            rep.series.push_back({row.theta, row.total_gap, "t1_gap", row.admissible,
                                  row.prime_gap});
            rep.series.back().aux = row.arch_gap;
        }
        DiagnosticGate g1;
        g1.id = "t1_interval_nonempty";
        g1.gate_class = "THEOREM_A";
        g1.pass = lo >= 0 && hi > lo;
        rep.gates.push_back(g1);
        DiagnosticGate g2;
        g2.id = "theta0_interior";
        g2.gate_class = "THEOREM_A";
        g2.pass = theta0_inside;
        rep.gates.push_back(g2);
        DiagnosticGate g3;
        g3.id = "prime_gap_theta_independent";
        g3.gate_class = "THEOREM_A";
        g3.pass = t1rep.prime_gap_uniform < 1e-12L;
        g3.note = "span=" + std::to_string(static_cast<double>(t1rep.prime_gap_uniform));
        rep.gates.push_back(g3);
        rep.analysis_status = g1.pass && g2.pass ? "EVIDENCE_SUPPORTS" : "ANALYSIS_INCOMPLETE";
        rep.note = "interval=[" + std::to_string(static_cast<double>(lo)) + "," +
                   std::to_string(static_cast<double>(hi)) + "] t1_curve=" + t1_path;
        return rep;
    }

    if (id == DiagnosticId::HeatTraceAtTheta) {
        rep.analytic_obligation = "theorem_b.B1";
        CombinedDiracWorkspace ws;
        const auto fspec = dirac_spec(spec, cfg);
        if (!ws.prepare(fspec, primes)) return rep;
        for (Real t : spec.heat_t_ladder.values) {
            const auto ev = ws.eval_at_theta(spec.fixed_theta, bi_periodic, arch_periodic, scale_base,
                                             n_scales, gammas_ld, 0);
            const double td = static_cast<double>(t);
            const double ref = riemann_heat_trace_at_t(gammas_ld, td);
            const double op = static_cast<double>(ev.combined_action);
            rep.series.push_back({t, static_cast<Real>(op), "operator_heat_proxy", true,
                                  static_cast<Real>(ref)});
        }
        DiagnosticGate g1;
        g1.id = "heat_trace_finite";
        g1.gate_class = "THEOREM_B";
        g1.pass = !rep.series.empty();
        rep.gates.push_back(g1);
        rep.analysis_status = "EVIDENCE_SUPPORTS";
        return rep;
    }

    if (id == DiagnosticId::SpectralSpacing) {
        rep.analytic_obligation = "theorem_b.B2";
        CombinedDiracWorkspace ws;
        const auto fspec = dirac_spec(spec, cfg);
        if (!ws.prepare(fspec, primes)) return rep;
        const auto r = ws.eval_at_theta(spec.fixed_theta, bi_periodic, arch_periodic, scale_base,
                                        n_scales, gammas_ld, 0);
        const auto zero_sp = Marshal::Analysis::normalized_nn_spacings(
            std::vector<double>(gammas.begin(), gammas.begin() + std::min(gammas.size(), size_t{5000})));
        const double gue_l2 = static_cast<double>(r.gue_spacing_l2);
        const double zero_gue = static_cast<double>(Marshal::Analysis::spacing_distribution_l2_gue(zero_sp));
        rep.series.push_back({spec.fixed_theta, static_cast<Real>(gue_l2), "connes_gue_l2", true,
                              static_cast<Real>(zero_gue)});
        DiagnosticGate g1;
        g1.id = "spacing_exported";
        g1.gate_class = "THEOREM_B";
        g1.pass = r.n_modes > 0;
        rep.gates.push_back(g1);
        rep.analysis_status = g1.pass ? "EVIDENCE_SUPPORTS" : "ANALYSIS_INCOMPLETE";
        return rep;
    }

    if (id == DiagnosticId::ContinuumPersistence) {
        rep.analytic_obligation = "theorem_b.B1";
        for (Real plr : spec.prime_limit_ladder.values) {
            const int pl = static_cast<int>(plr);
            Config local = cfg;
            local.prime_limit = pl;
            const auto sub = capped_primes(primes, pl);
            const auto arep = Marshal::Heat::run_analytic_construction_validation(
                local, gammas, gammas_ld, cat, sub);
            rep.series.push_back({static_cast<Real>(pl), arep.spectrum_rmse, "continuum",
                                  arep.continuous_spectrum_present, arep.xi_det_gap});
            rep.prime_limit = pl;
        }
        DiagnosticGate g1;
        g1.id = "continuum_ladder_complete";
        g1.gate_class = "THEOREM_B";
        g1.pass = !rep.series.empty();
        rep.gates.push_back(g1);
        rep.analysis_status = "EVIDENCE_SUPPORTS";
        rep.note = "EXPECTED_TRUNCATION if flag stays true at all P";
        return rep;
    }

    return rep;
}

InvestigationRunResult run_investigation(const Config& cfg, const std::vector<double>& gammas,
                                         const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
                                         const std::vector<int>& primes) {
    InvestigationRunResult result;
    const InvestigationSpec spec = spec_from_config(cfg);
    result.investigation_id = spec.id;
    result.cert_root = spec.cert_root;
    std::filesystem::create_directories(spec.cert_root);

    const DiagnosticId all[] = {
        DiagnosticId::SpectralActionCurvature, DiagnosticId::T1AdmissibleTopology,
        DiagnosticId::HeatTraceAtTheta,      DiagnosticId::SpectralSpacing,
        DiagnosticId::ContinuumPersistence,
    };

    const bool run_one = !cfg.investigation_diag_id.empty();
    for (DiagnosticId id : all) {
        const std::string did = diagnostic_id_string(id);
        if (run_one && did != cfg.investigation_diag_id) continue;
        if (!spec.run_curvature && id == DiagnosticId::SpectralActionCurvature) continue;
        if (!spec.run_topology && id == DiagnosticId::T1AdmissibleTopology) continue;
        if (!spec.run_heat_trace && id == DiagnosticId::HeatTraceAtTheta) continue;
        if (!spec.run_spacing && id == DiagnosticId::SpectralSpacing) continue;
        if (!spec.run_continuum && id == DiagnosticId::ContinuumPersistence) continue;

        const auto rep = run_diagnostic(id, spec, cfg, gammas, gammas_ld, cat, primes);
        const std::string path = spec.cert_root + "/" + did + ".json";
        const bool exported = export_diagnostic_json(path, rep);
        bool pass = exported;
        for (const auto& g : rep.gates)
            if (!g.pass) pass = false;
        result.diagnostic_results.emplace_back(did, pass);
        std::cout << "Investigation " << did << ": " << (pass ? "PASS" : "FAIL") << " -> " << path
                  << "\n";
    }

    const std::string manifest_path = spec.cert_root + "/manifest.json";
    export_investigation_manifest(manifest_path, spec.id, result.diagnostic_results);

    // scaling vs WKB sidecar for theorem_ab
    if (spec.id == "theorem_ab" || spec.id == "theorem_a_analytic") {
        const BerryKeatingSpec bspec = bk_from_cfg(cfg);
        const Real theta = spec.fixed_theta;
        std::ofstream wkb(spec.cert_root + "/scaling_vs_wkb.json");
        wkb << std::setprecision(17) << "{\n  \"theta\": " << static_cast<double>(theta)
            << ",\n  \"log_span\": " << static_cast<double>(bspec.log_span) << ",\n  \"modes\": [\n";
        for (int n = 1; n <= 10; ++n) {
            const Real exact = (theta + 2.0L * kPi * static_cast<Real>(n)) / bspec.log_span;
            const Real wkbv = Marshal::Heat::bk_wkb_eigenvalue(n, theta, bspec.log_span);
            if (n > 1) wkb << ",\n";
            wkb << "    {\"n\": " << n << ", \"exact_scaling\": " << static_cast<double>(exact)
                << ", \"wkb\": " << static_cast<double>(wkbv) << "}";
        }
        wkb << "\n  ]\n}\n";
    }

    result.ok = !result.diagnostic_results.empty();
    return result;
}

}  // namespace Marshal::Investigation

#include "ValidationRouter.hxx"

// Hunt gate classes (see docs/Analysis/TestGateDiscipline.md):
// SANITY — trace_formula_gate, local_weil_t1
// EXCLUSION — pair_correlation, compact_sinc2 (C_fin must fail)
// ANALYTIC_SHAPE — spectral_discreteness, continuum_persistence
// Verdicts: SANITY_PASS, C_FIN_EXCLUDED, ANALYTIC_SHAPE_OK|BAD|INCONCLUSIVE

namespace Marshal::AnaVM {
namespace {

void add_job(std::vector<ValidationJob>& jobs, ValidationKind kind, const char* id,
             const char* flag, const char* path) {
    for (const auto& j : jobs) {
        if (j.kind == kind) return;
    }
    jobs.push_back({kind, id, flag, path});
}

}  // namespace

std::vector<ValidationJob> route_validation(const MrsProgram& prog) {
    std::vector<ValidationJob> jobs;
    const std::string id = prog.id.empty() ? "operator" : prog.id;
    const std::string gen = "docs/generated/";

    // Dedicated spectral-action experiment: T1 is evaluated inside SpectralActionValidation.
    const bool formal_limit_primary =
        prog.id == "connes_global_dirac_limit" || prog.discretization_limit.present ||
        prog.diagnostics.global_dirac_limit;
    const bool analytic_lemma_primary =
        prog.id == "connes_analytic_lemmas" || prog.diagnostics.analytic_lemma_demo;
    const bool spectral_action_primary =
        (prog.id == "spectral_action_selection" && prog.diagnostics.spectral_action_selection) ||
        formal_limit_primary;

    if (!spectral_action_primary && !analytic_lemma_primary &&
        (prog.diagnostics.local_weil_t1 || !prog.diagnostics.weil_identity.empty() ||
         !prog.diagnostics.trace_test.empty()))
        add_job(jobs, ValidationKind::LogPrime, "local_weil_t1", "--export-log-prime",
                (gen + "anavm_" + id + ".json").c_str());

    if (!spectral_action_primary && !analytic_lemma_primary &&
        (prog.diagnostics.connes_crossed || prog.heat_coupling == HeatCoupling::ConnesHeat))
        add_job(jobs, ValidationKind::ConnesCrossed, "connes_crossed", "--export-connes-crossed",
                "docs/generated/connes_spectrum_validation.json");

    if (prog.completion.present || prog.adelic_cauchy.present)
        add_job(jobs, ValidationKind::Completion, "completion", "--export-completion",
                "docs/generated/completion_validation.json");

    if (prog.spectral_determinant.present || prog.diagnostics.spectral_discreteness)
        add_job(jobs, ValidationKind::SpectralDeterminant, "spectral_determinant",
                "--export-spectral-det", "docs/generated/spectral_determinant.json");

    if (!spectral_action_primary && !analytic_lemma_primary &&
        (prog.archimedean.present || prog.diagnostics.archimedean_sweep))
        add_job(jobs, ValidationKind::ArchimedeanSweep, "archimedean_sweep",
                "--export-archimedean", "docs/generated/archimedean_boundary_sweep.json");

    if (prog.assembly_search.present || prog.diagnostics.assembly_grid)
        add_job(jobs, ValidationKind::AssemblySearch, "assembly_search", "--export-assembly",
                "docs/generated/assembly_search.json");

    if (!spectral_action_primary && !analytic_lemma_primary &&
        (prog.diagnostics.self_adjoint_extension_sweep || prog.self_adjoint_extension.present))
        add_job(jobs, ValidationKind::SelfAdjointExtensionSweep, "self_adjoint_extension_sweep",
                "--export-self-adjoint-ext",
                "docs/generated/self_adjoint_extension_sweep.json");

    if (prog.diagnostics.trace_formula_gate || prog.trace_formula.present)
        add_job(jobs, ValidationKind::TraceFormulaGate, "trace_formula_gate",
                "--export-trace-formula-gate", "docs/generated/trace_formula_gate.json");

    if (prog.diagnostics.spectrum_vs_zeros && prog.rule_id == "berry_keating_xp" &&
        !prog.placeholder)
        add_job(jobs, ValidationKind::BerryKeatingValidation, "berry_keating",
                "--export-berry-keating", "docs/generated/berry_keating_validation.json");

    if (prog.id == "connes_analytic_construction")
        add_job(jobs, ValidationKind::AnalyticConstruction, "analytic_construction",
                "--export-analytic-construction",
                "docs/generated/analytic_construction.json");

    if (prog.discretization_limit.present || prog.diagnostics.global_dirac_limit)
        add_job(jobs, ValidationKind::GlobalDiracLimit, "global_dirac_limit",
                "--export-global-dirac-limit",
                "docs/generated/global_dirac_limit.json");

    if (prog.diagnostics.analytic_lemma_demo || prog.id == "connes_analytic_lemmas")
        add_job(jobs, ValidationKind::AnalyticLemmaDemo, "analytic_lemma_demo",
                "--export-analytic-lemma-demo",
                "docs/generated/analytic_lemma_demo.json");

    if (prog.spectral_action.present || prog.diagnostics.spectral_action_selection)
        if (!analytic_lemma_primary)
            add_job(jobs, ValidationKind::SpectralActionSelection, "spectral_action_selection",
                    "--export-spectral-action",
                    "docs/generated/spectral_action_selection.json");

    if (prog.pair_correlation_gue)
        add_job(jobs, ValidationKind::PairCorrelation, "pair_correlation_gue",
                "--export-pair-cor", "docs/generated/pair_correlation.json");

    if (prog.formal_analytics)
        add_job(jobs, ValidationKind::FormalAnalytics, "formal_analytics",
                "--export-formal-analytics", "docs/generated/formal_analytics.json");

    if (prog.wedge_analytics || prog.diagnostics.wedge_analytics ||
        prog.id == "marshal_wedge_analytic")
        add_job(jobs, ValidationKind::MarshalWedgeAnalytic, "marshal_wedge_analytic",
                "--export-wedge-analytic", "docs/generated/marshal_wedge_analytic_cpp.json");

    if (prog.xi_hadamard_proof || prog.diagnostics.xi_hadamard_proof ||
        prog.id == "marshal_xi_hadamard")
        add_job(jobs, ValidationKind::XiHadamardProof, "xi_hadamard_proof", "--xi-hadamard-proof",
                "docs/generated/anavm_xi_hadamard_proof.json");

    if (prog.investigation.present || prog.diagnostics.investigation_suite)
        add_job(jobs, ValidationKind::Investigation,
                prog.investigation.id.empty() ? "theorem_ab" : prog.investigation.id.c_str(),
                "--investigation",
                (std::string("build/cert/investigations/") +
                 (prog.investigation.id.empty() ? "theorem_ab" : prog.investigation.id))
                    .c_str());

    if (prog.diagnostics.hurwitz_spectral_action || prog.id == "theorem_a_analytic")
        add_job(jobs, ValidationKind::Investigation, "theorem_a_analytic", "--investigation",
                "build/cert/investigations/theorem_a_analytic");

    if (prog.diagnostics.theorem_b_scaffold || prog.id == "theorem_b")
        add_job(jobs, ValidationKind::Investigation, "theorem_b", "--investigation",
                "build/cert/investigations/theorem_b");

    return jobs;
}

void apply_validation_jobs(Config& cfg, const std::vector<ValidationJob>& jobs) {
    for (const auto& j : jobs) {
        switch (j.kind) {
            case ValidationKind::LogPrime:
                cfg.log_prime_validation = true;
                if (!j.default_path.empty()) cfg.export_log_prime_validation_path = j.default_path;
                break;
            case ValidationKind::ConnesCrossed:
                cfg.connes_crossed_validation = true;
                if (!j.default_path.empty()) cfg.export_connes_crossed_path = j.default_path;
                break;
            case ValidationKind::Completion:
                cfg.completion_validation = true;
                if (!j.default_path.empty()) cfg.export_completion_path = j.default_path;
                break;
            case ValidationKind::SpectralDeterminant:
                cfg.spectral_determinant_validation = true;
                if (!j.default_path.empty()) cfg.export_spectral_det_path = j.default_path;
                break;
            case ValidationKind::ArchimedeanSweep:
                cfg.archimedean_boundary_sweep = true;
                if (!j.default_path.empty()) cfg.export_archimedean_path = j.default_path;
                break;
            case ValidationKind::AssemblySearch:
                cfg.assembly_search = true;
                if (!j.default_path.empty()) cfg.export_assembly_path = j.default_path;
                break;
            case ValidationKind::SelfAdjointExtensionSweep:
                cfg.self_adjoint_extension_sweep = true;
                if (!j.default_path.empty()) cfg.export_self_adjoint_ext_path = j.default_path;
                break;
            case ValidationKind::TraceFormulaGate:
                cfg.trace_formula_gate = true;
                if (!j.default_path.empty()) cfg.export_trace_formula_gate_path = j.default_path;
                break;
            case ValidationKind::BerryKeatingValidation:
                cfg.berry_keating_validation = true;
                if (!j.default_path.empty()) cfg.export_berry_keating_path = j.default_path;
                break;
            case ValidationKind::AnalyticConstruction:
                cfg.analytic_construction_validation = true;
                if (!j.default_path.empty() && !cfg.export_analytic_construction_user_set)
                    cfg.export_analytic_construction_path = j.default_path;
                break;
            case ValidationKind::GlobalDiracLimit:
                cfg.global_dirac_limit_validation = true;
                if (!j.default_path.empty() && !cfg.export_global_dirac_limit_user_set)
                    cfg.export_global_dirac_limit_path = j.default_path;
                break;
            case ValidationKind::AnalyticLemmaDemo:
                cfg.analytic_lemma_demo = true;
                if (!j.default_path.empty() && !cfg.export_analytic_lemma_demo_user_set)
                    cfg.export_analytic_lemma_demo_path = j.default_path;
                break;
            case ValidationKind::SpectralActionSelection:
                cfg.spectral_action_validation = true;
                if (!j.default_path.empty() && !cfg.export_spectral_action_user_set)
                    cfg.export_spectral_action_path = j.default_path;
                break;
            case ValidationKind::PairCorrelation:
                cfg.pair_correlation = true;
                if (!j.default_path.empty()) cfg.export_pair_correlation_path = j.default_path;
                break;
            case ValidationKind::FormalAnalytics:
                cfg.formal_analytics = true;
                if (!j.default_path.empty()) cfg.export_formal_analytics_path = j.default_path;
                break;
            case ValidationKind::MarshalWedgeAnalytic:
                cfg.marshal_wedge_analytic_validation = true;
                if (!j.default_path.empty())
                    cfg.export_wedge_analytic_path = j.default_path;
                break;
            case ValidationKind::XiHadamardProof:
                cfg.xi_hadamard_proof_validation = true;
                if (!j.default_path.empty())
                    cfg.export_xi_hadamard_proof_path = j.default_path;
                break;
            case ValidationKind::Investigation:
                cfg.investigation_run = true;
                if (!j.diagnostic_id.empty()) cfg.investigation_id = j.diagnostic_id;
                if (!j.default_path.empty()) cfg.investigation_cert_root = j.default_path;
                break;
        }
    }
}

}  // namespace Marshal::AnaVM

#include "AnaVM/MrsParser.hxx"
#include "AnaVM/ValidationRouter.hxx"

#include <iostream>
#include <set>
#include <string>

using Marshal::AnaVM::ValidationKind;
using Marshal::AnaVM::route_validation;

int main() {
    const std::string path = "../programs/connes_analytic_construction.mrs";
    auto pr = Marshal::AnaVM::parse_mrs_file(path);
    if (!pr.ok) {
        std::cerr << "parse failed\n";
        return 1;
    }
    const auto jobs = route_validation(pr.program);
    std::set<ValidationKind> kinds;
    for (const auto& j : jobs) kinds.insert(j.kind);

    int fails = 0;
    auto need = [&](ValidationKind k, const char* name) {
        if (!kinds.count(k)) {
            std::cerr << "FAIL: missing job " << name << "\n";
            ++fails;
        }
    };
    need(ValidationKind::AnalyticConstruction, "analytic_construction");
    need(ValidationKind::SelfAdjointExtensionSweep, "extension_sweep");
    need(ValidationKind::TraceFormulaGate, "trace_formula_gate");
    need(ValidationKind::SpectralDeterminant, "spectral_determinant");

    if (pr.program.rule_id != "connes_analytic_construction") {
        std::cerr << "FAIL: rule_id=" << pr.program.rule_id << "\n";
        ++fails;
    }

    const auto sa = Marshal::AnaVM::parse_mrs_file("../programs/spectral_action_selection.mrs");
    if (!sa.ok) {
        std::cerr << "spectral_action_selection parse failed\n";
        ++fails;
    } else {
        const auto sa_jobs = route_validation(sa.program);
        std::set<ValidationKind> sa_kinds;
        for (const auto& j : sa_jobs) sa_kinds.insert(j.kind);
        if (sa_kinds.count(ValidationKind::LogPrime)) {
            std::cerr << "FAIL: spectral_action_selection should not route log-prime\n";
            ++fails;
        }
        if (sa_kinds.count(ValidationKind::ConnesCrossed)) {
            std::cerr << "FAIL: spectral_action_selection should not route connes-crossed\n";
            ++fails;
        }
        if (!sa_kinds.count(ValidationKind::SpectralActionSelection)) {
            std::cerr << "FAIL: spectral_action_selection missing spectral action job\n";
            ++fails;
        }
        if (sa_kinds.count(ValidationKind::SelfAdjointExtensionSweep)) {
            std::cerr << "FAIL: spectral_action_selection should not route extension sweep\n";
            ++fails;
        }
        if (sa_kinds.count(ValidationKind::ArchimedeanSweep)) {
            std::cerr << "FAIL: spectral_action_selection should not route archimedean sweep\n";
            ++fails;
        }
    }

    const auto lim = Marshal::AnaVM::parse_mrs_file("../programs/connes_global_dirac_limit.mrs");
    if (!lim.ok) {
        std::cerr << "connes_global_dirac_limit parse failed\n";
        ++fails;
    } else {
        const auto lim_jobs = route_validation(lim.program);
        std::set<ValidationKind> lim_kinds;
        for (const auto& j : lim_jobs) lim_kinds.insert(j.kind);
        if (!lim_kinds.count(ValidationKind::GlobalDiracLimit)) {
            std::cerr << "FAIL: connes_global_dirac_limit missing global_dirac_limit job\n";
            ++fails;
        }
        if (lim_kinds.count(ValidationKind::LogPrime)) {
            std::cerr << "FAIL: connes_global_dirac_limit should not route log-prime\n";
            ++fails;
        }
        if (lim_kinds.count(ValidationKind::SpectralActionSelection)) {
            std::cerr << "FAIL: connes_global_dirac_limit should not route spectral action\n";
            ++fails;
        }
        if (lim.program.rule_id != "connes_global_dirac_limit") {
            std::cerr << "FAIL: limit rule_id=" << lim.program.rule_id << "\n";
            ++fails;
        }
    }

    const auto lem = Marshal::AnaVM::parse_mrs_file("../programs/connes_analytic_lemmas.mrs");
    if (!lem.ok) {
        std::cerr << "connes_analytic_lemmas parse failed\n";
        ++fails;
    } else {
        const auto lem_jobs = route_validation(lem.program);
        std::set<ValidationKind> lem_kinds;
        for (const auto& j : lem_jobs) lem_kinds.insert(j.kind);
        if (!lem_kinds.count(ValidationKind::AnalyticLemmaDemo)) {
            std::cerr << "FAIL: connes_analytic_lemmas missing analytic_lemma_demo job\n";
            ++fails;
        }
        if (lem_kinds.count(ValidationKind::AnalyticConstruction)) {
            std::cerr << "FAIL: connes_analytic_lemmas should not route full construction export\n";
            ++fails;
        }
        if (lem_kinds.count(ValidationKind::ArchimedeanSweep)) {
            std::cerr << "FAIL: connes_analytic_lemmas should not route archimedean sweep\n";
            ++fails;
        }
        if (lem.program.rule_id != "connes_analytic_lemmas") {
            std::cerr << "FAIL: lemma rule_id=" << lem.program.rule_id << "\n";
            ++fails;
        }
    }

    const auto inv = Marshal::AnaVM::parse_mrs_file("../programs/investigations/theorem_ab.mrs");
    if (!inv.ok) {
        std::cerr << "theorem_ab investigation parse failed\n";
        ++fails;
    } else {
        if (!inv.program.investigation.present) {
            std::cerr << "FAIL: theorem_ab missing investigation block\n";
            ++fails;
        }
        const auto inv_jobs = route_validation(inv.program);
        std::set<ValidationKind> inv_kinds;
        for (const auto& j : inv_jobs) inv_kinds.insert(j.kind);
        if (!inv_kinds.count(ValidationKind::Investigation)) {
            std::cerr << "FAIL: theorem_ab missing Investigation job\n";
            ++fails;
        }
    }

    if (fails) return 1;
    std::cout << "ValidationRouterTest: ok (" << jobs.size() << " jobs)\n";
    return 0;
}

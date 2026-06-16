#include "OperatorInference.hxx"

#include "AnaVM/AnaVm.hxx"
#include "AnaVM/OperatorTraits.hxx"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Induction {

namespace {

struct ProgramEntry {
    const char* path;
    const char* registry_status;
};

const ProgramEntry kPrograms[] = {
    {"../programs/cylinder_direct_sum.mrs", "FALSIFIED"},
    {"../programs/quotient_gamma_tuned.mrs", "FALSIFIED"},
    {"../programs/quotient_sunit_lowmode.mrs", "FALSIFIED"},
    {"../programs/logp_frequency.mrs", "CANDIDATE"},
    {"../programs/exponent_klogp.mrs", "DIAGNOSTIC"},
    {"../programs/berry_keating.mrs", "OPEN"},
    {"../programs/connes_analytic_construction.mrs", "TARGET"},
    {"../programs/templates/berry_keating.mrs.stub", "OPEN"},
    {"../programs/templates/connes_triple.mrs.stub", "OPEN"},
};

std::string verdict_from(const AnaVM::OperatorTraits& t, const CandidateMetrics& m) {
    if (!elimination_reason_for(t).empty()) return "FALSIFIED";
    if (t.rule_id == "connes_analytic_construction" || t.rule_id == "connes_dirac")
        return t.scaffold || t.placeholder ? "TARGET_SCAFFOLD" : "TARGET";
    if (t.scaffold || t.placeholder) return "OPEN_SCAFFOLD";
    if (m.compact_sinc2_mismatch_proved) return "FALSIFIED";
    if (!t.violated_requirements.empty()) return "REQUIREMENTS_VIOLATED";
    if (m.gamma_free_gap_max < 1.0 && !m.compact_sinc2_mismatch_proved) return "PLAUSIBLE";
    return "OPEN";
}

}  // namespace

void ExportOperatorCandidatesJson(const std::string& path,
                                  const std::vector<OperatorCandidateResult>& results,
                                  const Config& cfg, size_t n_zeros) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"zeros\": " << n_zeros << ",\n";
    out << "  \"prime_limit\": " << cfg.prime_limit << ",\n";
    out << "  \"inferred_requirements_doc\": \"docs/Analysis/OperatorTraitRegistry.json\",\n";
    out << "  \"candidates\": [\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        const auto& t = r.traits;
        const auto& m = r.metrics;
        out << "    {\n";
        out << "      \"ansatz_id\": \"" << r.ansatz_id << "\",\n";
        out << "      \"registry_status\": \"" << r.registry_status << "\",\n";
        out << "      \"verdict\": \"" << r.verdict << "\",\n";
        out << "      \"plausibility_score\": " << r.plausibility_score << ",\n";
        if (!r.elimination_reason.empty()) {
            out << "      \"elimination_reason\": \"" << r.elimination_reason << "\",\n";
        }
        out << "      \"traits\": {\n";
        out << "        \"rule_id\": \"" << t.rule_id << "\",\n";
        out << "        \"space_kind\": \"" << AnaVM::space_kind_string(t.space) << "\",\n";
        out << "        \"spectrum_merge\": \"" << AnaVM::spectrum_merge_string(t.merge) << "\",\n";
        out << "        \"spectrum_scale\": \"" << AnaVM::spectrum_scale_string(t.scale) << "\",\n";
        out << "        \"numeric_backend\": \"" << t.numeric_backend << "\",\n";
        out << "        \"weil_poisson_compatible\": " << (t.weil_poisson_compatible ? "true" : "false")
            << ",\n";
        out << "        \"uses_gamma\": " << (t.uses_gamma ? "true" : "false") << ",\n";
        out << "        \"in_C_fin\": " << (t.in_C_fin ? "true" : "false") << ",\n";
        out << "        \"density_growth\": \"" << AnaVM::density_growth_string(t.density_growth)
            << "\",\n";
        out << "        \"scaffold\": " << (t.scaffold ? "true" : "false") << "\n";
        out << "      },\n";
        if (!t.connes_subtarget_status.empty()) {
            out << "      \"connes_subtargets\": [";
            for (size_t j = 0; j < t.connes_subtarget_status.size(); ++j) {
                if (j) out << ", ";
                out << "\"" << t.connes_subtarget_status[j] << "\"";
            }
            out << "],\n";
        }
        out << "      \"requirements\": {\n";
        out << "        \"satisfied\": [";
        for (size_t j = 0; j < t.satisfied_requirements.size(); ++j) {
            if (j) out << ", ";
            out << "\"" << t.satisfied_requirements[j] << "\"";
        }
        out << "],\n        \"violated\": [";
        for (size_t j = 0; j < t.violated_requirements.size(); ++j) {
            if (j) out << ", ";
            out << "\"" << t.violated_requirements[j] << "\"";
        }
        out << "],\n        \"missing\": [";
        for (size_t j = 0; j < t.missing_requirements.size(); ++j) {
            if (j) out << ", ";
            out << "\"" << t.missing_requirements[j] << "\"";
        }
        out << "]\n      },\n";
        out << "      \"metrics\": {\n";
        out << "        \"gamma_free_gap_max\": " << static_cast<double>(m.gamma_free_gap_max) << ",\n";
        out << "        \"gamma_free_gap_mean\": " << static_cast<double>(m.gamma_free_gap_mean) << ",\n";
        out << "        \"matched_sq_gap_max\": " << static_cast<double>(m.matched_sq_gap_max) << ",\n";
        out << "        \"quotient_sq_gap_max\": " << static_cast<double>(m.quotient_sq_gap_max) << ",\n";
        out << "        \"fixed_quotient_gap_max\": " << static_cast<double>(m.fixed_quotient_gap_max)
            << ",\n";
        out << "        \"compact_sinc2_residual\": " << static_cast<double>(m.compact_sinc2_residual)
            << ",\n";
        out << "        \"compact_sinc2_mismatch_proved\": "
            << (m.compact_sinc2_mismatch_proved ? "true" : "false") << "\n";
        out << "      }\n";
        out << "    }";
        if (i + 1 < results.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
}

void RunOperatorCandidates(const Config& cfg, const TestFunction& /*tf*/,
                           const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
                           Heat::PrimeCatalog& cat) {
    const int n_pairs = cfg.spectral_compare_n > 0 ? cfg.spectral_compare_n : 64;
    std::vector<OperatorCandidateResult> results;
    results.reserve(std::size(kPrograms));

    std::cout << "=== Operator trait inference + candidate evaluation ===\n";
    std::cout << "  zeros=" << gammas.size() << "  pairs=" << n_pairs
              << "  prime_limit=" << cfg.prime_limit << "\n\n";

    for (const auto& pe : kPrograms) {
        const std::string path = pe.path;
        auto cr = AnaVM::compile_program(path);
        if (!cr.ok) {
            std::cout << "  [" << path << "] compile FAIL\n";
            for (const auto& e : cr.errors) std::cout << "    " << e.code << ": " << e.message << "\n";
            continue;
        }
        AnaVM::OperatorTraits traits = AnaVM::infer_traits(cr.program);
        CandidateMetrics metrics =
            evaluate_candidate_metrics(traits, gammas, gammas_ld, cat, cfg, n_pairs);

        OperatorCandidateResult r;
        r.ansatz_id = traits.ansatz_id;
        r.registry_status = pe.registry_status;
        r.traits = std::move(traits);
        r.metrics = metrics;
        r.verdict = verdict_from(r.traits, metrics);
        r.elimination_reason = elimination_reason_for(r.traits);
        r.plausibility_score = score_plausibility(r.traits, metrics);
        results.push_back(std::move(r));

        std::cout << "  " << r.ansatz_id << "  verdict=" << r.verdict
                  << "  score=" << r.plausibility_score
                  << "  γ-free_gap=" << static_cast<double>(metrics.gamma_free_gap_max);
        if (metrics.compact_sinc2_mismatch_proved)
            std::cout << "  sinc²=" << static_cast<double>(metrics.compact_sinc2_residual);
        std::cout << "\n";
    }

    std::sort(results.begin(), results.end(),
              [](const OperatorCandidateResult& a, const OperatorCandidateResult& b) {
                  return a.plausibility_score > b.plausibility_score;
              });

    if (!cfg.export_formal_cal_path.empty()) {
        ExportOperatorCandidatesJson(cfg.export_formal_cal_path, results, cfg, gammas.size());
        std::cout << "\nWrote " << cfg.export_formal_cal_path << "\n";
    }
}

}  // namespace Marshal::Induction

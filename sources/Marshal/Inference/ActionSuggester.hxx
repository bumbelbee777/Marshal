#pragma once

#include "DependencyAnalyzer.hxx"
#include "JsonMinimal.hxx"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>

namespace Marshal::Inference {

enum class HuntPriority {
    TIER_1_SANITY = 1,
    TIER_2_FALSIFICATION = 2,
    TIER_3_ANALYTIC_SHAPE = 3,
    TIER_4_PROOF_TRACK = 4,
    TIER_5_NEW_AXIOM = 5,
};

inline const char* hunt_priority_string(HuntPriority t) {
    switch (t) {
        case HuntPriority::TIER_1_SANITY: return "TIER_1_SANITY";
        case HuntPriority::TIER_2_FALSIFICATION: return "TIER_2_FALSIFICATION";
        case HuntPriority::TIER_3_ANALYTIC_SHAPE: return "TIER_3_ANALYTIC_SHAPE";
        case HuntPriority::TIER_4_PROOF_TRACK: return "TIER_4_PROOF_TRACK";
        default: return "TIER_5_NEW_AXIOM";
    }
}

struct NextAction {
    int priority = 0;
    HuntPriority hunt_tier = HuntPriority::TIER_5_NEW_AXIOM;
    std::string action;
    std::string lemma;
    std::string status;
    std::vector<std::string> depends_on;
    bool all_dependencies_satisfied = false;
    std::string suggested_approach;
    std::string blocked_by;
    std::vector<std::string> blocked_verdicts;
    std::vector<std::string> constraints;
    std::string suggested_starting_point;
    std::string test_gate;
    std::string gate;
    double current = 0;
    double target = 0;
    std::string prediction;
    std::string note;
    bool permanently = false;
};

inline std::string approach_for_lemma(const std::string& id) {
    static const std::map<std::string, std::string> kTable = {
        {"cylinder_class_nogo",
         "Formalize full Sobolev d_s no-go; extend counting + sinc² + pair-correlation reduction"},
        {"weil_trace_duality",
         "Run weighted H_log heat trace sweep; certify trace duality vs explicit formula prime side"},
        {"weil_weighted_trace_match",
         "T1 proved; use H_log as local factor in Connes crossed-product assembly"},
        {"logp_prime_dual",
         "Verify T1 Weil vs Marshal prime + full Weil identity; trace duality not spectral identity"},
        {"poisson_gue_finite_coupling_nogo",
         "CLOSED — no finite C_fin assembly; see PoissonGueNoGo.md"},
        {"log_prime_measure_convergence",
         "CLOSED — adelic RMSE diverges; not a convergence target"},
        {"connes_crossed_product_assembly",
         "CLOSED — falsified; finite crossed product in C_fin"},
        {"operator_hunt_sanity_cert",
         "Run tools/Workload/RunOperatorHuntSanity.py --quick; T1 + trace + C_fin exclusion"},
        {"continuum_persistence_check",
         "Run ContinuumPersistenceCheck.py on P ladder; ANALYTIC_SHAPE_OK vs BAD"},
        {"connes_analytic_construction",
         "Revise scaffold: no BK height_map; spectral action selects extension; see SpectralDiscretenessTheorem.md"},
        {"self_adjoint_extension_selection",
         "Prove extension selected by spectral action / trace functional — not log n/(2pi) height map"},
        {"spectral_discreteness",
         "Analytic proof: spectral action selects discrete spectrum on critical strip"},
        {"density_growth_logarithmic",
         "Required trait; cylinder constant and BK inverse_log permanently excluded"},
        {"spectral_measure_limit_conjecture",
         "Prove weak-* limit exists; show Paley-Wiener discrimination vs μ_Riemann"},
        {"resolvent_limit", "Establish norm-resolvent limit H_P -> H outside cylinder class"},
        {"quotient_spectrum", "Quotient projected spectrum — blocked by cylinder falsification"},
        {"convergence_tail_bound", "Bound omitted-prime tail for Gaussian heat kernel"},
        {"global_connes_dirac_limit", "Run RunGlobalDiracLimit.py; formal limit cert"},
        {"connes_analytic_lemma_demo", "Run RunAnalyticLemmaDemo.py; per-lemma gate map"},
        {"proof_obligations", "Run EmitProofObligations.py; consolidated v1 registry"},
    };
    const auto it = kTable.find(id);
    return it != kTable.end() ? it->second : "See lemma doc in LemmaManifest.json";
}

inline HuntPriority tier_for_lemma(const std::string& id) {
    if (id == "operator_hunt_sanity_cert" || id == "weil_weighted_trace_match" ||
        id == "weil_trace_formula")
        return HuntPriority::TIER_1_SANITY;
    if (id == "poisson_gue_finite_coupling_nogo" || id == "connes_crossed_product_assembly" ||
        id == "log_prime_measure_convergence" || id == "density_growth_logarithmic")
        return HuntPriority::TIER_2_FALSIFICATION;
    if (id == "continuum_persistence_check") return HuntPriority::TIER_3_ANALYTIC_SHAPE;
    if (id == "spectral_discreteness" || id == "self_adjoint_extension_selection" ||
        id == "spectral_det_xi" || id == "global_connes_dirac_limit" ||
        id == "connes_analytic_lemma_demo" || id == "proof_obligations")
        return HuntPriority::TIER_4_PROOF_TRACK;
    return HuntPriority::TIER_5_NEW_AXIOM;
}

inline std::string inference_verdict(const ProofGraph& g) {
    if (g.hunt.hunt_closed) return "OPERATOR_HUNT_CLOSED";
    if (g.hunt.operator_hunt_pass) return "OPERATOR_HUNT_SANITY_PASS";
    return "INCONCLUSIVE";
}

inline std::vector<NextAction> suggest_actions_hunt_closed(const ProofGraph& g) {
    std::vector<NextAction> actions;
    int pri = 1;
    auto push = [&](NextAction act) {
        act.priority = pri++;
        actions.push_back(std::move(act));
    };

    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_2_FALSIFICATION;
        act.action = "closed_path";
        act.lemma = "poisson_gue_finite_coupling_nogo";
        act.status = "PROVED";
        act.suggested_approach =
            "C_fin closed — cylinder, adelic, crossed product, height map permanently excluded";
        act.note = "Finite operator hunt ended; see PoissonGueNoGo.md";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_2_FALSIFICATION;
        act.action = "closed_path";
        act.lemma = "global_operator_trait_profile";
        act.status = "PROVED";
        act.suggested_approach =
            "Trait profile locked: not_in_C_fin, trace_duality, density_growth logarithmic";
        act.note = "docs/Analysis/OperatorTraitRegistry.json";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_1_SANITY;
        act.action = "done";
        act.lemma = "operator_hunt_sanity_cert";
        act.status = "NUMERICAL_PASS";
        act.suggested_approach = approach_for_lemma("operator_hunt_sanity_cert");
        act.note = g.hunt.operator_hunt_verdict;
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_3_ANALYTIC_SHAPE;
        act.action = "done";
        act.lemma = "continuum_persistence_check";
        act.status = "CLASSIFIED";
        act.suggested_approach = approach_for_lemma("continuum_persistence_check");
        act.note = g.hunt.continuum_verdict + " — not a C_fin defect";
        push(std::move(act));
    }
    if (g.hunt.scaffold_revised) {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_3_ANALYTIC_SHAPE;
        act.action = "done";
        act.lemma = "connes_analytic_construction";
        act.status = "SCAFFOLD_V2";
        act.suggested_approach = approach_for_lemma("connes_analytic_construction");
        act.note = "BK height_map excluded; OPEN_SPECTRAL_DISCRETENESS at operational P";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "emit_registry";
        act.lemma = "proof_obligations";
        act.status = "V1_OPEN";
        act.suggested_approach = approach_for_lemma("proof_obligations");
        act.note = "Consolidated v1 obligations + ambiguity resolutions";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "run_analytic_lemma_demo";
        act.lemma = "connes_analytic_lemma_demo";
        act.status = "ANALYTIC_DEMONSTRATION_OPEN";
        act.suggested_approach = approach_for_lemma("connes_analytic_lemma_demo");
        act.note = "Per-lemma gate map; weil PROVED, three OPEN";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "run_formal_limit";
        act.lemma = "global_connes_dirac_limit";
        act.status = "FORMAL_LIMIT_OPEN";
        act.suggested_approach = approach_for_lemma("global_connes_dirac_limit");
        act.note = "DISCRETIZATION_IDENTIFICATION_FAILS";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "prove_lemma";
        act.lemma = "self_adjoint_extension_selection";
        act.status = "OPEN";
        act.all_dependencies_satisfied = true;
        act.suggested_approach = approach_for_lemma("self_adjoint_extension_selection");
        act.note = "Extension from spectral action — not height map";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "prove_lemma";
        act.lemma = "spectral_discreteness";
        act.status = "OPEN";
        act.suggested_approach = approach_for_lemma("spectral_discreteness");
        act.note = "THE GAP — sole remaining RH closure lemma on this track";
        push(std::move(act));
    }
    {
        NextAction act;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "prove_lemma";
        act.lemma = "spectral_det_xi";
        act.status = "OPEN";
        act.suggested_approach = "After extension + discreteness: det(s-D) vs completed xi";
        act.note = "Blocked until self_adjoint_extension_selection + spectral_discreteness";
        push(std::move(act));
    }
    for (size_t i = 0; i < actions.size(); ++i) actions[i].priority = static_cast<int>(i + 1);
    return actions;
}

inline std::vector<NextAction> suggest_actions(const ProofGraph& g, const AnalysisResult& a) {
    if (g.hunt.hunt_closed) return suggest_actions_hunt_closed(g);

    std::vector<NextAction> actions;
    int pri = 1;

    if (!g.hunt.operator_hunt_pass) {
        NextAction act;
        act.priority = pri++;
        act.hunt_tier = HuntPriority::TIER_1_SANITY;
        act.action = "run_validation";
        act.lemma = "operator_hunt_sanity_cert";
        act.status = "NUMERICAL";
        act.suggested_approach = approach_for_lemma("operator_hunt_sanity_cert");
        act.note = "OPERATOR_HUNT_SANITY_PASS != RH close; sanity only";
        actions.push_back(std::move(act));
    }

    for (const BlockedNode& bn : a.blocked_ready) {
        NextAction act;
        act.priority = pri++;
        act.action = "prove_lemma";
        act.lemma = bn.id;
        act.status = "OPEN";
        act.depends_on = bn.depends_on;
        act.all_dependencies_satisfied = true;
        act.hunt_tier = tier_for_lemma(bn.id);
        act.suggested_approach = approach_for_lemma(bn.id);
        actions.push_back(std::move(act));
    }

    for (const FalsifiedNode& fn : a.falsified) {
        NextAction act;
        act.priority = pri++;
        act.action = "replace_ansatz";
        act.blocked_by = fn.id;
        act.blocked_verdicts = fn.blocked_verdicts;
        act.hunt_tier = HuntPriority::TIER_2_FALSIFICATION;
        act.constraints = {"gamma_free", "not_in_C_fin", "density_growth_logarithmic"};
        act.suggested_approach =
            "C_fin closed by Poisson-GUE no-go; pursue Connes D via GlobalOperatorHunt.md";
        actions.push_back(std::move(act));
    }

    for (const auto& fa : g.falsified_ansatze) {
        if (fa.id == "cylinder_direct_sum") continue;
        NextAction act;
        act.priority = pri++;
        act.action = "replace_ansatz";
        act.blocked_by = fa.id;
        act.blocked_verdicts = {"SPECTRUM_IDENTIFIED"};
        act.hunt_tier = HuntPriority::TIER_2_FALSIFICATION;
        act.note = "Falsified ansatz in AnsatzRegistry";
        actions.push_back(std::move(act));
    }

    if (g.hunt.continuum_verdict.empty()) {
        NextAction act;
        act.priority = pri++;
        act.hunt_tier = HuntPriority::TIER_3_ANALYTIC_SHAPE;
        act.action = "run_validation";
        act.lemma = "continuum_persistence_check";
        act.status = "NUMERICAL";
        act.suggested_approach = approach_for_lemma("continuum_persistence_check");
        act.note = "P ladder; persistent continuum => ANALYTIC_SHAPE_BAD";
        actions.push_back(std::move(act));
    } else if (g.hunt.continuum_verdict == "ANALYTIC_SHAPE_BAD") {
        NextAction act;
        act.priority = pri++;
        act.hunt_tier = HuntPriority::TIER_3_ANALYTIC_SHAPE;
        act.action = "revise_scaffold";
        act.lemma = "connes_analytic_construction";
        act.status = "CLASSIFIED";
        act.suggested_approach = approach_for_lemma("connes_analytic_construction");
        act.note =
            "Continuum BAD at all P — finite truncation shows continuous spectrum; not a C_fin defect";
        actions.push_back(std::move(act));
    }

    bool has_connes_target =
        std::find(g.open_ansatze.begin(), g.open_ansatze.end(), "connes_analytic_construction") !=
            g.open_ansatze.end() ||
        std::find(g.open_ansatze.begin(), g.open_ansatze.end(), "connes_adele_quotient") !=
            g.open_ansatze.end();
    if (has_connes_target) {
        bool has_self_adjoint = false;
        for (const auto& act : actions)
            if (act.lemma == "self_adjoint_extension_selection") has_self_adjoint = true;
        if (g.hunt.operator_hunt_pass && !has_self_adjoint) {
            NextAction act;
            act.priority = pri++;
            act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
            act.action = "prove_lemma";
            act.lemma = "self_adjoint_extension_selection";
            act.status = "OPEN";
            act.suggested_approach = approach_for_lemma("self_adjoint_extension_selection");
            act.note = "BK height_map falsified; extension must come from spectral action";
            actions.push_back(std::move(act));
        }
        NextAction act;
        act.priority = pri++;
        act.hunt_tier = HuntPriority::TIER_4_PROOF_TRACK;
        act.action = "prove_lemma";
        act.lemma = "spectral_discreteness";
        act.status = "OPEN";
        act.suggested_approach = approach_for_lemma("spectral_discreteness");
        act.note = "THE GAP — T1/local Weil proved; finite spectrum identification excluded";
        actions.push_back(std::move(act));
    }

    for (const NumericalGap& ng : a.numerical_gaps) {
        NextAction act;
        act.priority = pri++;
        act.action = "tighten";
        act.gate = ng.gate;
        act.current = ng.current;
        act.target = ng.target;
        act.prediction = ng.prediction;
        actions.push_back(std::move(act));
    }

    for (const UnreachableNode& u : a.unreachable) {
        const auto it = g.nodes.find(u.id);
        if (it != g.nodes.end() && it->second.status == ProofStatus::Impossible) {
            NextAction act;
            act.priority = pri++;
            act.action = "new_axiom";
            act.blocked_by = u.blocked_by;
            act.permanently = true;
            act.note = "No ansatz of this type can exist in current framework";
            actions.push_back(std::move(act));
        }
    }

    const auto wwt = g.nodes.find("weil_weighted_trace_match");
    if (wwt != g.nodes.end() && wwt->second.status == ProofStatus::Numerical) {
        NextAction act;
        act.priority = pri++;
        act.hunt_tier = HuntPriority::TIER_1_SANITY;
        act.action = "run_validation";
        act.lemma = "weil_weighted_trace_match";
        act.status = "NUMERICAL";
        act.suggested_approach = approach_for_lemma("weil_weighted_trace_match");
        act.note = "Marshal --log-prime-validation or RunLogPrimeValidation.py";
        actions.push_back(std::move(act));
    }

    const auto wit = g.nodes.find("weil_trace_duality");
    if (wit != g.nodes.end() && wit->second.status == ProofStatus::Open) {
        bool already = false;
        for (const auto& act : actions)
            if (act.lemma == "weil_trace_duality") already = true;
        if (!already) {
            NextAction act;
            act.priority = pri++;
            act.hunt_tier = HuntPriority::TIER_1_SANITY;
            act.action = "certify_path_b";
            act.lemma = "weil_trace_duality";
            act.status = "OPEN";
            act.note =
                "Run weighted H_log heat trace sweep to numerically verify explicit formula prime side";
            act.suggested_approach = approach_for_lemma("weil_trace_duality");
            actions.push_back(std::move(act));
        }
    }

    auto sort_key = [](const NextAction& a) -> int {
        const int tier = static_cast<int>(a.hunt_tier);
        return tier * 1000 + a.priority;
    };
    std::sort(actions.begin(), actions.end(),
              [&](const NextAction& a, const NextAction& b) { return sort_key(a) < sort_key(b); });
    for (size_t i = 0; i < actions.size(); ++i) actions[i].priority = static_cast<int>(i + 1);
    return actions;
}

inline void export_next_actions_json(const std::string& path, const std::string& verdict,
                                     const std::vector<NextAction>& actions) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"engine\": \"Marshal\",\n";
    out << "  \"verdict\": ";
    json_escape(out, verdict.empty() ? "INCONCLUSIVE" : verdict);
    out << ",\n  \"next_actions\": [\n";
    for (size_t i = 0; i < actions.size(); ++i) {
        const auto& a = actions[i];
        out << "    {\n";
        out << "      \"priority\": " << a.priority << ",\n";
        out << "      \"hunt_tier\": ";
        json_escape(out, hunt_priority_string(a.hunt_tier));
        out << ",\n";
        out << "      \"action\": ";
        json_escape(out, a.action);
        out << ",\n";
        if (!a.lemma.empty()) {
            out << "      \"lemma\": ";
            json_escape(out, a.lemma);
            out << ",\n";
        }
        if (!a.status.empty()) {
            out << "      \"status\": ";
            json_escape(out, a.status);
            out << ",\n";
        }
        if (!a.depends_on.empty()) {
            out << "      \"depends_on\": [";
            for (size_t j = 0; j < a.depends_on.size(); ++j) {
                if (j) out << ", ";
                json_escape(out, a.depends_on[j]);
            }
            out << "],\n";
        }
        if (a.all_dependencies_satisfied)
            out << "      \"all_dependencies_satisfied\": true,\n";
        if (!a.suggested_approach.empty()) {
            out << "      \"suggested_approach\": ";
            json_escape(out, a.suggested_approach);
            out << ",\n";
        }
        if (!a.blocked_by.empty()) {
            out << "      \"blocked_by\": ";
            json_escape(out, a.blocked_by);
            out << ",\n";
        }
        if (!a.blocked_verdicts.empty()) {
            out << "      \"blocked_verdicts\": [";
            for (size_t j = 0; j < a.blocked_verdicts.size(); ++j) {
                if (j) out << ", ";
                json_escape(out, a.blocked_verdicts[j]);
            }
            out << "],\n";
        }
        if (!a.constraints.empty()) {
            out << "      \"constraints\": [";
            for (size_t j = 0; j < a.constraints.size(); ++j) {
                if (j) out << ", ";
                json_escape(out, a.constraints[j]);
            }
            out << "],\n";
        }
        if (!a.suggested_starting_point.empty()) {
            out << "      \"suggested_starting_point\": ";
            json_escape(out, a.suggested_starting_point);
            out << ",\n";
        }
        if (!a.test_gate.empty()) {
            out << "      \"test_gate\": ";
            json_escape(out, a.test_gate);
            out << ",\n";
        }
        if (!a.gate.empty()) {
            out << "      \"gate\": ";
            json_escape(out, a.gate);
            out << ",\n      \"current\": " << a.current << ",\n";
            out << "      \"target\": " << a.target << ",\n";
            out << "      \"prediction\": ";
            json_escape(out, a.prediction);
            out << ",\n";
        }
        if (!a.note.empty()) {
            out << "      \"note\": ";
            json_escape(out, a.note);
            out << ",\n";
        }
        if (a.permanently) out << "      \"permanently\": true,\n";
        out << "      \"_ordinal\": " << a.priority << "\n";
        out << "    }";
        if (i + 1 < actions.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
}

}  // namespace Marshal::Inference

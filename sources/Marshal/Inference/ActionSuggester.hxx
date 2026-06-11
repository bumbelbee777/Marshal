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

struct NextAction {
    int priority = 0;
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
        {"log_prime_measure_convergence",
         "Connes crossed-product: prove measure/spectrum convergence as P grows"},
        {"connes_crossed_product_assembly",
         "Scale ConnesCrossedProduct coupling sweep; prove spectrum converges to gamma_n"},
        {"spectral_measure_limit_conjecture",
         "Prove weak-* limit exists; show Paley-Wiener discrimination vs μ_Riemann"},
        {"resolvent_limit", "Establish norm-resolvent limit H_P -> H outside cylinder class"},
        {"quotient_spectrum", "Quotient projected spectrum — blocked by cylinder falsification"},
        {"convergence_tail_bound", "Bound omitted-prime tail for Gaussian heat kernel"},
        {"trace_mode_extraction", "Prony/log-slope mode extraction from heat trace sweep"},
    };
    const auto it = kTable.find(id);
    return it != kTable.end() ? it->second : "See lemma doc in LemmaManifest.json";
}

inline std::vector<NextAction> suggest_actions(const ProofGraph& g, const AnalysisResult& a) {
    std::vector<NextAction> actions;
    int pri = 1;

    for (const BlockedNode& bn : a.blocked_ready) {
        NextAction act;
        act.priority = pri++;
        act.action = "prove_lemma";
        act.lemma = bn.id;
        act.status = "OPEN";
        act.depends_on = bn.depends_on;
        act.all_dependencies_satisfied = true;
        act.suggested_approach = approach_for_lemma(bn.id);
        actions.push_back(std::move(act));
    }

    for (const FalsifiedNode& fn : a.falsified) {
        NextAction act;
        act.priority = pri++;
        act.action = "replace_ansatz";
        act.blocked_by = fn.id;
        act.blocked_verdicts = fn.blocked_verdicts;
        act.constraints = {"gamma_free", "sinc2_compatible", "not_in_cylinder_class"};
        act.suggested_approach =
            "Cylinder class dead end; pursue non-commutative global operator (Connes) or Path B duality";
        actions.push_back(std::move(act));
    }

    for (const auto& fa : g.falsified_ansatze) {
        if (fa.id == "cylinder_direct_sum") continue;
        NextAction act;
        act.priority = pri++;
        act.action = "replace_ansatz";
        act.blocked_by = fa.id;
        act.blocked_verdicts = {"SPECTRUM_IDENTIFIED"};
        act.note = "Falsified ansatz in AnsatzRegistry";
        actions.push_back(std::move(act));
    }

    bool has_open_connes = std::find(g.open_ansatze.begin(), g.open_ansatze.end(),
                                     "connes_adele_quotient") != g.open_ansatze.end();
    if (has_open_connes) {
        NextAction act;
        act.priority = pri++;
        act.action = "new_ansatz";
        act.blocked_by = "cylinder_direct_sum_falsified";
        act.constraints = {"gamma_free", "not_in_cylinder_class"};
        act.suggested_starting_point = "programs/templates/connes_triple.mrs.stub";
        act.test_gate = "compact_sinc2_residual < 1.0";
        act.suggested_approach =
            "Non-commutative spectral triple; recover p^{-k/2} weights via twisted trace";
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
        static const std::map<std::string, int> kBoost = {
            {"cylinder_class_nogo", 0},
        {"connes_crossed_product_assembly", 0},
        {"log_prime_measure_convergence", 1},
        {"weil_weighted_trace_match", 2},
        {"weil_trace_duality", 3},
        {"spectral_measure_limit_conjecture", 4},
        };
        int base = a.priority * 10;
        if (!a.lemma.empty()) {
            const auto it = kBoost.find(a.lemma);
            if (it != kBoost.end()) return it->second;
        }
        if (a.action == "new_ansatz") return 20 + base;
        if (a.action == "certify_path_b") return 25 + base;
        if (a.action == "replace_ansatz") return 30 + base;
        if (a.action == "tighten") return 40 + base;
        return 50 + base;
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

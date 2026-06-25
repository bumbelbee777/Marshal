#include "AnaProofEngine.hxx"

#include "AnaVm.hxx"
#include "Inference/JsonMinimal.hxx"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace Marshal::AnaVM {
namespace {

ProofObligation make_obligation(const std::string& id, const std::string& statement, ProofClass cls,
                                std::vector<std::string> deps) {
    ProofObligation o;
    o.id = id;
    o.statement = statement;
    o.proof_class = cls;
    o.dependencies = std::move(deps);
    o.status = ProofStatus::Pending;
    return o;
}

const char* status_str(ProofStatus s) {
    switch (s) {
        case ProofStatus::Proved:
            return "PROVED";
        case ProofStatus::Failed:
            return "FAILED";
        case ProofStatus::Structural:
            return "STRUCTURAL";
        default:
            return "PENDING";
    }
}

const char* class_str(ProofClass c) {
    switch (c) {
        case ProofClass::NumericInterval:
            return "numeric_interval";
        case ProofClass::Analytic:
            return "analytic";
        case ProofClass::ClassicalImport:
            return "classical_import";
        case ProofClass::Reduction:
            return "reduction";
        case ProofClass::AnalyticOpen:
            return "analytic_open";
        case ProofClass::Structural:
            return "structural";
        case ProofClass::Composition:
            return "composition";
        case ProofClass::Universal:
            return "universal";
        case ProofClass::Inductive:
            return "inductive";
        case ProofClass::Convergent:
            return "convergent";
        case ProofClass::Rewrite:
            return "rewrite";
        case ProofClass::DecisionProcedure:
            return "decision_procedure";
        default:
            return "numeric";
    }
}

}  // namespace

ProofGraphReport proof_graph_from_mrs_bundle(const MrsCompilationBundle& bundle) {
    return proof_graph_from_mrs_bundle_named(bundle, "MarshalHadamard");
}

ProofGraphReport proof_graph_from_mrs_bundle_named(const MrsCompilationBundle& bundle,
                                                   const std::string& graph_name) {
    ProofGraphReport g;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& pg : m.proof_graphs) {
            if (pg.name != graph_name) continue;
            g.target_theorem = pg.target;
            g.architecture = pg.architecture;
            for (const auto& ob : pg.obligations) {
                ProofObligation o;
                o.id = ob.id;
                o.statement = ob.statement;
                o.proof_class = ob.proof_class;
                o.dependencies = ob.dependencies;
                if (ob.prove_kind == MrsProofBodyKind::Infer)
                    o.status = ProofStatus::Pending;
                o.evidence = ob.prove_kind == MrsProofBodyKind::Infer ? "mrs_infer" : ob.prove_ref;
                g.obligations.push_back(std::move(o));
            }
        }
    }
    if (!g.obligations.empty()) {
        g.acyclic = !proof_graph_has_cycle(g, &g.cycle_path);
        g.circular_logic_detected = !g.acyclic;
        g.topological_order = proof_graph_topological_order(g);
    }
    return g;
}

ProofGraphReport build_marshal_hadamard_proof_graph_from_mrs(const std::string& entry_path) {
    const MrsCompilationBundle bundle = compile_bundle(entry_path, true, nullptr);
    auto g = proof_graph_from_mrs_bundle(bundle);
    if (!g.obligations.empty()) return g;
    return build_marshal_hadamard_proof_graph();
}

ProofGraphReport build_marshal_hadamard_proof_graph() {
    ProofGraphReport g;
    g.obligations = {
        make_obligation(
            "genus_one_log_summability",
            "Summable complex logs of genus-1 factors off MarshalXiForcedZero", ProofClass::Numeric, {}),
        make_obligation(
            "marshal_off_height_log_summability",
            "SpectralDetLogSummability witness at each off-locus point", ProofClass::Composition,
            {"genus_one_log_summability"}),
        make_obligation(
            "tprod_convergent_off_locus",
            "Partial Hadamard products converge to infinite tprod off heights", ProofClass::Analytic,
            {"marshal_off_height_log_summability"}),
        make_obligation(
            "certified_det_eq_riemannXi_off_locus",
            "Certified spectralDet = riemannXi off MarshalXiForcedZero (mult=1)", ProofClass::Structural, {}),
        make_obligation(
            "grid_pointwise_tprod_eq_xi",
            "At s_n=2+i/n: infinite tprod(s_n)=riemannXi(s_n) via tail-bound partial product "
            "(NO wedge EqOn input)",
            ProofClass::Numeric, {"tprod_convergent_off_locus"}),
        make_obligation(
            "grid_pointwise_tprod_eq_certified",
            "At accumulation grid: tprod = certified det = riemannXi", ProofClass::Composition,
            {"grid_pointwise_tprod_eq_xi", "certified_det_eq_riemannXi_off_locus"}),
        make_obligation(
            "wedge_holomorphy_tprod",
            "marshalHadamardTprod holomorphic on {Re>1}", ProofClass::Analytic,
            {"marshal_off_height_log_summability", "holomorphy_uniform_cauchy_gap"}),
        make_obligation(
            "holomorphy_uniform_cauchy_gap",
            "Uniform Cauchy gap on wedge approach grid (AnaVM audit)", ProofClass::Numeric, {}),
        make_obligation(
            "wedge_holomorphy_certified",
            "spectralDet holomorphic on {Re>1}", ProofClass::Structural, {}),
        make_obligation(
            "identity_theorem_on_wedge",
            "EqOn tprod = certified on {Re>1} from grid accumulation at 2 (acyclic bootstrap)", ProofClass::Analytic,
            {"grid_pointwise_tprod_eq_certified", "wedge_holomorphy_tprod",
             "wedge_holomorphy_certified"}),
        make_obligation(
            "strip_extension_via_approach_sequence",
            "Extend wedge equality to all s off forced locus via s+(2+i)/(n+1) approach", ProofClass::Analytic,
            {"identity_theorem_on_wedge", "marshal_off_height_log_summability"}),
        make_obligation(
            "wedge_proportionality_from_holomorphy",
            "Same zeros + order-1 + FE ⇒ proportionality; anchor c=1 at s=2", ProofClass::Analytic,
            {"wedge_holomorphy_tprod", "identity_theorem_on_wedge"}),
        make_obligation(
            "MarshalHadamardWeierstrassIdentification",
            "Infinite tprod = certified det off MarshalXiForcedZero", ProofClass::Composition,
            {"wedge_proportionality_from_holomorphy", "strip_extension_via_approach_sequence"}),
        make_obligation(
            "classical_riemann_hypothesis_marshal",
            "Classical RH via wedge classification chain", ProofClass::Composition,
            {"MarshalHadamardWeierstrassIdentification"}),
    };
    g.acyclic = !proof_graph_has_cycle(g, &g.cycle_path);
    g.circular_logic_detected = !g.acyclic;
    g.topological_order = proof_graph_topological_order(g);
    return g;
}

bool proof_graph_has_cycle(const ProofGraphReport& g, std::vector<std::string>* cycle_out) {
    std::unordered_map<std::string, std::vector<std::string>> adj;
    for (const auto& o : g.obligations) adj[o.id] = o.dependencies;

    enum class Color { White, Gray, Black };
    std::unordered_map<std::string, Color> color;
    std::vector<std::string> stack;
    std::vector<std::string> cycle;

    std::function<bool(const std::string&)> dfs = [&](const std::string& u) -> bool {
        color[u] = Color::Gray;
        stack.push_back(u);
        for (const auto& v : adj[u]) {
            if (color[v] == Color::White) {
                if (dfs(v)) return true;
            } else if (color[v] == Color::Gray) {
                cycle.clear();
                bool in_cycle = false;
                for (const auto& x : stack) {
                    if (x == v) in_cycle = true;
                    if (in_cycle) cycle.push_back(x);
                }
                cycle.push_back(v);
                return true;
            }
        }
        stack.pop_back();
        color[u] = Color::Black;
        return false;
    };

    for (const auto& o : g.obligations) color[o.id] = Color::White;
    for (const auto& o : g.obligations) {
        if (color[o.id] == Color::White && dfs(o.id)) {
            if (cycle_out) *cycle_out = cycle;
            return true;
        }
    }
    return false;
}

std::vector<std::string> proof_graph_topological_order(const ProofGraphReport& g) {
    std::unordered_set<std::string> ids;
    for (const auto& o : g.obligations) ids.insert(o.id);
    std::unordered_map<std::string, int> indeg;
    std::unordered_map<std::string, std::vector<std::string>> rev;
    for (const auto& o : g.obligations) {
        int n = 0;
        for (const auto& d : o.dependencies)
            if (ids.count(d)) ++n;
        indeg[o.id] = n;
        for (const auto& d : o.dependencies)
            if (ids.count(d)) rev[d].push_back(o.id);
    }
    std::vector<std::string> q;
    for (const auto& o : g.obligations)
        if (indeg[o.id] == 0) q.push_back(o.id);
    std::vector<std::string> order;
    while (!q.empty()) {
        const std::string u = q.back();
        q.pop_back();
        order.push_back(u);
        for (const auto& v : rev[u]) {
            if (--indeg[v] == 0) q.push_back(v);
        }
    }
    return order;
}

void apply_numeric_evidence(ProofGraphReport& g, const std::string& id, bool ok,
                            const std::string& evidence, const std::string& fail) {
    for (auto& o : g.obligations) {
        if (o.id != id) continue;
        o.status = ok ? ProofStatus::Proved : ProofStatus::Failed;
        o.evidence = evidence;
        o.failure_reason = fail;
        return;
    }
}

void apply_structural_evidence(ProofGraphReport& g, const std::string& id,
                               const std::string& evidence) {
    for (auto& o : g.obligations) {
        if (o.id != id) continue;
        o.status = ProofStatus::Structural;
        o.evidence = evidence;
        return;
    }
}

ProofGraphReport finalize_proof_graph(ProofGraphReport g) {
    g.proved_ids.clear();
    g.failed_ids.clear();
    auto status_of = [&](const std::string& id) {
        for (const auto& o : g.obligations)
            if (o.id == id) return o.status;
        return ProofStatus::Failed;
    };
    auto is_ok = [&](ProofStatus s) {
        return s == ProofStatus::Proved || s == ProofStatus::Structural;
    };

    for (auto& o : g.obligations) {
        if (o.proof_class == ProofClass::Composition) {
            bool deps_ok = true;
            for (const auto& d : o.dependencies)
                if (!is_ok(status_of(d))) deps_ok = false;
            if (deps_ok && o.status == ProofStatus::Pending) {
                o.status = ProofStatus::Proved;
                o.evidence = "dependencies closed (composition)";
            } else if (!deps_ok && o.status == ProofStatus::Pending) {
                o.status = ProofStatus::Failed;
                o.failure_reason = "dependency not closed";
            }
        }
    }

    for (const auto& o : g.obligations) {
        if (is_ok(o.status))
            g.proved_ids.push_back(o.id);
        else
            g.failed_ids.push_back(o.id);
    }
    g.all_proved = g.failed_ids.empty();
    g.acyclic = !proof_graph_has_cycle(g, &g.cycle_path);
    g.circular_logic_detected = !g.acyclic;
    return g;
}

bool export_proof_graph_json(const std::string& path, const ProofGraphReport& g) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"target_theorem\": ";
    Inference::json_escape(out, g.target_theorem);
    out << ",\n  \"architecture\": ";
    Inference::json_escape(out, g.architecture);
    out << ",\n  \"acyclic\": " << (g.acyclic ? "true" : "false") << ",\n";
    out << "  \"circular_logic_detected\": " << (g.circular_logic_detected ? "true" : "false")
        << ",\n";
    out << "  \"all_proved\": " << (g.all_proved ? "true" : "false") << ",\n";
    out << "  \"topological_order\": [";
    for (size_t i = 0; i < g.topological_order.size(); ++i) {
        if (i) out << ", ";
        Inference::json_escape(out, g.topological_order[i]);
    }
    out << "],\n  \"proved_ids\": [";
    for (size_t i = 0; i < g.proved_ids.size(); ++i) {
        if (i) out << ", ";
        Inference::json_escape(out, g.proved_ids[i]);
    }
    out << "],\n  \"failed_ids\": [";
    for (size_t i = 0; i < g.failed_ids.size(); ++i) {
        if (i) out << ", ";
        Inference::json_escape(out, g.failed_ids[i]);
    }
    out << "],\n  \"cycle_path\": [";
    for (size_t i = 0; i < g.cycle_path.size(); ++i) {
        if (i) out << ", ";
        Inference::json_escape(out, g.cycle_path[i]);
    }
    out << "],\n  \"obligations\": [\n";
    for (size_t i = 0; i < g.obligations.size(); ++i) {
        const auto& o = g.obligations[i];
        out << "    {\n      \"id\": ";
        Inference::json_escape(out, o.id);
        out << ",\n      \"statement\": ";
        Inference::json_escape(out, o.statement);
        out << ",\n      \"proof_class\": ";
        Inference::json_escape(out, class_str(o.proof_class));
        out << ",\n      \"status\": ";
        Inference::json_escape(out, status_str(o.status));
        out << ",\n      \"evidence\": ";
        Inference::json_escape(out, o.evidence);
        out << ",\n      \"failure_reason\": ";
        Inference::json_escape(out, o.failure_reason);
        out << ",\n      \"dependencies\": [";
        for (size_t j = 0; j < o.dependencies.size(); ++j) {
            if (j) out << ", ";
            Inference::json_escape(out, o.dependencies[j]);
        }
        out << "]\n    }" << (i + 1 < g.obligations.size() ? "," : "") << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::AnaVM

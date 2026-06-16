// AnaVM proof-graph unit tests (AnaProofEngine).
// Bound proofs and numeric audits will migrate to MRS (marshal_xi_hadamard.mrs);
// these tests pin the C++ proof-graph spine until that handoff is complete.

#include "AnaVM/AnaProofEngine.hxx"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {

int g_fails = 0;

void fail(const char* msg) {
    std::cerr << "FAIL: " << msg << "\n";
    ++g_fails;
}

void require(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

Marshal::AnaVM::ProofObligation make_ob(const std::string& id, std::vector<std::string> deps) {
    Marshal::AnaVM::ProofObligation o;
    o.id = id;
    o.dependencies = std::move(deps);
    o.proof_class = Marshal::AnaVM::ProofClass::Numeric;
    o.status = Marshal::AnaVM::ProofStatus::Pending;
    return o;
}

bool order_respects_deps(const Marshal::AnaVM::ProofGraphReport& g,
                         const std::vector<std::string>& order) {
    std::unordered_map<std::string, size_t> pos;
    for (size_t i = 0; i < order.size(); ++i) pos[order[i]] = i;
    for (const auto& o : g.obligations) {
        if (!pos.count(o.id)) return false;
        for (const auto& d : o.dependencies) {
            if (!pos.count(d)) return false;
            if (pos[d] >= pos[o.id]) return false;
        }
    }
    return true;
}

void test_marshal_hadamard_graph_acyclic() {
    using namespace Marshal::AnaVM;
    const auto g = build_marshal_hadamard_proof_graph();
    require(g.acyclic, "marshal hadamard graph must be acyclic");
    require(!g.circular_logic_detected, "circular_logic_detected must be false");
    require(g.cycle_path.empty(), "cycle_path must be empty on acyclic graph");
    require(g.architecture == "acyclic_marshal_hadamard", "architecture tag");
    require(g.target_theorem == "classical_riemann_hypothesis_marshal_proved", "target theorem");
    require(!g.obligations.empty(), "obligations non-empty");
    require(g.topological_order.size() == g.obligations.size(),
            "topological order covers all obligations");
    require(order_respects_deps(g, g.topological_order), "topological order respects deps");

    bool saw_ident = false;
    for (const auto& id : g.topological_order)
        if (id == "MarshalHadamardWeierstrassIdentification") saw_ident = true;
    require(saw_ident, "MarshalHadamardWeierstrassIdentification in topological order");
}

void test_cycle_detection() {
    using namespace Marshal::AnaVM;
    ProofGraphReport g;
    g.obligations = {
        make_ob("a", {"b"}),
        make_ob("b", {"c"}),
        make_ob("c", {"a"}),
    };
    std::vector<std::string> cycle;
    require(proof_graph_has_cycle(g, &cycle), "3-cycle must be detected");
    require(!cycle.empty(), "cycle path non-empty");
    require(!proof_graph_has_cycle(build_marshal_hadamard_proof_graph(), nullptr),
            "production graph has no cycle");
}

void test_composition_closure() {
    using namespace Marshal::AnaVM;
    auto g = build_marshal_hadamard_proof_graph();
    apply_numeric_evidence(g, "genus_one_log_summability", true, "test genus bound");
    apply_structural_evidence(g, "certified_det_eq_riemannXi_off_locus", "Lean structural");
    apply_structural_evidence(g, "wedge_holomorphy_certified", "Lean holomorphy");
    apply_numeric_evidence(g, "grid_pointwise_tprod_eq_xi", true, "grid ok");
    apply_structural_evidence(g, "holomorphy_uniform_cauchy_gap", "Lean holo gap");
    apply_numeric_evidence(g, "marshal_off_height_log_summability", true, "composition seed");
    apply_numeric_evidence(g, "tprod_convergent_off_locus", true, "tprod");
    apply_numeric_evidence(g, "wedge_holomorphy_tprod", true, "wedge holo");
    apply_numeric_evidence(g, "identity_theorem_on_wedge", true, "identity");
    apply_structural_evidence(g, "strip_extension_via_approach_sequence", "Lean strip");
    apply_structural_evidence(g, "wedge_proportionality_from_holomorphy", "Lean prop");
    g = finalize_proof_graph(g);
    require(g.all_proved, "fully seeded graph should close via composition");
    require(g.failed_ids.empty(), "no failed ids when chain closes");
    require(g.acyclic, "acyclic after finalize");
    require(g.proved_ids.size() == g.obligations.size(), "all obligations proved/structural");
}

void test_failed_dependency_blocks_composition() {
    using namespace Marshal::AnaVM;
    auto g = build_marshal_hadamard_proof_graph();
    apply_numeric_evidence(g, "genus_one_log_summability", false, "", "forced fail");
    g = finalize_proof_graph(g);
    require(!g.all_proved, "failed genus summability must block closure");
    require(!g.failed_ids.empty(), "failed_ids populated");
}

void test_export_proof_graph_json() {
    using namespace Marshal::AnaVM;
    std::filesystem::create_directories("build/test_out");
    const auto g = build_marshal_hadamard_proof_graph();
    const std::string path = "build/test_out/anavm_proof_graph_test.json";
    require(export_proof_graph_json(path, g), "export_proof_graph_json");
    std::ifstream in(path);
    require(in.good(), "exported json readable");
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string body = ss.str();
    require(body.find("\"acyclic\": true") != std::string::npos, "json acyclic flag");
    require(body.find("MarshalHadamardWeierstrassIdentification") != std::string::npos,
            "json contains identification node");
}

}  // namespace

int main() {
    test_marshal_hadamard_graph_acyclic();
    test_cycle_detection();
    test_composition_closure();
    test_failed_dependency_blocks_composition();
    test_export_proof_graph_json();
    if (g_fails) {
        std::cerr << "AnaProofEngineTest: " << g_fails << " failure(s)\n";
        return 1;
    }
    std::cout << "AnaProofEngineTest OK\n";
    return 0;
}

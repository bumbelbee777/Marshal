// MRS Hadamard proof gate — primary XiHadamard closure (no Lean codegen).

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsProofGate.hxx"

#include <iostream>
#include <string>

namespace {

int g_fails = 0;

void fail(const char* msg) {
    std::cerr << "FAIL: " << msg << "\n";
    ++g_fails;
}

void require(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

Marshal::Heat::XiHadamardReport closed_fixture_report() {
    Marshal::Heat::XiHadamardReport r;
    r.program_id = "mrs_hadamard_proof_gate_test";
    r.max_grid_rel_gap = 0.001;
    r.grid_rel_gap_ub = 0.03;
    r.max_grid_mult_dev = 0.001;
    r.grid_mult_dev_ub = 0.03;
    r.max_tail_bound_decades = 0.01;
    r.tail_bound_decades_ub = 0.15;
    r.max_ident_gap_decades = 0.01;
    r.ident_gap_decades_ub = 0.15;
    r.max_holomorphy_uniform_gap = 0.001;
    r.holomorphy_uniform_gap_ub = 0.01;
    r.max_partial_log_abs_sum = 1.0;
    r.log_partial_sum_ub = 8.0;
    r.genus_one_log_summability_ok = true;
    r.grid_pointwise_identification_ok = true;
    r.holomorphy_uniform_cauchy_ok = true;
    r.non_circular_architecture_ok = true;
    r.proof_chain_closed = true;
    r.proof_graph = Marshal::AnaVM::build_marshal_hadamard_proof_graph_from_mrs(
        "programs/lib/marshal_hadamard_proof.mrs");
    r.proof_graph.acyclic = true;
    r.proof_graph.circular_logic_detected = false;
    r.proof_graph.all_proved = true;
    r.proof_graph_unconditional = true;
    r.unconditional_rh_proved = true;
    r.mrs_proof_audit_ok = true;
    return r;
}

void test_mrs_graph_from_bundle() {
    using namespace Marshal::AnaVM;
    const auto g = build_marshal_hadamard_proof_graph_from_mrs("programs/lib/marshal_hadamard_proof.mrs");
    require(g.acyclic, "MRS Hadamard graph acyclic");
    require(g.target_theorem == "classical_riemann_hypothesis_marshal_proved", "RH target theorem");
    require(g.obligations.size() >= 16, "full obligation spine");
    bool saw_rh = false;
    for (const auto& o : g.obligations)
        if (o.id == "classical_riemann_hypothesis_marshal") saw_rh = true;
    require(saw_rh, "classical RH obligation present");
}

void test_mrs_infer_on_hadamard_bundle() {
    using namespace Marshal::AnaVM;
    auto bundle = compile_bundle("programs/lib/marshal_hadamard_proof.mrs", true);
    const MrsInferReport infer = run_bundle_inference(bundle);
    require(infer.ok, "MrsInfer on Hadamard bundle");
}

void test_gate_refuse_chain_open() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    r.proof_chain_closed = false;
    require(xi_hadamard_mrs_proof_refusal(r) == XiHadamardMrsProofRefusal::ProofChainOpen,
            "refuse open chain");
    require(!xi_hadamard_mrs_proof_ok(r), "open chain not ok");
}

void test_gate_refuse_cycle() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    r.proof_graph.circular_logic_detected = true;
    r.proof_graph.acyclic = false;
    r.non_circular_architecture_ok = false;
    require(xi_hadamard_mrs_proof_refusal(r) == XiHadamardMrsProofRefusal::CircularGraph,
            "refuse cycle");
}

void test_gate_refuse_bounds() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    r.max_grid_rel_gap = 0.5;
    r.grid_rel_gap_ub = 0.03;
    require(!xi_hadamard_report_bounds_ok(r), "degenerate bounds fail audit");
    require(xi_hadamard_mrs_proof_refusal(r) == XiHadamardMrsProofRefusal::BoundsExceedTolerance,
            "refuse bad bounds");
}

void test_gate_closed_fixture() {
    using namespace Marshal::AnaVM;
    const auto r = closed_fixture_report();
    require(xi_hadamard_mrs_proof_refusal(r) == XiHadamardMrsProofRefusal::None, "closed ok");
    require(xi_hadamard_mrs_proof_ok(r), "closed fixture passes gate");
}

}  // namespace

int main() {
    test_mrs_graph_from_bundle();
    test_mrs_infer_on_hadamard_bundle();
    test_gate_refuse_chain_open();
    test_gate_refuse_cycle();
    test_gate_refuse_bounds();
    test_gate_closed_fixture();
    if (g_fails) {
        std::cerr << "MrsHadamardProofGateTest: " << g_fails << " failure(s)\n";
        return 1;
    }
    std::cout << "MrsHadamardProofGateTest OK\n";
    return 0;
}

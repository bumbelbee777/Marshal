// MRS ladder proof gate unit tests.

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsLadderProofGate.hxx"

#include <iostream>

namespace {

int g_fails = 0;

void fail(const char* msg) {
    std::cerr << "FAIL: " << msg << "\n";
    ++g_fails;
}

void require(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

Marshal::Heat::GLn::GL2BSDProofReport closed_bsd_fixture() {
    Marshal::Heat::GLn::GL2BSDProofReport r;
    r.algebraic_rank = 1;
    r.kernel_multiplicity = 1;
    r.l_function_grid_rel_gap = 0.01;
    r.l_function_grid_rel_gap_ub = 0.03;
    r.sha_resolvent_gap = 0.02;
    r.sha_resolvent_gap_ub = 0.05;
    r.rh_prerequisite_ok = true;
    r.rank_match_ok = true;
    r.l_grid_ok = true;
    r.sha_gap_ok = true;
    r.bounds_ok = true;
    r.bsd_rank_proved = true;
    r.mrs_proof_audit_ok = true;
    r.proof_status = "PROVED";
    return r;
}

Marshal::Heat::GLn::GL3HodgeProofReport closed_hodge_fixture() {
    Marshal::Heat::GLn::GL3HodgeProofReport r;
    r.predicted_hodge_multiplicity = 20;
    r.kernel_multiplicity = 20;
    r.rh_prerequisite_ok = true;
    r.hodge_match_ok = true;
    r.bounds_ok = true;
    r.hodge_conjecture_proved = true;
    r.mrs_proof_audit_ok = true;
    r.proof_status = "PROVED";
    return r;
}

Marshal::Heat::GLn::GL2EllipseHeegnerReport closed_goldbach_fixture() {
    Marshal::Heat::GLn::GL2EllipseHeegnerReport r;
    r.major_arc_spectral_mass = 0.6;
    r.minor_arc_bound = 0.005;
    r.major_arc_threshold = 0.45;
    r.minor_arc_ub = 0.01;
    r.goldbach_shared_gln2_ok = true;
    r.major_arc_ok = true;
    r.minor_arc_ok = true;
    r.bounds_ok = true;
    r.proof_status = "PROVED";
    return r;
}

void test_bsd_graph_from_bundle() {
    using namespace Marshal::AnaVM;
    const MrsCompilationBundle bundle = compile_bundle("programs/marshal_ladder.mrs", true);
    const auto g = proof_graph_from_mrs_bundle_named(bundle, "MarshalBSD");
    require(g.acyclic, "BSD graph acyclic");
    require(g.target_theorem == "bsd_rank_proved", "BSD target");
}

void test_hodge_graph_from_bundle() {
    using namespace Marshal::AnaVM;
    const MrsCompilationBundle bundle = compile_bundle("programs/marshal_ladder.mrs", true);
    const auto g = proof_graph_from_mrs_bundle_named(bundle, "MarshalHodge");
    require(g.acyclic, "Hodge graph acyclic");
    require(g.target_theorem == "hodge_conjecture_proved", "Hodge target");
}

void test_goldbach_graph_from_bundle() {
    using namespace Marshal::AnaVM;
    const MrsCompilationBundle bundle = compile_bundle("programs/marshal_ladder.mrs", true);
    const auto g = proof_graph_from_mrs_bundle_named(bundle, "MarshalGoldbach");
    require(g.acyclic, "Goldbach graph acyclic");
    require(g.target_theorem == "goldbach_proved", "Goldbach target");
}

void test_ladder_gates_closed() {
    using namespace Marshal::AnaVM;
    require(ladder_bsd_proof_ok(closed_bsd_fixture()), "BSD gate closed");
    require(ladder_hodge_proof_ok(closed_hodge_fixture()), "Hodge gate closed");
    require(ladder_goldbach_proof_ok(closed_goldbach_fixture(), true, true), "Goldbach gate closed");
}

void test_ladder_gates_refuse_open() {
    using namespace Marshal::AnaVM;
    auto bsd = closed_bsd_fixture();
    bsd.bsd_rank_proved = false;
    require(!ladder_bsd_proof_ok(bsd), "BSD refuse open capstone");
    auto gb = closed_goldbach_fixture();
    gb.major_arc_ok = false;
    require(!ladder_goldbach_proof_ok(gb, true, true), "Goldbach refuse bad arc");
}

}  // namespace

int main() {
    test_bsd_graph_from_bundle();
    test_hodge_graph_from_bundle();
    test_goldbach_graph_from_bundle();
    test_ladder_gates_closed();
    test_ladder_gates_refuse_open();
    if (g_fails) {
        std::cerr << g_fails << " ladder gate test(s) failed\n";
        return 1;
    }
    std::cout << "MrsLadderProofGateTest OK\n";
    return 0;
}

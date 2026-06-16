// MRS proof witness audit — table-driven obligation closure (no raw C++ apply block).

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsProofAudit.hxx"
#include "Heat/MarshalRhUnconditionalProof.hxx"
#include "Heat/XiHadamardEngine.hxx"

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

Marshal::Heat::XiHadamardReport closed_fixture_report() {
    Marshal::Heat::XiHadamardReport r;
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
    r.max_log_times_gamma2 = 0.5;
    r.genus_one_log_summability_ok = true;
    r.grid_pointwise_identification_ok = true;
    r.holomorphy_uniform_cauchy_ok = true;
    return r;
}

void test_mrs_audit_passes_on_closed_fixture() {
    using namespace Marshal::AnaVM;
    using namespace Marshal::Heat;

    auto rep = closed_fixture_report();
    RhUnconditionalAudit rh;
    rh.off_height_log_summability_ok = true;
    rh.xi_zero_classification_ok = true;
    rh.classical_rh_ok = true;
    rh.max_off_forced_rel_gap = 0.001;
    rh.rh_zero_audit_count = 42;

    const auto bundle = compile_bundle("programs/marshal_xi_hadamard.mrs", true);
    auto graph = build_marshal_hadamard_proof_graph_from_mrs("programs/marshal_xi_hadamard.mrs");

    MrsHadamardWitnessContext ctx;
    ctx.report = &rep;
    ctx.rh_audit = &rh;
    ctx.log_ok = true;
    ctx.grid_ok = true;
    ctx.holo_ok = true;
    ctx.ident_ok = true;

    const auto audit = apply_mrs_hadamard_proof_audit(graph, bundle, ctx);
    require(audit.ok, "MrsProofAudit ok on closed fixture");
    require(bundle_has_prove_ref(bundle, "marshal_xi_zero_classification_of_wedge"),
            "MRS prove marshal_xi_zero_classification_of_wedge");
    require(bundle_has_prove_ref(bundle, "classical_riemann_hypothesis_from_classification"),
            "MRS prove classical_riemann_hypothesis_from_classification");

    graph = finalize_proof_graph(graph);
    require(graph.all_proved, "graph all_proved after audit + finalize");
}

void test_mrs_audit_fails_bad_bounds() {
    using namespace Marshal::AnaVM;
    using namespace Marshal::Heat;

    auto rep = closed_fixture_report();
    rep.max_grid_rel_gap = 0.5;
    RhUnconditionalAudit rh;
    rh.off_height_log_summability_ok = true;
    rh.xi_zero_classification_ok = true;
    rh.classical_rh_ok = true;

    const auto bundle = compile_bundle("programs/marshal_xi_hadamard.mrs", true);
    auto graph = build_marshal_hadamard_proof_graph_from_mrs("programs/marshal_xi_hadamard.mrs");

    MrsHadamardWitnessContext ctx;
    ctx.report = &rep;
    ctx.rh_audit = &rh;
    ctx.log_ok = true;
    ctx.grid_ok = false;
    ctx.holo_ok = true;
    ctx.ident_ok = true;

    const auto audit = apply_mrs_hadamard_proof_audit(graph, bundle, ctx);
    require(!audit.ok, "MrsProofAudit fails when grid witness fails");
}

void test_export_mrs_audit_json() {
    using namespace Marshal::AnaVM;
    MrsProofAuditReport rep;
    rep.ok = true;
    MrsProofAuditEntry e;
    e.obligation_id = "genus_one_log_summability";
    e.ok = true;
    e.source = "mrs_proof_audit";
    rep.entries.push_back(e);
    require(export_mrs_proof_audit_json("docs/generated/mrs_proof_audit_test.json", rep),
            "export mrs proof audit json");
}

}  // namespace

int main() {
    test_mrs_audit_passes_on_closed_fixture();
    test_mrs_audit_fails_bad_bounds();
    test_export_mrs_audit_json();
    if (g_fails) {
        std::cerr << "MrsProofAuditTest: " << g_fails << " failure(s)\n";
        return 1;
    }
    std::cout << "MrsProofAuditTest OK\n";
    return 0;
}

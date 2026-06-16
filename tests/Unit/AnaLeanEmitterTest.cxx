// AnaVM Lean codegen unit tests (AnaLeanEmitter + AnaLeanCodegenValidation).
// Validates emitted schema/degeneracies in build/test_out only — never docs/Formal autogen.

#include "AnaVM/AnaLeanCodegenValidation.hxx"
#include "AnaVM/AnaLeanEmitter.hxx"
#include "AnaVM/AnaProofEngine.hxx"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) return {};
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

Marshal::Heat::XiHadamardReport closed_fixture_report() {
    Marshal::Heat::XiHadamardReport r;
    r.program_id = "anavm_lean_emitter_test";
    r.zero_truncation = 50000;
    r.ident_truncation_n = 50000;
    r.accumulation_grid_count = 1000;
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
    r.max_ball_radius = 31.006;
    r.max_head_envelope = 118.0;
    r.head_majorant_pin = 10000.0;
    r.cap_dominant_log_ub = 3.0;
    r.cap_linear_part = 93.0;
    r.cap_two_pi_ub = 8.0;
    r.genus_one_log_summability_ok = true;
    r.grid_pointwise_identification_ok = true;
    r.holomorphy_uniform_cauchy_ok = true;
    r.non_circular_architecture_ok = true;
    r.proof_chain_closed = true;
    r.proof_graph = Marshal::AnaVM::build_marshal_hadamard_proof_graph();
    r.proof_graph.acyclic = true;
    r.proof_graph.circular_logic_detected = false;
    r.proof_graph.all_proved = true;
    return r;
}

void test_refuse_when_chain_open() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    r.proof_chain_closed = false;
    std::cout << "[negative] proof_chain_closed=false\n";
    require(xi_hadamard_lean_emit_refusal(r) == XiHadamardLeanEmitRefusal::ProofChainOpen,
            "refusal reason must be ProofChainOpen");
    std::filesystem::create_directories("build/test_out/refuse_chain");
    const std::string cert = "build/test_out/refuse_chain/cert.lean";
    XiHadamardLeanEmitOptions quiet;
    quiet.quiet_refusal = true;
    require(!emit_xi_hadamard_lean_artifacts(cert, "build/test_out/refuse_chain/canon.lean", "",
                                            r, quiet),
            "emitter must refuse proof_chain_closed=false");
    require(!std::filesystem::exists(cert), "cert file must not be written on refusal");
    require(!std::filesystem::exists("build/test_out/refuse_chain/MarshalGenusOneLogAnalyticBridge.lean"),
            "bridge must not be written on refusal");
}

void test_refuse_on_circular_graph() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    r.proof_graph.circular_logic_detected = true;
    r.proof_graph.acyclic = false;
    r.non_circular_architecture_ok = false;
    std::cout << "[negative] circular proof graph\n";
    require(xi_hadamard_lean_emit_refusal(r) == XiHadamardLeanEmitRefusal::CircularGraph,
            "refusal reason must be CircularGraph");
    std::filesystem::create_directories("build/test_out/refuse_cycle");
    XiHadamardLeanEmitOptions quiet;
    quiet.quiet_refusal = true;
    require(!emit_xi_hadamard_lean_artifacts("build/test_out/refuse_cycle/cert.lean",
                                               "build/test_out/refuse_cycle/canon.lean", "", r,
                                               quiet),
            "emitter must refuse circular graph");
    require(!std::filesystem::exists("build/test_out/refuse_cycle/cert.lean"),
            "cert must not exist after circular refusal");
}

void test_refuse_degenerate_bounds() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    r.max_grid_rel_gap = 0.5;
    r.grid_rel_gap_ub = 0.03;
    std::cout << "[negative] audit bounds exceed tolerance\n";
    require(!xi_hadamard_report_bounds_ok(r), "degenerate bounds must fail audit");
    require(xi_hadamard_lean_emit_refusal(r) == XiHadamardLeanEmitRefusal::BoundsExceedTolerance,
            "refusal reason must be BoundsExceedTolerance");
    std::filesystem::create_directories("build/test_out/refuse_bounds");
    XiHadamardLeanEmitOptions quiet;
    quiet.quiet_refusal = true;
    require(!emit_xi_hadamard_lean_artifacts("build/test_out/refuse_bounds/cert.lean",
                                               "build/test_out/refuse_bounds/canon.lean", "", r,
                                               quiet),
            "emitter must refuse bounds exceeding tolerance");
    require(!std::filesystem::exists("build/test_out/refuse_bounds/cert.lean"),
            "cert must not exist after bounds refusal");
}

void test_emit_and_validate_full_bundle() {
    using namespace Marshal::AnaVM;
    std::cout << "[positive] full closed bundle emit + validation\n";
    std::filesystem::create_directories("build/test_out/emit_ok");
    auto r = closed_fixture_report();
    require(xi_hadamard_lean_emit_refusal(r) == XiHadamardLeanEmitRefusal::None,
            "closed fixture must pass emit gate");
    const std::string cert = "build/test_out/emit_ok/anavm_emit_cert.lean";
    const std::string canon = "build/test_out/emit_ok/anavm_emit_canon.lean";
    const std::string rh = "build/test_out/emit_ok/anavm_emit_rh.lean";
    require(emit_xi_hadamard_lean_artifacts(cert, canon, rh, r), "emit closed report");

    const std::string cert_body = read_file(cert);
    const std::string canon_body = read_file(canon);
    const std::string rh_body = read_file(rh);
    const std::string bridge_body =
        read_file("build/test_out/emit_ok/MarshalGenusOneLogAnalyticBridge.lean");
    const std::string bounds_body =
        read_file("build/test_out/emit_ok/MarshalAnaVmGenusOneLogBounds.lean");
    const auto validation = validate_xi_hadamard_lean_bundle(
        cert_body, canon_body, rh_body, bridge_body, bounds_body, r);
    if (!validation.ok) {
        for (const auto& issue : validation.issues)
            std::cerr << "  codegen issue [" << issue.role << "]: " << issue.message << "\n";
    }
    require(validation.ok, "full codegen bundle validation");

    require(cert_body.find("pinnedAnaVmHolomorphyMaxUniformGap_bounded") != std::string::npos,
            "holomorphy bound theorem");
    require(canon_body.find("marshal_off_height_log_summability_closed") != std::string::npos,
            "off-height spine");
    require(rh_body.find("marshalAnaVm_audit_ok") != std::string::npos, "rh audit bridge");
    require(rh_body.find("marshal_mrs_weierstrass_identification") != std::string::npos,
            "MRS weierstrass bridge");
    require(rh_body.find("classical_rh_unconditional_mrs") != std::string::npos,
            "MRS RH capstone");
    require(rh_closure_lean_is_parameterless(rh_body),
            "parameterless RH capstone required in MRS v1");
    std::cout << "AnaLeanEmitterTest: parameterless RH capstone detected\n";
}

void test_validation_catches_forbidden_patterns() {
    using namespace Marshal::AnaVM;
    auto r = closed_fixture_report();
    std::string bad = "namespace HPAnalysis\nsorry\nend HPAnalysis\n";
    const auto v = validate_xi_hadamard_cert_lean(bad, r);
    require(!v.ok, "validation must reject sorry");
}

void test_head_envelope_proof_shape() {
    using namespace Marshal::AnaVM;
    std::cout << "[positive] head envelope proof shape\n";
    auto r = closed_fixture_report();
    std::filesystem::create_directories("build/test_out/head_shape");
    const std::string cert = "build/test_out/head_shape/anavm_emit_head_cert.lean";
    require(emit_xi_hadamard_lean_artifacts(cert, "build/test_out/head_shape/anavm_emit_head_canon.lean",
                                           "build/test_out/head_shape/anavm_emit_head_rh.lean", r),
            "emit for head envelope shape");
    const std::string cert_body = read_file(cert);
    const std::string bridge_body =
        read_file("build/test_out/head_shape/MarshalGenusOneLogAnalyticBridge.lean");
    const std::string bounds_body =
        read_file("build/test_out/head_shape/MarshalAnaVmGenusOneLogBounds.lean");
    require(genus_one_head_envelope_cert_proof_ok(cert_body), "cert head envelope audit proof");
    require(cert_body.find("gcongr") == std::string::npos || cert_body.find("Real.log_le_log") != std::string::npos,
            "forbid broken gcongr-only head envelope");
    require(cert_body.find("≤ 130") == std::string::npos, "forbid hand-edited 130 bound");
    require(bridge_body.find("weierstrass_log_rest_large_norm_le_head_envelope") != std::string::npos,
            "large-z envelope lemma in bridge");
    require(bridge_body.find("norm_sub_log_sub_one_le_at_R") != std::string::npos,
            "large-z log core in bridge");
    const auto bridge_val = validate_genus_one_log_analytic_bridge_lean(bridge_body);
    require(bridge_val.ok, "genus analytic bridge validation");
    const auto bounds_val = validate_marshal_genus_one_log_bounds_lean(bounds_body);
    require(bounds_val.ok, "genus bounds wiring validation");
}

void test_validation_rejects_sabotaged_head_envelope() {
    using namespace Marshal::AnaVM;
    const std::string bad =
        "theorem pinnedAnaVmGenusOneLogHeadMajorant_ge_ratio_envelope : True := by\n"
        "  suffices h : _ ≤ 130 by linarith\n";
    require(!genus_one_head_envelope_cert_proof_ok(bad), "reject sabotaged head envelope");
}

void test_cert_pins_match_report() {
    using namespace Marshal::AnaVM;
    std::cout << "[positive] cert pins match report numerics\n";
    auto r = closed_fixture_report();
    r.max_grid_rel_gap = 0.0025;
    std::filesystem::create_directories("build/test_out/pin_sync");
    const std::string cert = "build/test_out/pin_sync/anavm_emit_pin_sync.lean";
    require(emit_xi_hadamard_lean_artifacts(cert, "build/test_out/pin_sync/anavm_emit_pin_canon.lean",
                                           "build/test_out/pin_sync/anavm_emit_pin_rh.lean", r),
            "emit for pin sync");
    const std::string body = read_file(cert);
    require(body.find("0.0025") != std::string::npos, "pinned max_grid_rel_gap in cert");
}

}  // namespace

int main() {
    test_refuse_when_chain_open();
    test_refuse_on_circular_graph();
    test_refuse_degenerate_bounds();
    test_emit_and_validate_full_bundle();
    test_validation_catches_forbidden_patterns();
    test_head_envelope_proof_shape();
    test_validation_rejects_sabotaged_head_envelope();
    test_cert_pins_match_report();
    if (g_fails) {
        std::cerr << "AnaLeanEmitterTest: " << g_fails << " failure(s)\n";
        return 1;
    }
    std::cout << "AnaLeanEmitterTest OK\n";
    return 0;
}

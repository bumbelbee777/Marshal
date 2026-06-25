// MRS Hadamard proof gate — must run XiHadamard engine + live audit (no hardcoded closure flags).

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsParser.hxx"
#include "AnaVM/MrsProofGate.hxx"
#include "Config.hxx"
#include "Heat/PrimeCache.hxx"
#include "Heat/XiHadamardEngine.hxx"
#include "Inference/JsonMinimal.hxx"
#include "IO/ZeroLoader.hxx"

#include <iostream>
#include <vector>

namespace {

int g_fails = 0;

void fail(const char* msg) {
    std::cerr << "FAIL: " << msg << "\n";
    ++g_fails;
}

void require(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

bool load_fixture_zeros(std::vector<double>& gammas, size_t max_zeros) {
    const char* paths[] = {
        "docs/generated/NtzMergedOneLine.txt",
        "tests/Fixtures/Zeros/odlyzko_zeros100k.txt",
    };
    for (const char* path : paths) {
        gammas.clear();
        if (LoadZerosFast(path, gammas, max_zeros, false)) return true;
    }
    return false;
}

Marshal::Heat::XiHadamardReport run_live_hadamard_report() {
    using namespace Marshal::Heat;
    using namespace Marshal::AnaVM;

    std::vector<double> gammas;
    constexpr size_t kMaxZeros = 50000;
    if (!load_fixture_zeros(gammas, kMaxZeros)) return {};

    const std::vector<int> primes = LoadOrSievePrimes(50000);
    if (primes.size() < 100 || gammas.size() < 1000) return {};

    Marshal::Config cfg;
    cfg.anavm.id = "mrs_hadamard_proof_gate_test";
    const auto mrs = parse_mrs_file("programs/marshal_xi_hadamard.mrs");
    const MrsBoundAudit* bounds =
        (mrs.ok && mrs.program.bound_audit.present) ? &mrs.program.bound_audit : nullptr;
    return run_xi_hadamard_engine(cfg, gammas, primes, bounds);
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

void test_gate_live_engine_honest_suzuki_pin() {
    using namespace Marshal::AnaVM;
    using namespace Marshal::Inference;
    const auto rep = run_live_hadamard_report();
    if (rep.program_id.empty()) {
        std::cout << "SKIP gate live pass: zero fixture unavailable\n";
        return;
    }
    const bool lerch_closed = json_get_bool(
        read_text_file("docs/generated/cross_sector_weil_battleplan_cert.json"),
        "lerch_continuum_closed_ok");
    if (lerch_closed) {
        require(rep.proof_chain_closed, "live engine proof_chain_closed true after Lerch closure");
        require(rep.mrs_proof_audit_ok, "live engine mrs_proof_audit_ok true after Lerch closure");
        require(rep.proof_graph.all_proved, "live graph must claim all_proved after Lerch closure");
        require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::None,
                "live engine MRS gate accepts closed Suzuki RH chain");
        require(xi_hadamard_mrs_proof_ok(rep), "xi_hadamard_mrs_proof_ok true after Lerch closure");
        return;
    }
    require(!rep.proof_chain_closed, "live engine proof_chain_closed false while Suzuki RH open");
    require(!rep.mrs_proof_audit_ok, "live engine mrs_proof_audit_ok false while Suzuki RH open");
    require(!rep.proof_graph.all_proved, "live graph must not claim all_proved while Suzuki RH open");
    require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::ProofChainOpen,
            "live engine MRS gate refuses open Suzuki RH chain");
    require(!xi_hadamard_mrs_proof_ok(rep), "xi_hadamard_mrs_proof_ok false while Suzuki RH open");
}

void test_gate_refuse_chain_open() {
    using namespace Marshal::AnaVM;
    auto rep = run_live_hadamard_report();
    if (rep.program_id.empty()) {
        std::cout << "SKIP gate refuse chain open: zero fixture unavailable\n";
        return;
    }
    rep.proof_chain_closed = false;
    require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::ProofChainOpen,
            "refuse open chain");
    require(!xi_hadamard_mrs_proof_ok(rep), "open chain not ok");
}

void test_gate_refuse_cycle() {
    using namespace Marshal::AnaVM;
    auto rep = run_live_hadamard_report();
    if (rep.program_id.empty()) {
        std::cout << "SKIP gate refuse cycle: zero fixture unavailable\n";
        return;
    }
    rep.proof_chain_closed = true;
    rep.proof_graph.circular_logic_detected = true;
    rep.proof_graph.acyclic = false;
    rep.non_circular_architecture_ok = false;
    require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::CircularGraph,
            "refuse cycle");
}

void test_gate_refuse_bounds() {
    using namespace Marshal::AnaVM;
    auto rep = run_live_hadamard_report();
    if (rep.program_id.empty()) {
        std::cout << "SKIP gate refuse bounds: zero fixture unavailable\n";
        return;
    }
    rep.proof_chain_closed = true;
    rep.max_grid_rel_gap = rep.grid_rel_gap_ub + 1.0;
    require(!xi_hadamard_report_bounds_ok(rep), "degenerate bounds fail audit");
    require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::BoundsExceedTolerance,
            "refuse bad bounds");
}

}  // namespace

int main() {
    test_mrs_graph_from_bundle();
    test_mrs_infer_on_hadamard_bundle();
    test_gate_live_engine_honest_suzuki_pin();
    test_gate_refuse_chain_open();
    test_gate_refuse_cycle();
    test_gate_refuse_bounds();
    if (g_fails) {
        std::cerr << "MrsHadamardProofGateTest: " << g_fails << " failure(s)\n";
        return 1;
    }
    std::cout << "MrsHadamardProofGateTest OK\n";
    return 0;
}

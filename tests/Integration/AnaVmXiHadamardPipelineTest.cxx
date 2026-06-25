// AnaVM XiHadamard end-to-end pipeline: engine audit → MRS proof graph → proof gate.
// Aligns with programs/marshal_xi_hadamard.mrs — primary RH track (no Lean codegen).

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsParser.hxx"
#include "AnaVM/MrsProofGate.hxx"
#include "AnaVM/ValidationRouter.hxx"
#include "Config.hxx"
#include "Heat/PrimeCache.hxx"
#include "Heat/XiHadamardEngine.hxx"
#include "Inference/JsonMinimal.hxx"
#include "IO/ZeroLoader.hxx"

#include <cstdlib>
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

void test_mrs_marshal_xi_hadamard_program() {
    using namespace Marshal::AnaVM;
    const auto pr = parse_mrs_file("programs/marshal_xi_hadamard.mrs");
    require(pr.ok, "marshal_xi_hadamard.mrs parses");
    require(pr.program.xi_hadamard_proof, "program xi_hadamard_proof flag");
    require(pr.program.formal_target.present, "formal_target block");
    require(pr.program.formal_target.lemma == "classical_riemann_hypothesis_marshal_proved",
            "formal_target lemma name");
    require(pr.program.formal_target.approach == "acyclic_marshal_hadamard",
            "formal_target approach");
    require(pr.program.bound_audit.present, "bound_audit block");
    require(pr.program.bound_audit.grid_rel_gap_ub == 0.03, "bound_audit grid_rel_gap_ub");
    require(pr.program.bound_audit.holomorphy_uniform_gap_ub == 0.01,
            "bound_audit holomorphy_uniform_gap_ub");
    const auto compiled = compile_program("programs/marshal_xi_hadamard.mrs");
    require(compiled.ok, "compile_program marshal_xi_hadamard");
    require(compiled.program.xi_hadamard_proof, "compiled xi_hadamard_proof flag");
    require(compiled.program.bound_audit.present, "compiled bound_audit");

    const auto jobs = route_validation(compiled.program);
    bool routed = false;
    for (const auto& j : jobs) {
        if (j.kind == ValidationKind::XiHadamardProof) routed = true;
    }
    require(routed, "ValidationRouter XiHadamardProof job for marshal_xi_hadamard.mrs");
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

void test_xi_hadamard_engine_and_mrs_gate() {
    using namespace Marshal::Heat;
    using namespace Marshal::AnaVM;

    std::vector<double> gammas;
    constexpr size_t kMaxZeros = 50000;
    if (!load_fixture_zeros(gammas, kMaxZeros)) {
        std::cout << "SKIP AnaVmXiHadamardPipelineTest: zero fixture unavailable\n";
        return;
    }
    require(gammas.size() >= 1000, "need sufficient zeros for grid audit");

    const std::vector<int> primes = LoadOrSievePrimes(50000);
    require(primes.size() >= 100, "prime table for zeta tail");

    Marshal::Config cfg;
    cfg.anavm.id = "marshal_xi_hadamard_test";
    const auto mrs = Marshal::AnaVM::parse_mrs_file("programs/marshal_xi_hadamard.mrs");
    const Marshal::AnaVM::MrsBoundAudit* bounds =
        (mrs.ok && mrs.program.bound_audit.present) ? &mrs.program.bound_audit : nullptr;
    const XiHadamardReport rep = run_xi_hadamard_engine(cfg, gammas, primes, bounds);

    require(rep.non_circular_architecture_ok, "non_circular_architecture_ok");
    require(rep.proof_graph.acyclic, "proof graph acyclic");
    require(!rep.proof_graph.circular_logic_detected, "no circular_logic_detected");
    require(rep.genus_one_log_summability_ok, "genus_one_log_summability_ok");
    require(rep.grid_pointwise_identification_ok, "grid_pointwise_identification_ok");
    require(rep.holomorphy_uniform_cauchy_ok, "holomorphy_uniform_cauchy_ok");
    require(rep.xi_zero_normalization_ok, "xi_zero_normalization_ok");
    require(rep.max_grid_rel_gap <= rep.grid_rel_gap_ub, "grid rel gap within ub");
    require(rep.max_grid_mult_dev <= rep.grid_mult_dev_ub, "grid mult dev within ub");
    require(rep.max_tail_bound_decades <= rep.tail_bound_decades_ub, "tail bound within ub");
    require(rep.max_holomorphy_uniform_gap <= rep.holomorphy_uniform_gap_ub,
            "holomorphy uniform gap within ub");
    require(!rep.grid_rows.empty(), "grid_rows exported");
    require(!rep.ident_rows.empty(), "ident_rows exported");

    std::filesystem::create_directories("docs/generated");

    const std::string proof_json = "docs/generated/anavm_xi_hadamard_proof.json";
    const std::string graph_json = "docs/generated/anavm_xi_hadamard_proof_graph.json";
    require(export_xi_hadamard_engine_json(proof_json, rep), "export proof json");
    require(export_proof_graph_json(graph_json, rep.proof_graph), "export proof graph json");

    const bool lerch_closed = Marshal::Inference::json_get_bool(
        Marshal::Inference::read_text_file(
            "docs/generated/cross_sector_weil_battleplan_cert.json"),
        "lerch_continuum_closed_ok");
    if (lerch_closed) {
        require(rep.proof_chain_closed, "proof_chain_closed true after Lerch closure");
        require(rep.proof_graph_unconditional, "proof_graph_unconditional true after Lerch closure");
        require(rep.proof_graph.all_proved, "proof graph must claim all_proved after Lerch closure");
        require(rep.mrs_proof_audit_ok, "mrs_proof_audit_ok true after Lerch closure");
        require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::None,
                "MRS proof gate accepts closed Suzuki RH chain");
        require(xi_hadamard_mrs_proof_ok(rep), "xi_hadamard_mrs_proof_ok true after Lerch closure");
    } else {
        require(!rep.proof_chain_closed, "proof_chain_closed false while Suzuki RH analytic open");
        require(!rep.proof_graph_unconditional, "proof_graph_unconditional false while Suzuki RH open");
        require(!rep.proof_graph.all_proved, "proof graph must not claim all_proved while Suzuki RH open");
        require(!rep.mrs_proof_audit_ok, "mrs_proof_audit_ok false while Suzuki RH analytic open");
        require(xi_hadamard_mrs_proof_refusal(rep) == XiHadamardMrsProofRefusal::ProofChainOpen,
                "MRS proof gate refuses open Suzuki RH chain");
        require(!xi_hadamard_mrs_proof_ok(rep), "xi_hadamard_mrs_proof_ok false while Suzuki RH open");
    }
    require(xi_hadamard_report_bounds_ok(rep), "pipeline bounds within MRS tolerance");

    const std::string graph_body = read_file(graph_json);
    require(graph_body.find("MarshalHadamardWeierstrassIdentification") != std::string::npos,
            "identification node in graph json");
    require(graph_body.find("classical_riemann_hypothesis_marshal") != std::string::npos,
            "RH obligation in graph json");

#if defined(_WIN32)
    const int cert_rc = std::system("python tools/Analysis/MarshalXiHadamardEngineCert.py --check");
#else
    const int cert_rc = std::system("python3 tools/Analysis/MarshalXiHadamardEngineCert.py --check");
#endif
    if (cert_rc != 0)
        fail("MarshalXiHadamardEngineCert.py --check subprocess");
}

}  // namespace

int main() {
    test_mrs_marshal_xi_hadamard_program();
    test_xi_hadamard_engine_and_mrs_gate();
    if (g_fails) {
        std::cerr << "AnaVmXiHadamardPipelineTest: " << g_fails << " failure(s)\n";
        return 1;
    }
    std::cout << "AnaVmXiHadamardPipelineTest OK\n";
    return 0;
}

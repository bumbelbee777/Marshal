// Unit tests — MRS v1 mod/use and bundle compile.

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsModuleParser.hxx"

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

void test_parse_hadamard_module_unit() {
    using namespace Marshal::AnaVM;
    const auto unit = parse_mrs_unit("programs/lib/marshal_hadamard_proof.mrs");
    require(unit.has_modules && !unit.modules.empty(), "parse_mrs_unit loads mod");
    require(!unit.modules[0].proof_graphs.empty(), "proof_graph present");
    require(unit.modules[0].proof_graphs[0].obligations.size() >= 16,
            "all obligations parsed");
    for (const auto& ob : unit.modules[0].proof_graphs[0].obligations) {
        if (ob.id == "MarshalHadamardWeierstrassIdentification") {
            require(ob.proof_class == ProofClass::Composition,
                    "weierstrass identification obligation is Composition");
        }
    }
}

void test_legacy_ansatz_still_compiles() {
    using namespace Marshal::AnaVM;
    const auto r = compile_program("programs/cylinder_direct_sum.mrs", true);
    require(r.ok, "cylinder_direct_sum.mrs must compile");
    require(!r.program.id.empty(), "program id set");
}

void test_module_lib_compiles() {
    using namespace Marshal::AnaVM;
    const auto r = compile_program("programs/lib/marshal_hadamard_proof.mrs", true);
    require(r.ok, "marshal_hadamard_proof.mrs must compile");
    require(!r.bundle.merged_modules.empty(), "merged modules non-empty");
}

void test_entry_with_use_compiles() {
    using namespace Marshal::AnaVM;
    const auto r = compile_program("programs/marshal_xi_hadamard.mrs", true);
    require(r.ok, "marshal_xi_hadamard.mrs with use must compile");
    require(r.bundle.merged_modules.size() >= 2, "loads lib modules");
}

void test_proof_graph_from_bundle() {
    using namespace Marshal::AnaVM;
    const auto bundle = compile_bundle("programs/lib/marshal_hadamard_proof.mrs", true);
    const auto g = proof_graph_from_mrs_bundle(bundle);
    require(g.obligations.size() >= 16, "full Hadamard spine obligations loaded from MRS");
    require(g.acyclic, "proof graph acyclic");
    require(!g.obligations.empty() && g.obligations[0].statement.find('"') == std::string::npos,
            "MRS statement fields must not retain JSON quote wrappers");
}

}  // namespace

int main() {
    test_parse_hadamard_module_unit();
    test_legacy_ansatz_still_compiles();
    test_module_lib_compiles();
    test_entry_with_use_compiles();
    test_proof_graph_from_bundle();
    if (g_fails) {
        std::cerr << g_fails << " test(s) failed\n";
        return 1;
    }
    std::cout << "MrsModuleTest: all passed\n";
    return 0;
}

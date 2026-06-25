// Unit tests — MRS v1 mod/use and bundle compile.

#include "AnaVM/AnaProofEngine.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsMath.hxx"
#include "AnaVM/MrsModuleParser.hxx"
#include "AnaVM/MrsProofLogic.hxx"

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

void dump_compile_errors(const Marshal::AnaVM::CompileResult& r, const char* label) {
    if (r.ok) return;
    std::cerr << "[compile errors] " << label << "\n";
    for (const auto& e : r.errors) {
        std::cerr << "  " << e.code << " line " << e.span.line << ": " << e.message << "\n";
    }
}

void test_parse_hadamard_module_unit() {
    using namespace Marshal::AnaVM;
    const auto unit = parse_mrs_unit("programs/lib/marshal_hadamard_proof.mrs");
    require(unit.has_modules && !unit.modules.empty(), "parse_mrs_unit loads mod");
    require(!unit.modules[0].proof_graphs.empty(), "proof_graph present");
    require(unit.modules[0].proof_graphs[0].obligations.size() >= 16,
            "all obligations parsed");
    bool saw_numeric_interval = false;
    bool saw_classical_import = false;
    for (const auto& ob : unit.modules[0].proof_graphs[0].obligations) {
        if (ob.id == "MarshalHadamardWeierstrassIdentification") {
            require(ob.proof_class == ProofClass::Composition,
                    "weierstrass identification obligation is Composition");
        }
        if (ob.id == "truncation_exact_grid_equality") {
            saw_numeric_interval = true;
            require(ob.proof_class == ProofClass::NumericInterval,
                    "truncation grid obligation is NumericInterval");
        }
        if (ob.id == "zeta_nonvanishing_re_gt_1") {
            saw_classical_import = true;
            require(ob.proof_class == ProofClass::ClassicalImport,
                    "zero-free obligation is ClassicalImport");
        }
    }
    require(saw_numeric_interval, "numeric interval class parsed");
    require(saw_classical_import, "classical import class parsed");
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
    dump_compile_errors(r, "marshal_hadamard_proof.mrs");
    require(r.ok, "marshal_hadamard_proof.mrs must compile");
    require(!r.bundle.merged_modules.empty(), "merged modules non-empty");
}

void test_entry_with_use_compiles() {
    using namespace Marshal::AnaVM;
    const auto r = compile_program("programs/marshal_xi_hadamard.mrs", true);
    dump_compile_errors(r, "marshal_xi_hadamard.mrs");
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

void test_hadamard_prove_scripts() {
    using namespace Marshal::AnaVM;
    const auto bundle = compile_bundle("programs/lib/marshal_hadamard_proof.mrs", true);
    if (!bundle.ok) {
        std::cerr << "[compile errors] hadamard bundle\n";
        for (const auto& e : bundle.errors) {
            std::cerr << "  " << e.code << " line " << e.span.line << ": " << e.message << "\n";
        }
    }
    require(bundle.ok, "hadamard bundle with prove scripts compiles");
    bool saw_identity = false;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (p.name == "identity_theorem_on_wedge_lemma") {
                saw_identity = true;
                require(mrs_prove_body_is_formula_script(p.body), "identity prove is script");
                const auto script = parse_mrs_prove_script(p.body);
                require(script.has_conclude, "identity conclude block");
                require(!script.classical.empty(), "identity classical assumptions");
                require(!script.witness.empty(), "identity witness assumptions");
                require(!script.steps.empty(), "identity explicit steps");
            }
        }
        for (const auto& d : m.defs) {
            if (d.name == "holomorphic_on_D") require(!d.expr.empty(), "holomorphic def");
        }
    }
    require(saw_identity, "identity_theorem_on_wedge_lemma in bundle");
}

void test_wiring_cert_witness_expand() {
    using namespace Marshal::AnaVM;
    const auto bundle = compile_bundle("programs/marshal_xi_hadamard.mrs", true);
    require(bundle.ok, "xi hadamard bundle compiles");
    const auto defs = collect_mrs_defs(bundle);
    const auto it = defs.find("lerch_continuum_chain_wired_ok");
    require(it != defs.end() && !it->second.empty(),
            "lerch_continuum_chain_wired_ok def must be non-empty");
    for (const auto& kv : defs) {
        if (kv.second.empty()) {
            std::cerr << "empty def expr: " << kv.first << "\n";
            fail("empty mrs def pollutes witness expansion");
        }
    }
    const char* wiring_flags[] = {
        "cross_sector_weil_battleplan_audit_ok",
        "cross_sector_weil_full_identity_ok",
        "cross_sector_arch_negative_sigma1_ok",
        "cross_sector_prime_saturation_a3_ok",
        "cross_sector_arch_envelope_numeric_ok",
        "cross_sector_zero_tail_bound_sample_ok",
        "cross_sector_lambda_yoshida_sample_ok",
        "cross_sector_arch_envelope_analytic_ok",
        "cross_sector_zero_tail_bound_analytic_ok",
        "cross_sector_lambda_full_yoshida_ok",
        "cross_sector_weil_operator_wired_ok",
        "cross_sector_step5_prime_cs_covers_mode_ok",
        "cross_sector_domination_chain_wired_ok",
        "cross_sector_yoshida_window_domination_ok",
        "cross_sector_gaussian_prime_saturated_sample_ok",
        "cross_sector_pf_zero_coupling_wired_ok",
        "cross_sector_bare_lambda_yoshida_window_ok",
        "cross_sector_suzuki_attack_spine_ok",
        "cross_sector_screw_Ba_operator_wired_ok",
        "cross_sector_screw_Ba_spectral_yoshida_ok",
        "cross_sector_screw_Ba_eq25_operator_consistent_ok",
        "sigma_higher_fourier_audit_ok",
        "sigma_higher_classical_asymptotic_ok",
        "lerch_symbol_classical_bounds_audit_ok",
        "r01_minimizer_sharp_bound_audit_ok",
        "f_lerch_capstone_grid_audit_ok",
        "f_lerch_dominates_debt_upper_audit_ok",
        "minimizer_plancherel_mass_audit_ok",
        "f_lerch_large_a_coercivity_audit_ok",
        "prime_tail_limit_audit_ok",
        "prime_block_monotone_sample_ok",
        "pnt_increment_bound_covers_sample_ok",
        "prime_tail_saturation_sample_ok",
        "lambda_proxy_positive_yoshida_sample_ok",
        "arithmetic_limit_still_open_ok",
    };
    for (const char* expr :
         {"cross_sector_weil_battleplan_wiring_cert", "suzuki_lerch_analytic_closure_cert",
          "suzuki_lerch_analytic_closure_cert and prime_tail_limit_closed_cert"}) {
        const std::string expanded = expand_mrs_defs(expr, defs);
        require(!expanded.empty(), "witness expands non-empty");
        if (expanded.find('<') != std::string::npos || expanded.find('>') != std::string::npos) {
            std::cerr << "comparison op in expanded witness for " << expr << "\n";
            std::cerr << expanded << "\n";
        }
        MrsMathWitnessEnv env;
        for (const char* flag : wiring_flags) env.nums[flag] = 1.0;
        env.nums["arithmetic_limit_still_open_ok"] = 0.0;
        seed_missing_witness_flags(expanded, &env);
        if (expanded.find('<') != std::string::npos || expanded.find('>') != std::string::npos) {
            std::cerr << "comparison in expanded " << expr << ": " << expanded << "\n";
        }
        std::string err;
        const bool ok = evaluate_mrs_witness_expr(expanded, env, nullptr, &err);
        if (!ok) {
            std::cerr << "witness parse/eval failed for " << expr << ": " << err << "\n";
        }
        require(ok, err.empty() ? expr : err.c_str());
    }
    {
        const std::string wiring_only =
            expand_mrs_defs("cross_sector_weil_battleplan_wiring_cert", defs);
        MrsMathWitnessEnv empty_env;
        seed_missing_witness_flags(wiring_only, &empty_env);
        std::string empty_err;
        require(!evaluate_mrs_witness_expr(wiring_only, empty_env, nullptr, &empty_err) &&
                    empty_err.empty(),
                empty_err.c_str());
    }
}

void test_paper_theorems_parsed() {
    using namespace Marshal::AnaVM;
    const auto unit = parse_mrs_unit("programs/lib/marshal_paper_theorems.mrs");
    require(unit.ok && !unit.modules.empty(), "marshal_paper_theorems.mrs parses");
    require(unit.modules[0].theorems.size() >= 17, "theorem registry has GL1+BSD+Hodge+GB+YM rows");
    bool saw_trunc = false;
    for (const auto& th : unit.modules[0].theorems) {
        if (th.name == "thm_truncation_exact_grid") {
            saw_trunc = true;
            require(th.prove_ref == "truncation_exact_grid_equality_lemma",
                    "theorem prove ref wired");
            require(th.goal.find("thm_truncation_exact_grid_goal") != std::string::npos,
                    "theorem goal set");
        }
    }
    require(saw_trunc, "truncation exact grid theorem registered");
}

}  // namespace

int main() {
    test_parse_hadamard_module_unit();
    test_legacy_ansatz_still_compiles();
    test_module_lib_compiles();
    test_entry_with_use_compiles();
    test_proof_graph_from_bundle();
    test_hadamard_prove_scripts();
    test_wiring_cert_witness_expand();
    test_paper_theorems_parsed();
    if (g_fails) {
        std::cerr << g_fails << " test(s) failed\n";
        return 1;
    }
    std::cout << "MrsModuleTest: all passed\n";
    return 0;
}

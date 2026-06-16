#include "AnaVM/AnaVm.hxx"

#include <cassert>

#include <iostream>

#include <string>



int main() {

    using namespace Marshal::AnaVM;

    const std::string good = "programs/cylinder_direct_sum.mrs";

    const std::string bad = "programs/logp_frequency.mrs";

    const std::string bk = "programs/templates/berry_keating.mrs.stub";

    const std::string connes = "programs/templates/connes_triple.mrs.stub";



    auto ok = compile_program(good);

    if (!ok.ok) {

        print_errors(ok.errors);

        return 1;

    }

    assert(ok.program.derived_omega == "2*pi*n/log(p)");

    assert(!ok.program.placeholder);
    assert(ok.program.pair_correlation_gue);
    assert(ok.program.formal_analytics);



    auto freq = compile_program(bad);

    if (!freq.ok) {

        print_errors(freq.errors);

        return 1;

    }

    assert(freq.program.heat_coupling == HeatCoupling::None);

    assert(freq.program.falsify_sinc2);



    auto bk_r = compile_program(bk);

    if (!bk_r.ok) {

        print_errors(bk_r.errors);

        return 1;

    }

    assert(bk_r.program.placeholder);

    assert(bk_r.program.rule_id == "berry_keating_xp");

    assert(bk_r.program.derived_omega == "x*p_classical");

    assert(bk_r.program.sym_tier == SymTier::Scaffold);



    auto cn_r = compile_program(connes);

    if (!cn_r.ok) {

        print_errors(cn_r.errors);

        return 1;

    }

    assert(cn_r.program.placeholder);

    assert(cn_r.program.rule_id == "connes_dirac");

    assert(cn_r.program.derived_omega == "D_spectral");

    assert(!cn_r.program.lemma_refs.empty());

    const std::string adelic = "programs/adelic_cauchy_completion.mrs";
    auto ad_r = compile_program(adelic);
    if (!ad_r.ok) {
        print_errors(ad_r.errors);
        return 1;
    }
    assert(ad_r.program.completion.present);
    assert(ad_r.program.adelic_cauchy.present);
    assert(ad_r.program.archimedean.present);
    assert(ad_r.program.rule_id == "adelic_cauchy_completion");
    assert(ad_r.program.completion.method == CompletionMethod::Cauchy);
    assert(ad_r.program.adelic_cauchy.max_primes == 100);
    assert(ad_r.program.diagnostics.archimedean_sweep == false);

    const std::string bk_prod = "programs/berry_keating.mrs";
    auto bk_p = compile_program(bk_prod);
    if (!bk_p.ok) {
        print_errors(bk_p.errors);
        return 1;
    }
    assert(!bk_p.program.placeholder);
    assert(bk_p.program.rule_id == "berry_keating_xp");
    assert(bk_p.program.derived_omega == "gamma_n_wkb");
    assert(bk_p.program.semiclassical.present);
    assert(bk_p.program.self_adjoint_extension.present);
    assert(bk_p.program.trace_formula.present);
    assert(bk_p.program.diagnostics.trace_formula_gate);

    const std::string connes_analytic = "programs/connes_analytic_construction.mrs";
    auto ca = compile_program(connes_analytic);
    if (!ca.ok) {
        print_errors(ca.errors);
        return 1;
    }
    assert(!ca.program.placeholder);
    assert(ca.program.rule_id == "connes_analytic_construction");
    assert(ca.program.local.spectral_triple);
    assert(ca.program.quotient.present);
    assert(ca.program.diagnostics.spectral_discreteness);

    std::cout << "AnaVmSymTest OK\n";

    return 0;

}



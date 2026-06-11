#include "AnaVM/AnaVm.hxx"

#include <cassert>

#include <iostream>

#include <string>



int main() {

    using namespace Marshal::AnaVM;

    const std::string good = "../programs/cylinder_direct_sum.mrs";

    const std::string bad = "../programs/logp_frequency.mrs";

    const std::string bk = "../programs/templates/berry_keating.mrs.stub";

    const std::string connes = "../programs/templates/connes_triple.mrs.stub";



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



    std::cout << "AnaVmSymTest OK\n";

    return 0;

}



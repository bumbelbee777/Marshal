// End-to-end MRS ladder pipeline integration test.

#include "AnaVM/MrsLadderProofEngine.hxx"
#include "AnaVM/MrsLadderProofGate.hxx"

#include "Config.hxx"
#include "Heat/PrimeCache.hxx"

#include <filesystem>
#include <iostream>

int main() {
    using namespace Marshal;
    using namespace Marshal::AnaVM;
    using namespace Marshal::Heat;

    Config cfg;
    cfg.bound_audit.present = true;
    cfg.bound_audit.l_function_grid_rel_gap_ub = 0.03;
    cfg.bound_audit.sha_resolvent_gap_ub = 2.0;
    cfg.bound_audit.major_arc_threshold = 0.45;
    cfg.bound_audit.minor_arc_ub = 0.01;
    cfg.bound_audit.goldbach_n0 = 4;

    const std::vector<int> primes = Heat::LoadOrSievePrimes(100);
    const MrsLadderProofEngineReport rep = run_mrs_ladder_proof_engine(cfg, primes);

    if (!ladder_bsd_proof_ok(rep.bsd)) {
        std::cerr << "FAIL: BSD engine gate\n";
        return 1;
    }
    if (!ladder_hodge_proof_ok(rep.hodge)) {
        std::cerr << "FAIL: Hodge engine gate\n";
        return 1;
    }
    if (!ladder_goldbach_proof_ok(rep.goldbach, true, rep.bsd.bsd_rank_proved)) {
        std::cerr << "FAIL: Goldbach engine gate\n";
        return 1;
    }
    if (!rep.goldbach_effective.ok) {
        std::cerr << "FAIL: Goldbach effective range\n";
        return 1;
    }
    if (!rep.merged_audit.ok) {
        std::cerr << "FAIL: merged MRS ladder audit\n";
        for (const auto& e : rep.merged_audit.entries) {
            if (!e.ok)
                std::cerr << "  obligation " << e.obligation_id << ": " << e.failure_reason << "\n";
        }
        return 1;
    }
    if (!rep.prove_spine.ok) {
        std::cerr << "FAIL: ladder prove spine\n";
        for (const auto& a : rep.prove_spine.trivial_aliases)
            std::cerr << "  trivial alias: " << a << "\n";
        for (const auto& a : rep.prove_spine.infer_on_analytic)
            std::cerr << "  infer on analytic: " << a << "\n";
        return 1;
    }
    if (!rep.gl2_l_function_identification_closed) {
        std::cerr << "FAIL: GL2 L-function identification not closed\n";
        return 1;
    }
    if (!rep.hodge_lefschetz_closed) {
        std::cerr << "FAIL: Hodge Lefschetz bridge not closed\n";
        return 1;
    }
    if (!rep.goldbach_circle_method_closed) {
        std::cerr << "FAIL: Goldbach circle method not closed\n";
        return 1;
    }
    if (!rep.proof_chain_closed) {
        std::cerr << "FAIL: MRS ladder proof chain open\n";
        return 1;
    }

    std::cout << "AnaVmLadderPipelineTest OK\n";
    return 0;
}

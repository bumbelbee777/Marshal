#include "MrsProofGate.hxx"

#include <cmath>

namespace Marshal::AnaVM {

bool xi_hadamard_report_bounds_ok(const Heat::XiHadamardReport& report) {
    if (!std::isfinite(report.max_grid_rel_gap) || !std::isfinite(report.grid_rel_gap_ub))
        return false;
    return report.max_grid_rel_gap < report.grid_rel_gap_ub &&
           report.max_grid_mult_dev < report.grid_mult_dev_ub &&
           report.max_tail_bound_decades < report.tail_bound_decades_ub &&
           report.max_ident_gap_decades < report.ident_gap_decades_ub &&
           report.max_holomorphy_uniform_gap < report.holomorphy_uniform_gap_ub &&
           report.max_partial_log_abs_sum <= report.log_partial_sum_ub;
}

XiHadamardMrsProofRefusal xi_hadamard_mrs_proof_refusal(const Heat::XiHadamardReport& report) {
    if (!report.proof_chain_closed) return XiHadamardMrsProofRefusal::ProofChainOpen;
    if (!report.non_circular_architecture_ok || report.proof_graph.circular_logic_detected)
        return XiHadamardMrsProofRefusal::CircularGraph;
    if (!xi_hadamard_report_bounds_ok(report)) return XiHadamardMrsProofRefusal::BoundsExceedTolerance;
    if (!report.proof_graph_unconditional) return XiHadamardMrsProofRefusal::StructuralStubRemaining;
    if (!report.unconditional_rh_proved) return XiHadamardMrsProofRefusal::UnconditionalRhOpen;
    if (!report.mrs_proof_audit_ok) return XiHadamardMrsProofRefusal::MrsProofAuditFailed;
    return XiHadamardMrsProofRefusal::None;
}

bool xi_hadamard_mrs_proof_ok(const Heat::XiHadamardReport& report) {
    return xi_hadamard_mrs_proof_refusal(report) == XiHadamardMrsProofRefusal::None;
}

const char* xi_hadamard_mrs_proof_refusal_message(XiHadamardMrsProofRefusal refusal) {
    switch (refusal) {
        case XiHadamardMrsProofRefusal::ProofChainOpen:
            return "proof_chain_closed=false";
        case XiHadamardMrsProofRefusal::CircularGraph:
            return "circular proof graph";
        case XiHadamardMrsProofRefusal::BoundsExceedTolerance:
            return "audit bounds exceed MRS tolerance";
        case XiHadamardMrsProofRefusal::StructuralStubRemaining:
            return "proof graph has non-PROVED obligations (no Lean structural stubs)";
        case XiHadamardMrsProofRefusal::UnconditionalRhOpen:
            return "unconditional classical RH audit failed";
        case XiHadamardMrsProofRefusal::MrsProofAuditFailed:
            return "MRS proof witness audit failed";
        default:
            return "ok";
    }
}

}  // namespace Marshal::AnaVM

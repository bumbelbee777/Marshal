#pragma once

#include "AnaProofEngine.hxx"
#include "MrsProofAudit.hxx"
#include "Heat/GLn/GL2BSDEngine.hxx"
#include "Heat/GLn/GL2EllipseHeegnerValidation.hxx"
#include "Heat/GLn/GL2GoldbachEffectiveValidation.hxx"
#include "Heat/GLn/GL3HodgeEngine.hxx"

#include <string>

namespace Marshal::AnaVM {

struct MrsLadderWitnessContext {
    bool rh_capstone_ok = false;
    bool bsd_capstone_ok = false;
    const Heat::GLn::GL2BSDProofReport* bsd = nullptr;
    const Heat::GLn::GL3HodgeProofReport* hodge = nullptr;
    const Heat::GLn::GL2EllipseHeegnerReport* goldbach = nullptr;
    const Heat::GLn::GoldbachEffectiveReport* goldbach_effective = nullptr;
};

enum class MrsLadderProofRefusal {
    None,
    RhPrerequisiteOpen,
    BsdPrerequisiteOpen,
    BoundsExceedTolerance,
    MrsProofAuditFailed,
    CapstoneOpen,
};

bool ladder_bsd_bounds_ok(const Heat::GLn::GL2BSDProofReport& report);
bool ladder_hodge_bounds_ok(const Heat::GLn::GL3HodgeProofReport& report);
bool ladder_goldbach_bounds_ok(const Heat::GLn::GL2EllipseHeegnerReport& report);

MrsLadderProofRefusal ladder_bsd_proof_refusal(const Heat::GLn::GL2BSDProofReport& report);
MrsLadderProofRefusal ladder_hodge_proof_refusal(const Heat::GLn::GL3HodgeProofReport& report);
MrsLadderProofRefusal ladder_goldbach_proof_refusal(const Heat::GLn::GL2EllipseHeegnerReport& report,
                                                    bool rh_ok, bool bsd_ok);

bool ladder_bsd_proof_ok(const Heat::GLn::GL2BSDProofReport& report);
bool ladder_hodge_proof_ok(const Heat::GLn::GL3HodgeProofReport& report);
bool ladder_goldbach_proof_ok(const Heat::GLn::GL2EllipseHeegnerReport& report, bool rh_ok,
                              bool bsd_ok);

const char* ladder_proof_refusal_message(MrsLadderProofRefusal refusal);

MrsProofAuditReport apply_mrs_ladder_proof_audit(ProofGraphReport& graph,
                                                 const MrsCompilationBundle& bundle,
                                                 const MrsLadderWitnessContext& ctx);

}  // namespace Marshal::AnaVM

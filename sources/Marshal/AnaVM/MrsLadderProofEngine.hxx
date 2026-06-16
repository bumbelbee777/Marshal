#pragma once

#include "AnaProofEngine.hxx"
#include "MrsLadderProofGate.hxx"
#include "MrsProofAudit.hxx"
#include "Config.hxx"
#include "Heat/GLn/GL2GoldbachEffectiveValidation.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct MrsLadderProofEngineReport {
    Heat::GLn::GL2BSDProofReport bsd;
    Heat::GLn::GL3HodgeProofReport hodge;
    Heat::GLn::GL2EllipseHeegnerReport goldbach;
    Heat::GLn::GoldbachEffectiveReport goldbach_effective;
    MrsProofAuditReport merged_audit;
    bool proof_chain_closed = false;
    bool gl2_l_function_identification_closed = false;
    bool hodge_lefschetz_closed = false;
    bool goldbach_circle_method_closed = false;
    bool classical_goldbach_closed = false;
};

MrsLadderProofEngineReport run_mrs_ladder_proof_engine(const Config& cfg,
                                                       const std::vector<int>& primes);

bool export_mrs_ladder_proof_audit_json(const std::string& path,
                                        const MrsLadderProofEngineReport& rep);

bool export_mrs_ladder_closure_json(const std::string& path,
                                    const MrsLadderProofEngineReport& rep);

}  // namespace Marshal::AnaVM

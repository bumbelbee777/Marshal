#pragma once

#include "AnaProofEngine.hxx"
#include "MrsProofTypes.hxx"
#include "Heat/MarshalRhUnconditionalProof.hxx"
#include "Heat/XiHadamardEngine.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct MrsProofAuditEntry {
    std::string obligation_id;
    std::string source;
    std::string witness;
    std::string prove_ref;
    bool ok = false;
    std::string failure_reason;
};

struct MrsProofAuditReport {
    bool ok = false;
    std::vector<MrsProofAuditEntry> entries;
};

/// Runtime witness inputs for Marshal Hadamard obligation closure.
struct MrsHadamardWitnessContext {
    const Heat::XiHadamardReport* report = nullptr;
    const Heat::RhUnconditionalAudit* rh_audit = nullptr;
    bool log_ok = false;
    bool grid_ok = false;
    bool holo_ok = false;
    bool ident_ok = false;
};

bool bundle_has_prove_ref(const MrsCompilationBundle& bundle, const std::string& prove_ref);

MrsProofAuditReport apply_mrs_hadamard_proof_audit(ProofGraphReport& graph,
                                                   const MrsCompilationBundle& bundle,
                                                   const MrsHadamardWitnessContext& ctx);

bool export_mrs_proof_audit_json(const std::string& path, const MrsProofAuditReport& report);

}  // namespace Marshal::AnaVM

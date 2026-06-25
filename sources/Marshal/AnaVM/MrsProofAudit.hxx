#pragma once

#include "AnaProofEngine.hxx"
#include "MrsProofTypes.hxx"
#include "Heat/MarshalRhUnconditionalProof.hxx"
#include "Heat/XiHadamardEngine.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct MrsMathWitnessEnv;

struct MrsProofAuditEntry {
    std::string obligation_id;
    std::string source;
    std::string witness;
    std::string prove_ref;
    std::string tier;
    std::string referee_class;
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

/// Load a prior `mrs_proof_audit.json` export (GL(1) graph tier propagation).
bool import_mrs_proof_audit_json(const std::string& path, MrsProofAuditReport& report);

std::string mrs_obligation_tier(ProofClass pc);
std::string mrs_obligation_referee_class(const std::string& id, ProofClass pc,
                                           const std::string& witness_expr);

/// Promote REDUCTION / STRUCTURAL_PIN up the obligation DAG (capstone honesty).
void propagate_referee_class_from_dependencies(MrsProofAuditReport& report,
                                               const MrsCompilationBundle& bundle);

/// When Suzuki Lerch closure certs show analytic pins discharged, promote spine rows.
void promote_suzuki_discharged_referee_classes(MrsProofAuditReport& report);

bool suzuki_rh_analytic_closed_from_disk();

/// Load RH + Lerch + cross-sector cert pins from docs/generated/*.json into MrsMath env.
/// Used by Hadamard proof audit and MRS ladder theorem catalog (global registry).
void merge_disk_witness_cert_pins(MrsMathWitnessEnv* env);

/// GLOBAL publication tier: PROVED only if no transitive dep is REDUCTION or STRUCTURAL_PIN.
std::string mrs_global_publication_tier(const MrsProofAuditReport& report,
                                        const MrsCompilationBundle& bundle,
                                        const std::string& obligation_id);

}  // namespace Marshal::AnaVM

#pragma once

#include "MrsProofTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct MrsInferAuditEntry {
    std::string obligation_id;
    std::string source;
    std::string rational_num;
    std::string rational_den;
    std::string emitted_theorem_id;
    /// Provenance: which JSON file and field the bound came from (empty = compile-time heuristic).
    std::string provenance_engine;
    std::string provenance_field;
    /// If true, the rational was resolved from the cert manifest rather than a name heuristic.
    bool from_manifest = false;
    bool ok = false;
};

struct MrsInferReport {
    bool ok = false;
    std::vector<MrsInferAuditEntry> audit;
    std::vector<MrsError> errors;
};

struct MrsInferContext {
    std::string cert_root = "docs/generated";
    std::string program_id;
    const MrsProgram* program = nullptr;
    /// Compile-time bundle check: numeric infer defers to runtime MrsProofAudit.
    bool defer_runtime_witness = true;
};

bool try_infer_prove(const MrsProveDecl& prove, const MrsInferContext& ctx, MrsInferAuditEntry* out,
                     std::string* err);

bool try_infer_obligation(const MrsProofObligationDecl& ob, const MrsInferContext& ctx,
                          MrsInferAuditEntry* out, std::string* err);

bool export_infer_audit_json(const std::string& path, const MrsInferReport& report);

MrsInferReport run_bundle_inference(MrsCompilationBundle& bundle);

}  // namespace Marshal::AnaVM

#pragma once

#include "MrsMath.hxx"
#include "MrsProofTypes.hxx"
#include "MrsTransform.hxx"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Marshal::AnaVM {

/// Parsed MRS prove body: `let` / `assume:` / `transform:` / `derive:` / `steps:` / `conclude:`.
struct MrsProveScript {
    std::vector<std::string> lets;
    std::vector<std::string> classical;
    std::vector<std::string> witness;
    std::vector<std::string> deps;
    std::vector<MrsTransformStep> transforms;
    /// Machine-checked intermediate goals (each must evaluate true as MrsMath).
    std::vector<std::string> derive;
    std::vector<std::string> steps;
    std::string conclusion;
    bool has_conclude = false;
    /// Optional LaTeX emission metadata (`paper:` block inside prove body).
    std::string paper_label;
    std::string paper_env;
    std::string paper_title;
};

struct MrsTheoremAuditRow {
    std::string name;
    bool ok = false;
    std::string prove_ref;
    std::string goal;
    std::string failure_reason;
    std::string paper_label;
    std::string paper_env;
    std::string paper_title;
    std::vector<std::string> derive;
    std::vector<std::string> steps;
    std::string conclude;
};

struct MrsTheoremAuditReport {
    std::vector<MrsTheoremAuditRow> rows;
    bool ok = false;
};

MrsProveScript parse_mrs_prove_script(const std::string& body);

std::unordered_map<std::string, std::string> collect_mrs_defs(const MrsCompilationBundle& bundle);

std::unordered_map<std::string, MrsFnRecord> collect_mrs_fns(const MrsCompilationBundle& bundle);

std::string expand_mrs_defs(const std::string& expr,
                            const std::unordered_map<std::string, std::string>& defs);

/// Evaluate witness assumptions; verify fully-parseable formal `conclude:` and optional `derive:` chain.
/// When require_formal_conclude is true, conclude must parse as MrsMath and evaluate to true.
bool evaluate_mrs_prove_witnesses(const MrsProveScript& script,
                                  const MrsCompilationBundle& bundle,
                                  const MrsMathWitnessEnv& env, std::string* detail,
                                  std::string* error, bool require_formal_conclude = false);

MrsTheoremAuditReport audit_mrs_theorems(const MrsCompilationBundle& bundle,
                                         const MrsMathWitnessEnv& env);

bool write_mrs_theorem_catalog_json(const MrsTheoremAuditReport& report,
                                    const std::string& path, std::string* error);

bool mrs_prove_body_is_formula_script(const std::string& body);

/// Script, forall_extension, induction, or multi-callee composition (≥2 prove refs).
bool mrs_prove_body_is_disciplined(const std::string& body,
                                   const std::unordered_set<std::string>& prove_names);

/// Every `dep` in script must appear in the obligation dependency list.
bool validate_prove_script_deps(const MrsProveScript& script,
                                const std::vector<std::string>& graph_deps, std::string* error);

}  // namespace Marshal::AnaVM

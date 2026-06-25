#pragma once

#include "MrsAlgebra.hxx"
#include "MrsProofTypes.hxx"

#include <string>
#include <unordered_map>
#include <vector>

namespace Marshal::AnaVM {

struct MrsTransformStep {
    std::string target;
    std::vector<std::string> hints;
    std::vector<MrsTransformRule> inline_rules;
};

struct MrsTransformResult {
    std::string output;
    std::vector<std::string> applied_rule_ids;
    bool changed = false;
    bool ok = false;
};

/// Built-in Marshal transform DB (ratio, grid, spectral, arith, def_expand).
const std::vector<MrsTransformRule>& marshal_transform_db();

std::vector<MrsTransformRule> collect_mrs_transform_rules(const MrsCompilationBundle& bundle);

/// Apply explicit transforms: marshal DB (filtered by hints) + inline + obligation rules.
MrsTransformResult apply_mrs_transforms(const std::string& expr,
                                        const std::vector<std::string>& hints,
                                        const std::vector<MrsTransformRule>& extra_rules,
                                        const MrsMathWitnessEnv& env,
                                        const std::unordered_map<std::string, std::string>& defs,
                                        std::string* error = nullptr);

bool evaluate_mrs_transform_steps(const std::vector<MrsTransformStep>& steps,
                                    const MrsCompilationBundle& bundle,
                                    MrsMathWitnessEnv* env,
                                    std::string* audit_trail, std::string* error);

bool evaluate_rewrite_obligation(const MrsProofObligationDecl& ob,
                                 const MrsCompilationBundle& bundle,
                                 const MrsMathWitnessEnv& env, std::string* detail,
                                 std::string* error);

bool evaluate_decision_procedure_obligation(const MrsProofObligationDecl& ob,
                                            const MrsMathWitnessEnv& env, std::string* detail,
                                            std::string* error);

}  // namespace Marshal::AnaVM

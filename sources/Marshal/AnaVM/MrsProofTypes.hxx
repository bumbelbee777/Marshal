#pragma once

#include "MrsTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

enum class ProofStatus { Pending, Proved, Failed, Structural };

enum class ProofClass {
    Numeric,
    NumericInterval,
    Analytic,
    ClassicalImport,
    Reduction,
    AnalyticOpen,
    Structural,
    Composition,
    Universal,
    Inductive,
    Convergent,
    Rewrite,
    DecisionProcedure,
};

enum class MrsVisibility { Private, Pub };

enum class MrsProofBodyKind { Infer, Explicit, Ref };

struct MrsUseDecl {
    std::string path;
    bool glob_all = false;
    SourceSpan span;
};

struct MrsConstDecl {
    std::string name;
    std::string type_expr;
    std::string value_expr;
    MrsVisibility visibility = MrsVisibility::Private;
    bool infer_value = false;
    bool infer_rational_bound = false;
    SourceSpan span;
};

struct MrsDefDecl {
    std::string name;
    std::string expr;
    MrsVisibility visibility = MrsVisibility::Private;
    SourceSpan span;
};

/// v1: first-class `fn` — parametric witness expressions (`fn half(x) := x / 2`).
struct MrsFnDecl {
    std::string name;
    std::vector<std::string> params;
    std::string body;
    MrsVisibility visibility = MrsVisibility::Private;
    SourceSpan span;
};

/// v2: compile-time numeric array literal (`array gaps := [0.01, 0.02]`).
struct MrsArrayDecl {
    std::string name;
    std::vector<double> elements;
    MrsVisibility visibility = MrsVisibility::Private;
    SourceSpan span;
};

/// Explicit rewrite rule — Marshal DB + module + proof-local append.
struct MrsTransformRule {
    std::string id;
    std::vector<std::string> hints;
    std::string from_pattern;
    std::string to_pattern;
};

struct MrsProveDecl {
    std::string name;
    std::string type_expr;
    MrsProofBodyKind body_kind = MrsProofBodyKind::Infer;
    std::string body;
    MrsVisibility visibility = MrsVisibility::Private;
    SourceSpan span;
};

/// Published paper theorem: formal `goal` discharged by machine-checked `prove:` script.
struct MrsTheoremDecl {
    std::string name;
    std::string goal;
    std::string prove_ref;
    ProofClass proof_class = ProofClass::Analytic;
    std::string paper_label;
    std::string paper_env;
    std::string paper_title;
    SourceSpan span;
};

struct MrsProofObligationDecl {
    std::string id;
    std::string statement;
    ProofClass proof_class = ProofClass::Numeric;
    std::vector<std::string> dependencies;
    MrsProofBodyKind prove_kind = MrsProofBodyKind::Ref;
    std::string prove_ref;
    std::string domain_kind;
    std::string domain_lower;
    std::string domain_upper;
    std::string predicate;
    std::string extend_via;
    std::string witness_expr;
    std::vector<std::string> rewrite_rules;
    std::vector<std::string> transform_hints;
    std::string decision_procedure;
    SourceSpan span;
};

struct MrsProofGraphDecl {
    std::string name;
    std::string target;
    std::string architecture = "acyclic_marshal_hadamard";
    std::vector<MrsProofObligationDecl> obligations;
    SourceSpan span;
};

struct MrsModule {
    std::string name;
    std::vector<MrsUseDecl> uses;
    std::vector<MrsConstDecl> consts;
    std::vector<MrsArrayDecl> arrays;
    std::vector<MrsDefDecl> defs;
    std::vector<MrsFnDecl> fns;
    std::vector<MrsTransformRule> transforms;
    std::vector<MrsProveDecl> proves;
    std::vector<MrsTheoremDecl> theorems;
    std::vector<MrsProofGraphDecl> proof_graphs;
    SourceSpan span;
};

struct MrsCompilationUnit {
    std::string source_path;
    std::vector<MrsUseDecl> top_uses;
    std::vector<MrsModule> modules;
    MrsProgram program;
    bool has_ansatz = false;
    bool has_modules = false;
    std::vector<MrsError> errors;
    bool ok = true;
};

struct MrsCompilationBundle {
    std::string entry_path;
    std::vector<MrsCompilationUnit> units;
    std::vector<MrsModule> merged_modules;
    MrsProgram program;
    bool has_ansatz = false;
    std::vector<MrsError> errors;
    bool ok = false;
};

}  // namespace Marshal::AnaVM

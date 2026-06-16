#pragma once

#include "MrsTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

enum class ProofStatus { Pending, Proved, Failed, Structural };

enum class ProofClass {
    Numeric,
    Analytic,
    Structural,
    Composition,
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
    SourceSpan span;
};

struct MrsProveDecl {
    std::string name;
    std::string type_expr;
    MrsProofBodyKind body_kind = MrsProofBodyKind::Infer;
    std::string body;
    MrsVisibility visibility = MrsVisibility::Private;
    SourceSpan span;
};

struct MrsProofObligationDecl {
    std::string id;
    std::string statement;
    ProofClass proof_class = ProofClass::Numeric;
    std::vector<std::string> dependencies;
    std::string lean_theorem;
    MrsProofBodyKind prove_kind = MrsProofBodyKind::Ref;
    std::string prove_ref;
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
    std::vector<MrsProveDecl> proves;
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

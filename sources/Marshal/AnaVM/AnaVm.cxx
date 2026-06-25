#include "AnaVm.hxx"

#include "MrsProveSpine.hxx"
#include "MrsSym.hxx"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace Marshal::AnaVM {
namespace {

std::string parent_dir(const std::string& path) {
    std::filesystem::path p(path);
    if (p.has_parent_path()) return p.parent_path().string();
    return ".";
}

std::string resolve_use_path(const std::string& base_dir, const std::string& use_path) {
    std::string p = use_path;
    if (const size_t glob = p.find("::*"); glob != std::string::npos)
        p = p.substr(0, glob);
    else if (const size_t glob2 = p.find(".*"); glob2 != std::string::npos)
        p = p.substr(0, glob2);
    for (char& c : p)
        if (c == ':') c = '/';
    if (p.find('/') == std::string::npos && p.find('\\') == std::string::npos) {
        const std::filesystem::path base_path(base_dir);
        const bool in_lib = base_path.filename() == "lib";
        if (!in_lib) p = "lib/" + p;
    }
    if (p.size() < 4 || p.substr(p.size() - 4) != ".mrs") p += ".mrs";
    std::filesystem::path full = std::filesystem::path(base_dir) / p;
    if (std::filesystem::exists(full)) return full.string();
    full = std::filesystem::path(base_dir) / "programs" / p;
    if (std::filesystem::exists(full)) return full.string();
    const std::filesystem::path cwd = std::filesystem::current_path();
    full = cwd / "programs" / p;
    if (std::filesystem::exists(full)) return full.string();
    return (std::filesystem::path(base_dir) / p).string();
}

void add_error(std::vector<MrsError>& errors, const std::string& code, int line,
               const std::string& msg, const std::string& hint = {}) {
    MrsError e;
    e.code = code;
    e.span.line = line;
    e.message = msg;
    e.hint = hint;
    errors.push_back(std::move(e));
}

void merge_modules(MrsCompilationBundle& bundle) {
    for (const auto& u : bundle.units) {
        for (const auto& m : u.modules) bundle.merged_modules.push_back(m);
    }
}

bool resolve_uses_recursive(const std::string& path, MrsCompilationBundle& bundle,
                            std::unordered_set<std::string>& visiting,
                            std::unordered_set<std::string>& visited) {
    std::string norm = std::filesystem::weakly_canonical(std::filesystem::path(path)).string();
    if (visited.count(norm)) return true;
    if (visiting.count(norm)) {
        add_error(bundle.errors, "E0900", 0, "import cycle detected at " + path,
                  "reorder mod/use graph to be acyclic");
        return false;
    }
    visiting.insert(norm);

    MrsCompilationUnit unit = parse_mrs_unit(path);
    unit.source_path = path;
    for (const auto& e : unit.errors) bundle.errors.push_back(e);
    if (!unit.ok) bundle.ok = false;
    bundle.units.push_back(unit);

    const std::string base = parent_dir(path);
    std::vector<MrsUseDecl> uses = unit.top_uses;
    for (const auto& m : unit.modules)
        for (const auto& u : m.uses) uses.push_back(u);

    for (const auto& u : uses) {
        const std::string child = resolve_use_path(base, u.path);
        if (!std::filesystem::exists(child)) {
            add_error(bundle.errors, "E0100", u.span.line, "cannot resolve use: " + u.path,
                      "expected " + child);
            continue;
        }
        if (!resolve_uses_recursive(child, bundle, visiting, visited)) return false;
    }

    visiting.erase(norm);
    visited.insert(norm);
    return true;
}

}  // namespace

MrsCompilationBundle compile_bundle(const std::string& entry_path, bool /*check_only*/,
                                    MrsInferReport* infer_out) {
    MrsCompilationBundle bundle;
    bundle.entry_path = entry_path;

    std::unordered_set<std::string> visiting;
    std::unordered_set<std::string> visited;
    if (!resolve_uses_recursive(entry_path, bundle, visiting, visited)) {
        bundle.ok = false;
        return bundle;
    }

    for (const auto& u : bundle.units) {
        if (u.has_ansatz) {
            bundle.has_ansatz = true;
            bundle.program = u.program;
        }
    }

    merge_modules(bundle);

    if (bundle.has_ansatz) {
        const SymDeriveResult sym = derive_spectrum(bundle.program);
        bundle.program.rule_id = sym.rule_id;
        if (sym.ok) {
            bundle.program.derived_omega = sym.omega_derived;
            bundle.program.derived_lambda = sym.lambda_derived;
        }
    }

    MrsKernelReport kr = check_compilation_bundle(bundle);
    for (const auto& e : kr.errors) bundle.errors.push_back(e);

    const MrsInferReport infer_report = run_bundle_inference(bundle);
    if (infer_out) *infer_out = infer_report;
    for (const auto& e : infer_report.errors) bundle.errors.push_back(e);

    const MrsProveSpineReport hadamard_spine = validate_mrs_proof_discipline(bundle);
    if (!hadamard_spine.ok) {
        for (const auto& item : hadamard_spine.infer_on_analytic) {
            add_error(bundle.errors, "E0902", 0,
                      "MRS prove discipline — analytic script required — " + item);
        }
        for (const auto& item : hadamard_spine.missing_witness_expr) {
            add_error(bundle.errors, "E0903", 0,
                      "MRS prove discipline — obligation missing witness_expr — " + item);
        }
        for (const auto& item : hadamard_spine.missing_prove_ref) {
            add_error(bundle.errors, "E0904", 0,
                      "MRS prove discipline — obligation missing prove ref — " + item);
        }
        for (const auto& item : hadamard_spine.undisciplined_prove) {
            add_error(bundle.errors, "E0905", 0, "MRS prove discipline — " + item);
        }
        for (const auto& item : hadamard_spine.script_dep_mismatch) {
            add_error(bundle.errors, "E0906", 0,
                      "MRS prove discipline — script dep mismatch — " + item);
        }
        for (const auto& item : hadamard_spine.trivial_aliases) {
            add_error(bundle.errors, "E0907", 0,
                      "MRS prove discipline — trivial alias — " + item);
        }
        if (!hadamard_spine.acyclic) {
            add_error(bundle.errors, "E0908", 0, "MRS prove discipline — prove cycle detected");
        }
        if (!hadamard_spine.obligation_graph_acyclic) {
            add_error(bundle.errors, "E0909", 0,
                      "MRS prove discipline — obligation dependency cycle detected");
        }
        for (const auto& item : hadamard_spine.circular_witness) {
            add_error(bundle.errors, "E0910", 0,
                      "MRS prove discipline — circular witness_expr — " + item);
        }
        for (const auto& item : hadamard_spine.weak_witness) {
            add_error(bundle.errors, "E0911", 0,
                      "MRS prove discipline — weak/trivial witness_expr — " + item);
        }
        for (const auto& item : hadamard_spine.capstone_in_witness) {
            add_error(bundle.errors, "E0912", 0,
                      "MRS prove discipline — capstone embedded in witness_expr — " + item);
        }
        for (const auto& item : hadamard_spine.opaque_composition) {
            add_error(bundle.errors, "E0913", 0,
                      "MRS prove discipline — opaque single-callee composition — " + item);
        }
        for (const auto& item : hadamard_spine.tautological_prove) {
            add_error(bundle.errors, "E0914", 0,
                      "MRS prove discipline — tautological prove body — " + item);
        }
        for (const auto& item : hadamard_spine.circular_identification) {
            add_error(bundle.errors, "E0915", 0,
                      "MRS prove discipline — circular identification witness — " + item);
        }
        for (const auto& item : hadamard_spine.weak_analytic_reduction) {
            add_error(bundle.errors, "E0916", 0,
                      "MRS prove discipline — weak analytic reduction (deps only) — " + item);
        }
        for (const auto& item : hadamard_spine.goal_equality_in_witness) {
            add_error(bundle.errors, "E0917", 0,
                      "MRS prove discipline — goal equality in witness_expr — " + item);
        }
        for (const auto& item : hadamard_spine.rh_assumption_smuggle) {
            add_error(bundle.errors, "E0918", 0,
                      "MRS prove discipline — RH capstone smuggled in witness — " + item);
        }
        for (const auto& item : hadamard_spine.assume_target_leak) {
            add_error(bundle.errors, "E0919", 0,
                      "MRS prove discipline — assume block mentions target proposition — " + item);
        }
        for (const auto& item : hadamard_spine.missing_explicit_steps) {
            add_error(bundle.errors, "E0920", 0,
                      "MRS prove discipline — composition prove missing explicit steps: — " + item);
        }
    }

    bundle.ok = bundle.errors.empty() && kr.ok && infer_report.ok && hadamard_spine.ok;
    if (bundle.has_ansatz && bundle.program.id.empty()) bundle.ok = false;
    return bundle;
}

CompileResult compile_program(const std::string& path, bool check_only) {
    CompileResult r;
    r.bundle = compile_bundle(path, check_only, &r.infer_report);
    r.program = r.bundle.program;
    r.errors = r.bundle.errors;
    r.ok = r.bundle.ok;
    return r;
}

}  // namespace Marshal::AnaVM

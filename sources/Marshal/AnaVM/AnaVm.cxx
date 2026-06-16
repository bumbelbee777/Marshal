#include "AnaVm.hxx"

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
    const size_t glob = p.find("::*");
    if (glob != std::string::npos) p = p.substr(0, glob);
    for (char& c : p)
        if (c == ':') c = '/';
    if (p.find('/') == std::string::npos) p = "lib/" + p;
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

    bundle.ok = bundle.errors.empty() && kr.ok && infer_report.ok;
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

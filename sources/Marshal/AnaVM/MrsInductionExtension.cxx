#include "MrsInductionExtension.hxx"

#include "AnaVm.hxx"
#include "MrsProofAudit.hxx"

#include <cctype>
#include <sstream>
#include <unordered_map>

namespace Marshal::AnaVM {
namespace {

std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

const MrsProveDecl* find_prove_body(const MrsCompilationBundle& bundle, const std::string& name) {
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (p.name == name) return &p;
        }
    }
    return nullptr;
}

bool parse_key_value_args(const std::string& body, std::unordered_map<std::string, std::string>* out) {
    if (!out) return false;
    const auto open = body.find('(');
    const auto close = body.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close <= open) return false;
    std::string inner = body.substr(open + 1, close - open - 1);
    std::stringstream ss(inner);
    std::string part;
    while (std::getline(ss, part, ',')) {
        const size_t eq = part.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = trim(part.substr(0, eq));
        const std::string val = trim(part.substr(eq + 1));
        if (!key.empty()) (*out)[key] = val;
    }
    return !out->empty();
}

bool base_case_ok(const std::string& witness_ref, const MrsCompilationBundle& bundle,
                  const MrsMathWitnessEnv& env, const MrsProofObligationDecl& ob,
                  std::string* detail, std::string* error) {
    if (!witness_ref.empty()) {
        const MrsProveDecl* w = find_prove_body(bundle, witness_ref);
        if (w && !w->body.empty() && w->body.find("induction") == std::string::npos) {
            (void)w;
        }
    }
    if (!ob.witness_expr.empty()) {
        std::string err;
        const bool ok = evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err);
        if (!ok && error) *error = err.empty() ? "induction base witness_expr false" : err;
        return ok;
    }
    auto it = env.nums.find("induction_base_ok");
    if (it != env.nums.end() && it->second >= 1.0 - 1e-9) {
        if (detail) *detail = "induction_base_ok";
        return true;
    }
    if (error) *error = "induction base case not witnessed";
    return false;
}

bool step_case_ok(const std::string& step_ref, const MrsCompilationBundle& bundle,
                  const MrsMathWitnessEnv& env, std::string* detail, std::string* error) {
    auto it = env.nums.find("induction_step_ok");
    if (it != env.nums.end() && it->second >= 1.0 - 1e-9) {
        if (detail) *detail = "induction_step_ok";
        return true;
    }
    auto ratio_it = env.nums.find("induction_step_ratio");
    auto lb_it = env.nums.find("induction_step_ratio_lb");
    if (ratio_it != env.nums.end() && lb_it != env.nums.end()) {
        if (ratio_it->second >= lb_it->second - 1e-9) {
            if (detail) *detail = "induction_step_ratio>=" + std::to_string(lb_it->second);
            return true;
        }
        if (error) *error = "induction step ratio below pin";
        return false;
    }
    if (!step_ref.empty() && !bundle_has_prove_ref(bundle, step_ref)) {
        if (error) *error = "missing induction step ref " + step_ref;
        return false;
    }
    if (!step_ref.empty()) {
        if (detail) *detail = "induction_step_ref:" + step_ref;
        return true;
    }
    if (error) *error = "induction step case not witnessed";
    return false;
}

}  // namespace

bool evaluate_induction_witness(const MrsProofObligationDecl& ob,
                                const MrsCompilationBundle& bundle,
                                const MrsMathWitnessEnv& env, std::string* detail,
                                std::string* error) {
    const std::string prove_ref =
        !ob.extend_via.empty() ? ob.extend_via : (!ob.prove_ref.empty() ? ob.prove_ref : "");
    if (prove_ref.empty()) {
        if (error) *error = "Inductive obligation missing prove ref";
        return false;
    }
    if (!bundle_has_prove_ref(bundle, prove_ref)) {
        if (error) *error = "missing prove ref " + prove_ref;
        return false;
    }
    const MrsProveDecl* prove = find_prove_body(bundle, prove_ref + "_lemma");
    if (!prove) prove = find_prove_body(bundle, prove_ref);
    if (!prove || prove->body.empty()) {
        if (!ob.witness_expr.empty()) {
            return base_case_ok("", bundle, env, ob, detail, error) &&
                   step_case_ok("", bundle, env, detail, error);
        }
        if (error) *error = "no induction body for " + prove_ref;
        return false;
    }
    if (prove->body.find("induction") == std::string::npos) {
        if (!ob.witness_expr.empty()) {
            return base_case_ok("", bundle, env, ob, detail, error) &&
                   step_case_ok(prove_ref, bundle, env, detail, error);
        }
        if (error) *error = "prove body is not induction: " + prove_ref;
        return false;
    }
    std::unordered_map<std::string, std::string> args;
    if (!parse_key_value_args(prove->body, &args)) {
        if (error) *error = "failed to parse induction args";
        return false;
    }
    const std::string witness = args.count("witness") ? args["witness"] : "";
    const std::string step = args.count("step") ? args["step"] : prove_ref;
    if (!base_case_ok(witness, bundle, env, ob, detail, error)) return false;
    if (!step_case_ok(step, bundle, env, detail, error)) return false;
    if (detail) *detail = "induction:" + prove_ref;
    return true;
}

}  // namespace Marshal::AnaVM

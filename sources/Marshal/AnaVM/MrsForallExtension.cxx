#include "MrsForallExtension.hxx"

#include "AnaVm.hxx"
#include "MrsProofAudit.hxx"

#include <algorithm>
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

double extension_ratio_lb_for_domain(const std::string& domain, const MrsMathWitnessEnv& env) {
    if (domain.find("goldbach") != std::string::npos) {
        auto it = env.nums.find("goldbach_extension_ratio_lb");
        return it != env.nums.end() ? it->second : 10.0;
    }
    if (domain.find("bsd") != std::string::npos) {
        auto it = env.nums.find("bsd_millennium_extension_ratio_lb");
        if (it != env.nums.end()) return it->second;
        it = env.nums.find("bsd_extension_ratio_lb");
        return it != env.nums.end() ? it->second : 2.0;
    }
    if (domain.find("hodge") != std::string::npos || domain.find("gl3") != std::string::npos) {
        auto it = env.nums.find("hodge_millennium_extension_ratio_lb");
        if (it != env.nums.end() && domain.find("millennium") != std::string::npos)
            return it->second;
        it = env.nums.find("hodge_extension_ratio_lb");
        return it != env.nums.end() ? it->second : 1.0;
    }
    if (domain.find("ym") != std::string::npos || domain.find("gl4") != std::string::npos) {
        if (domain.find("millennium") != std::string::npos) {
            auto it = env.nums.find("ym_millennium_extension_ratio_lb");
            if (it != env.nums.end()) return it->second;
        }
        auto it = env.nums.find("ym_extension_ratio_lb");
        return it != env.nums.end() ? it->second : 1.0;
    }
    return 1.0;
}

bool spectral_dominance_ok(const MrsMathWitnessEnv& env, double ratio_lb,
                           const std::string& domain) {
    if (domain.find("goldbach") != std::string::npos) {
        if (!env.nums.count("major_minor_ratio")) return false;
        return env.nums.at("major_minor_ratio") >= ratio_lb - 1e-9;
    }
    if (domain.find("bsd") != std::string::npos) {
        if (domain.find("millennium") != std::string::npos) {
            if (!env.nums.count("bsd_formula_margin_ratio")) return false;
            auto it = env.nums.find("bsd_millennium_extension_ratio_lb");
            const double ratio_lb =
                it != env.nums.end() ? it->second : extension_ratio_lb_for_domain(domain, env);
            return env.nums.at("bsd_formula_margin_ratio") >= ratio_lb - 1e-9;
        }
        if (!env.nums.count("l_grid_margin_ratio")) return false;
        return env.nums.at("l_grid_margin_ratio") >= ratio_lb - 1e-9;
    }
    if (domain.find("hodge") != std::string::npos || domain.find("gl3") != std::string::npos) {
        if (domain.find("millennium") != std::string::npos) {
            if (!env.nums.count("hodge_millennium_pp_match") ||
                !env.nums.count("hodge_millennium_pp_target"))
                return false;
            const double target = env.nums.at("hodge_millennium_pp_target");
            if (target <= 0) return false;
            auto it = env.nums.find("hodge_millennium_extension_ratio_lb");
            const double ratio_lb =
                it != env.nums.end() ? it->second : extension_ratio_lb_for_domain(domain, env);
            return env.nums.at("hodge_millennium_pp_match") / target >= ratio_lb - 1e-9;
        }
        if (!env.nums.count("hodge_kernel_multiplicity") || !env.nums.count("hodge_h11_target"))
            return false;
        const double target = env.nums.at("hodge_h11_target");
        if (target <= 0) return false;
        return env.nums.at("hodge_kernel_multiplicity") / target >= ratio_lb - 1e-9;
    }
    if (domain.find("ym") != std::string::npos || domain.find("gl4") != std::string::npos) {
        if (domain.find("millennium") != std::string::npos) {
            if (!env.nums.count("ym_gap_extension_ratio")) return false;
            auto it = env.nums.find("ym_millennium_extension_ratio_lb");
            const double millennium_lb =
                it != env.nums.end() ? it->second : extension_ratio_lb_for_domain(domain, env);
            return env.nums.at("ym_gap_extension_ratio") >= millennium_lb - 1e-9;
        }
        if (!env.nums.count("gauge_smallest_positive_eigenvalue") ||
            !env.nums.count("ym_mass_gap_lb"))
            return false;
        const double lb = env.nums.at("ym_mass_gap_lb");
        if (lb <= 0) return false;
        return env.nums.at("gauge_smallest_positive_eigenvalue") / lb >= ratio_lb - 1e-9;
    }
    return ratio_lb <= 1.0;
}

}  // namespace

bool evaluate_forall_extension_witness(const MrsProofObligationDecl& ob,
                                       const MrsCompilationBundle& bundle,
                                       const MrsMathWitnessEnv& env, std::string* detail,
                                       std::string* error) {
    const std::string prove_ref =
        !ob.extend_via.empty() ? ob.extend_via : (!ob.prove_ref.empty() ? ob.prove_ref : "");
    if (prove_ref.empty()) {
        if (error) *error = "Universal obligation missing extend_via/prove ref";
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
            std::string err;
            const bool ok = evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err);
            if (!ok && error) *error = err;
            return ok;
        }
        if (error) *error = "no forall_extension body for " + prove_ref;
        return false;
    }
    if (prove->body.find("forall_extension") == std::string::npos) {
        if (!ob.witness_expr.empty()) {
            std::string err;
            const bool ok = evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err);
            if (!ok && error) *error = err;
            return ok;
        }
        if (error) *error = "prove body is not forall_extension: " + prove_ref;
        return false;
    }
    std::unordered_map<std::string, std::string> args;
    if (!parse_key_value_args(prove->body, &args)) {
        if (error) *error = "failed to parse forall_extension args";
        return false;
    }
    const std::string domain = args.count("domain") ? args["domain"] : ob.domain_kind;
    const std::string upper = args.count("upper") ? args["upper"] : ob.domain_upper;
    if (!ob.witness_expr.empty()) {
        std::string err;
        const bool finite_ok = evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err);
        if (!finite_ok) {
            if (error) *error = err.empty() ? "witness_expr false" : err;
            return false;
        }
    }
    if (upper == "unbounded") {
        const double ratio_lb = extension_ratio_lb_for_domain(domain, env);
        if (!spectral_dominance_ok(env, ratio_lb, domain)) {
            if (error) *error = "spectral dominance extension ratio below pin for " + domain;
            return false;
        }
    }
    if (detail) *detail = "forall_extension:" + prove_ref + " domain=" + domain;
    return true;
}

}  // namespace Marshal::AnaVM

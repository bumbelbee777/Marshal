#include "MrsConvergenceExtension.hxx"

#include "AnaVm.hxx"
#include "MrsProofAudit.hxx"

#include <cmath>
#include <cctype>
#include <sstream>
#include <unordered_map>

namespace Marshal::AnaVM {
namespace {

constexpr double kEps = 1e-9;

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

double env_num(const MrsMathWitnessEnv& env, const std::string& key, double fallback) {
    const auto it = env.nums.find(key);
    return it != env.nums.end() ? it->second : fallback;
}

double resolve_bound_token(const MrsMathWitnessEnv& env, const std::string& token,
                           double fallback) {
    try {
        size_t idx = 0;
        const double v = std::stod(token, &idx);
        if (idx == token.size()) return v;
    } catch (...) {
    }
    return env_num(env, token, fallback);
}

bool env_flag(const MrsMathWitnessEnv& env, const std::string& key) {
    return env_num(env, key, 0.0) >= 0.5;
}

bool array_monotone(const std::vector<double>& xs, bool inc) {
    if (xs.size() < 2) return false;
    for (size_t i = 1; i < xs.size(); ++i) {
        if (inc && xs[i] + kEps < xs[i - 1]) return false;
        if (!inc && xs[i] > xs[i - 1] + kEps) return false;
    }
    return true;
}

bool consecutive_rel_change_below(const std::vector<double>& xs, double tol) {
    if (xs.size() < 2) return false;
    const double a = xs[xs.size() - 2];
    const double b = xs[xs.size() - 1];
    const double diff = std::fabs(b - a);
    const double scale = std::max({1.0, std::fabs(a), std::fabs(b)});
    return diff / scale <= tol + kEps;
}

double tail_ratio(const std::vector<double>& xs) {
    if (xs.size() < 2) return 0.0;
    const double coarse = xs.front();
    const double fine = xs.back();
    if (std::fabs(coarse) < kEps) return 0.0;
    return fine / coarse;
}

bool finite_term_ok(const MrsProofObligationDecl& ob, const MrsMathWitnessEnv& env,
                    std::string* detail, std::string* error) {
    if (env_flag(env, "convergence_finite_ok")) {
        if (detail) *detail = "convergence_finite_ok";
        return true;
    }
    if (!ob.witness_expr.empty()) {
        std::string err;
        const bool ok = evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err);
        if (!ok && error) *error = err.empty() ? "convergence finite witness_expr false" : err;
        return ok;
    }
    if (error) *error = "convergence finite term not witnessed";
    return false;
}

bool limit_ok(const std::unordered_map<std::string, std::string>& args,
              const MrsMathWitnessEnv& env, std::string* detail, std::string* error) {
    if (env_flag(env, "convergence_limit_ok")) {
        if (detail) *detail = "convergence_limit_ok";
        return true;
    }

    const std::string seq_name = args.count("sequence") ? args.at("sequence") : "";
    const std::string mono = args.count("monotone") ? args.at("monotone") : "none";
    const std::string tail_key =
        args.count("tail_tol") ? args.at("tail_tol") : "convergence_mesh_tail_tol";
    const double tail_tol = env_num(env, tail_key, 0.25);

    if (!seq_name.empty()) {
        const auto it = env.arrays.find(seq_name);
        if (it == env.arrays.end()) {
            if (error) *error = "convergence unknown sequence array " + seq_name;
            return false;
        }
        const auto& xs = it->second;
        if (mono == "inc" && !array_monotone(xs, true)) {
            if (error) *error = "sequence not monotone inc: " + seq_name;
            return false;
        }
        if (mono == "dec" && !array_monotone(xs, false)) {
            if (error) *error = "sequence not monotone dec: " + seq_name;
            return false;
        }

        const std::string bound = args.count("bound") ? args.at("bound") : "none";
        if (bound == "one_sided") {
            const std::string lb_key = args.count("bound_lb") ? args.at("bound_lb") : "0";
            const double lb = resolve_bound_token(env, lb_key, 0.0);
            for (double v : xs) {
                if (v + kEps < lb) {
                    if (error) *error = "one_sided bound failed on " + seq_name;
                    return false;
                }
            }
        } else if (bound == "two_sided") {
            const std::string lb_key = args.count("bound_lb") ? args.at("bound_lb") : "0";
            const std::string ub_key = args.count("bound_ub") ? args.at("bound_ub") : "0";
            const double lb = resolve_bound_token(env, lb_key, 0.0);
            const double ub = resolve_bound_token(env, ub_key, 0.0);
            for (double v : xs) {
                if (v + kEps < lb || v > ub + kEps) {
                    if (error) *error = "two_sided bound failed on " + seq_name;
                    return false;
                }
            }
        }

        const double ratio = tail_ratio(xs);
        if (detail) *detail = "tail_ratio=" + std::to_string(ratio);
        if (env_flag(env, "convergence_tail_rel_ok")) return true;
        const double pinned_ratio = env_num(env, "convergence_tail_ratio", 0.0);
        if (pinned_ratio > kEps && std::fabs(ratio - pinned_ratio) <= tail_tol + 0.05)
            return true;
        if (consecutive_rel_change_below(xs, tail_tol)) return true;
        if (error) *error = "convergence tail not within tol " + std::to_string(tail_tol);
        return false;
    }

    const double ratio = env_num(env, "convergence_tail_ratio", 0.0);
    if (ratio > kEps) {
        if (detail) *detail = "convergence_tail_ratio=" + std::to_string(ratio);
        return true;
    }
    if (error) *error = "convergence limit not witnessed";
    return false;
}

}  // namespace

bool evaluate_convergence_witness(const MrsProofObligationDecl& ob,
                                  const MrsCompilationBundle& bundle,
                                  const MrsMathWitnessEnv& env, std::string* detail,
                                  std::string* error) {
    const std::string prove_ref =
        !ob.extend_via.empty() ? ob.extend_via : (!ob.prove_ref.empty() ? ob.prove_ref : "");
    if (prove_ref.empty()) {
        if (error) *error = "Convergent obligation missing prove ref";
        return false;
    }
    if (!bundle_has_prove_ref(bundle, prove_ref) &&
        !bundle_has_prove_ref(bundle, prove_ref + "_lemma") &&
        !bundle_has_prove_ref(bundle, prove_ref + "_witness")) {
        if (error) *error = "missing prove ref " + prove_ref;
        return false;
    }

    const MrsProveDecl* prove = find_prove_body(bundle, prove_ref + "_lemma");
    if (!prove) prove = find_prove_body(bundle, prove_ref);
    if (!prove || prove->body.empty()) {
        return finite_term_ok(ob, env, detail, error) &&
               limit_ok({}, env, detail, error);
    }

    std::unordered_map<std::string, std::string> args;
    if (prove->body.find("convergence(") != std::string::npos) {
        if (!parse_key_value_args(prove->body, &args)) {
            if (error) *error = "failed to parse convergence args";
            return false;
        }
    }

    std::string fin_detail;
    if (!finite_term_ok(ob, env, &fin_detail, error)) return false;

    std::string lim_detail;
    if (!limit_ok(args, env, &lim_detail, error)) return false;

    if (detail) {
        *detail = "convergence:" + prove_ref;
        if (!fin_detail.empty()) *detail += ";" + fin_detail;
        if (!lim_detail.empty()) *detail += ";" + lim_detail;
    }
    return true;
}

}  // namespace Marshal::AnaVM

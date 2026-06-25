#include "MrsTransform.hxx"

#include "MrsProofLogic.hxx"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace Marshal::AnaVM {
namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}

bool hint_matches(const MrsTransformRule& rule, const std::vector<std::string>& hints) {
    if (hints.empty()) return true;
    for (const auto& h : hints) {
        for (const auto& rh : rule.hints) {
            if (rh == h) return true;
        }
    }
    return false;
}

bool parse_func_pattern(const std::string& pat, std::string* name, std::vector<std::string>* slots) {
    if (!name || !slots) return false;
    const size_t lpar = pat.find('(');
    if (lpar == std::string::npos) return false;
    *name = trim(pat.substr(0, lpar));
    const size_t rpar = pat.rfind(')');
    if (rpar == std::string::npos || rpar <= lpar) return false;
    std::string inside = pat.substr(lpar + 1, rpar - lpar - 1);
    std::stringstream ss(inside);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        slots->push_back(trim(tok));
    }
    return !name->empty();
}

std::string substitute_slots(const std::string& tmpl, const std::vector<std::string>& slot_names,
                             const std::vector<std::string>& captures) {
    std::string out = tmpl;
    for (size_t i = 0; i < slot_names.size() && i < captures.size(); ++i) {
        const std::string key = slot_names[i];
        if (key.size() >= 3 && key.front() == '{' && key.back() == '}') {
            size_t pos = 0;
            while ((pos = out.find(key, pos)) != std::string::npos) {
                out.replace(pos, key.size(), captures[i]);
                pos += captures[i].size();
            }
        }
    }
    return out;
}

bool split_top_level_args(const std::string& inside, std::vector<std::string>* args) {
    if (!args) return false;
    args->clear();
    std::string cur;
    int depth = 0;
    for (char c : inside) {
        if (c == '(') ++depth;
        if (c == ')') --depth;
        if (c == ',' && depth == 0) {
            args->push_back(trim(cur));
            cur.clear();
            continue;
        }
        cur += c;
    }
    if (!trim(cur).empty()) args->push_back(trim(cur));
    return !args->empty();
}

bool apply_func_rule(std::string* expr, const MrsTransformRule& rule) {
    std::string fname;
    std::vector<std::string> slots;
    if (!parse_func_pattern(rule.from_pattern, &fname, &slots)) return false;
    const std::string needle = fname + "(";
    size_t pos = 0;
    while ((pos = expr->find(needle, pos)) != std::string::npos) {
        const size_t start = pos + needle.size();
        size_t end = start;
        int depth = 1;
        while (end < expr->size()) {
            if ((*expr)[end] == '(') ++depth;
            if ((*expr)[end] == ')') {
                --depth;
                if (depth == 0) break;
            }
            ++end;
        }
        if (end >= expr->size()) break;
        std::vector<std::string> captures;
        if (!split_top_level_args(expr->substr(start, end - start), &captures)) {
            pos += needle.size();
            continue;
        }
        if (captures.size() != slots.size()) {
            pos += needle.size();
            continue;
        }
        const std::string repl = substitute_slots(rule.to_pattern, slots, captures);
        expr->replace(pos, end - pos + 1, repl);
        return true;
    }
    return false;
}

bool apply_ident_rule(std::string* expr, const std::string& from, const std::string& to) {
    if (from.empty()) return false;
    size_t pos = 0;
    bool changed = false;
    while ((pos = expr->find(from, pos)) != std::string::npos) {
        const bool left_ok = pos == 0 || !std::isalnum(static_cast<unsigned char>((*expr)[pos - 1]));
        const bool right_ok =
            pos + from.size() >= expr->size() ||
            !std::isalnum(static_cast<unsigned char>((*expr)[pos + from.size()]));
        if (left_ok && right_ok) {
            expr->replace(pos, from.size(), to);
            pos += to.size();
            changed = true;
        } else {
            pos += from.size();
        }
    }
    return changed;
}

}  // namespace

const std::vector<MrsTransformRule>& marshal_transform_db() {
    static const std::vector<MrsTransformRule> kDb = {
        {"ratio_to_div", {"ratio", "grid", "arith"}, "ratio({0},{1})", "(({0})/({1}))"},
        {"ratio_of_self", {"ratio", "arith"}, "ratio({0},{0})", "1"},
        {"mul_one", {"arith", "normalize"}, "1 * {0}", "{0}"},
        {"add_zero", {"arith", "normalize"}, "0 + {0}", "{0}"},
        {"sub_self", {"arith", "normalize"}, "{0} - {0}", "0"},
        {"grid_margin", {"grid"}, "max_grid_rel_gap", "ratio(max_grid_rel_gap, grid_rel_gap_ub)"},
        {"spectral_gap_lb", {"spectral"}, "gauge_smallest_positive_eigenvalue",
         "max(gauge_smallest_positive_eigenvalue, ym_mass_gap_lb)"},
    };
    return kDb;
}

std::vector<MrsTransformRule> collect_mrs_transform_rules(const MrsCompilationBundle& bundle) {
    std::vector<MrsTransformRule> out;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& r : m.transforms) out.push_back(r);
    }
    return out;
}

MrsTransformResult apply_mrs_transforms(const std::string& expr,
                                        const std::vector<std::string>& hints,
                                        const std::vector<MrsTransformRule>& extra_rules,
                                        const MrsMathWitnessEnv& env,
                                        const std::unordered_map<std::string, std::string>& defs,
                                        std::string* error) {
    MrsTransformResult result;
    result.output = expand_mrs_defs(expr, defs);
    std::vector<MrsTransformRule> rules;
    for (const auto& r : marshal_transform_db()) {
        if (hint_matches(r, hints)) rules.push_back(r);
    }
    for (const auto& r : extra_rules) {
        if (hint_matches(r, hints) || hints.empty()) rules.push_back(r);
    }

    if (std::find(hints.begin(), hints.end(), "def_expand") != hints.end() ||
        std::find(hints.begin(), hints.end(), "def") != hints.end()) {
        result.output = expand_mrs_defs(result.output, defs);
        result.applied_rule_ids.push_back("def_expand");
        result.changed = true;
    }

    for (int pass = 0; pass < 32; ++pass) {
        bool pass_changed = false;
        for (const auto& rule : rules) {
            if (rule.from_pattern.find('(') != std::string::npos) {
                if (apply_func_rule(&result.output, rule)) {
                    result.applied_rule_ids.push_back(rule.id);
                    result.changed = true;
                    pass_changed = true;
                    break;
                }
            } else if (apply_ident_rule(&result.output, rule.from_pattern, rule.to_pattern)) {
                result.applied_rule_ids.push_back(rule.id);
                result.changed = true;
                pass_changed = true;
                break;
            }
        }
        if (!pass_changed) break;
    }

    if (std::find(hints.begin(), hints.end(), "normalize") != hints.end() ||
        std::find(hints.begin(), hints.end(), "arith") != hints.end()) {
        const std::string normed = algebraic_normalize_expr(result.output, env);
        if (normed != result.output) {
            result.output = normed;
            result.applied_rule_ids.push_back("algebraic_normalize");
            result.changed = true;
        }
    }

    result.ok = true;
    (void)error;
    return result;
}

bool evaluate_mrs_transform_steps(const std::vector<MrsTransformStep>& steps,
                                    const MrsCompilationBundle& bundle, MrsMathWitnessEnv* env,
                                    std::string* audit_trail, std::string* error) {
    if (!env) {
        if (error) *error = "null env for transform steps";
        return false;
    }
    const auto defs = collect_mrs_defs(bundle);
    const auto module_rules = collect_mrs_transform_rules(bundle);
    std::ostringstream trail;
    for (const auto& step : steps) {
        std::vector<MrsTransformRule> extra = module_rules;
        extra.insert(extra.end(), step.inline_rules.begin(), step.inline_rules.end());
        const MrsTransformResult tr =
            apply_mrs_transforms(step.target, step.hints, extra, *env, defs, error);
        if (!tr.ok) {
            if (error) *error = "transform failed on " + step.target;
            return false;
        }
        if (!trail.str().empty()) trail << ";";
        trail << step.target << "=>" << tr.output << "[";
        for (size_t i = 0; i < tr.applied_rule_ids.size(); ++i) {
            if (i) trail << ",";
            trail << tr.applied_rule_ids[i];
        }
        trail << "]";
        const size_t arrow = step.target.find("->");
        if (arrow != std::string::npos) {
            const std::string expect = trim(step.target.substr(arrow + 2));
            if (!algebraic_equiv_expr(tr.output, expect, *env, error)) {
                if (error) *error = "transform did not reach target: " + tr.output + " vs " + expect;
                return false;
            }
        } else {
            const size_t eq = step.target.find(":=");
            if (eq != std::string::npos) {
                const std::string name = trim(step.target.substr(0, eq));
                std::string nerr;
                if (const auto v = evaluate_mrs_numeric_expr(tr.output, *env, &nerr))
                    env->nums[name] = *v;
            }
        }
    }
    if (audit_trail) *audit_trail = trail.str();
    return true;
}

bool evaluate_rewrite_obligation(const MrsProofObligationDecl& ob,
                                 const MrsCompilationBundle& bundle,
                                 const MrsMathWitnessEnv& env, std::string* detail,
                                 std::string* error) {
    if (ob.witness_expr.empty()) {
        if (error) *error = "Rewrite obligation missing witness_expr";
        return false;
    }
    const auto defs = collect_mrs_defs(bundle);
    std::vector<MrsTransformRule> rules;
    for (const auto& raw : ob.rewrite_rules) {
        const size_t arrow = raw.find("->");
        if (arrow == std::string::npos) continue;
        MrsTransformRule r;
        r.id = "rewrite_" + ob.id;
        r.from_pattern = trim(raw.substr(0, arrow));
        r.to_pattern = trim(raw.substr(arrow + 2));
        r.hints = ob.transform_hints;
        rules.push_back(r);
    }
    const MrsTransformResult tr =
        apply_mrs_transforms(ob.witness_expr, ob.transform_hints, rules, env, defs, error);
    std::string err;
    if (!evaluate_mrs_witness_expr(tr.output, env, detail, &err)) {
        if (error) *error = "rewrite witness failed after transform: " + err;
        return false;
    }
    if (detail) {
        *detail = "rewrite:" + tr.output + ";rules=" + std::to_string(tr.applied_rule_ids.size());
    }
    return true;
}

bool evaluate_decision_procedure_obligation(const MrsProofObligationDecl& ob,
                                            const MrsMathWitnessEnv& env, std::string* detail,
                                            std::string* error) {
    if (ob.witness_expr.empty()) {
        if (error) *error = "DecisionProcedure missing witness_expr";
        return false;
    }
    const std::string proc = ob.decision_procedure;
    if (proc == "interval_arithmetic" || proc == "interval") {
        std::string err;
        if (!evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err)) {
            if (error) *error = "interval_arithmetic witness failed: " + err;
            return false;
        }
        if (detail) *detail = "decision:interval_arithmetic";
        return true;
    }
    if (proc == "spectral" || proc == "spectral_gap") {
        std::string err;
        if (!evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err)) {
            if (error) *error = "spectral decision witness failed: " + err;
            return false;
        }
        if (detail) *detail = "decision:spectral";
        return true;
    }
    if (proc == "grid" || proc == "grid_ratio") {
        std::string err;
        if (!evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err)) {
            if (error) *error = "grid decision witness failed: " + err;
            return false;
        }
        if (detail) *detail = "decision:grid";
        return true;
    }
    if (proc.empty()) {
        std::string err;
        return evaluate_mrs_witness_expr(ob.witness_expr, env, detail, &err);
    }
    if (error) *error = "unknown decision_procedure: " + proc;
    return false;
}

}  // namespace Marshal::AnaVM

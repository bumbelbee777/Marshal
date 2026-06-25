#include "MrsProofLogic.hxx"

#include "MrsMath.hxx"
#include "MrsTypeInfer.hxx"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>

namespace Marshal::AnaVM {
namespace {

std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string strip_trailing_semicolon(std::string s) {
    s = trim(s);
    if (!s.empty() && s.back() == ';') s.pop_back();
    return trim(s);
}

enum class Section { None, Assume, Transform, Derive, Paper, Steps, Conclude };

void parse_paper_field(const std::string& line, MrsProveScript* out) {
    if (!out) return;
    auto val = [&](const char* key) -> std::string {
        const std::string k(key);
        if (line.rfind(k, 0) != 0) return {};
        return trim(line.substr(k.size()));
    };
    if (const std::string v = val("label:"); !v.empty()) out->paper_label = v;
    if (const std::string v = val("env:"); !v.empty()) out->paper_env = v;
    if (const std::string v = val("title:"); !v.empty()) out->paper_title = v;
}

const MrsProveDecl* find_prove_decl(const MrsCompilationBundle& bundle, const std::string& name) {
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (p.name == name) return &p;
        }
    }
    return nullptr;
}

bool parse_transform_via_line(const std::string& line, MrsTransformStep* step) {
    if (!step) return false;
    std::string rest = trim(line);
    if (rest.rfind("transform ", 0) != 0) return false;
    rest = trim(rest.substr(10));
    const size_t via = rest.rfind(" via ");
    if (via == std::string::npos) return false;
    step->target = trim(rest.substr(0, via));
    std::stringstream hs(trim(rest.substr(via + 5)));
    std::string hint;
    while (std::getline(hs, hint, ',')) {
        hint = trim(hint);
        if (!hint.empty()) step->hints.push_back(hint);
    }
    return !step->target.empty() && !step->hints.empty();
}

std::string normalize_witness_line(std::string line) {
    line = trim(line);
    if (line.rfind("witness ", 0) == 0) line = trim(line.substr(8));
    if (line.rfind("using ", 0) == 0) return "dep " + trim(line.substr(6));
    return strip_trailing_semicolon(line);
}

bool is_ident_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

bool is_single_identifier_body(const std::string& body) {
    const std::string t = trim(body);
    if (t.empty() || t.find('(') != std::string::npos) return false;
    if (!std::isalpha(static_cast<unsigned char>(t[0])) && t[0] != '_') return false;
    for (char c : t) {
        if (!is_ident_char(c)) return false;
    }
    return true;
}

std::vector<std::string> prove_body_callee_ids(const std::string& body) {
    std::vector<std::string> out;
    const std::string t = trim(body);
    if (t.empty()) return out;
    const auto open = t.find('(');
    if (open == std::string::npos) {
        if (is_single_identifier_body(t)) out.push_back(t);
        return out;
    }
    int depth = 0;
    std::string token;
    auto flush = [&]() {
        const std::string id = trim(token);
        token.clear();
        if (!id.empty() && (std::isalpha(static_cast<unsigned char>(id[0])) || id[0] == '_'))
            out.push_back(id);
    };
    for (size_t i = open + 1; i < t.size(); ++i) {
        const char c = t[i];
        if (c == '(') {
            flush();
            ++depth;
            continue;
        }
        if (c == ')') {
            flush();
            if (depth == 0) break;
            --depth;
            continue;
        }
        if (depth == 0 && c == ',') {
            flush();
            continue;
        }
        if (depth == 0 && !std::isspace(static_cast<unsigned char>(c))) token += c;
    }
    flush();
    return out;
}

std::string json_escape(std::string s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\\' || c == '"') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else {
            out += c;
        }
    }
    return out;
}

}  // namespace

MrsProveScript parse_mrs_prove_script(const std::string& body) {
    MrsProveScript out;
    Section section = Section::None;
    std::stringstream ss(body);
    std::string line;
    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty() || line.rfind("//", 0) == 0) continue;
        if (line == "assume:" || line == "from:") {
            section = Section::Assume;
            continue;
        }
        if (line == "derive:") {
            section = Section::Derive;
            continue;
        }
        if (line == "paper:") {
            section = Section::Paper;
            continue;
        }
        if (line == "steps:") {
            section = Section::Steps;
            continue;
        }
        if (line == "transform:") {
            section = Section::Transform;
            continue;
        }
        if (line == "conclude:") {
            section = Section::Conclude;
            out.has_conclude = true;
            continue;
        }
        if (line.rfind("let ", 0) == 0) {
            out.lets.push_back(strip_trailing_semicolon(line.substr(4)));
            continue;
        }
        if (line.rfind("transform ", 0) == 0) {
            MrsTransformStep step;
            if (parse_transform_via_line(line, &step)) out.transforms.push_back(std::move(step));
            continue;
        }
        if (section == Section::Assume) {
            if (line.rfind("classical ", 0) == 0 || line.rfind("axiom ", 0) == 0) {
                const size_t skip = line.rfind("classical ", 0) == 0 ? 10 : 6;
                out.classical.push_back(trim(line.substr(skip)));
            } else if (line.rfind("dep ", 0) == 0) {
                out.deps.push_back(trim(line.substr(4)));
            } else {
                out.witness.push_back(normalize_witness_line(line));
            }
        } else if (section == Section::Transform) {
            MrsTransformStep step;
            if (parse_transform_via_line(line, &step)) out.transforms.push_back(std::move(step));
        } else if (section == Section::Derive) {
            std::string f = line;
            if (f.rfind("check:", 0) == 0) f = trim(f.substr(6));
            f = strip_trailing_semicolon(f);
            if (!f.empty()) out.derive.push_back(f);
        } else if (section == Section::Paper) {
            parse_paper_field(line, &out);
        } else if (section == Section::Steps) {
            out.steps.push_back(line);
        } else if (section == Section::Conclude) {
            if (!out.conclusion.empty()) out.conclusion += " ";
            out.conclusion += line;
        }
    }
    out.conclusion = trim(out.conclusion);
    return out;
}

std::unordered_map<std::string, std::string> collect_mrs_defs(const MrsCompilationBundle& bundle) {
    std::unordered_map<std::string, std::string> out;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& d : m.defs) {
            if (!d.name.empty()) out[d.name] = d.expr;
        }
    }
    return out;
}

std::unordered_map<std::string, MrsFnRecord> collect_mrs_fns(const MrsCompilationBundle& bundle) {
    std::unordered_map<std::string, MrsFnRecord> out;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& f : m.fns) {
            if (!f.name.empty()) out[f.name] = MrsFnRecord{f.params, f.body};
        }
    }
    return out;
}

bool mrs_ident_char(unsigned char c) { return std::isalnum(c) || c == '_'; }

std::string expand_mrs_defs(const std::string& expr,
                            const std::unordered_map<std::string, std::string>& defs) {
    std::vector<std::pair<std::string, std::string>> ordered;
    ordered.reserve(defs.size());
    for (const auto& kv : defs) {
        if (!kv.first.empty()) ordered.push_back(kv);
    }
    std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        return a.first.size() > b.first.size();
    });
    std::string out = expr;
    for (int pass = 0; pass < 16; ++pass) {
        bool changed = false;
        for (const auto& kv : ordered) {
            const std::string& needle = kv.first;
            size_t pos = 0;
            while ((pos = out.find(needle, pos)) != std::string::npos) {
                const bool left_ok =
                    pos == 0 || !mrs_ident_char(static_cast<unsigned char>(out[pos - 1]));
                const bool right_ok =
                    pos + needle.size() >= out.size() ||
                    !mrs_ident_char(static_cast<unsigned char>(out[pos + needle.size()]));
                if (left_ok && right_ok) {
                    out.replace(pos, needle.size(), kv.second);
                    pos += kv.second.size();
                    changed = true;
                } else {
                    pos += needle.size();
                }
            }
        }
        if (!changed) break;
    }
    return out;
}

bool evaluate_mrs_prove_witnesses(const MrsProveScript& script,
                                  const MrsCompilationBundle& bundle,
                                  const MrsMathWitnessEnv& env, std::string* detail,
                                  std::string* error, const bool require_formal_conclude) {
    if (!script.has_conclude) {
        if (error) *error = "prove body missing conclude: block";
        return false;
    }
    if (script.conclusion.empty()) {
        if (error) *error = "prove body has empty conclusion";
        return false;
    }
    if (script.classical.empty() && script.witness.empty() && script.deps.empty()) {
        if (error) *error = "prove body missing assume witnesses or deps";
        return false;
    }
    if (script.classical.empty() && script.witness.empty() && !script.deps.empty() &&
        script.steps.empty()) {
        if (error) *error = "prove script is deps-only weak reduction (E0916)";
        return false;
    }
    const auto defs = collect_mrs_defs(bundle);
    MrsMathWitnessEnv local = env;
    merge_mrs_fns_into_env(bundle, &local);
    for (const auto& let_line : script.lets) {
        const size_t eq = let_line.find(":=");
        if (eq == std::string::npos) continue;
        std::string name = trim(let_line.substr(0, eq));
        std::string type_annot;
        const size_t colon = name.find(':');
        if (colon != std::string::npos) {
            type_annot = trim(name.substr(colon + 1));
            name = trim(name.substr(0, colon));
        }
        const std::string expr = expand_mrs_defs(trim(let_line.substr(eq + 2)), defs);
        if (name.empty() || expr.empty()) continue;
        if (!type_annot.empty()) {
            MrsInferredType expected;
            if (parse_mrs_type_annotation(type_annot, &expected)) {
                const MrsInferredType got = infer_mrs_expr_type(expr, local, defs);
                std::string terr;
                if (!mrs_type_compatible(expected, got, &terr)) {
                    if (error) *error = "let type mismatch for " + name + ": " + terr;
                    return false;
                }
            }
        }
        std::string nerr;
        if (const auto val = evaluate_mrs_numeric_expr(expr, local, &nerr)) {
            local.nums[name] = *val;
        }
    }
    if (!script.transforms.empty()) {
        std::string trail;
        std::string terr;
        if (!evaluate_mrs_transform_steps(script.transforms, bundle, &local, &trail, &terr)) {
            if (error) *error = terr.empty() ? "transform steps failed" : terr;
            return false;
        }
    }
    std::ostringstream oss;
    for (const auto& w : script.witness) {
        const std::string expanded = expand_mrs_defs(w, defs);
        std::string err;
        std::string sub;
        if (!evaluate_mrs_witness_expr(expanded, local, &sub, &err)) {
            if (error) *error = "prove witness failed: " + expanded + " (" + err + ")";
            return false;
        }
        if (!oss.str().empty()) oss << ";";
        oss << expanded;
    }
    const std::string conclusion_expanded =
        expand_mrs_defs(extract_formal_mrs_conclusion(script.conclusion), defs);
    const auto truth = try_evaluate_mrs_formula_truth(conclusion_expanded, local);
    const bool formal_required = require_formal_conclude || !script.derive.empty();
    if (formal_required) {
        if (!truth) {
            if (error) {
                *error = "conclude must be a formal MrsMath formula evaluating true (E0921): " +
                         conclusion_expanded;
            }
            return false;
        }
        if (!*truth) {
            if (error) *error = "prove conclusion formula false: " + conclusion_expanded;
            return false;
        }
    } else if (truth && !*truth) {
        if (error) *error = "prove conclusion formula false: " + conclusion_expanded;
        return false;
    }
    for (size_t i = 0; i < script.derive.size(); ++i) {
        std::string f = normalize_mrs_formula_unicode(script.derive[i]);
        if (f.rfind("check:", 0) == 0) f = trim(f.substr(6));
        const std::string expanded = expand_mrs_defs(f, defs);
        const auto dt = try_evaluate_mrs_formula_truth(expanded, local);
        if (!dt || !*dt) {
            if (error) {
                *error = "derive step " + std::to_string(i + 1) + " not formally true: " + expanded;
            }
            return false;
        }
    }
    if (detail) {
        std::ostringstream d;
        d << "classical_n=" << script.classical.size() << ";deps_n=" << script.deps.size()
          << ";transforms_n=" << script.transforms.size()
          << ";derive_n=" << script.derive.size()
          << ";steps_n=" << script.steps.size() << ";witness={" << oss.str()
          << "};conclude=" << script.conclusion;
        for (size_t i = 0; i < script.steps.size(); ++i) {
            d << ";step" << (i + 1) << "=" << script.steps[i];
        }
        for (size_t i = 0; i < script.derive.size(); ++i) {
            d << ";derive" << (i + 1) << "=" << script.derive[i];
        }
        *detail = d.str();
    }
    return true;
}

MrsTheoremAuditReport audit_mrs_theorems(const MrsCompilationBundle& bundle,
                                         const MrsMathWitnessEnv& env) {
    MrsTheoremAuditReport rep;
    rep.ok = true;
    const auto defs = collect_mrs_defs(bundle);
    MrsMathWitnessEnv local = env;
    merge_mrs_fns_into_env(bundle, &local);
    for (const auto& m : bundle.merged_modules) {
        for (const auto& th : m.theorems) {
            MrsTheoremAuditRow row;
            row.name = th.name;
            row.goal = th.goal;
            row.prove_ref = th.prove_ref;
            row.paper_label = th.paper_label;
            row.paper_env = th.paper_env;
            row.paper_title = th.paper_title;
            const MrsProveDecl* prove = find_prove_decl(bundle, th.prove_ref);
            if (!prove || !mrs_prove_body_is_formula_script(prove->body)) {
                row.failure_reason = "theorem missing formula prove script: " + th.prove_ref;
                rep.rows.push_back(row);
                rep.ok = false;
                continue;
            }
            const MrsProveScript script = parse_mrs_prove_script(prove->body);
            if (row.paper_label.empty()) row.paper_label = script.paper_label;
            if (row.paper_env.empty()) row.paper_env = script.paper_env;
            if (row.paper_title.empty()) row.paper_title = script.paper_title;
            row.derive = script.derive;
            row.steps = script.steps;
            row.conclude = script.conclusion;
            std::string detail;
            std::string err;
            if (!evaluate_mrs_prove_witnesses(script, bundle, env, &detail, &err, true)) {
                row.failure_reason = err;
                rep.rows.push_back(row);
                rep.ok = false;
                continue;
            }
            const std::string goal_exp = expand_mrs_defs(th.goal, defs);
            const auto goal_truth = try_evaluate_mrs_formula_truth(goal_exp, local);
            if (!goal_truth || !*goal_truth) {
                row.failure_reason = "theorem goal not formally true: " + goal_exp;
                rep.rows.push_back(row);
                rep.ok = false;
                continue;
            }
            row.ok = true;
            rep.rows.push_back(row);
        }
    }
    return rep;
}

bool write_mrs_theorem_catalog_json(const MrsTheoremAuditReport& report,
                                    const std::string& path, std::string* error) {
    std::ostringstream out;
    out << "{\n  \"ok\": " << (report.ok ? "true" : "false") << ",\n  \"theorems\": [\n";
    for (size_t i = 0; i < report.rows.size(); ++i) {
        const auto& r = report.rows[i];
        out << "    {\"name\": \"" << json_escape(r.name) << "\", \"prove_ref\": \""
            << json_escape(r.prove_ref) << "\", \"goal\": \"" << json_escape(r.goal)
            << "\", \"paper_label\": \"" << json_escape(r.paper_label) << "\", \"paper_env\": \""
            << json_escape(r.paper_env) << "\", \"paper_title\": \"" << json_escape(r.paper_title)
            << "\", \"conclude\": \"" << json_escape(r.conclude) << "\", \"ok\": "
            << (r.ok ? "true" : "false");
        if (!r.failure_reason.empty()) {
            out << ", \"failure_reason\": \"" << json_escape(r.failure_reason) << "\"";
        }
        out << ", \"derive\": [";
        for (size_t j = 0; j < r.derive.size(); ++j) {
            if (j) out << ", ";
            out << "\"" << json_escape(r.derive[j]) << "\"";
        }
        out << "], \"steps\": [";
        for (size_t j = 0; j < r.steps.size(); ++j) {
            if (j) out << ", ";
            out << "\"" << json_escape(r.steps[j]) << "\"";
        }
        out << "]}";
        if (i + 1 < report.rows.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    std::ofstream f(path);
    if (!f) {
        if (error) *error = "cannot write " + path;
        return false;
    }
    f << out.str();
    return true;
}

bool mrs_prove_body_is_formula_script(const std::string& body) {
    return body.find("assume:") != std::string::npos && body.find("conclude:") != std::string::npos;
}

bool mrs_prove_body_is_disciplined(const std::string& body,
                                   const std::unordered_set<std::string>& prove_names) {
    if (mrs_prove_body_is_formula_script(body)) return true;
    const std::string t = trim(body);
    if (t.find("forall_extension(") != std::string::npos) return true;
    if (t.find("induction(") != std::string::npos) return true;
    if (t.find("convergence(") != std::string::npos) return true;
    const auto callees = prove_body_callee_ids(body);
    int prove_callee_count = 0;
    for (const auto& c : callees) {
        if (prove_names.count(c)) ++prove_callee_count;
    }
    return prove_callee_count >= 2 && !is_single_identifier_body(body);
}

bool validate_prove_script_deps(const MrsProveScript& script,
                                const std::vector<std::string>& graph_deps,
                                std::string* error) {
    for (const auto& dep : script.deps) {
        const auto it = std::find(graph_deps.begin(), graph_deps.end(), dep);
        if (it == graph_deps.end()) {
            if (error) *error = "script dep not in graph deps: " + dep;
            return false;
        }
    }
    return true;
}

}  // namespace Marshal::AnaVM

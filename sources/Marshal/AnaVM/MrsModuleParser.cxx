#include "MrsModuleParser.hxx"

#include "MrsParser.hxx"

#include <cctype>
#include <fstream>
#include <sstream>

namespace Marshal::AnaVM {
namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}

std::string strip_mrs_field_value(std::string v) {
    v = trim(std::move(v));
    if (!v.empty() && v.back() == ',') v.pop_back();
    v = trim(v);
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"') return v.substr(1, v.size() - 2);
    return v;
}

void bump_brace_depth(const std::string& raw, int& brace_depth) {
    for (char c : raw) {
        if (c == '{') ++brace_depth;
        if (c == '}') --brace_depth;
    }
}

void maybe_exit_scopes(int brace_depth, int& mod_entered_at, MrsModule*& cur_mod, int& graph_entered_at,
                       MrsProofGraphDecl*& cur_graph) {
    if (cur_graph && graph_entered_at >= 0 && brace_depth < graph_entered_at) {
        cur_graph = nullptr;
        graph_entered_at = -1;
    }
    if (cur_mod && mod_entered_at >= 0 && brace_depth < mod_entered_at) {
        cur_mod = nullptr;
        mod_entered_at = -1;
    }
}

void finish_mrs_line(const std::string& raw, int& brace_depth, bool& pending_mod_enter,
                     int& mod_entered_at, bool& pending_graph_enter, int& graph_entered_at,
                     MrsModule*& cur_mod, MrsProofGraphDecl*& cur_graph) {
    bump_brace_depth(raw, brace_depth);
    if (pending_mod_enter) {
        mod_entered_at = brace_depth;
        pending_mod_enter = false;
    }
    if (pending_graph_enter) {
        graph_entered_at = brace_depth;
        pending_graph_enter = false;
    }
    maybe_exit_scopes(brace_depth, mod_entered_at, cur_mod, graph_entered_at, cur_graph);
}

std::string strip_comment(const std::string& line) {
    const size_t p = line.find("//");
    if (p != std::string::npos) return line.substr(0, p);
    const size_t h = line.find('#');
    return h == std::string::npos ? line : line.substr(0, h);
}

ProofClass parse_proof_class(const std::string& v) {
    const auto s = trim(v);
    if (s == "NumericInterval" || s == "numeric_interval") return ProofClass::NumericInterval;
    if (s == "Analytic" || s == "analytic") return ProofClass::Analytic;
    if (s == "ClassicalImport" || s == "classical_import") return ProofClass::ClassicalImport;
    if (s == "Reduction" || s == "reduction") return ProofClass::Reduction;
    if (s == "AnalyticOpen" || s == "analytic_open") return ProofClass::AnalyticOpen;
    if (s == "Structural" || s == "structural") return ProofClass::Structural;
    if (s == "Composition" || s == "composition") return ProofClass::Composition;
    if (s == "Universal" || s == "universal") return ProofClass::Universal;
    if (s == "Inductive" || s == "inductive") return ProofClass::Inductive;
    if (s == "Convergent" || s == "convergent") return ProofClass::Convergent;
    if (s == "Rewrite" || s == "rewrite") return ProofClass::Rewrite;
    if (s == "DecisionProcedure" || s == "decision_procedure" || s == "decision")
        return ProofClass::DecisionProcedure;
    return ProofClass::Numeric;
}

bool parse_use_line(const std::string& line, MrsUseDecl& out) {
    if (line.rfind("use ", 0) != 0) return false;
    auto rest = trim(line.substr(4));
    if (!rest.empty() && rest.back() == ';') rest.pop_back();
    out.path = trim(rest);
    if (!out.path.empty() && out.path.back() == '*') {
        out.glob_all = true;
        out.path.pop_back();
        if (!out.path.empty() && out.path.back() == ':') out.path.pop_back();
        if (!out.path.empty() && out.path.back() == ':') out.path.pop_back();
        if (!out.path.empty() && out.path.back() == '.') out.path.pop_back();
    }
    return true;
}

std::vector<std::string> parse_bracket_list(const std::string& s) {
    std::vector<std::string> out;
    size_t i = s.find('[');
    size_t j = s.find(']');
    if (i == std::string::npos || j == std::string::npos || j <= i) return out;
    std::string inner = s.substr(i + 1, j - i - 1);
    std::stringstream ss(inner);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        tok = trim(tok);
        if (!tok.empty()) out.push_back(tok);
    }
    return out;
}

std::vector<double> parse_numeric_bracket_list(const std::string& s) {
    std::vector<double> out;
    for (const auto& tok : parse_bracket_list(s)) {
        try {
            out.push_back(std::stod(tok));
        } catch (...) {
            out.clear();
            return out;
        }
    }
    return out;
}

std::vector<std::string> parse_paren_id_list(const std::string& inside) {
    std::vector<std::string> out;
    std::string tok;
    for (char c : inside) {
        if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
            const std::string t = trim(tok);
            if (!t.empty()) out.push_back(t);
            tok.clear();
        } else {
            tok += c;
        }
    }
    const std::string t = trim(tok);
    if (!t.empty()) out.push_back(t);
    return out;
}

}  // namespace

MrsCompilationUnit parse_mrs_unit(const std::string& path) {
    MrsCompilationUnit unit;
    unit.source_path = path;

    std::ifstream in(path);
    if (!in) {
        MrsError e;
        e.code = "E0100";
        e.message = "cannot open file: " + path;
        // caller merges errors
        return unit;
    }

    MrsModule* cur_mod = nullptr;
    MrsProofGraphDecl* cur_graph = nullptr;
    int brace_depth = 0;
    int mod_entered_at = -1;
    int graph_entered_at = -1;
    bool pending_mod_enter = false;
    bool pending_graph_enter = false;
    int line_no = 0;
    bool in_prove_body = false;
    MrsProveDecl* cur_prove = nullptr;
    std::string prove_body_accum;
    MrsTheoremDecl* cur_theorem = nullptr;

    bool saw_ansatz = false;

    std::string line;
    while (std::getline(in, line)) {
        ++line_no;
        const std::string raw = line;
        line = trim(strip_comment(line));
        if (line.empty()) continue;

        bool absorb_prove_interior = false;

        if (in_prove_body && cur_prove) {
            if (line == "}") {
                cur_prove->body = trim(prove_body_accum);
                if (cur_prove->body == "infer" || cur_prove->body.empty()) {
                    cur_prove->body_kind = MrsProofBodyKind::Infer;
                } else {
                    cur_prove->body_kind = MrsProofBodyKind::Explicit;
                }
                in_prove_body = false;
                cur_prove = nullptr;
                prove_body_accum.clear();
            } else {
                if (!prove_body_accum.empty()) prove_body_accum += '\n';
                prove_body_accum += line;
                absorb_prove_interior = true;
            }
        } else if (cur_theorem && line == "}") {
            cur_theorem = nullptr;
            absorb_prove_interior = true;
        } else if (cur_theorem) {
            if (line.find("class:") != std::string::npos) {
                cur_theorem->proof_class =
                    parse_proof_class(strip_mrs_field_value(line.substr(line.find(':') + 1)));
            } else if (line.find("goal:") != std::string::npos) {
                cur_theorem->goal = strip_mrs_field_value(line.substr(line.find(':') + 1));
            } else if (line.find("prove:") != std::string::npos) {
                cur_theorem->prove_ref = strip_mrs_field_value(line.substr(line.find(':') + 1));
            } else if (line.find("paper_label:") != std::string::npos) {
                cur_theorem->paper_label =
                    strip_mrs_field_value(line.substr(line.find(':') + 1));
            } else if (line.find("paper_env:") != std::string::npos) {
                cur_theorem->paper_env = strip_mrs_field_value(line.substr(line.find(':') + 1));
            } else if (line.find("paper_title:") != std::string::npos) {
                cur_theorem->paper_title =
                    strip_mrs_field_value(line.substr(line.find(':') + 1));
            }
            absorb_prove_interior = true;
        }

        if (!absorb_prove_interior) {
            MrsUseDecl use;
            if (parse_use_line(line, use)) {
                use.span.line = line_no;
                if (cur_mod)
                    cur_mod->uses.push_back(use);
                else
                    unit.top_uses.push_back(use);
            } else if (line.rfind("mod ", 0) == 0) {
                unit.has_modules = true;
                MrsModule m;
                auto rest = trim(line.substr(4));
                const size_t brace = rest.find('{');
                m.name = brace == std::string::npos ? trim(rest) : trim(rest.substr(0, brace));
                m.span.line = line_no;
                unit.modules.push_back(std::move(m));
                cur_mod = &unit.modules.back();
                if (brace != std::string::npos && rest.back() == '{') pending_mod_enter = true;
            } else if (line.rfind("ansatz ", 0) == 0) {
                saw_ansatz = true;
            } else if (line.rfind("pub const ", 0) == 0 || line.rfind("const ", 0) == 0) {
                if (cur_mod) {
                    MrsConstDecl c;
                    c.span.line = line_no;
                    bool pub = line.rfind("pub ", 0) == 0;
                    auto rest = pub ? trim(line.substr(4)) : line;
                    if (rest.rfind("const ", 0) == 0) rest = trim(rest.substr(6));
                    const size_t colon = rest.find(':');
                    const size_t eq = rest.find('=');
                    if (colon != std::string::npos) {
                        c.name = trim(rest.substr(0, colon));
                        if (eq != std::string::npos) {
                            c.type_expr = trim(rest.substr(colon + 1, eq - colon - 1));
                            c.value_expr = trim(rest.substr(eq + 1));
                            if (!c.value_expr.empty() && c.value_expr.back() == ';')
                                c.value_expr.pop_back();
                            if (c.value_expr.rfind("infer!", 0) == 0) {
                                c.infer_value = true;
                                if (c.value_expr.rfind("infer!rational_bound", 0) == 0)
                                    c.infer_rational_bound = true;
                            }
                        } else {
                            c.type_expr = trim(rest.substr(colon + 1));
                        }
                    }
                    c.visibility = pub ? MrsVisibility::Pub : MrsVisibility::Private;
                    cur_mod->consts.push_back(std::move(c));
                }
            } else if (line.rfind("pub array ", 0) == 0 || line.rfind("array ", 0) == 0) {
                if (cur_mod) {
                    MrsArrayDecl a;
                    a.span.line = line_no;
                    const bool pub = line.rfind("pub ", 0) == 0;
                    auto rest = pub ? trim(line.substr(4)) : line;
                    if (rest.rfind("array ", 0) == 0) rest = trim(rest.substr(6));
                    const size_t eq = rest.find(":=");
                    if (eq != std::string::npos) {
                        a.name = trim(rest.substr(0, eq));
                        const std::string vals = trim(rest.substr(eq + 2));
                        a.elements = parse_numeric_bracket_list(vals);
                    }
                    a.visibility = pub ? MrsVisibility::Pub : MrsVisibility::Private;
                    if (!a.name.empty() && !a.elements.empty()) cur_mod->arrays.push_back(std::move(a));
                }
            } else if (line.rfind("def ", 0) == 0) {
                if (cur_mod) {
                    MrsDefDecl d;
                    d.span.line = line_no;
                    auto rest = trim(line.substr(4));
                    const size_t eq = rest.find(":=");
                    if (eq != std::string::npos) {
                        d.name = trim(rest.substr(0, eq));
                        d.expr = trim(rest.substr(eq + 2));
                        if (!d.expr.empty() && d.expr.back() == ';') d.expr.pop_back();
                        d.expr = trim(d.expr);
                        cur_mod->defs.push_back(std::move(d));
                    }
                }
            } else if (line.rfind("pub fn ", 0) == 0 || line.rfind("fn ", 0) == 0) {
                if (cur_mod) {
                    MrsFnDecl f;
                    f.span.line = line_no;
                    const bool pub = line.rfind("pub ", 0) == 0;
                    auto rest = pub ? trim(line.substr(4)) : line;
                    if (rest.rfind("fn ", 0) == 0) rest = trim(rest.substr(3));
                    const size_t lpar = rest.find('(');
                    const size_t rpar = rest.find(')');
                    const size_t eq = rest.find(":=");
                    if (lpar != std::string::npos && rpar != std::string::npos && rpar > lpar &&
                        eq != std::string::npos && eq > rpar) {
                        f.name = trim(rest.substr(0, lpar));
                        f.params = parse_paren_id_list(rest.substr(lpar + 1, rpar - lpar - 1));
                        f.body = trim(rest.substr(eq + 2));
                        if (!f.body.empty() && f.body.back() == ';') f.body.pop_back();
                        f.body = trim(f.body);
                        f.visibility = pub ? MrsVisibility::Pub : MrsVisibility::Private;
                        if (!f.name.empty() && !f.body.empty()) cur_mod->fns.push_back(std::move(f));
                    }
                }
            } else if (line.rfind("prove ", 0) == 0) {
                if (cur_mod) {
                    MrsProveDecl p;
                    p.span.line = line_no;
                    auto rest = trim(line.substr(6));
                    const size_t colon = rest.find(':');
                    if (colon != std::string::npos) {
                        p.name = trim(rest.substr(0, colon));
                        rest = trim(rest.substr(colon + 1));
                        if (!rest.empty() && rest.back() == '{') {
                            p.type_expr = trim(rest.substr(0, rest.size() - 1));
                            cur_mod->proves.push_back(p);
                            cur_prove = &cur_mod->proves.back();
                            in_prove_body = true;
                            prove_body_accum.clear();
                        }
                    }
                }
            } else if (line.rfind("theorem ", 0) == 0) {
                if (cur_mod) {
                    MrsTheoremDecl t;
                    t.span.line = line_no;
                    auto rest = trim(line.substr(8));
                    const size_t brace = rest.find('{');
                    t.name = brace == std::string::npos ? trim(rest) : trim(rest.substr(0, brace));
                    cur_mod->theorems.push_back(std::move(t));
                    cur_theorem = &cur_mod->theorems.back();
                }
            } else if (line.rfind("proof_graph ", 0) == 0) {
                if (cur_mod) {
                    MrsProofGraphDecl g;
                    g.span.line = line_no;
                    auto rest = trim(line.substr(12));
                    const size_t brace = rest.find('{');
                    g.name = brace == std::string::npos ? trim(rest) : trim(rest.substr(0, brace));
                    cur_mod->proof_graphs.push_back(std::move(g));
                    cur_graph = &cur_mod->proof_graphs.back();
                    if (brace != std::string::npos) pending_graph_enter = true;
                }
            } else if (cur_graph) {
                if (line.rfind("target:", 0) == 0) {
                    cur_graph->target = strip_mrs_field_value(line.substr(7));
                } else if (line.rfind("obligation ", 0) == 0) {
                    MrsProofObligationDecl ob;
                    ob.span.line = line_no;
                    auto rest = trim(line.substr(11));
                    const size_t brace = rest.find('{');
                    ob.id = brace == std::string::npos ? trim(rest) : trim(rest.substr(0, brace));
                    cur_graph->obligations.push_back(std::move(ob));
                } else if (!cur_graph->obligations.empty()) {
                    auto& ob = cur_graph->obligations.back();
                    if (line.find("class:") != std::string::npos) {
                        ob.proof_class =
                            parse_proof_class(strip_mrs_field_value(line.substr(line.find(':') + 1)));
                    } else if (line.find("deps:") != std::string::npos) {
                        ob.dependencies = parse_bracket_list(line);
                    } else if (line.find("prove:") != std::string::npos) {
                        auto pv = trim(line.substr(line.find(':') + 1));
                        if (!pv.empty() && pv.back() == ',') pv.pop_back();
                        if (pv == "infer") {
                            ob.prove_kind = MrsProofBodyKind::Infer;
                        } else {
                            ob.prove_kind = MrsProofBodyKind::Ref;
                            ob.prove_ref = pv;
                        }
                    } else if (line.find("statement:") != std::string::npos) {
                        ob.statement = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("domain:") != std::string::npos &&
                               line.find("domain_lower:") == std::string::npos &&
                               line.find("domain_upper:") == std::string::npos) {
                        ob.domain_kind = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("domain_lower:") != std::string::npos) {
                        ob.domain_lower = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("domain_upper:") != std::string::npos) {
                        ob.domain_upper = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("predicate:") != std::string::npos) {
                        ob.predicate = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("extend_via:") != std::string::npos) {
                        ob.extend_via = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("witness_expr:") != std::string::npos) {
                        ob.witness_expr = strip_mrs_field_value(line.substr(line.find(':') + 1));
                    } else if (line.find("rewrite_rules:") != std::string::npos) {
                        ob.rewrite_rules = parse_bracket_list(line);
                    } else if (line.find("transform_hints:") != std::string::npos) {
                        ob.transform_hints = parse_bracket_list(line);
                    } else if (line.find("decision_procedure:") != std::string::npos) {
                        ob.decision_procedure =
                            strip_mrs_field_value(line.substr(line.find(':') + 1));
                    }
                }
            } else if (cur_mod && line.rfind("transform rule ", 0) == 0) {
                MrsTransformRule tr;
                auto rest = trim(line.substr(15));
                const size_t brace = rest.find('{');
                tr.id = brace == std::string::npos ? trim(rest) : trim(rest.substr(0, brace));
                if (line.find("from:") != std::string::npos) {
                    tr.from_pattern = strip_mrs_field_value(line.substr(line.find("from:") + 5));
                }
                if (line.find("to:") != std::string::npos) {
                    tr.to_pattern = strip_mrs_field_value(line.substr(line.find("to:") + 3));
                }
                if (line.find("hints:") != std::string::npos) {
                    tr.hints = parse_bracket_list(line);
                }
                if (!tr.id.empty()) cur_mod->transforms.push_back(std::move(tr));
            } else if (cur_mod && !cur_mod->transforms.empty() && !cur_graph) {
                auto& tr = cur_mod->transforms.back();
                if (line.find("hints:") != std::string::npos) {
                    tr.hints = parse_bracket_list(line);
                } else if (line.find("from:") != std::string::npos) {
                    tr.from_pattern = strip_mrs_field_value(line.substr(line.find(':') + 1));
                } else if (line.find("to:") != std::string::npos) {
                    tr.to_pattern = strip_mrs_field_value(line.substr(line.find(':') + 1));
                }
            }
        }

        if (!absorb_prove_interior)
            finish_mrs_line(raw, brace_depth, pending_mod_enter, mod_entered_at, pending_graph_enter,
                            graph_entered_at, cur_mod, cur_graph);
    }

    if (saw_ansatz) {
        ParseResult pr = parse_mrs_file(path);
        unit.has_ansatz = true;
        unit.program = std::move(pr.program);
        unit.program.source_path = path;
        unit.errors = std::move(pr.errors);
        unit.ok = pr.ok;
    } else if (!unit.has_modules) {
        ParseResult pr = parse_mrs_file(path);
        if (pr.ok || !pr.program.id.empty()) {
            unit.has_ansatz = true;
            unit.program = std::move(pr.program);
            unit.program.source_path = path;
        }
        unit.errors = std::move(pr.errors);
        unit.ok = pr.ok;
    }

    return unit;
}

}  // namespace Marshal::AnaVM

#include "MrsParser.hxx"

#include "MrsSym.hxx"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Marshal::AnaVM {
namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}

std::string strip_comment(const std::string& line) {
    const size_t p = line.find('#');
    return p == std::string::npos ? line : line.substr(0, p);
}

bool parse_bool(const std::string& v) {
    return v == "true" || v == "yes";
}

HeatCoupling parse_heat(const std::string& v) {
    if (v == "none") return HeatCoupling::None;
    if (v == "berry_keating") return HeatCoupling::BerryKeating;
    if (v == "connes_heat") return HeatCoupling::ConnesHeat;
    return HeatCoupling::Poisson;
}

bool expr_uses_gamma(const std::string& expr) {
    for (size_t i = 0; i + 5 <= expr.size(); ++i) {
        if (expr.compare(i, 5, "gamma") == 0) {
            const bool left = i == 0 || !std::isalnum(static_cast<unsigned char>(expr[i - 1]));
            const bool right = i + 5 >= expr.size() ||
                               !std::isalnum(static_cast<unsigned char>(expr[i + 5]));
            if (left && right) return true;
        }
    }
    return false;
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

}  // namespace

ParseResult parse_mrs_file(const std::string& path) {
    ParseResult out;
    out.program.source_path = path;
    std::ifstream in(path);
    if (!in) {
        add_error(out.errors, "E0100", 0, "cannot open file: " + path);
        return out;
    }

    enum class Block { None, Meta, Rescale, On, Coupling, Diagnostics, Expect };
    Block block = Block::None;
    int brace_depth = 0;
    int line_no = 0;
    OnBlock cur_on;
    bool in_ansatz = false;

    std::string line;
    while (std::getline(in, line)) {
        ++line_no;
        line = trim(strip_comment(line));
        if (line.empty()) continue;

        if (line.rfind("ansatz ", 0) == 0) {
            in_ansatz = true;
            auto name = trim(line.substr(7));
            const size_t sp = name.find_first_of(" \t{");
            out.program.id = sp == std::string::npos ? name : name.substr(0, sp);
            if (!name.empty() && name.back() == '{') brace_depth = 1;
            continue;
        }
        if (!in_ansatz) continue;

        for (char c : line) {
            if (c == '{') ++brace_depth;
            if (c == '}') --brace_depth;
        }

        if (line.find("meta") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Meta;
            continue;
        }
        if (line.find("rescale") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Rescale;
            continue;
        }
        if (line.rfind("on ", 0) == 0) {
            block = Block::On;
            cur_on = OnBlock{};
            cur_on.span.line = line_no;
            const size_t sp = line.find("circle");
            if (sp != std::string::npos) cur_on.space = "circle S¹_log(p)";
            else if (line.find("phase_space") != std::string::npos) cur_on.space = "phase_space";
            else if (line.find("adele_class") != std::string::npos) cur_on.space = "adele_class";
            else if (line.find("adelic_grid") != std::string::npos) cur_on.space = "adelic_quotient";
            continue;
        }
        if (line.rfind("space ", 0) == 0) {
            block = Block::On;
            cur_on = OnBlock{};
            cur_on.span.line = line_no;
            if (line.find("phase_space") != std::string::npos) cur_on.space = "phase_space";
            else if (line.find("adele_class") != std::string::npos) cur_on.space = "adele_class";
            if (line.find('{') != std::string::npos && line.find('}') != std::string::npos)
                out.program.on_blocks.push_back(cur_on);
            continue;
        }
        if (line.rfind("lemma ref ", 0) == 0) {
            FormalRef ref;
            const auto body = trim(line.substr(10));
            const size_t br = body.find('{');
            ref.lemma_id = trim(br == std::string::npos ? body : body.substr(0, br));
            if (br != std::string::npos) {
                const auto st = body.find("status:");
                if (st != std::string::npos) {
                    auto v = trim(body.substr(st + 7));
                    const size_t end = v.find_first_of(" \t}");
                    ref.status = end == std::string::npos ? v : v.substr(0, end);
                }
            }
            if (ref.status.empty()) ref.status = "open";
            out.program.lemma_refs.push_back(std::move(ref));
            continue;
        }
        if (line.find("coupling") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Coupling;
            continue;
        }
        if (line.find("diagnostics") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Diagnostics;
            continue;
        }
        if (line.rfind("expect ", 0) == 0) {
            out.program.expects.push_back(trim(line.substr(7)));
            continue;
        }

        if (block == Block::Meta) {
            if (line.find("uses_gamma:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                out.program.uses_gamma = parse_bool(v);
            } else if (line.find("sym_tier:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                if (v == "scaffold") out.program.sym_tier = SymTier::Scaffold;
            } else if (line.find("kind:") != std::string::npos) {
                out.program.kind = trim(line.substr(line.find(':') + 1));
            }
        } else if (block == Block::Rescale) {
            if (line.find("derivative") != std::string::npos && line.find("*=") != std::string::npos) {
                out.program.rescale.has_derivative_scale = true;
                const size_t p = line.find("*=");
                out.program.rescale.derivative_factor = trim(line.substr(p + 2));
            }
        } else if (block == Block::On) {
            if (line.find("frequency omega") != std::string::npos ||
                line.find("omega =") != std::string::npos) {
                const size_t eq = line.find('=');
                cur_on.omega_expr = trim(line.substr(eq + 1));
            } else if (line.find("eigenvalue lambda") != std::string::npos ||
                       line.find("lambda =") != std::string::npos) {
                const size_t eq = line.find('=');
                cur_on.lambda_expr = trim(line.substr(eq + 1));
            }
            if (line == "}" || (line.size() == 1 && line[0] == '}')) {
                out.program.on_blocks.push_back(cur_on);
                block = Block::None;
            }
        } else if (block == Block::Coupling) {
            if (line.find("heat:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                out.program.heat_coupling = parse_heat(v);
            }
        } else if (block == Block::Diagnostics) {
            if (line.find("falsify compact_sinc2") != std::string::npos)
                out.program.falsify_sinc2 = true;
            if (line.find("trace_lhs quotient_spectrum") != std::string::npos)
                out.program.trace_lhs_quotient = true;
            if (line.find("pair_correlation gue") != std::string::npos)
                out.program.pair_correlation_gue = true;
            if (line.find("formal analytics") != std::string::npos)
                out.program.formal_analytics = true;
        }

        if (line == "}" && brace_depth <= 0) break;
    }

    if (out.program.id.empty()) {
        add_error(out.errors, "E0100", 0, "missing ansatz declaration");
        return out;
    }

    for (const auto& b : out.program.on_blocks) {
        if (!out.program.uses_gamma && expr_uses_gamma(b.omega_expr)) {
            add_error(out.errors, "E0400", b.span.line,
                      "gamma referenced without uses_gamma: true",
                      "set uses_gamma: true or remove gamma from expressions");
        }
    }
    if (!out.program.uses_gamma) {
        for (const auto& e : out.program.expects) {
            if (expr_uses_gamma(e))
                add_error(out.errors, "E0400", 0, "gamma in expect without uses_gamma");
        }
    }

    const SymDeriveResult sym = derive_spectrum(out.program);
    out.program.placeholder = sym.placeholder;
    out.program.rule_id = sym.rule_id;
    if (sym.ok) {
        out.program.derived_omega = sym.omega_derived;
        out.program.derived_lambda = sym.lambda_derived;
        if (!sym.placeholder && out.program.heat_coupling == HeatCoupling::Poisson)
            out.program.required_omega = "2*pi*n/log(p)";
    }
    if (sym.placeholder && out.program.sym_tier != SymTier::Scaffold) {
        add_error(out.errors, "E0603", 0,
                  "placeholder ansatz requires meta { sym_tier: scaffold }",
                  "set sym_tier: scaffold for Berry-Keating / Connes programs");
    }
    if (!sym.placeholder)
        out.program.weil_ok = check_weil_coupling(out.program, out.errors);
    else if (out.program.heat_coupling != HeatCoupling::None) {
        out.program.weil_ok = check_weil_coupling(out.program, out.errors);
    }

    out.ok = out.errors.empty();
    return out;
}

void print_errors(const std::vector<MrsError>& errors) {
    for (const auto& e : errors) {
        std::cerr << "error[" << e.code << "]: " << e.message;
        if (e.span.line > 0) std::cerr << " (line " << e.span.line << ")";
        std::cerr << "\n";
        if (!e.hint.empty()) std::cerr << "  = hint: " << e.hint << "\n";
    }
}

}  // namespace Marshal::AnaVM

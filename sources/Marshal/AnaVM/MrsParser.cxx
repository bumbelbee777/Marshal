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

bool parse_diag_flag(const std::string& v) {
    return parse_bool(v) || v == "sanity" || v == "analytic_shape" || v == "analytic_only" ||
           v == "selection" || v == "formal";
}

HeatCoupling parse_heat(const std::string& v) {
    if (v == "none") return HeatCoupling::None;
    if (v == "berry_keating") return HeatCoupling::BerryKeating;
    if (v == "connes_heat") return HeatCoupling::ConnesHeat;
    return HeatCoupling::Poisson;
}

std::string lower_ascii(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

CompletionMethod parse_completion_method(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s == "inductive_limit") return CompletionMethod::InductiveLimit;
    if (s == "crossed_product") return CompletionMethod::CrossedProduct;
    return CompletionMethod::Cauchy;
}

CompletionConstraint parse_completion_constraint(const std::string& v) {
    if (lower_ascii(v).find("exact") != std::string::npos) return CompletionConstraint::Exact;
    return CompletionConstraint::Asymptotic;
}

CompletionNorm parse_completion_norm(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s == "sup") return CompletionNorm::Sup;
    if (s == "resolvent") return CompletionNorm::Resolvent;
    return CompletionNorm::L2;
}

AdelicMetric parse_adelic_metric(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s == "real") return AdelicMetric::Real;
    if (s == "adelic") return AdelicMetric::Adelic;
    return AdelicMetric::Mixed;
}

ArchimedeanType parse_arch_type(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s == "torus") return ArchimedeanType::Torus;
    if (s == "half_line") return ArchimedeanType::HalfLine;
    return ArchimedeanType::RealLine;
}

ArchimedeanBoundary parse_arch_boundary(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s.find("dirichlet") != std::string::npos) return ArchimedeanBoundary::Dirichlet;
    if (s.find("neumann") != std::string::npos) return ArchimedeanBoundary::Neumann;
    if (s.find("periodic") != std::string::npos) return ArchimedeanBoundary::Periodic;
    return ArchimedeanBoundary::BerryKeating;
}

ArchimedeanCutoff parse_arch_cutoff(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s.find("parameter") != std::string::npos || s.find("lambda") != std::string::npos)
        return ArchimedeanCutoff::ParameterLambda;
    return ArchimedeanCutoff::PlanckScale;
}

HeightMapType parse_height_map_type(const std::string& v) {
    const auto s = lower_ascii(v);
    if (s == "power") return HeightMapType::Power;
    if (s == "custom") return HeightMapType::Custom;
    return HeightMapType::Log;
}

SpectralDetTarget parse_spectral_target(const std::string& v) {
    if (lower_ascii(v).find("xi") != std::string::npos) return SpectralDetTarget::RiemannXi;
    return SpectralDetTarget::Custom;
}

SpectralDetMethod parse_spectral_method(const std::string& v) {
    if (lower_ascii(v).find("zeta") != std::string::npos)
        return SpectralDetMethod::ZetaRegularization;
    return SpectralDetMethod::HeatKernel;
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

    enum class Block {
        None, Meta, Rescale, On, Local, Quotient, SelfAdjointExtension, TraceFormula, Semiclassical,
        Coupling, CrossedProduct, Completion, AdelicCauchy, Archimedean, HeightMap, SpectralDeterminant,
        SpectralAction, DiscretizationLimit, FormalTarget, BoundAudit, GenusOneLogBounds,
        AssemblySearch, Investigation,
        InvestigationThetaCentered, InvestigationThetaFull, InvestigationLadderHeatT,
        InvestigationLadderPrimeLimit, Diagnostics, Expect
    };
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
        if (line.rfind("local ", 0) == 0 ||
            (line.find("local") != std::string::npos && line.find('{') != std::string::npos)) {
            block = Block::Local;
            out.program.local.present = true;
            continue;
        }
        if (line.rfind("quotient ", 0) == 0 ||
            (line.find("quotient") != std::string::npos && line.find('{') != std::string::npos &&
             line.find("quotient_spectrum") == std::string::npos)) {
            block = Block::Quotient;
            out.program.quotient.present = true;
            if (line.find("sunit") != std::string::npos) out.program.quotient.kind = "sunit";
            continue;
        }
        if (line.find("self_adjoint_extension") != std::string::npos &&
            line.find('{') != std::string::npos) {
            block = Block::SelfAdjointExtension;
            out.program.self_adjoint_extension.present = true;
            continue;
        }
        if (line.find("trace_formula") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::TraceFormula;
            out.program.trace_formula.present = true;
            continue;
        }
        if (line.find("semiclassical") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Semiclassical;
            out.program.semiclassical.present = true;
            continue;
        }
        if (line.find("coupling") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Coupling;
            continue;
        }
        if (line.find("crossed_product") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::CrossedProduct;
            continue;
        }
        if (line.find("adelic_cauchy") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::AdelicCauchy;
            out.program.adelic_cauchy.present = true;
            continue;
        }
        if (line.rfind("completion", 0) == 0 ||
            (line.find("completion") != std::string::npos && line.find('{') != std::string::npos &&
             line.find("adelic") == std::string::npos)) {
            block = Block::Completion;
            out.program.completion.present = true;
            continue;
        }
        if (line.find("archimedean") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Archimedean;
            out.program.archimedean.present = true;
            continue;
        }
        if (line.find("height_map") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::HeightMap;
            out.program.height_map.present = true;
            continue;
        }
        if (line.find("spectral_determinant") != std::string::npos &&
            line.find('{') != std::string::npos) {
            block = Block::SpectralDeterminant;
            out.program.spectral_determinant.present = true;
            continue;
        }
        if (line.find("spectral_action") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::SpectralAction;
            out.program.spectral_action.present = true;
            continue;
        }
        if (line.find("discretization_limit") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::DiscretizationLimit;
            out.program.discretization_limit.present = true;
            continue;
        }
        if (line.find("formal_target") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::FormalTarget;
            out.program.formal_target.present = true;
            continue;
        }
        if (line.find("bound_audit") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::BoundAudit;
            out.program.bound_audit.present = true;
            continue;
        }
        if (line.find("genus_one_log_bounds") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::GenusOneLogBounds;
            out.program.genus_one_log_bounds.present = true;
            continue;
        }
        if (line.find("assembly_search") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::AssemblySearch;
            out.program.assembly_search.present = true;
            continue;
        }
        if (line.find("investigation") != std::string::npos && line.find('{') != std::string::npos) {
            block = Block::Investigation;
            out.program.investigation.present = true;
            out.program.diagnostics.investigation_suite = true;
            continue;
        }
        if (block == Block::Investigation && line.find("theta_sweep_centered") != std::string::npos &&
            line.find('{') != std::string::npos) {
            block = Block::InvestigationThetaCentered;
            continue;
        }
        if (block == Block::Investigation && line.find("theta_sweep_full") != std::string::npos &&
            line.find('{') != std::string::npos) {
            block = Block::InvestigationThetaFull;
            continue;
        }
        if (block == Block::Investigation && line.find("ladder heat_t") != std::string::npos &&
            line.find('{') != std::string::npos) {
            block = Block::InvestigationLadderHeatT;
            continue;
        }
        if (block == Block::Investigation && line.find("ladder prime_limit") != std::string::npos &&
            line.find('{') != std::string::npos) {
            block = Block::InvestigationLadderPrimeLimit;
            continue;
        }
        if (block == Block::Investigation && line.find("diagnostic spectral_spacing") != std::string::npos) {
            out.program.investigation.run_spacing = true;
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
        } else if (block == Block::Local) {
            if (line.find("spectral_triple") != std::string::npos)
                out.program.local.spectral_triple = true;
            if (line.find("H =") != std::string::npos || line.find("H=") != std::string::npos) {
                const size_t eq = line.find('=');
                out.program.local.hamiltonian_expr = trim(line.substr(eq + 1));
            }
        } else if (block == Block::Quotient) {
            if (line.find("generators:") != std::string::npos)
                out.program.quotient.generators = trim(line.substr(line.find(':') + 1));
        } else if (block == Block::SelfAdjointExtension) {
            if (line.find("deficiency_indices:") != std::string::npos) {
                const auto v = line.substr(line.find(':') + 1);
                const size_t l = v.find('(');
                const size_t c = v.find(',');
                const size_t r = v.find(')');
                if (l != std::string::npos && c != std::string::npos && r != std::string::npos) {
                    out.program.self_adjoint_extension.deficiency_n =
                        std::stoi(trim(v.substr(l + 1, c - l - 1)));
                    out.program.self_adjoint_extension.deficiency_m =
                        std::stoi(trim(v.substr(c + 1, r - c - 1)));
                }
            }
            if (line.find("family:") != std::string::npos) {
                const auto v = lower_ascii(trim(line.substr(line.find(':') + 1)));
                if (v.find("u(2)") != std::string::npos || v.find("u2") != std::string::npos)
                    out.program.self_adjoint_extension.family = ExtensionFamily::U2;
                else
                    out.program.self_adjoint_extension.family = ExtensionFamily::U1;
            }
            if (line.find("sweep_steps:") != std::string::npos)
                out.program.self_adjoint_extension.sweep_steps =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::TraceFormula) {
            if (line.find("status:") != std::string::npos) {
                const auto v = lower_ascii(trim(line.substr(line.find(':') + 1)));
                if (v.find("proved") != std::string::npos)
                    out.program.trace_formula.status = TraceFormulaStatus::Proved;
                else if (v.find("open") != std::string::npos)
                    out.program.trace_formula.status = TraceFormulaStatus::Open;
            }
            if (line.find("mode:") != std::string::npos) {
                const auto v = lower_ascii(trim(line.substr(line.find(':') + 1)));
                if (v.find("weil") != std::string::npos)
                    out.program.trace_formula.mode = TraceFormulaMode::WeilFull;
            }
        } else if (block == Block::Semiclassical) {
            if (line.find("cutoff:") != std::string::npos)
                out.program.semiclassical.cutoff =
                    parse_arch_cutoff(trim(line.substr(line.find(':') + 1)));
            if (line.find("ladder:") != std::string::npos) {
                const auto v = lower_ascii(trim(line.substr(line.find(':') + 1)));
                if (v.find("wkb") != std::string::npos)
                    out.program.semiclassical.ladder = SemiclassicalLadder::Wkb;
            }
            if (line.find("log_span:") != std::string::npos)
                out.program.semiclassical.log_span =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("max_modes:") != std::string::npos)
                out.program.semiclassical.max_modes =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("height_renormalize:") != std::string::npos) {
                const auto v = lower_ascii(trim(line.substr(line.find(':') + 1)));
                out.program.semiclassical.height_renormalize_log_n =
                    v.find("false") == std::string::npos && v.find("none") == std::string::npos;
            }
        } else if (block == Block::Coupling) {
            if (line.find("heat:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                out.program.heat_coupling = parse_heat(v);
            }
        } else if (block == Block::CrossedProduct) {
            if (line.find("coupling:") != std::string::npos)
                out.program.crossed_product.coupling =
                    trim(line.substr(line.find(':') + 1));
            if (line.find("lambda:") != std::string::npos)
                out.program.crossed_product.lambda =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("kmax:") != std::string::npos)
                out.program.crossed_product.kmax =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("prime_ladder:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                std::stringstream ss(v);
                std::string tok;
                while (std::getline(ss, tok, ',')) {
                    tok = trim(tok);
                    if (!tok.empty()) out.program.crossed_product.prime_ladder.push_back(std::stoi(tok));
                }
            }
        } else if (block == Block::Completion) {
            if (line.find("method:") != std::string::npos)
                out.program.completion.method =
                    parse_completion_method(trim(line.substr(line.find(':') + 1)));
            if (line.find("constraint:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                out.program.completion.constraint = parse_completion_constraint(v);
                const auto tol_pos = v.find("tolerance");
                if (tol_pos != std::string::npos) {
                    auto rest = trim(v.substr(tol_pos + 9));
                    if (!rest.empty()) out.program.completion.tolerance = std::stod(rest);
                }
            }
            if (line.find("tolerance:") != std::string::npos)
                out.program.completion.tolerance =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("norm:") != std::string::npos)
                out.program.completion.norm =
                    parse_completion_norm(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::AdelicCauchy) {
            if (line.find("metric:") != std::string::npos)
                out.program.adelic_cauchy.metric =
                    parse_adelic_metric(trim(line.substr(line.find(':') + 1)));
            if (line.find("search:") != std::string::npos)
                out.program.adelic_cauchy.search = trim(line.substr(line.find(':') + 1));
            if (line.find("max_denominator:") != std::string::npos)
                out.program.adelic_cauchy.max_denominator =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("max_primes:") != std::string::npos)
                out.program.adelic_cauchy.max_primes =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("include_raw_ladder:") != std::string::npos)
                out.program.adelic_cauchy.include_raw_ladder =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("target_zeros:") != std::string::npos)
                out.program.adelic_cauchy.target_zeros =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::HeightMap) {
            if (line.find("type:") != std::string::npos)
                out.program.height_map.type =
                    parse_height_map_type(trim(line.substr(line.find(':') + 1)));
            if (line.find("formula:") != std::string::npos)
                out.program.height_map.formula = trim(line.substr(line.find(':') + 1));
            if (line.find("a:") != std::string::npos)
                out.program.height_map.a = std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("b:") != std::string::npos)
                out.program.height_map.b = std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("alpha:") != std::string::npos)
                out.program.height_map.alpha = std::stod(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::SpectralDeterminant) {
            if (line.find("target:") != std::string::npos)
                out.program.spectral_determinant.target =
                    parse_spectral_target(trim(line.substr(line.find(':') + 1)));
            if (line.find("method:") != std::string::npos)
                out.program.spectral_determinant.method =
                    parse_spectral_method(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::SpectralAction) {
            if (line.find("selection:") != std::string::npos)
                out.program.spectral_action.selection =
                    trim(line.substr(line.find(':') + 1));
            if (line.find("action_proxy:") != std::string::npos)
                out.program.spectral_action.action_proxy =
                    trim(line.substr(line.find(':') + 1));
            if (line.find("t1_tolerance:") != std::string::npos)
                out.program.spectral_action.t1_tolerance =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("weil_residual_max:") != std::string::npos)
                out.program.spectral_action.weil_residual_max =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("heat_scale_base:") != std::string::npos)
                out.program.spectral_action.heat_scale_base =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("heat_scales:") != std::string::npos)
                out.program.spectral_action.heat_scales =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("coupling_lambda:") != std::string::npos)
                out.program.spectral_action.coupling_lambda =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("mode_kmax:") != std::string::npos)
                out.program.spectral_action.mode_kmax =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("arch_action_weight:") != std::string::npos)
                out.program.spectral_action.arch_action_weight =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("gue_spacing_l2_max:") != std::string::npos)
                out.program.spectral_action.gue_spacing_l2_max =
                    std::stod(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::DiscretizationLimit) {
            if (line.find("kind:") != std::string::npos)
                out.program.discretization_limit.kind = trim(line.substr(line.find(':') + 1));
            if (line.find("caps:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                std::stringstream ss(v);
                std::string tok;
                while (std::getline(ss, tok, ',')) {
                    tok = trim(tok);
                    if (!tok.empty()) out.program.discretization_limit.caps.push_back(std::stoi(tok));
                }
            }
            if (line.find("metric:") != std::string::npos)
                out.program.discretization_limit.metric = trim(line.substr(line.find(':') + 1));
            if (line.find("limit_target:") != std::string::npos)
                out.program.discretization_limit.limit_target =
                    trim(line.substr(line.find(':') + 1));
            if (line.find("formal_status:") != std::string::npos)
                out.program.discretization_limit.formal_status =
                    trim(line.substr(line.find(':') + 1));
        } else if (block == Block::FormalTarget) {
            if (line.find("lemma:") != std::string::npos)
                out.program.formal_target.lemma = trim(line.substr(line.find(':') + 1));
            else if (line.rfind("lemma ", 0) == 0)
                out.program.formal_target.lemma = trim(line.substr(6));
            if (line.find("approach:") != std::string::npos)
                out.program.formal_target.approach = trim(line.substr(line.find(':') + 1));
            else if (line.rfind("approach ", 0) == 0)
                out.program.formal_target.approach = trim(line.substr(9));
            if (line.find("proof_status:") != std::string::npos)
                out.program.formal_target.proof_status = trim(line.substr(line.find(':') + 1));
        } else if (block == Block::BoundAudit) {
            auto parse_d = [&](const char* key, double& dst) {
                if (line.find(key) == std::string::npos) return;
                const size_t colon = line.find(':');
                const size_t eq = line.find('=');
                size_t start = std::string::npos;
                if (colon != std::string::npos)
                    start = colon + 1;
                else if (eq != std::string::npos)
                    start = eq + 1;
                else {
                    const std::string k(key);
                    if (line.rfind(k, 0) == 0) start = k.size();
                }
                if (start == std::string::npos) return;
                dst = std::stod(trim(line.substr(start)));
            };
            parse_d("grid_rel_gap_ub", out.program.bound_audit.grid_rel_gap_ub);
            parse_d("grid_mult_dev_ub", out.program.bound_audit.grid_mult_dev_ub);
            parse_d("tail_bound_decades_ub", out.program.bound_audit.tail_bound_decades_ub);
            parse_d("ident_gap_decades_ub", out.program.bound_audit.ident_gap_decades_ub);
            parse_d("holomorphy_uniform_gap_ub", out.program.bound_audit.holomorphy_uniform_gap_ub);
            parse_d("log_partial_sum_ub", out.program.bound_audit.log_partial_sum_ub);
            parse_d("log_majorant_c", out.program.bound_audit.log_majorant_c);
            parse_d("l_function_grid_rel_gap_ub", out.program.bound_audit.l_function_grid_rel_gap_ub);
            parse_d("sha_resolvent_gap_ub", out.program.bound_audit.sha_resolvent_gap_ub);
            parse_d("kernel_tolerance", out.program.bound_audit.kernel_tolerance);
            parse_d("hodge_h11_target", out.program.bound_audit.hodge_h11_target);
            parse_d("major_arc_threshold", out.program.bound_audit.major_arc_threshold);
            parse_d("minor_arc_ub", out.program.bound_audit.minor_arc_ub);
            parse_d("goldbach_n0", out.program.bound_audit.goldbach_n0);
        } else if (block == Block::GenusOneLogBounds) {
            auto parse_d = [&](const char* key, double& dst) {
                if (line.find(key) == std::string::npos) return;
                const size_t colon = line.find(':');
                const size_t eq = line.find('=');
                size_t start = std::string::npos;
                if (colon != std::string::npos)
                    start = colon + 1;
                else if (eq != std::string::npos)
                    start = eq + 1;
                else {
                    const std::string k(key);
                    if (line.rfind(k, 0) == 0) start = k.size();
                }
                if (start == std::string::npos) return;
                dst = std::stod(trim(line.substr(start)));
            };
            parse_d("small_z_threshold", out.program.genus_one_log_bounds.small_z_threshold);
            parse_d("head_majorant_margin", out.program.genus_one_log_bounds.head_majorant_margin);
            if (line.find("lemma:") != std::string::npos)
                out.program.genus_one_log_bounds.lemma =
                    trim(line.substr(line.find(':') + 1));
            else if (line.rfind("lemma ", 0) == 0)
                out.program.genus_one_log_bounds.lemma = trim(line.substr(6));
        } else if (block == Block::AssemblySearch) {
            if (line.find("quick_zeros:") != std::string::npos)
                out.program.assembly_search.quick_zeros =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("quick_primes:") != std::string::npos)
                out.program.assembly_search.quick_primes =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("full_zeros:") != std::string::npos)
                out.program.assembly_search.full_zeros =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("top_k:") != std::string::npos)
                out.program.assembly_search.top_k = std::stoi(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::Investigation) {
            if (line.find("id:") != std::string::npos)
                out.program.investigation.id = trim(line.substr(line.find(':') + 1));
            if (line.find("cert_root:") != std::string::npos)
                out.program.investigation.cert_root = trim(line.substr(line.find(':') + 1));
            if (line.find("quick:") != std::string::npos)
                out.program.investigation.quick = parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("fixed_theta:") != std::string::npos)
                out.program.investigation.fixed_theta =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("fixed_boundary:") != std::string::npos)
                out.program.investigation.fixed_boundary = trim(line.substr(line.find(':') + 1));
            if (line.find("t1_tolerance:") != std::string::npos)
                out.program.investigation.t1_tolerance =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("heat_scale_base:") != std::string::npos)
                out.program.investigation.heat_scale_base =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("heat_scales:") != std::string::npos)
                out.program.investigation.heat_scales =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("coupling_lambda:") != std::string::npos)
                out.program.investigation.coupling_lambda =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("combined_cap:") != std::string::npos)
                out.program.investigation.combined_cap =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("arch_cap:") != std::string::npos)
                out.program.investigation.arch_cap =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line.find("mode_kmax:") != std::string::npos)
                out.program.investigation.mode_kmax =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::InvestigationThetaCentered) {
            if (line.find("center:") != std::string::npos)
                out.program.investigation.curvature_center =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("range:") != std::string::npos)
                out.program.investigation.curvature_range =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("steps:") != std::string::npos)
                out.program.investigation.curvature_steps =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line == "}") block = Block::Investigation;
        } else if (block == Block::InvestigationThetaFull) {
            if (line.find("min:") != std::string::npos)
                out.program.investigation.topology_min =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("max:") != std::string::npos)
                out.program.investigation.topology_max =
                    std::stod(trim(line.substr(line.find(':') + 1)));
            if (line.find("steps:") != std::string::npos)
                out.program.investigation.topology_steps =
                    std::stoi(trim(line.substr(line.find(':') + 1)));
            if (line == "}") block = Block::Investigation;
        } else if (block == Block::InvestigationLadderHeatT) {
            if (line.find("values:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                std::stringstream ss(v);
                std::string tok;
                out.program.investigation.heat_t_values.clear();
                while (std::getline(ss, tok, ',')) {
                    tok = trim(tok);
                    if (!tok.empty()) out.program.investigation.heat_t_values.push_back(std::stod(tok));
                }
            }
            if (line == "}") block = Block::Investigation;
        } else if (block == Block::InvestigationLadderPrimeLimit) {
            if (line.find("values:") != std::string::npos) {
                const auto v = trim(line.substr(line.find(':') + 1));
                std::stringstream ss(v);
                std::string tok;
                out.program.investigation.prime_limit_values.clear();
                while (std::getline(ss, tok, ',')) {
                    tok = trim(tok);
                    if (!tok.empty())
                        out.program.investigation.prime_limit_values.push_back(std::stoi(tok));
                }
            }
            if (line == "}") block = Block::Investigation;
        } else if (block == Block::Archimedean) {
            if (line.find("type:") != std::string::npos)
                out.program.archimedean.type =
                    parse_arch_type(trim(line.substr(line.find(':') + 1)));
            if (line.find("boundary:") != std::string::npos)
                out.program.archimedean.boundary =
                    parse_arch_boundary(trim(line.substr(line.find(':') + 1)));
            if (line.find("cutoff:") != std::string::npos)
                out.program.archimedean.cutoff =
                    parse_arch_cutoff(trim(line.substr(line.find(':') + 1)));
            if (line.find("lambda:") != std::string::npos)
                out.program.archimedean.lambda_cutoff =
                    std::stod(trim(line.substr(line.find(':') + 1)));
        } else if (block == Block::Diagnostics) {
            if (line.find("falsify compact_sinc2") != std::string::npos)
                out.program.falsify_sinc2 = true;
            if (line.find("trace_lhs quotient_spectrum") != std::string::npos)
                out.program.trace_lhs_quotient = true;
            if (line.find("pair_correlation gue") != std::string::npos)
                out.program.pair_correlation_gue = true;
            if (line.find("formal analytics") != std::string::npos)
                out.program.formal_analytics = true;
            if (line.find("wedge_analytics") != std::string::npos ||
                line.find("wedge analytics") != std::string::npos)
                out.program.wedge_analytics = true;
            if (line.find("xi_hadamard_proof") != std::string::npos ||
                line.find("xi hadamard proof") != std::string::npos)
                out.program.xi_hadamard_proof = true;
            if (line.find("wedge_analytics:") != std::string::npos)
                out.program.diagnostics.wedge_analytics =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("xi_hadamard_proof:") != std::string::npos)
                out.program.diagnostics.xi_hadamard_proof =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("local_weil_t1:") != std::string::npos)
                out.program.diagnostics.local_weil_t1 =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("connes_crossed:") != std::string::npos)
                out.program.diagnostics.connes_crossed =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("archimedean_sweep:") != std::string::npos)
                out.program.diagnostics.archimedean_sweep =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("assembly_grid:") != std::string::npos)
                out.program.diagnostics.assembly_grid =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("weil_identity:") != std::string::npos)
                out.program.diagnostics.weil_identity =
                    trim(line.substr(line.find(':') + 1));
            if (line.find("trace_test:") != std::string::npos)
                out.program.diagnostics.trace_test = trim(line.substr(line.find(':') + 1));
            if (line.find("test_param:") != std::string::npos)
                out.program.diagnostics.test_param =
                    std::stold(trim(line.substr(line.find(':') + 1)));
            if (line.find("sinc2_kappa:") != std::string::npos)
                out.program.diagnostics.sinc2_kappa =
                    std::stold(trim(line.substr(line.find(':') + 1)));
            if (line.find("weil_residual") != std::string::npos && line.find('<') != std::string::npos) {
                const size_t lt = line.find('<');
                out.program.diagnostics.weil_residual_max =
                    std::stold(trim(line.substr(lt + 1)));
            }
            if (line.find("spectrum_rmse") != std::string::npos && line.find('<') != std::string::npos) {
                const size_t lt = line.find('<');
                out.program.diagnostics.spectrum_rmse_max =
                    std::stold(trim(line.substr(lt + 1)));
            }
            if (line.find("spectrum_max_gap") != std::string::npos && line.find('<') != std::string::npos) {
                const size_t lt = line.find('<');
                out.program.diagnostics.spectrum_max_gap_max =
                    std::stold(trim(line.substr(lt + 1)));
            }
            if (line.find("sinc2_spectral_gap") != std::string::npos && line.find('<') != std::string::npos) {
                const size_t lt = line.find('<');
                out.program.diagnostics.sinc2_spectral_gap_max =
                    std::stold(trim(line.substr(lt + 1)));
            }
            if (line.find("spectrum_identified") != std::string::npos &&
                line.find("==") == std::string::npos)
                out.program.diagnostics.expect_spectrum_identified =
                    line.find(':') != std::string::npos
                        ? parse_bool(trim(line.substr(line.find(':') + 1)))
                        : true;
            if (line.find("spectrum_identified") != std::string::npos && line.find("==") != std::string::npos)
                out.program.diagnostics.expect_spectrum_identified = true;
            if (line.find("expect_finite_spectrum_mismatch") != std::string::npos)
                out.program.diagnostics.expect_finite_spectrum_mismatch =
                    parse_bool(trim(line.substr(line.find(':') + 1)));
            if (line.find("self_adjoint_extension_sweep:") != std::string::npos)
                out.program.diagnostics.self_adjoint_extension_sweep =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("trace_formula_gate:") != std::string::npos)
                out.program.diagnostics.trace_formula_gate =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("weil_full") != std::string::npos && line.find("trace") != std::string::npos)
                out.program.diagnostics.trace_formula_gate = true;
            if (line.find("spectral_discreteness:") != std::string::npos)
                out.program.diagnostics.spectral_discreteness =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("spectrum_vs_zeros:") != std::string::npos)
                out.program.diagnostics.spectrum_vs_zeros =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("local_weil_t1:") != std::string::npos)
                out.program.diagnostics.local_weil_t1 =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("spectral_action_selection:") != std::string::npos)
                out.program.diagnostics.spectral_action_selection =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("global_dirac_limit:") != std::string::npos)
                out.program.diagnostics.global_dirac_limit =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("analytic_lemma_demo:") != std::string::npos)
                out.program.diagnostics.analytic_lemma_demo =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            if (line.find("investigation_suite:") != std::string::npos) {
                out.program.diagnostics.investigation_suite =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
                out.program.investigation.present = out.program.diagnostics.investigation_suite;
            }
            if (line.find("hurwitz_spectral_action:") != std::string::npos) {
                out.program.diagnostics.hurwitz_spectral_action =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            }
            if (line.find("theorem_b_scaffold:") != std::string::npos) {
                out.program.diagnostics.theorem_b_scaffold =
                    parse_diag_flag(trim(line.substr(line.find(':') + 1)));
            }
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

    if (out.program.trace_formula.present &&
        out.program.trace_formula.status != TraceFormulaStatus::Proved &&
        out.program.sym_tier != SymTier::Scaffold) {
        add_error(out.errors, "E0700", 0,
                  "trace_formula block requires status: proved on production ansätze",
                  "set status: proved or sym_tier: scaffold");
    }
    if (out.program.self_adjoint_extension.present && out.program.sym_tier != SymTier::Scaffold &&
        sym.rule_id != "berry_keating_xp" && sym.rule_id != "connes_analytic_construction" &&
        sym.rule_id != "connes_analytic_lemmas") {
        add_error(out.errors, "E0701", 0,
                  "self_adjoint_extension on production ansatz without analytic backend",
                  "use berry_keating or connes_analytic_construction rule");
    }
    if (out.program.heat_coupling == HeatCoupling::BerryKeating) {
        bool phase = false;
        for (const auto& b : out.program.on_blocks) {
            if (b.space.find("phase_space") != std::string::npos) phase = true;
        }
        if (!phase && !out.program.local.hamiltonian_expr.empty()) phase = true;
        if (!phase && out.program.id.find("berry") != std::string::npos) phase = true;
        if (!phase) {
            add_error(out.errors, "E0702", 0,
                      "berry_keating coupling requires phase_space or BK local Hamiltonian",
                      "set space phase_space or local { H = ... }");
        }
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

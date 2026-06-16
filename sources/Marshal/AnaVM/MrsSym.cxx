#include "MrsSym.hxx"

#include <cctype>

namespace Marshal::AnaVM {
namespace {

std::string normalize_ws(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s = s.substr(i);
    std::string out;
    out.reserve(s.size());
    for (size_t j = 0; j < s.size(); ++j) {
        if (j + 1 < s.size() && static_cast<unsigned char>(s[j]) == 0xCE &&
            static_cast<unsigned char>(s[j + 1]) == 0xB0) {
            out += "pi";
            j += 1;
            continue;
        }
        out.push_back(s[j]);
    }
    for (size_t p = 0; (p = out.find(' ', p)) != std::string::npos;) out.erase(p, 1);
    return out;
}

bool has_adele_space(const MrsProgram& prog) {
    for (const auto& b : prog.on_blocks) {
        if (b.space.find("adele_class") != std::string::npos) return true;
    }
    return prog.id.find("connes") != std::string::npos && prog.local.spectral_triple;
}

bool has_phase_space(const MrsProgram& prog) {
    for (const auto& b : prog.on_blocks) {
        if (b.space.find("phase_space") != std::string::npos) return true;
    }
    return !prog.local.hamiltonian_expr.empty() || prog.id.find("berry") != std::string::npos;
}

bool is_connes_analytic(const MrsProgram& prog) {
    if (!prog.local.spectral_triple) return false;
    const bool global = prog.quotient.present || has_adele_space(prog);
    const bool analytic = prog.trace_formula.present || prog.self_adjoint_extension.present ||
                          prog.spectral_determinant.present || prog.spectral_action.present;
    return global && analytic;
}

bool is_bk_analytic(const MrsProgram& prog) {
    return has_phase_space(prog) &&
           (prog.semiclassical.present || prog.self_adjoint_extension.present);
}

bool is_legacy_placeholder_space(const MrsProgram& prog) {
    if (is_connes_analytic(prog) || is_bk_analytic(prog)) return false;
    for (const auto& b : prog.on_blocks) {
        const auto& s = b.space;
        if (s.find("phase_space") != std::string::npos ||
            (s.find("adele_class") != std::string::npos && s.find("adelic_quotient") == std::string::npos))
            return true;
    }
    return false;
}

}  // namespace

SymDeriveResult derive_spectrum(const MrsProgram& prog) {
    SymDeriveResult r;

    if (prog.discretization_limit.present || prog.diagnostics.global_dirac_limit) {
        r.ok = true;
        r.placeholder = false;
        r.omega_derived = "D_global_limit";
        r.lambda_derived = "adelic_quotient_limit";
        r.rule_id = "connes_global_dirac_limit";
        return r;
    }

    if (prog.investigation.present || prog.diagnostics.investigation_suite) {
        r.ok = true;
        r.placeholder = false;
        r.omega_derived = "D_spectral";
        r.lambda_derived = "investigation";
        r.rule_id = prog.investigation.id.empty() ? "theorem_ab" : prog.investigation.id;
        return r;
    }

    if (prog.diagnostics.hurwitz_spectral_action || prog.id == "theorem_a_analytic") {
        r.ok = true;
        r.placeholder = false;
        r.omega_derived = "D_arch_scaling";
        r.lambda_derived = "hurwitz_zeta";
        r.rule_id = "theorem_a_analytic";
        return r;
    }

    if (prog.diagnostics.theorem_b_scaffold || prog.id == "theorem_b") {
        r.ok = true;
        r.placeholder = false;
        r.omega_derived = "D_theta0";
        r.lambda_derived = "crossed_product";
        r.rule_id = "theorem_b";
        return r;
    }

    if (prog.diagnostics.analytic_lemma_demo || prog.id == "connes_analytic_lemmas") {
        r.ok = true;
        r.placeholder = false;
        r.omega_derived = "D_spectral";
        r.lambda_derived = "spectral_action";
        r.rule_id = "connes_analytic_lemmas";
        return r;
    }

    if (is_connes_analytic(prog)) {
        r.ok = true;
        r.placeholder = false;
        r.rule_id = "connes_analytic_construction";
        r.omega_derived = "D_spectral";
        r.lambda_derived = "D^2";
        return r;
    }

    if (is_bk_analytic(prog)) {
        r.ok = true;
        r.placeholder = false;
        r.rule_id = "berry_keating_xp";
        r.omega_derived = "gamma_n_wkb";
        r.lambda_derived = "H_BK";
        return r;
    }

    if (is_legacy_placeholder_space(prog)) {
        r.placeholder = true;
        r.ok = true;
        if (prog.id.find("connes") != std::string::npos ||
            (prog.on_blocks.size() && prog.on_blocks[0].space.find("adele") != std::string::npos)) {
            r.rule_id = "connes_dirac";
            r.omega_derived = "D_spectral";
            r.lambda_derived = "D^2";
        } else {
            r.rule_id = "berry_keating_xp";
            r.omega_derived = "x*p_classical";
            r.lambda_derived = "H_BK";
        }
        return r;
    }

    std::string omega = "2*pi*n/log(p)";
    if (prog.rescale.has_derivative_scale) {
        const auto& f = prog.rescale.derivative_factor;
        if (f == "log(p)" || f == "logp") omega = "2*pi*n";
        else omega = "2*pi*n/log(p)*(" + f + ")";
    }
    if (!prog.on_blocks.empty() && !prog.on_blocks[0].omega_expr.empty())
        omega = normalize_ws(prog.on_blocks[0].omega_expr);

    if (prog.completion.present || prog.adelic_cauchy.present || prog.archimedean.present) {
        r.ok = true;
        r.omega_derived = omega.empty() ? "k*log(p)" : omega;
        r.lambda_derived = "adelic_cauchy_completion";
        if (prog.archimedean.present && !prog.completion.present && !prog.adelic_cauchy.present)
            r.rule_id = "archimedean_boundary_sweep";
        else
            r.rule_id = "adelic_cauchy_completion";
        return r;
    }

    if (prog.heat_coupling == HeatCoupling::ConnesHeat ||
        prog.diagnostics.connes_crossed || !prog.crossed_product.coupling.empty()) {
        r.ok = true;
        r.omega_derived = omega.empty() ? "k*log(p)" : omega;
        r.lambda_derived = "k_log_p_coupled";
        r.rule_id = "connes_crossed_logp";
        return r;
    }

    r.ok = true;
    r.omega_derived = omega;
    r.lambda_derived = "omega^2";
    r.rule_id = "circle_logp_poisson";
    return r;
}

bool check_weil_coupling(const MrsProgram& prog, std::vector<MrsError>& errors) {
    if (prog.investigation.present || prog.diagnostics.investigation_suite) return true;
    if (prog.heat_coupling == HeatCoupling::None) return true;

    if (prog.completion.present || prog.adelic_cauchy.present || prog.archimedean.present)
        return true;

    if (prog.heat_coupling == HeatCoupling::ConnesHeat || prog.diagnostics.connes_crossed)
        return true;

    if (prog.heat_coupling == HeatCoupling::BerryKeating) {
        if (is_bk_analytic(prog)) return true;
        MrsError e;
        e.code = "E0602";
        e.message = "berry_keating coupling requires semiclassical or self_adjoint_extension block";
        e.hint = "add semiclassical { ladder: wkb } or self_adjoint_extension { ... }";
        errors.push_back(std::move(e));
        return false;
    }

    const SymDeriveResult sym = derive_spectrum(prog);
    if (sym.placeholder) {
        MrsError e;
        e.code = "E0602";
        e.message = "placeholder ansatz cannot use coupling heat: poisson";
        e.hint = "set coupling { heat: none } for scaffold programs";
        errors.push_back(std::move(e));
        return false;
    }

    if (sym.rule_id == "connes_analytic_construction" || sym.rule_id == "berry_keating_xp")
        return true;

    const std::string required = "2*pi*n/log(p)";
    if (sym.omega_derived != required) {
        MrsError e;
        e.code = "E0600";
        e.message = "weil coupling poisson incompatible with derived spectrum";
        e.hint = "derived omega=" + sym.omega_derived + " required omega=" + required;
        errors.push_back(std::move(e));
        return false;
    }
    return true;
}

}  // namespace Marshal::AnaVM

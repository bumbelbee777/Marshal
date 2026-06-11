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

bool is_placeholder_space(const MrsProgram& prog) {
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
    if (is_placeholder_space(prog)) {
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

    r.ok = true;
    r.omega_derived = omega;
    r.lambda_derived = "omega^2";
    r.rule_id = "circle_logp_poisson";
    return r;
}

bool check_weil_coupling(const MrsProgram& prog, std::vector<MrsError>& errors) {
    if (prog.heat_coupling == HeatCoupling::None) return true;

    if (prog.heat_coupling == HeatCoupling::BerryKeating ||
        prog.heat_coupling == HeatCoupling::ConnesHeat) {
        MrsError e;
        e.code = "E0602";
        e.message = "coupling not implemented (placeholder rule)";
        e.hint = "use coupling { heat: none } and meta { sym_tier: scaffold }";
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

#include "OperatorTraits.hxx"
#include "MrsSym.hxx"

namespace Marshal::AnaVM {
namespace {

bool has_rescale_logp(const MrsProgram& prog) {
    return prog.rescale.has_derivative_scale &&
           (prog.rescale.derivative_factor == "log(p)" ||
            prog.rescale.derivative_factor == "logp");
}

bool is_connes_global(const MrsProgram& prog) {
    return prog.rule_id == "connes_analytic_construction" || prog.rule_id == "connes_dirac";
}

bool infer_in_C_fin(const MrsProgram& prog, const OperatorTraits& t) {
    if (is_connes_global(prog)) return false;
    if (t.merge == SpectrumMerge::DirectSum) return true;
    if (t.merge == SpectrumMerge::GammaTunedQuotient) return true;
    if (t.merge == SpectrumMerge::FixedLowMode) return true;
    if (t.merge == SpectrumMerge::SUnitMatrix) return true;
    if (t.merge == SpectrumMerge::FrequencyLocked) return true;
    if (prog.rule_id == "connes_crossed_logp" || prog.diagnostics.connes_crossed) return true;
    if (prog.completion.present || prog.adelic_cauchy.present) return true;
    if (prog.semiclassical.present && prog.semiclassical.height_renormalize_log_n) return true;
    if (prog.rule_id == "berry_keating_xp") return true;
    if (t.merge == SpectrumMerge::Semiclassical && !is_connes_global(prog)) return true;
    return false;
}

DensityGrowthClass infer_density_growth(const MrsProgram& prog, const OperatorTraits& t) {
    if (is_connes_global(prog)) return DensityGrowthClass::EmergentFromQuotient;
    if (prog.semiclassical.present && prog.semiclassical.height_renormalize_log_n)
        return DensityGrowthClass::InverseLogarithmic;
    if (prog.height_map.present) return DensityGrowthClass::InverseLogarithmic;
    if (t.merge == SpectrumMerge::DirectSum || t.merge == SpectrumMerge::SUnitMatrix ||
        t.merge == SpectrumMerge::FixedLowMode)
        return DensityGrowthClass::Constant;
    if (prog.rule_id == "connes_crossed_logp" || prog.diagnostics.connes_crossed)
        return DensityGrowthClass::Constant;
    if (t.merge == SpectrumMerge::Semiclassical && prog.rule_id == "berry_keating_xp")
        return DensityGrowthClass::InverseLogarithmic;
    return DensityGrowthClass::Unknown;
}

void fill_connes_subtargets(const MrsProgram& prog, OperatorTraits& t) {
    if (!is_connes_global(prog)) return;
    const bool t1_gate = prog.trace_formula.present || prog.diagnostics.trace_formula_gate ||
                         prog.diagnostics.local_weil_t1;
    const bool bk_height =
        prog.height_map.present ||
        (prog.semiclassical.present && prog.semiclassical.height_renormalize_log_n);
    t.connes_subtarget_status = {
        "connes_algebra_type:SCAFFOLD",
        t1_gate ? "connes_trace_formula:PASS" : "connes_trace_formula:OPEN",
        "connes_kms_phase:OPEN",
        "connes_spectral_action:THE_GAP",
        bk_height ? "connes_bk_height_map:FALSIFIED" : "connes_bk_height_map:EXCLUDED",
        bk_height ? "connes_self_adjoint_extension:OPEN_VIA_HEIGHT_MAP"
                  : "connes_self_adjoint_extension:OPEN_VIA_SPECTRAL_ACTION",
        "connes_critical_line:CIRCULAR_EXCLUDED",
    };
}

}  // namespace

const char* space_kind_string(SpaceKind k) {
    switch (k) {
        case SpaceKind::CircleLogp: return "circle_logp";
        case SpaceKind::AdelicTruncated: return "adelic_truncated";
        case SpaceKind::PhaseSpace: return "phase_space";
        default: return "unknown";
    }
}

const char* spectrum_merge_string(SpectrumMerge m) {
    switch (m) {
        case SpectrumMerge::DirectSum: return "direct_sum";
        case SpectrumMerge::FrequencyLocked: return "frequency_locked";
        case SpectrumMerge::GammaTunedQuotient: return "gamma_tuned_quotient";
        case SpectrumMerge::SUnitMatrix: return "s_unit_matrix";
        case SpectrumMerge::FixedLowMode: return "fixed_low_mode";
        case SpectrumMerge::Semiclassical: return "semiclassical";
        default: return "unknown";
    }
}

const char* spectrum_scale_string(SpectrumScale s) {
    switch (s) {
        case SpectrumScale::Omega2PiNOverLogp: return "omega_2pi_n_over_logp";
        case SpectrumScale::Omega2PiN: return "omega_2pi_n";
        case SpectrumScale::DSpectral: return "D_spectral";
        default: return "unknown";
    }
}

const char* density_growth_string(DensityGrowthClass d) {
    switch (d) {
        case DensityGrowthClass::Constant: return "constant";
        case DensityGrowthClass::InverseLogarithmic: return "inverse_logarithmic";
        case DensityGrowthClass::Logarithmic: return "logarithmic";
        case DensityGrowthClass::EmergentFromQuotient: return "emergent_from_quotient";
        default: return "unknown";
    }
}

static bool is_connes_global_rule(const std::string& rule_id) {
    return rule_id == "connes_analytic_construction" || rule_id == "connes_dirac";
}

static void score_requirements(OperatorTraits& t) {
    static const char* kReq[] = {
        "self_adjoint",
        "not_in_C_fin",
        "density_growth_logarithmic",
        "discrete_spike_measure",
        "paley_wiener_discriminating",
        "gamma_free_identification",
        "gue_level_repulsion",
        "trace_duality",
        "not_independent_cylinder_tensor",
        "noncommutative_algebra",
        "spectral_discreteness",
        nullptr,
    };
    for (const char** p = kReq; *p; ++p) {
        const std::string id = *p;
        bool ok = true;
        if (id == "not_in_C_fin" && t.in_C_fin) ok = false;
        if (id == "density_growth_logarithmic") {
            if (t.density_growth == DensityGrowthClass::Constant ||
                t.density_growth == DensityGrowthClass::InverseLogarithmic)
                ok = false;
            else if (t.density_growth == DensityGrowthClass::EmergentFromQuotient) {
                t.missing_requirements.push_back(id);
                continue;
            }
        }
        if (id == "gamma_free_identification" && t.uses_gamma) ok = false;
        if (id == "not_independent_cylinder_tensor" &&
            t.merge == SpectrumMerge::DirectSum && t.space == SpaceKind::CircleLogp &&
            t.scale == SpectrumScale::Omega2PiNOverLogp && !t.placeholder)
            ok = false;
        if (id == "paley_wiener_discriminating" && t.scaffold) {
            t.missing_requirements.push_back(id);
            continue;
        }
        if (id == "discrete_spike_measure" && t.merge == SpectrumMerge::DirectSum &&
            t.space == SpaceKind::CircleLogp && !t.placeholder)
            ok = false;
        if (id == "gue_level_repulsion" && t.in_C_fin && !t.placeholder) ok = false;
        if (id == "trace_duality") {
            if (t.rule_id == "connes_analytic_construction" || t.rule_id == "connes_dirac")
                ok = true;
            else {
                t.missing_requirements.push_back(id);
                continue;
            }
        }
        if (id == "noncommutative_algebra") {
            if (is_connes_global_rule(t.rule_id)) ok = true;
            else if (t.in_C_fin) ok = false;
            else {
                t.missing_requirements.push_back(id);
                continue;
            }
        }
        if (id == "spectral_discreteness") {
            t.missing_requirements.push_back(id);
            continue;
        }
        if (ok)
            t.satisfied_requirements.push_back(id);
        else
            t.violated_requirements.push_back(id);
    }
    if (t.space == SpaceKind::AdelicTruncated && !t.in_C_fin)
        t.satisfied_requirements.push_back("adelic_or_global_quotient");
    if (t.space == SpaceKind::PhaseSpace || t.coupling == HeatCoupling::BerryKeating)
        t.satisfied_requirements.push_back("classical_periods_log_p");
}

OperatorTraits infer_traits(const MrsProgram& prog) {
    OperatorTraits t;
    t.ansatz_id = prog.id;
    t.rule_id = prog.rule_id;
    t.uses_gamma = prog.uses_gamma;
    t.placeholder = prog.placeholder;
    t.scaffold = prog.sym_tier == SymTier::Scaffold;
    t.coupling = prog.heat_coupling;
    t.falsify_sinc2 = prog.falsify_sinc2;
    t.trace_lhs_quotient = prog.trace_lhs_quotient;

    for (const auto& b : prog.on_blocks) {
        if (b.space.find("phase_space") != std::string::npos)
            t.space = SpaceKind::PhaseSpace;
        else if (b.space.find("adelic") != std::string::npos)
            t.space = SpaceKind::AdelicTruncated;
        else if (b.space.find("circle") != std::string::npos || b.space.empty())
            t.space = SpaceKind::CircleLogp;
    }
    if (prog.placeholder) {
        if (prog.rule_id == "connes_dirac") t.space = SpaceKind::AdelicTruncated;
        if (prog.rule_id == "berry_keating_xp") t.space = SpaceKind::PhaseSpace;
    }

    if (prog.id.find("quotient_gamma") != std::string::npos || prog.trace_lhs_quotient)
        t.merge = SpectrumMerge::GammaTunedQuotient;
    else if (prog.id.find("sunit") != std::string::npos || prog.id.find("fixed") != std::string::npos)
        t.merge = SpectrumMerge::FixedLowMode;
    else if (has_rescale_logp(prog))
        t.merge = SpectrumMerge::DirectSum;
    else if (prog.rule_id == "connes_crossed_logp" || prog.diagnostics.connes_crossed)
        t.merge = SpectrumMerge::Semiclassical;
    else if (is_connes_global(prog))
        t.merge = SpectrumMerge::Semiclassical;
    else if (t.space == SpaceKind::CircleLogp)
        t.merge = SpectrumMerge::DirectSum;
    else if (t.placeholder)
        t.merge = SpectrumMerge::Semiclassical;

    if (has_rescale_logp(prog) || prog.derived_omega == "2*pi*n")
        t.scale = SpectrumScale::Omega2PiN;
    else if (is_connes_global(prog))
        t.scale = SpectrumScale::DSpectral;
    else if (prog.rule_id == "berry_keating_xp" && !prog.placeholder)
        t.scale = SpectrumScale::Omega2PiN;
    else if (!prog.placeholder)
        t.scale = SpectrumScale::Omega2PiNOverLogp;

    const SymDeriveResult sym = derive_spectrum(prog);
    t.weil_poisson_compatible = sym.placeholder || sym.omega_derived == "2*pi*n/log(p)";

    if (prog.rule_id == "berry_keating_xp" && !t.placeholder)
        t.numeric_backend = "berry_keating";
    else if (prog.rule_id == "connes_analytic_construction")
        t.numeric_backend = "analytic_construction";
    else if (t.scaffold || (t.placeholder && !is_connes_global(prog)))
        t.numeric_backend = "none";
    else if (prog.rule_id == "connes_crossed_logp" || prog.diagnostics.connes_crossed)
        t.numeric_backend = "connes_crossed";
    else if (t.merge == SpectrumMerge::GammaTunedQuotient || t.merge == SpectrumMerge::FixedLowMode)
        t.numeric_backend = "quotient_toy";
    else
        t.numeric_backend = "cylinder";

    t.in_C_fin = infer_in_C_fin(prog, t);
    t.density_growth = infer_density_growth(prog, t);
    fill_connes_subtargets(prog, t);

    if ((prog.rule_id == "berry_keating_xp" || prog.rule_id == "connes_analytic_construction") &&
        prog.self_adjoint_extension.present)
        t.satisfied_requirements.push_back("self_adjoint");

    score_requirements(t);
    return t;
}

}  // namespace Marshal::AnaVM

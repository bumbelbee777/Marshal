#include "OperatorTraits.hxx"
#include "MrsSym.hxx"

namespace Marshal::AnaVM {
namespace {

bool has_rescale_logp(const MrsProgram& prog) {
    return prog.rescale.has_derivative_scale &&
           (prog.rescale.derivative_factor == "log(p)" ||
            prog.rescale.derivative_factor == "logp");
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

static void score_requirements(OperatorTraits& t) {
    static const char* kReq[] = {
        "self_adjoint",
        "discrete_spike_measure",
        "paley_wiener_discriminating",
        "gamma_free_identification",
        "not_independent_cylinder_tensor",
        nullptr,
    };
    for (const char** p = kReq; *p; ++p) {
        const std::string id = *p;
        bool ok = true;
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
        if (ok)
            t.satisfied_requirements.push_back(id);
        else
            t.violated_requirements.push_back(id);
    }
    if (t.space == SpaceKind::AdelicTruncated ||
        t.merge == SpectrumMerge::FrequencyLocked || t.merge == SpectrumMerge::SUnitMatrix)
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

    if (prog.id.find("quotient_gamma") != std::string::npos ||
        prog.trace_lhs_quotient)
        t.merge = SpectrumMerge::GammaTunedQuotient;
    else if (prog.id.find("sunit") != std::string::npos ||
             prog.id.find("fixed") != std::string::npos)
        t.merge = SpectrumMerge::FixedLowMode;
    else if (has_rescale_logp(prog))
        t.merge = SpectrumMerge::DirectSum;
    else if (t.space == SpaceKind::CircleLogp)
        t.merge = SpectrumMerge::DirectSum;
    else if (t.placeholder)
        t.merge = SpectrumMerge::Semiclassical;

    if (has_rescale_logp(prog) || prog.derived_omega == "2*pi*n")
        t.scale = SpectrumScale::Omega2PiN;
    else if (prog.rule_id == "connes_dirac")
        t.scale = SpectrumScale::DSpectral;
    else if (!prog.placeholder)
        t.scale = SpectrumScale::Omega2PiNOverLogp;

    const SymDeriveResult sym = derive_spectrum(prog);
    t.weil_poisson_compatible =
        sym.placeholder || sym.omega_derived == "2*pi*n/log(p)";

    if (t.scaffold || t.placeholder)
        t.numeric_backend = "none";
    else if (t.merge == SpectrumMerge::GammaTunedQuotient ||
             t.merge == SpectrumMerge::FixedLowMode)
        t.numeric_backend = "quotient_toy";
    else
        t.numeric_backend = "cylinder";

    score_requirements(t);
    return t;
}

}  // namespace Marshal::AnaVM

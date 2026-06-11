#pragma once
#include "MrsTypes.hxx"
#include <string>
#include <vector>

namespace Marshal::AnaVM {

enum class SpaceKind { CircleLogp, AdelicTruncated, PhaseSpace, Unknown };
enum class SpectrumMerge {
    DirectSum,
    FrequencyLocked,
    GammaTunedQuotient,
    SUnitMatrix,
    FixedLowMode,
    Semiclassical,
    Unknown
};
enum class SpectrumScale { Omega2PiNOverLogp, Omega2PiN, DSpectral, Unknown };

struct OperatorTraits {
    std::string ansatz_id;
    std::string rule_id;
    SpaceKind space = SpaceKind::Unknown;
    SpectrumMerge merge = SpectrumMerge::Unknown;
    HeatCoupling coupling = HeatCoupling::Poisson;
    SpectrumScale scale = SpectrumScale::Unknown;
    bool uses_gamma = false;
    bool placeholder = false;
    bool scaffold = false;
    bool weil_poisson_compatible = true;
    bool falsify_sinc2 = false;
    bool trace_lhs_quotient = false;
    std::string numeric_backend;  // cylinder | quotient_toy | idele_laplacian | none
    std::vector<std::string> satisfied_requirements;
    std::vector<std::string> violated_requirements;
    std::vector<std::string> missing_requirements;
};

const char* space_kind_string(SpaceKind k);
const char* spectrum_merge_string(SpectrumMerge m);
const char* spectrum_scale_string(SpectrumScale s);

OperatorTraits infer_traits(const MrsProgram& prog);

}  // namespace Marshal::AnaVM

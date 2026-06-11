#pragma once
#include "Induction.hxx"

namespace Marshal::Induction {

constexpr Real kLhsMinValid = 1e-20L;
constexpr Real kProofMarginRel = 0.01L;

inline Real ComputeProofEps(Real arch_floor, Real analytic_tail_bound, Real float_floor) {
    const Real base = arch_floor + analytic_tail_bound + float_floor;
    const Real margin = std::max(1e-18L, base * kProofMarginRel);
    return base + margin;
}
constexpr Real kSpecMaxGapDefault = 1.0L;
constexpr Real kSpecMeanGapDefault = 0.35L;
constexpr int kQuotientKFixedDefault = 50;

const char* tier4_verdict_label(bool trace_proved, bool spectrum_identified,
    bool spectrum_approximated, bool lhs_underflow);

std::vector<Real> CollectCylinderSpectrum(const Heat::PrimeCatalog& cat, int N);

}  // namespace Marshal::Induction

#include "GLnLadderMetrics.hxx"

#include <algorithm>
#include <cmath>

namespace Marshal::Heat::GLn {

namespace {

constexpr Real kPi = 3.14159265358979323846L;

}  // namespace

int ladder_count_kernel(const std::vector<Real>& eig, Real tol) {
    int n = 0;
    for (Real v : eig)
        if (std::fabs(v) < tol) ++n;
    return n;
}

Real ladder_maass_grid_rel_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol) {
    if (arch_ladder.size() < 2 || theta <= 0) return 1e300L;
    Real max_rel = 0;
    int excited = 0;
    for (Real v : arch_ladder) {
        if (std::fabs(v) < tol) continue;
        ++excited;
        const Real t = static_cast<Real>(excited) * kPi / theta;
        const Real pred = std::sqrt(0.25L + t * t);
        const Real rel = std::fabs(v - pred) / std::max(pred, Real{1e-12});
        max_rel = std::max(max_rel, rel);
        if (excited >= 8) break;
    }
    return max_rel;
}

Real ladder_min_positive_arch(const std::vector<Real>& arch_ladder, Real tol) {
    Real best = 1e300L;
    for (Real v : arch_ladder)
        if (std::fabs(v) >= tol) best = std::min(best, std::fabs(v));
    return best < 1e200L ? best : 1e300L;
}

LadderArcSplit ladder_arc_split(const std::vector<Real>& eigenvalues, Real theta,
                               Real major_arc_lambda_ub) {
    LadderArcSplit out;
    if (eigenvalues.empty() || theta <= 0) return out;
    Real total = 0;
    Real major = 0;
    for (Real v : eigenvalues) {
        const Real m = std::exp(-std::max(std::fabs(v), Real{1e-12}) / theta);
        total += m;
        if (std::fabs(v) < major_arc_lambda_ub) major += m;
    }
    if (total <= 0) return out;
    out.major_arc_mass = major / total;
    out.minor_arc_bound = 1.0L - out.major_arc_mass;
    return out;
}

Real ladder_maass_major_arc_mass(const std::vector<Real>& arch_ladder, Real theta, Real tol,
                                 int major_maass_levels) {
    if (arch_ladder.empty() || theta <= 0 || major_maass_levels < 1) return 0;
    Real major = 0;
    Real total = 0;
    int excited = 0;
    for (Real v : arch_ladder) {
        const Real m = std::exp(-std::max(std::fabs(v), Real{1e-12}) / theta);
        total += m;
        if (std::fabs(v) < tol) {
            major += m;
            continue;
        }
        if (excited < major_maass_levels) {
            major += m;
            ++excited;
        }
    }
    return total > 0 ? major / total : 0;
}

Real ladder_maass_minor_arc_tail_bound(Real theta, int major_maass_levels) {
    if (theta <= 0 || major_maass_levels < 1) return 1e300L;
    return std::exp(-Real{2} * kPi * static_cast<Real>(major_maass_levels) / theta);
}

}  // namespace Marshal::Heat::GLn

#include "GLnLadderMetrics.hxx"

#include <algorithm>
#include <cmath>
#include <complex>

namespace Marshal::Heat::GLn {

namespace {
using C = std::complex<double>;

}  // namespace

namespace {

C partial_genus_one_det_impl(double s_re, double s_im, const std::vector<Real>& gammas) {
    C acc(0.5, 0);
    for (Real g : gammas) {
        const C s(s_re, s_im);
        const C denom(0.5, static_cast<double>(g));
        const C z = s / denom;
        const C one(1, 0);
        acc *= (one - z) * std::exp(z);
    }
    return acc;
}

Real relative_complex_gap(const C& a, const C& b) {
    const Real mb = std::max(static_cast<Real>(std::abs(b)), Real{1e-300});
    return static_cast<Real>(std::abs(a - b)) / mb;
}

Real gap_decades_ld(const C& a, const C& b) {
    const Real ma = std::max(static_cast<Real>(std::abs(a)), Real{1e-300});
    const Real mb = std::max(static_cast<Real>(std::abs(b)), Real{1e-300});
    return std::abs(std::log10(ma) - std::log10(mb));
}

}  // namespace

int ladder_count_kernel(const std::vector<Real>& eig, Real tol) {
    int n = 0;
    for (Real v : eig)
        if (std::fabs(v) < tol) ++n;
    return n;
}

std::vector<Real> ladder_maass_predicted_gammas(Real theta, int max_levels) {
    constexpr Real kPi = 3.14159265358979323846L;
    std::vector<Real> out;
    for (int j = 1; j <= max_levels; ++j) {
        const Real t = static_cast<Real>(j) * kPi / theta;
        out.push_back(std::sqrt(0.25L + t * t));
    }
    return out;
}

std::vector<Real> ladder_excited_arch_gammas(const std::vector<Real>& arch_ladder, Real tol,
                                             int max_levels) {
    std::vector<Real> out;
    for (Real v : arch_ladder) {
        if (std::fabs(v) < tol) continue;
        out.push_back(std::fabs(v));
        if (static_cast<int>(out.size()) >= max_levels) break;
    }
    return out;
}

void ladder_partial_genus_one_det(Real s_re, Real s_im, const std::vector<Real>& gammas,
                                  Real* out_re, Real* out_im) {
    const std::complex<double> det =
        partial_genus_one_det_impl(static_cast<double>(s_re), static_cast<double>(s_im), gammas);
    if (out_re) *out_re = static_cast<Real>(det.real());
    if (out_im) *out_im = static_cast<Real>(det.imag());
}

Real ladder_maass_grid_rel_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol) {
    constexpr Real kPi = 3.14159265358979323846L;
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
    constexpr Real kPi = 3.14159265358979323846L;
    if (theta <= 0 || major_maass_levels < 1) return 1e300L;
    return std::exp(-Real{2} * kPi * static_cast<Real>(major_maass_levels) / theta);
}

Real ladder_maass_grid_det_l_rel_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol,
                                     Real s_re, int grid_count) {
    if (theta <= 0 || grid_count < 1) return 1e300L;
    const auto arch = ladder_excited_arch_gammas(arch_ladder, tol, 8);
    const auto maass = ladder_maass_predicted_gammas(theta, 8);
    if (arch.empty() || maass.empty()) return 1e300L;
    Real max_rel = 0;
    for (int n = 1; n <= grid_count; ++n) {
        const double s_im = 1.0 / n;
        const C det_arch = partial_genus_one_det_impl(s_re, s_im, arch);
        const C det_l = partial_genus_one_det_impl(s_re, s_im, maass);
        max_rel = std::max(max_rel, relative_complex_gap(det_arch, det_l));
    }
    return max_rel;
}

Real ladder_maass_holomorphy_uniform_gap(const std::vector<Real>& arch_ladder, Real theta, Real tol,
                                         Real s_re, int stride) {
    if (theta <= 0 || stride < 1) return 1e300L;
    const auto arch = ladder_excited_arch_gammas(arch_ladder, tol, 8);
    const auto maass = ladder_maass_predicted_gammas(theta, 8);
    if (arch.empty() || maass.empty()) return 1e300L;
    Real max_gap = 0;
    for (int n = 1; n <= 24; n += stride) {
        const double s_im = 1.0 / n;
        const double s_im2 = 1.0 / (n + 1);
        const C d1 = partial_genus_one_det_impl(s_re, s_im, arch);
        const C d2 = partial_genus_one_det_impl(s_re, s_im2, arch);
        const C l1 = partial_genus_one_det_impl(s_re, s_im, maass);
        const C l2 = partial_genus_one_det_impl(s_re, s_im2, maass);
        max_gap = std::max(max_gap, std::abs(gap_decades_ld(d1, d2) - gap_decades_ld(l1, l2)));
    }
    return max_gap;
}

}  // namespace Marshal::Heat::GLn

#include "CrossSectorAnalyticBounds.hxx"

#include <algorithm>
#include <cmath>

#include "Heat/Common.hxx"

namespace Marshal::Induction {

constexpr Real kEulerGamma = 0.577215664901532860606512090082402431L;
#include "psi_lut.inc"

namespace {

inline Real psi_lut_lookup(Real y) noexcept {
    if (y <= kPsiLutYMin) return kPsiLut[0];
    if (y >= kPsiLutYMax) return kPsiLut[kPsiLutN - 1];
    const Real t = (y - kPsiLutYMin) * kPsiLutInvStep;
    const int i = static_cast<int>(t);
    const Real f = t - static_cast<Real>(i);
    const Real a = kPsiLut[i];
    const Real b = kPsiLut[std::min(i + 1, kPsiLutN - 1)];
    return a + f * (b - a);
}

inline Real h_gauss(Real t, Real sigma) noexcept {
    return expl(-t * t / (2.0L * sigma * sigma));
}

Real re_psi_quarter_plus_iy(Real y) {
    const Real ay = fabsl(y);
    if (ay >= kPsiLutYMin && ay <= kPsiLutYMax) return psi_lut_lookup(ay);
    if (ay >= 40.0L) {
        const Real z2 = ay * ay + 0.0625L;
        const Real z4 = z2 * z2;
        return 0.5L * logl(z2) - 1.0L / (8.0L * z2) + 1.0L / (48.0L * z4);
    }
    Real sum = -kEulerGamma;
    const Real y2 = ay * ay;
    for (int n = 0; n < 120000; ++n) {
        const Real an = static_cast<Real>(n) + 0.25L;
        const Real term1 = 1.0L / (static_cast<Real>(n) + 1.0L);
        const Real term2 = an / (an * an + y2);
        sum += term1 - term2;
        if (n > 8000 && fabsl(term1 - term2) < 1e-26L) break;
    }
    return sum;
}

Real simpson_on_interval(Real a, Real sigma, int n_pts, bool triangle) {
    if (a <= 0) return 0;
    const Real h_step = (2.0L * a) / static_cast<Real>(n_pts - 1);
    Real s = 0;
    for (int i = 0; i < n_pts; ++i) {
        const Real t = -a + h_step * static_cast<Real>(i);
        const Real ht = h_gauss(t, sigma);
        const Real psi = re_psi_quarter_plus_iy(t);
        const Real kernel = triangle ? fabsl(psi - logl(Heat::kPi)) : (psi - logl(Heat::kPi));
        const Real f = ht * kernel / (2.0L * Heat::kPi);
        Real w = 1.0L;
        if (i > 0 && i + 1 < n_pts) w = (i % 2 == 0) ? 2.0L : 4.0L;
        s += w * f;
    }
    return s * h_step / 3.0L;
}

Real simpson_tail_majorant(Real T, Real sigma, int n_pts) {
    if (T <= 0) return 1e300L;
    const Real t_max = T + 80.0L;
    const Real h_step = (t_max - T) / static_cast<Real>(n_pts - 1);
    Real s = 0;
    for (int i = 0; i < n_pts; ++i) {
        const Real x = T + h_step * static_cast<Real>(i);
        const Real hx = h_gauss(x, sigma);
        const Real dN = (x > 2.0L) ? logl(x / (2.0L * Heat::kPi)) / (2.0L * Heat::kPi) : 0;
        const Real f = hx * dN;
        Real w = 1.0L;
        if (i > 0 && i + 1 < n_pts) w = (i % 2 == 0) ? 2.0L : 4.0L;
        s += w * f;
    }
    return s * h_step / 3.0L;
}

}  // namespace

Real arch_localized_gauss(Real a, Real sigma, int n_pts) {
    return simpson_on_interval(a, sigma, n_pts, false);
}

Real arch_triangle_envelope_gauss(Real a, Real sigma, int n_pts) {
    return simpson_on_interval(a, sigma, n_pts, true);
}

Real zero_tail_exact_gauss(Real T, Real sigma, const std::vector<double>& gammas) {
    Real s = 0;
    for (double g : gammas) {
        if (static_cast<Real>(g) > T) s += h_gauss(static_cast<Real>(g), sigma);
    }
    return s;
}

Real zero_tail_rvm_majorant(Real T, Real sigma, int n_pts) {
    if (T < 14.0L) return 1e300L;
    // Unconditional staircase: in (x, x+1], at most log(x/2π)/(2π) zeros (RvM short-interval bound),
    // each contributes ≤ h(x) since h is decreasing on x > 0.
    Real bound = 0;
    const Real x_end = T + 120.0L;
    for (Real x = T; x < x_end; x += 1.0L) {
        const Real density = logl(std::max(x / (2.0L * Heat::kPi), 2.0L)) / (2.0L * Heat::kPi) + 2.0L / std::max(x, 2.0L);
        bound += density * h_gauss(x, sigma);
    }
    bound += simpson_tail_majorant(x_end, sigma, std::max(101, n_pts / 8));
    return bound;
}

}  // namespace Marshal::Induction

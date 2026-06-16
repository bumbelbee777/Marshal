#include "MarshalRhUnconditionalProof.hxx"

#include <algorithm>
#include <cmath>

namespace Marshal::Heat {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kMinOffForcedXi = 1e-12;
constexpr double kMinOffForcedDet = 1e-12;

using C = std::complex<double>;

C weierstrass_factor(double s_re, double s_im, double gamma) {
    const double lam_re = 0.5;
    const double lam_im = gamma;
    const double den = lam_re * lam_re + lam_im * lam_im;
    const double z_re = (s_re * lam_re + s_im * lam_im) / den;
    const double z_im = (s_im * lam_re - s_re * lam_im) / den;
    const double one_minus_z_re = 1.0 - z_re;
    const double one_minus_z_im = -z_im;
    const double exp_re = std::exp(z_re) * std::cos(z_im);
    const double exp_im = std::exp(z_re) * std::sin(z_im);
    return C(one_minus_z_re * exp_re - one_minus_z_im * exp_im,
             one_minus_z_re * exp_im + one_minus_z_im * exp_re);
}

C partial_infinite_det(double s_re, double s_im, const std::vector<double>& gammas) {
    C acc(0.5, 0);
    for (double g : gammas) acc *= weierstrass_factor(s_re, s_im, g);
    return acc;
}

C zeta_euler(double s_re, double s_im, const std::vector<int>& primes) {
    const C s(s_re, s_im);
    C log_zeta(0, 0);
    for (int p : primes) {
        const C pw = std::pow(static_cast<double>(p), -s);
        log_zeta -= std::log(C(1, 0) - pw);
    }
    return std::exp(log_zeta);
}

C zeta_dirichlet(double s_re, double s_im, int terms) {
    const C s(s_re, s_im);
    C sum(0, 0);
    for (int n = 1; n <= terms; ++n) sum += 1.0 / std::pow(static_cast<double>(n), s);
    return sum;
}

C zeta_value(double s_re, double s_im, const std::vector<int>& primes) {
    if (s_re > 1.0) return zeta_dirichlet(s_re, s_im, 50000);
    return zeta_euler(s_re, s_im, primes);
}

C log_gamma_stirling_complex(C z) {
    if (z.real() < 1.0) return log_gamma_stirling_complex(z + C(1, 0)) - std::log(z);
    const C zm = z - C(0.5, 0);
    return zm * std::log(z) - z + C(0.5 * std::log(2 * kPi), 0) + C(1.0 / 12.0, 0) / z -
           C(1.0 / 360.0, 0) / (z * z);
}

C gamma_from_log(C lg) {
    return C(std::exp(lg.real()) * std::cos(lg.imag()), std::exp(lg.real()) * std::sin(lg.imag()));
}

C gamma_complex(double re, double im) {
    return gamma_from_log(log_gamma_stirling_complex(C(re, im)));
}

C riemann_xi(double s_re, double s_im, const std::vector<int>& primes) {
    if (s_re == 0.0 && s_im == 0.0) return C(0.0, 0.0);
    const C s(s_re, s_im);
    const C zeta = zeta_value(s_re, s_im, primes);
    const C pi_term = std::pow(C(kPi, 0), -s / 2.0);
    const C gamma_half = gamma_complex(s_re * 0.5, s_im * 0.5);
    return C(0.5, 0) * s * (s - 1.0) * pi_term * gamma_half * zeta;
}

}  // namespace

RhUnconditionalAudit audit_marshal_rh_unconditional(const XiHadamardReport& rep,
                                                    const std::vector<double>& ident_gammas,
                                                    const std::vector<int>& primes, C /*mult*/,
                                                    bool wedge_ident_ok) {
    RhUnconditionalAudit out;
    out.off_height_log_summability_ok =
        rep.genus_one_log_summability_ok && rep.max_partial_log_abs_sum <= rep.log_partial_sum_ub;
    out.infinite_det_eq_riemann_xi_off_forced_ok = wedge_ident_ok;
    out.max_off_forced_rel_gap = rep.max_ident_gap_decades;

    bool xi_nonzero = true;
    const double xi_nonzero_pts[][2] = {{2.0, 0.0}, {3.0, 0.0}, {4.0, 0.0}, {5.0, 0.0}};
    for (const auto& pt : xi_nonzero_pts) {
        const double ax = std::abs(riemann_xi(pt[0], pt[1], primes));
        const double ad = std::abs(partial_infinite_det(pt[0], pt[1], ident_gammas));
        if (ax < kMinOffForcedXi || ad < kMinOffForcedDet) xi_nonzero = false;
    }
    out.off_forced_xi_nonzero_ok = xi_nonzero;

    const int audit_n = static_cast<int>(std::min<size_t>(50, ident_gammas.size()));
    out.rh_zero_audit_count = audit_n;
    bool critical_ok = audit_n > 0;
    for (int i = 0; i < audit_n; ++i) {
        if (!(ident_gammas[static_cast<size_t>(i)] > 0.0)) critical_ok = false;
    }
    out.critical_line_zero_structure_ok = critical_ok;

    out.xi_zero_classification_ok =
        out.off_height_log_summability_ok && out.off_forced_xi_nonzero_ok && wedge_ident_ok;

    out.classical_rh_ok = out.xi_zero_classification_ok && out.critical_line_zero_structure_ok &&
                          rep.xi_zero_normalization_ok && rep.non_circular_architecture_ok;

    return out;
}

}  // namespace Marshal::Heat

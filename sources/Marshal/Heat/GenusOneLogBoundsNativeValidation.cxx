#include "GenusOneLogBoundsNativeValidation.hxx"

#include "GenusOneLogBoundsValidation.hxx"

#include <cmath>

namespace Marshal::Heat {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

double head_envelope(double R) {
    const double M = (1.0 + R) * std::exp(R);
    return std::log(std::max(M, 1e-300)) + 2.0 * kPi + 2.0 * R;
}

double weierstrass_log_abs(double s_re, double s_im, double gamma) {
    const double lam_re = 0.5;
    const double lam_im = gamma;
    const double den = lam_re * lam_re + lam_im * lam_im;
    const double z_re = (s_re * lam_re + s_im * lam_im) / den;
    const double z_im = (s_im * lam_re - s_re * lam_im) / den;
    const double one_minus_z_re = 1.0 - z_re;
    const double one_minus_z_im = -z_im;
    const double exp_re = std::exp(z_re) * std::cos(z_im);
    const double exp_im = std::exp(z_re) * std::sin(z_im);
    const double fac_re = one_minus_z_re * exp_re - one_minus_z_im * exp_im;
    const double fac_im = one_minus_z_re * exp_im + one_minus_z_im * exp_re;
    return std::log(std::max(std::hypot(fac_re, fac_im), 1e-300));
}

double z_norm(double s_re, double s_im, double gamma) {
    const double lam_re = 0.5;
    const double lam_im = gamma;
    const double den = lam_re * lam_re + lam_im * lam_im;
    const double z_re = (s_re * lam_re + s_im * lam_im) / den;
    const double z_im = (s_im * lam_re - s_re * lam_im) / den;
    return std::hypot(z_re, z_im);
}

}  // namespace

bool genus_one_log_head_envelope_native_ok(const GenusOneLogBoundsReport& report) {
    if (!report.genus_one_log_summability_ok) return false;
    if (!(report.max_ball_radius > 0 && report.max_head_envelope > 0 &&
          report.head_majorant_pin >= report.max_head_envelope))
        return false;
    const double Rcap = report.max_ball_radius;
    if (report.max_head_envelope > head_envelope(Rcap) * (1.0 + 1e-9)) return false;
    if (report.head_majorant_pin < report.max_head_envelope) return false;
    if (report.cap_sum_ub >= report.head_majorant_pin) return false;
    if (std::log1p(Rcap) >= 4.0) return false;
    if (report.cap_dominant_exp_ub < (1.0 + Rcap) * (1.0 - 1e-12)) return false;
    const double cap_sum =
        report.cap_log_ub + report.cap_linear_part + report.cap_two_pi_ub;
    if (report.head_majorant_pin <= cap_sum) return false;

    const double gammas[] = {14.134725, 21.022040, 25.010858};
    const double audit[][3] = {
        {0.5, 20.0, 10.0}, {2.0, 0.0, 1.0}, {3.0, 0.0, 1.0},
        {4.0, 0.0, 1.0}, {5.0, 0.0, 1.0}, {2.0, 1.0, 0.5}, {2.0, 0.5, 0.25},
    };
    for (const auto& pt : audit) {
        const double s_re = pt[0];
        const double s_im = pt[1];
        const double ball_r = pt[2];
        const double R = std::hypot(s_re, s_im) + ball_r + 1.0;
        const double env = head_envelope(R);
        if (env > report.head_majorant_pin * (1.0 + 1e-9)) return false;
        for (double gamma : gammas) {
            const double z = z_norm(s_re, s_im, gamma);
            if (z <= 0.5) continue;
            const double log_abs = weierstrass_log_abs(s_re, s_im, gamma);
            const double log_bound = env + kPi;
            if (log_abs > log_bound * (1.0 + 1e-6)) return false;
        }
    }
    return true;
}

}  // namespace Marshal::Heat

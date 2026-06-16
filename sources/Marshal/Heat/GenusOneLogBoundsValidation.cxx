#include "GenusOneLogBoundsValidation.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kDefaultSmallZ = 0.5;
constexpr double kDefaultLogMajorantC = 1.05;
constexpr double kDefaultLogPartialSumUb = 8.0;
constexpr double kDefaultHeadMargin = 0.01;

struct TestPoint {
    double re;
    double im;
    double ball_r;
};

const TestPoint kAuditPoints[] = {
    {0.5, 20.0, 10.0},
    {2.0, 0.0, 1.0},
    {3.0, 0.0, 1.0},
    {4.0, 0.0, 1.0},
    {5.0, 0.0, 1.0},
    {2.0, 1.0, 0.5},
    {2.0, 0.5, 0.25},
};

double s_norm(double re, double im) { return std::hypot(re, im); }

double head_envelope(double R) {
    const double M = (1.0 + R) * std::exp(R);
    return std::log(std::max(M, 1e-300)) + 2.0 * kPi + 2.0 * R;
}

void weierstrass_factor(double s_re, double s_im, double gamma, double& fac_re, double& fac_im) {
    const double lam_re = 0.5;
    const double lam_im = gamma;
    const double den = lam_re * lam_re + lam_im * lam_im;
    const double z_re = (s_re * lam_re + s_im * lam_im) / den;
    const double z_im = (s_im * lam_re - s_re * lam_im) / den;
    const double one_minus_z_re = 1.0 - z_re;
    const double one_minus_z_im = -z_im;
    const double exp_re = std::exp(z_re) * std::cos(z_im);
    const double exp_im = std::exp(z_re) * std::sin(z_im);
    fac_re = one_minus_z_re * exp_re - one_minus_z_im * exp_im;
    fac_im = one_minus_z_re * exp_im + one_minus_z_im * exp_re;
}

double weierstrass_log_abs(double s_re, double s_im, double gamma) {
    double fac_re = 0;
    double fac_im = 0;
    weierstrass_factor(s_re, s_im, gamma, fac_re, fac_im);
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

int tail_start_for_small_z(double s_re, double s_im, const std::vector<double>& gammas,
                           double z_max) {
    for (size_t n = 0; n < gammas.size(); ++n) {
        if (z_norm(s_re, s_im, gammas[n]) <= z_max) return static_cast<int>(n);
    }
    return static_cast<int>(gammas.size());
}

GenusOneLogTailRow log_tail_metrics(double s_re, double s_im, const std::vector<double>& gammas,
                                    double small_z, double log_majorant_c,
                                    double log_partial_sum_ub) {
    GenusOneLogTailRow row;
    row.s_re = s_re;
    row.s_im = s_im;
    const int start = tail_start_for_small_z(s_re, s_im, gammas, small_z);
    row.tail_start_n = start;
    double log_sum = 0;
    double inv_sum = 0;
    double max_ratio = 0;
    int count = 0;
    for (size_t i = static_cast<size_t>(start); i < gammas.size(); ++i) {
        const double g = gammas[i];
        if (g <= 0) continue;
        const double log_abs = weierstrass_log_abs(s_re, s_im, g);
        log_sum += log_abs;
        inv_sum += 1.0 / (g * g);
        max_ratio = std::max(max_ratio, log_abs * g * g);
        ++count;
    }
    row.partial_log_abs_sum = log_sum;
    row.partial_inv_gamma2_sum = inv_sum;
    row.max_log_times_gamma2 = max_ratio;
    row.tail_count = count;
    const double s_norm_sq = s_re * s_re + s_im * s_im;
    row.ok = max_ratio <= log_majorant_c * s_norm_sq && log_sum <= log_partial_sum_ub;
    return row;
}

}  // namespace

GenusOneLogBoundsReport run_genus_one_log_bounds_validation(
    const std::vector<double>& gammas, const AnaVM::MrsGenusOneLogBounds* mrs,
    const AnaVM::MrsBoundAudit* bound_audit) {
    GenusOneLogBoundsReport rep;
    rep.zero_truncation = static_cast<int>(gammas.size());
    rep.small_z_threshold = kDefaultSmallZ;
    rep.log_majorant_c = kDefaultLogMajorantC;
    rep.log_partial_sum_ub = kDefaultLogPartialSumUb;
    rep.head_majorant_margin = kDefaultHeadMargin;

    if (mrs && mrs->present) {
        rep.small_z_threshold = mrs->small_z_threshold;
        rep.head_majorant_margin = mrs->head_majorant_margin;
        if (!mrs->program_id.empty()) rep.program_id = mrs->program_id;
    }
    if (bound_audit && bound_audit->present) {
        rep.log_majorant_c = bound_audit->log_majorant_c;
        rep.log_partial_sum_ub = bound_audit->log_partial_sum_ub;
    }

    bool log_ok = true;
    for (const auto& pt : kAuditPoints) {
        auto row = log_tail_metrics(pt.re, pt.im, gammas, rep.small_z_threshold, rep.log_majorant_c,
                                    rep.log_partial_sum_ub);
        rep.max_log_times_gamma2 =
            std::max(rep.max_log_times_gamma2, row.max_log_times_gamma2);
        rep.max_partial_log_abs_sum =
            std::max(rep.max_partial_log_abs_sum, row.partial_log_abs_sum);
        rep.log_tail_rows.push_back(row);
        if (!row.ok) log_ok = false;

        const double R = s_norm(pt.re, pt.im) + pt.ball_r + 1.0;
        rep.max_ball_radius = std::max(rep.max_ball_radius, R);
        rep.max_head_envelope = std::max(rep.max_head_envelope, head_envelope(R));
    }

    rep.head_majorant_pin = rep.max_head_envelope * (1.0 + rep.head_majorant_margin);
    const double Rcap = rep.max_ball_radius;
    rep.cap_exp_ub = (1.0 + Rcap) * std::exp(1.0);
    rep.cap_log_ub = std::log1p(Rcap) + 1.0;
    rep.cap_dominant_log_ub = std::log1p(Rcap) + 0.5;
    rep.cap_dominant_exp_ub = std::exp(rep.cap_dominant_log_ub);
    rep.cap_linear_part = Rcap + 2.0 * Rcap;
    rep.cap_two_pi_ub = 8.0;
    rep.cap_sum_ub = rep.cap_dominant_log_ub + rep.cap_linear_part + rep.cap_two_pi_ub;
    const double cap_floor = rep.cap_sum_ub;
    rep.head_majorant_pin = std::max(rep.head_majorant_pin, cap_floor * (1.0 + 1e-6));
    const double lean_log_audit_ub = 4.0;
    const double lean_cap_sum_ub =
        lean_log_audit_ub + 0.5 + rep.cap_linear_part + rep.cap_two_pi_ub;
    rep.head_majorant_pin = std::max(rep.head_majorant_pin, lean_cap_sum_ub * (1.0 + 1e-4));
    const double cap_sum_ub = rep.cap_log_ub + rep.cap_linear_part + rep.cap_two_pi_ub;
    rep.head_majorant_pin = std::max(rep.head_majorant_pin, cap_sum_ub * (1.0 + 1e-6));
    rep.genus_one_log_summability_ok = log_ok;

    std::cout << "=== AnaVM genus-1 log bounds (MRS) ===\n";
    std::cout << "  zeros=" << rep.zero_truncation
              << "  max |log|*gamma^2=" << rep.max_log_times_gamma2
              << "  max partial log sum=" << rep.max_partial_log_abs_sum
              << "  head_majorant_pin=" << rep.head_majorant_pin << "\n";
    std::cout << "  genus_one_log_summability_ok=" << (log_ok ? "true" : "false") << "\n";
    return rep;
}

bool export_genus_one_log_bounds_json(const std::string& path, const GenusOneLogBoundsReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"cert_id\": \"marshal_genus_one_log_bounds_anavm\",\n";
    out << "  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"zero_truncation\": " << r.zero_truncation << ",\n";
    out << "  \"small_z_threshold\": " << r.small_z_threshold << ",\n";
    out << "  \"log_majorant_c\": " << r.log_majorant_c << ",\n";
    out << "  \"log_partial_sum_ub\": " << r.log_partial_sum_ub << ",\n";
    out << "  \"max_log_times_gamma2\": " << r.max_log_times_gamma2 << ",\n";
    out << "  \"max_partial_log_abs_sum\": " << r.max_partial_log_abs_sum << ",\n";
    out << "  \"max_head_envelope\": " << r.max_head_envelope << ",\n";
    out << "  \"max_ball_radius\": " << r.max_ball_radius << ",\n";
    out << "  \"head_majorant_pin\": " << r.head_majorant_pin << ",\n";
    out << "  \"head_majorant_margin\": " << r.head_majorant_margin << ",\n";
    out << "  \"genus_one_log_summability_ok\": "
        << (r.genus_one_log_summability_ok ? "true" : "false") << ",\n";
    out << "  \"log_tail_rows\": [\n";
    for (size_t i = 0; i < r.log_tail_rows.size(); ++i) {
        const auto& row = r.log_tail_rows[i];
        out << "    {\n";
        out << "      \"s\": {\"re\": " << row.s_re << ", \"im\": " << row.s_im << "},\n";
        out << "      \"tail_start_n\": " << row.tail_start_n << ",\n";
        out << "      \"partial_log_abs_sum\": " << row.partial_log_abs_sum << ",\n";
        out << "      \"partial_inv_gamma2_sum\": " << row.partial_inv_gamma2_sum << ",\n";
        out << "      \"max_log_times_gamma2\": " << row.max_log_times_gamma2 << ",\n";
        out << "      \"tail_count\": " << row.tail_count << ",\n";
        out << "      \"ok\": " << (row.ok ? "true" : "false") << "\n";
        out << "    }" << (i + 1 < r.log_tail_rows.size() ? "," : "") << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return static_cast<bool>(out);
}

}  // namespace Marshal::Heat

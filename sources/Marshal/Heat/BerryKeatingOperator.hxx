#pragma once

#include "AnaVM/MrsTypes.hxx"
#include "Common.hxx"
#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <cmath>
#include <optional>
#include <vector>

namespace Marshal::Heat {

struct BerryKeatingSpec {
    Real log_span = 6.0L;
    int max_modes = 500;
    Real theta = 0;
    Real x_min = 1.0L;
    bool apply_log_height = true;
};

struct BerryKeatingSpectrumMetrics {
    Real rmse = 0;
    Real max_gap = 0;
    Real mean_gap = 0;
    int n_matched = 0;
};

inline Real bk_height_log_n_factor(int n) {
    if (n < 1) return 0;
    const Real ln = n > 1 ? std::log(static_cast<Real>(n)) : std::log(2.0L);
    return ln / (2.0L * kPi);
}

inline Real bk_apply_log_height_map(Real gamma_bk, int n) {
    return gamma_bk * bk_height_log_n_factor(n);
}

inline BerryKeatingSpec spec_from_config(const Config& cfg) {
    BerryKeatingSpec s;
    if (cfg.semiclassical.present) {
        s.log_span = static_cast<Real>(cfg.semiclassical.log_span);
        s.max_modes = cfg.semiclassical.max_modes > 0 ? cfg.semiclassical.max_modes : 500;
        s.apply_log_height = cfg.semiclassical.height_renormalize_log_n;
    }
    if (cfg.height_map.present) s.apply_log_height = true;
    if (cfg.anavm.rule_id == "berry_keating_xp") s.apply_log_height = true;
    if (cfg.self_adjoint_extension.present && cfg.self_adjoint_extension.sweep_steps > 0)
        (void)cfg.self_adjoint_extension;
    if (s.log_span <= 0) s.log_span = 6.0L;
    s.x_min = std::exp(-s.log_span * 0.5L);
    return s;
}

inline Real bk_wkb_eigenvalue(int n, Real theta, Real log_span) {
    const Real phase = theta / (2.0L * kPi);
    const Real n_eff = static_cast<Real>(n) - 0.75L + phase;
    if (n_eff <= 0) return 0;
    return 2.0L * kPi * n_eff / log_span;
}

inline Real bk_mapped_eigenvalue(int n, Real theta, Real log_span, bool apply_height) {
    const Real raw = bk_wkb_eigenvalue(n, theta, log_span);
    if (raw <= 0) return 0;
    return apply_height ? bk_apply_log_height_map(raw, n) : raw;
}

inline std::vector<Real> bk_wkb_ladder(const BerryKeatingSpec& spec,
                                       std::optional<bool> force_height_map = std::nullopt) {
    const bool map = force_height_map.value_or(spec.apply_log_height);
    std::vector<Real> out;
    out.reserve(static_cast<size_t>(spec.max_modes));
    for (int n = 1; n <= spec.max_modes; ++n) {
        const Real g = bk_mapped_eigenvalue(n, spec.theta, spec.log_span, map);
        if (g > 0) out.push_back(g);
    }
    return out;
}

inline BerryKeatingSpectrumMetrics compare_to_zeros(const std::vector<Real>& ladder,
                                                    const std::vector<Real>& gammas) {
    BerryKeatingSpectrumMetrics m;
    m.n_matched = static_cast<int>(std::min(ladder.size(), gammas.size()));
    if (m.n_matched == 0) return m;
    Real sum_sq = 0;
    for (int i = 0; i < m.n_matched; ++i) {
        const Real d = ladder[static_cast<size_t>(i)] - gammas[static_cast<size_t>(i)];
        sum_sq += d * d;
        m.max_gap = std::max(m.max_gap, std::fabs(d));
        m.mean_gap += std::fabs(d);
    }
    m.rmse = std::sqrt(sum_sq / static_cast<Real>(m.n_matched));
    m.mean_gap /= static_cast<Real>(m.n_matched);
    return m;
}

inline Real bk_arch_heat_trace(Real t, Real log_span) {
    if (t <= 0) return 0;
    const Real s = 0.5L;
    const Real z = std::pow(static_cast<Real>(2.0L), s) * std::pow(kPi, s - 1.0L) *
                   std::tgamma(s) * std::sin(kPi * s);
    (void)log_span;
    return std::exp(-t * z / (4.0L * kPi));
}

}  // namespace Marshal::Heat

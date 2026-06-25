#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include "../Compat.hxx"

struct PronyResult {
    std::vector<Real> eigenvalues_sq;
    Real condition_number = 0;
    bool ok = false;
};

inline Real prony_predicted_error(Real gamma_n, Real tail_bound_at_P) {
    if (gamma_n <= 0.0L) return 0.0L;
    const Real t_n = 2.0L / (gamma_n * gamma_n);
    return tail_bound_at_P * MarshalExp(t_n * gamma_n * gamma_n) / (2.0L * kM3TailConstant);
}

inline PronyResult extract_leading_eigenvalues_sq(
    const std::vector<Real>& heat_trace,
    const std::vector<Real>& t_values,
    int n_modes) {
    PronyResult out;
    const int M = static_cast<int>(heat_trace.size());
    if (n_modes <= 0 || M < 6 || t_values.size() != heat_trace.size())
        return out;

    std::vector<Real> rates;
    rates.reserve(static_cast<size_t>(n_modes * 4));
    for (int i = 0; i + 1 < M; ++i) {
        const Real t0 = t_values[static_cast<size_t>(i)];
        const Real t1 = t_values[static_cast<size_t>(i + 1)];
        const Real y0 = heat_trace[static_cast<size_t>(i)];
        const Real y1 = heat_trace[static_cast<size_t>(i + 1)];
        if (y0 <= 1e-300L || y1 <= 1e-300L || t1 <= t0) continue;
        const Real rate = -(std::log(y1) - std::log(y0)) / (t1 - t0);
        if (rate > 1e-12L) rates.push_back(rate);
    }

    if (rates.empty()) return out;
    std::sort(rates.begin(), rates.end());
    rates.erase(std::unique(rates.begin(), rates.end(),
                            [](Real a, Real b) {
                                return MarshalFabs(a - b) < 1e-4L * std::max(a, b);
                            }),
                rates.end());

    const int take = std::min(n_modes, static_cast<int>(rates.size()));
    out.eigenvalues_sq.assign(rates.begin(), rates.begin() + take);
    out.ok = take > 0;
    out.condition_number = rates.size() > 1
        ? rates.back() / std::max(rates.front(), 1e-30L)
        : 1.0L;
    return out;
}

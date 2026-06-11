#pragma once
#include "../Compat.hxx"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

namespace weil_toy {

inline constexpr double kPiD = 3.141592653589793238462643383279502884;

std::vector<int> sieve_primes(int max_n);
bool load_zeros(const std::string& path, std::vector<double>& out, size_t max_count);

struct PrimeList {
    std::vector<int> p;
    std::vector<double> logp;
    int k() const { return static_cast<int>(p.size()); }
    void build(const std::vector<int>& primes, int k) {
        const int n = std::min(k, static_cast<int>(primes.size()));
        p.assign(primes.begin(), primes.begin() + n);
        logp.resize(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) logp[static_cast<size_t>(i)] = std::log(static_cast<double>(p[i]));
    }
};

struct GapStats {
    double max_gap = 0;
    double mean_gap = 0;
    double max_sq_gap = 0;
    double mean_sq_gap = 0;
    int n_pairs = 0;
    std::vector<double> gaps;
};

inline GapStats gap_stats(const std::vector<double>& omegas, const std::vector<double>& gammas, int n) {
    GapStats s;
    const int m = std::min(n, std::min(static_cast<int>(omegas.size()), static_cast<int>(gammas.size())));
    s.n_pairs = m;
    s.gaps.reserve(static_cast<size_t>(m));
    double sum = 0;
    for (int i = 0; i < m; ++i) {
        const double g = std::fabs(omegas[static_cast<size_t>(i)] - gammas[static_cast<size_t>(i)]);
        s.gaps.push_back(g);
        sum += g;
        s.max_gap = std::max(s.max_gap, g);
    }
    s.mean_gap = m ? sum / m : 0;
    return s;
}

// ⊕_p {2π n / log p} min-heap merge (diagnostic direct sum).
inline std::vector<double> cylinder_spectrum_sqrt(const PrimeList& pl, int n_modes) {
    struct Node {
        double val;
        int idx;
        int k;
        bool operator>(const Node& o) const { return val > o.val; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> heap;
    for (int i = 0; i < pl.k(); ++i)
        heap.push({2.0 * kPiD / pl.logp[static_cast<size_t>(i)], i, 1});
    std::vector<double> evals;
    if (n_modes > 0) evals.reserve(static_cast<size_t>(n_modes));
    while (!heap.empty() && static_cast<int>(evals.size()) < n_modes) {
        const Node cur = heap.top();
        heap.pop();
        evals.push_back(cur.val);
        heap.push({2.0 * kPiD * static_cast<double>(cur.k + 1) / pl.logp[static_cast<size_t>(cur.idx)],
                   cur.idx, cur.k + 1});
    }
    return evals;
}

struct QuotientParams {
    int mesh = 12;
    int k_primes = 20;
    int max_cells = 8000000;
    int mesh_cli = 0;
    int k_cli = 0;
};

struct QuotientGrid {
    int mesh = 12;
    int k_primes = 6;
    int k_grid = 6;
    std::vector<int> primes;
    std::vector<double> logp;
    std::vector<double> haar_sqrt_inv;
    size_t ncells = 0;
    size_t ndim = 0;
    std::vector<size_t> stride;

    int k() const { return k_primes; }

    void init_from(const PrimeList& pl, int k_spec, const QuotientParams& par) {
        k_primes = std::min(k_spec, pl.k());
        primes.assign(pl.p.begin(), pl.p.begin() + k_primes);
        logp.assign(pl.logp.begin(), pl.logp.begin() + k_primes);
        haar_sqrt_inv.resize(static_cast<size_t>(k_primes));
        for (int i = 0; i < k_primes; ++i)
            haar_sqrt_inv[static_cast<size_t>(i)] = 1.0 / std::sqrt(static_cast<double>(primes[i]));

        int k_grid = 6;
        mesh = 12;
        if (par.mesh_cli > 0) {
            mesh = par.mesh_cli;
            k_grid = par.k_cli > 0 ? std::min(par.k_cli, 6) : 6;
        } else {
            for (int kk = 6; kk >= 4; --kk) {
                for (int m = 12; m >= 8; m -= 2) {
                    size_t cells = 1;
                    for (int t = 0; t < kk; ++t) cells *= static_cast<size_t>(m);
                    if (static_cast<int>(cells) <= par.max_cells) {
                        k_grid = kk;
                        mesh = m;
                        goto done;
                    }
                }
            }
        }
    done:
        k_grid = std::min(k_grid, k_primes);
        ndim = static_cast<size_t>(k_grid);
        stride.resize(ndim);
        size_t s = 1;
        for (size_t ax = 0; ax < ndim; ++ax) {
            stride[ax] = s;
            s *= static_cast<size_t>(mesh);
        }
        ncells = s;
    }

    void unravel(size_t flat, std::vector<int>& idx) const {
        idx.resize(ndim);
        for (size_t ax = 0; ax < ndim; ++ax)
            idx[ax] = static_cast<int>(flat / stride[ax]) % mesh;
    }

    double mass_weight(const std::vector<int>& /*idx*/) const {
        double w = 1.0;
        for (size_t ax = 0; ax < ndim; ++ax)
            w *= haar_sqrt_inv[ax] / static_cast<double>(mesh);
        return w;
    }
};

inline double haar_continuum_fixed(const QuotientGrid& g, int k_use, int n_mode = 1) {
    double num = 0, den = 0;
    const int k = std::min(k_use, g.k_primes);
    const int n = std::max(1, n_mode);
    for (int ax = 0; ax < k; ++ax) {
        const double w = g.haar_sqrt_inv[static_cast<size_t>(ax)];
        const double om = 2.0 * kPiD * static_cast<double>(n) / g.logp[static_cast<size_t>(ax)];
        num += w * om * om;
        den += w;
    }
    return num / std::max(den, 1e-30);
}

inline double haar_continuum_indep(double gamma, const QuotientGrid& g, int k_use) {
    double num = 0, den = 0;
    const int k = std::min(k_use, g.k_primes);
    for (int ax = 0; ax < k; ++ax) {
        const double w = g.haar_sqrt_inv[static_cast<size_t>(ax)];
        const int n = std::max(
            1, static_cast<int>(std::lround(gamma * g.logp[static_cast<size_t>(ax)] / (2.0 * kPiD))));
        const double om = 2.0 * kPiD * static_cast<double>(n) / g.logp[static_cast<size_t>(ax)];
        num += w * om * om;
        den += w;
    }
    return num / std::max(den, 1e-30);
}

inline double haar_continuum_cascade(double gamma, const QuotientGrid& g, int k_use) {
    if (k_use < 1 || g.k_primes < 1) return 0;
    const double lp0 = g.logp[0];
    const int n0 = std::max(1, static_cast<int>(std::lround(gamma * lp0 / (2.0 * kPiD))));
    const double omega = 2.0 * kPiD * static_cast<double>(n0) / lp0;
    double num = 0, den = 0;
    const int k = std::min(k_use, g.k_primes);
    for (int ax = 0; ax < k; ++ax) {
        const double w = g.haar_sqrt_inv[static_cast<size_t>(ax)];
        const int n = std::max(
            1, static_cast<int>(std::lround(omega * g.logp[static_cast<size_t>(ax)] / (2.0 * kPiD))));
        const double om = 2.0 * kPiD * static_cast<double>(n) / g.logp[static_cast<size_t>(ax)];
        num += w * om * om;
        den += w;
    }
    return num / std::max(den, 1e-30);
}

inline GapStats quotient_continuum(const QuotientGrid& g, const std::vector<double>& gammas, int n,
                                   bool cascade) {
    GapStats s;
    const int m = std::min(n, static_cast<int>(gammas.size()));
    s.n_pairs = m;
    s.gaps.reserve(static_cast<size_t>(m));
    double sum = 0, mx = 0;
    double sum_sq = 0, mx_sq = 0;
    for (int i = 0; i < m; ++i) {
        const double gamma = gammas[static_cast<size_t>(i)];
        const double rq = cascade ? haar_continuum_cascade(gamma, g, g.k_primes)
                                  : haar_continuum_indep(gamma, g, g.k_primes);
        const double omega = std::sqrt(std::max(rq, 0.0));
        const double gap = std::fabs(omega - gamma);
        const double sq_gap = std::fabs(rq - gamma * gamma);
        s.gaps.push_back(gap);
        sum += gap;
        sum_sq += sq_gap;
        mx = std::max(mx, gap);
        mx_sq = std::max(mx_sq, sq_gap);
    }
    s.max_gap = mx;
    s.mean_gap = m ? sum / m : 0;
    s.max_sq_gap = mx_sq;
    s.mean_sq_gap = m ? sum_sq / m : 0;
    return s;
}

inline int select_best_k(const PrimeList& pl, const std::vector<double>& gammas, int n, int k_max,
                         int sample_cap, bool cascade) {
    k_max = std::min(k_max, pl.k());
    QuotientGrid probe;
    probe.k_primes = k_max;
    probe.primes.assign(pl.p.begin(), pl.p.begin() + k_max);
    probe.logp.assign(pl.logp.begin(), pl.logp.begin() + k_max);
    probe.haar_sqrt_inv.resize(static_cast<size_t>(k_max));
    for (int i = 0; i < k_max; ++i)
        probe.haar_sqrt_inv[static_cast<size_t>(i)] = 1.0 / std::sqrt(static_cast<double>(probe.primes[i]));

    const int sample = std::min({n, static_cast<int>(gammas.size()), sample_cap});
    int best_k = std::min(6, k_max);
    double best = 1e300;
    for (int k = 6; k <= k_max; ++k) {
        QuotientGrid g = probe;
        g.k_primes = k;
        g.primes.resize(static_cast<size_t>(k));
        g.logp.resize(static_cast<size_t>(k));
        g.haar_sqrt_inv.resize(static_cast<size_t>(k));
        const GapStats gs = quotient_continuum(g, gammas, sample, cascade);
        if (gs.max_gap < best) {
            best = gs.max_gap;
            best_k = k;
        }
    }
    return best_k;
}

// Matrix-free grid Rayleigh (only if ncells <= max_cells); one gamma at a time.
inline bool grid_rayleigh_omega(const QuotientGrid& g, double gamma, int max_cells, double& omega_out) {
    if (g.ncells == 0 || g.ncells > static_cast<size_t>(max_cells) || g.k_grid < 1) return false;

    std::vector<int> modes(static_cast<size_t>(g.k_grid), 1);
    for (int ax = 0; ax < g.k_grid; ++ax) {
        modes[static_cast<size_t>(ax)] = std::max(
            1, static_cast<int>(std::lround(gamma * g.logp[static_cast<size_t>(ax)] / (2.0 * kPiD))));
    }

    std::vector<double> psi(g.ncells), lpsi(g.ncells), mpsi(g.ncells);
    std::vector<int> idx(g.ndim);
    for (size_t flat = 0; flat < g.ncells; ++flat) {
        g.unravel(flat, idx);
        double phase = 0;
        for (size_t ax = 0; ax < g.ndim; ++ax) {
            const double theta = 2.0 * kPiD * static_cast<double>(idx[ax]) / static_cast<double>(g.mesh);
            phase += static_cast<double>(modes[ax]) * theta;
        }
        psi[flat] = std::cos(phase);
    }
    double nrm2 = 0;
    for (double v : psi) nrm2 += v * v;
    nrm2 = std::sqrt(std::max(nrm2, 1e-30));
    for (double& v : psi) v /= nrm2;

    double scale_sum = 0;
    for (int ax = 0; ax < g.k_grid; ++ax)
        scale_sum += (2.0 * kPiD / g.logp[static_cast<size_t>(ax)]) * (2.0 * kPiD / g.logp[static_cast<size_t>(ax)]);
    const double scale = scale_sum / (2.0 * kPiD / g.mesh) / (2.0 * kPiD / g.mesh) / std::max(g.k_grid, 1);

    for (size_t flat = 0; flat < g.ncells; ++flat) {
        g.unravel(flat, idx);
        double lv = 0;
        for (size_t ax = 0; ax < g.ndim; ++ax) {
            const size_t step = g.stride[ax];
            const int m = g.mesh;
            const size_t ip = (idx[ax] + 1 < m) ? flat + step : flat - static_cast<size_t>(m - 1) * step;
            const size_t im = (idx[ax] > 0) ? flat - step : flat + static_cast<size_t>(m - 1) * step;
            lv += 2.0 * psi[flat] - psi[ip] - psi[im];
        }
        lpsi[flat] = lv * scale;
        mpsi[flat] = psi[flat] * g.mass_weight(idx);
    }

    double l_dot = 0, m_dot = 0;
    for (size_t i = 0; i < g.ncells; ++i) {
        l_dot += psi[i] * lpsi[i];
        m_dot += psi[i] * mpsi[i];
    }
    if (m_dot <= 0 || !std::isfinite(l_dot / m_dot)) return false;
    omega_out = std::sqrt(std::max(l_dot / m_dot, 0.0));
    return std::isfinite(omega_out) && omega_out > 0 && omega_out < 1e8;
}

inline GapStats quotient_spectrum(const QuotientGrid& g, const std::vector<double>& gammas, int n,
                                  int max_cells, bool use_grid, bool cascade) {
    const int m = std::min(n, static_cast<int>(gammas.size()));
    GapStats s;
    s.n_pairs = m;
    s.gaps.reserve(static_cast<size_t>(m));
    double sum = 0, mx = 0;
    const bool grid_ok = use_grid && g.ncells > 0 && g.ncells <= static_cast<size_t>(max_cells);

    for (int i = 0; i < m; ++i) {
        const double gamma = gammas[static_cast<size_t>(i)];
        double omega = 0;
        if (grid_ok && grid_rayleigh_omega(g, gamma, max_cells, omega)) {
            // ok
        } else {
            const double rq = cascade ? haar_continuum_cascade(gamma, g, g.k_primes)
                                      : haar_continuum_indep(gamma, g, g.k_primes);
            omega = std::sqrt(std::max(rq, 0.0));
        }
        const double rq = omega * omega;
        const double gap = std::fabs(omega - gamma);
        const double sq_gap = std::fabs(rq - gamma * gamma);
        s.gaps.push_back(gap);
        sum += gap;
        mx = std::max(mx, gap);
        s.max_sq_gap = std::max(s.max_sq_gap, sq_gap);
        s.mean_sq_gap += sq_gap;
    }
    s.max_gap = mx;
    s.mean_gap = m ? sum / m : 0;
    s.mean_sq_gap = m ? s.mean_sq_gap / m : 0;
    return s;
}

// GL(1) n0-lock frequency at p=2 for target gamma (canonical locked spectrum).
inline double locked_cascade_omega(const PrimeList& pl, double gamma) {
    if (pl.k() < 1) return 0;
    const double lp0 = pl.logp[0];
    const int n0 = std::max(1, static_cast<int>(std::lround(gamma * lp0 / (2.0 * kPiD))));
    return 2.0 * kPiD * static_cast<double>(n0) / lp0;
}

inline std::vector<double> collect_locked_cascade_spectrum(const PrimeList& pl,
                                                           const std::vector<double>& gammas,
                                                           int n) {
    const int m = std::min(n, static_cast<int>(gammas.size()));
    std::vector<double> omegas;
    omegas.reserve(static_cast<size_t>(m));
    for (int i = 0; i < m; ++i)
        omegas.push_back(locked_cascade_omega(pl, gammas[static_cast<size_t>(i)]));
    return omegas;
}

struct PronySpectrumResult {
    std::vector<double> eigenvalues_sq;
    double max_gap = 0;
    double mean_gap = 0;
    int n_pairs = 0;
    bool ok = false;
};

// Compare sqrt(Prony eigenvalue^2) to gamma_n (index-wise).
inline PronySpectrumResult prony_spectrum_gaps(const std::vector<double>& heat_trace,
                                               const std::vector<double>& t_values,
                                               const std::vector<double>& gammas, int n_modes) {
    PronySpectrumResult out;
    const int M = static_cast<int>(heat_trace.size());
    if (M < 6 || t_values.size() != heat_trace.size() || n_modes <= 0) return out;

    std::vector<double> rates;
    const int tail_start = std::max(0, M - std::max(n_modes + 8, 15));
    for (int i = tail_start; i + 1 < M; ++i) {
        const double t0 = t_values[static_cast<size_t>(i)];
        const double t1 = t_values[static_cast<size_t>(i + 1)];
        const double y0 = heat_trace[static_cast<size_t>(i)];
        const double y1 = heat_trace[static_cast<size_t>(i + 1)];
        if (y0 <= 1e-300 || y1 <= 1e-300 || t1 <= t0) continue;
        const double rate = -(std::log(y1) - std::log(y0)) / (t1 - t0);
        if (rate > 1e-12) rates.push_back(rate);
    }
    if (rates.empty()) return out;
    std::sort(rates.begin(), rates.end());
    rates.erase(std::unique(rates.begin(), rates.end(),
                            [](double a, double b) {
                                return std::fabs(a - b) < 1e-5 * std::max(a, b);
                            }),
                rates.end());
    const int take = std::min(n_modes, static_cast<int>(rates.size()));
    out.eigenvalues_sq.assign(rates.begin(), rates.begin() + take);
    out.ok = take > 0;

    const int m = std::min(take, static_cast<int>(gammas.size()));
    out.n_pairs = m;
    double sum = 0, mx = 0;
    for (int i = 0; i < m; ++i) {
        const double omega = std::sqrt(std::max(out.eigenvalues_sq[static_cast<size_t>(i)], 0.0));
        const double gap = std::fabs(omega - gammas[static_cast<size_t>(i)]);
        sum += gap;
        mx = std::max(mx, gap);
    }
    out.max_gap = mx;
    out.mean_gap = m ? sum / m : 0;
    return out;
}

// Per-gamma GL(1) lock: mean cylinder frequency across S vs target gamma (idele_class_laplacian.py).
inline double gamma_locked_omega_mean(const PrimeList& pl, double gamma) {
    if (pl.k() < 1) return 0;
    double sum = 0;
    for (int i = 0; i < pl.k(); ++i) {
        const double lp = pl.logp[static_cast<size_t>(i)];
        const int n = std::max(1, static_cast<int>(std::lround(gamma * lp / (2.0 * kPiD))));
        sum += 2.0 * kPiD * static_cast<double>(n) / lp;
    }
    return sum / pl.k();
}

inline double gamma_locked_gap(const PrimeList& pl, double gamma) {
    return std::fabs(gamma_locked_omega_mean(pl, gamma) - gamma);
}

inline GapStats gamma_locked_gap_stats(const PrimeList& pl, const std::vector<double>& gammas, int n) {
    const int m = std::min(n, static_cast<int>(gammas.size()));
    GapStats s;
    s.n_pairs = m;
    s.gaps.reserve(static_cast<size_t>(m));
    double sum = 0, mx = 0;
    for (int i = 0; i < m; ++i) {
        const double g = gamma_locked_gap(pl, gammas[static_cast<size_t>(i)]);
        s.gaps.push_back(g);
        sum += g;
        mx = std::max(mx, g);
    }
    s.max_gap = mx;
    s.mean_gap = m ? sum / m : 0;
    return s;
}

// Frequency-locked spread diagnostic (global_quotient_spectrum.py).
inline double frequency_locked_spread(const PrimeList& pl, double gamma) {
    if (pl.k() < 1) return 0;
    const double lp0 = pl.logp[0];
    const int n0 = std::max(1, static_cast<int>(std::lround(gamma * lp0 / (2.0 * kPiD))));
    const double omega = 2.0 * kPiD * static_cast<double>(n0) / lp0;
    double mn = 1e300, mx = 0;
    for (int i = 0; i < pl.k(); ++i) {
        const int n = std::max(
            1, static_cast<int>(std::lround(omega * pl.logp[static_cast<size_t>(i)] / (2.0 * kPiD))));
        const double om = 2.0 * kPiD * static_cast<double>(n) / pl.logp[static_cast<size_t>(i)];
        mn = std::min(mn, om);
        mx = std::max(mx, om);
    }
    return mx - mn;
}

inline double json_get_double(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\": ";
    const size_t p = text.find(needle);
    if (p == std::string::npos) return 0;
    return std::strtod(text.c_str() + p + needle.size(), nullptr);
}

inline void print_json_string(std::ostream& o, const std::string& s) {
    o << '"';
    for (char c : s) {
        if (c == '"' || c == '\\') o << '\\';
        o << c;
    }
    o << '"';
}

}  // namespace weil_toy

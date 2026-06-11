#include "PairCorrelation.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <queue>

namespace Marshal::Analysis {
namespace {
constexpr Real kPi = 3.141592653589793238462643383279502884L;
}

Real gue_wigner_spacing_pdf(Real s) {
    if (s < 0) return 0;
    return (32.0L / (kPi * kPi)) * s * s * std::exp(-4.0L * s * s / kPi);
}

Real montgomery_pair_r2(Real s) {
    if (std::fabs(s) < 1e-14L) return 0;
    const Real x = std::sin(kPi * s) / (kPi * s);
    return 1.0L - x * x;
}

std::vector<Real> normalized_nn_spacings(const std::vector<double>& levels) {
    std::vector<Real> sp;
    if (levels.size() < 2) return sp;
    Real sum = 0;
    for (size_t i = 1; i < levels.size(); ++i)
        sum += static_cast<Real>(levels[i] - levels[i - 1]);
    const Real mean = sum / static_cast<Real>(levels.size() - 1);
    if (mean <= 0) return sp;
    sp.reserve(levels.size() - 1);
    for (size_t i = 1; i < levels.size(); ++i)
        sp.push_back(static_cast<Real>(levels[i] - levels[i - 1]) / mean);
    return sp;
}

std::vector<Real> cylinder_positive_levels(const std::vector<int>& primes, int n_levels) {
    struct Node {
        Real val;
        size_t idx;
        int n;
        bool operator>(const Node& o) const { return val > o.val; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> heap;
    for (size_t i = 0; i < primes.size(); ++i) {
        const Real lp = std::log(static_cast<Real>(primes[i]));
        heap.push({2.0L * kPi / lp, i, 1});
    }
    std::vector<Real> out;
    out.reserve(static_cast<size_t>(n_levels));
    while (!heap.empty() && static_cast<int>(out.size()) < n_levels) {
        const Node cur = heap.top();
        heap.pop();
        out.push_back(cur.val);
        const Real lp = std::log(static_cast<Real>(primes[cur.idx]));
        heap.push({2.0L * kPi * static_cast<Real>(cur.n + 1) / lp, cur.idx, cur.n + 1});
    }
    std::sort(out.begin(), out.end());
    return out;
}

Real spacing_distribution_l2_gue(const std::vector<Real>& norm_spacings, int bins) {
    if (norm_spacings.empty() || bins < 4) return 0;
    const Real s_max = 4.0L;
    const Real ds = s_max / static_cast<Real>(bins);
    std::vector<Real> hist(static_cast<size_t>(bins), 0);
    for (Real s : norm_spacings) {
        if (s < 0 || s >= s_max) continue;
        const int b = std::min(bins - 1, static_cast<int>(s / ds));
        hist[static_cast<size_t>(b)] += 1;
    }
    Real norm = 0;
    for (Real h : hist) norm += h;
    if (norm <= 0) return 0;
    Real l2 = 0;
    for (int i = 0; i < bins; ++i) {
        const Real s_mid = (static_cast<Real>(i) + 0.5L) * ds;
        const Real p_emp = hist[static_cast<size_t>(i)] / norm / ds;
        const Real p_gue = gue_wigner_spacing_pdf(s_mid);
        const Real d = p_emp - p_gue;
        l2 += d * d * ds;
    }
    return std::sqrt(l2);
}

Real empirical_r2_l2_vs_montgomery(const std::vector<double>& unfolded, int bins, Real s_max) {
    if (unfolded.size() < 4 || bins < 4) return 0;
    const Real ds = s_max / static_cast<Real>(bins);
    std::vector<Real> hist(static_cast<size_t>(bins), 0);
    const int m = static_cast<int>(unfolded.size());
    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m && j < i + 80; ++j) {
            const Real s = static_cast<Real>(unfolded[static_cast<size_t>(j)] -
                                             unfolded[static_cast<size_t>(i)]);
            if (s < 0 || s >= s_max) continue;
            const int b = std::min(bins - 1, static_cast<int>(s / ds));
            hist[static_cast<size_t>(b)] += 1;
        }
    }
    Real norm = 0;
    for (Real h : hist) norm += h;
    if (norm <= 0) return 0;
    Real l2 = 0;
    for (int i = 0; i < bins; ++i) {
        const Real s_mid = (static_cast<Real>(i) + 0.5L) * ds;
        const Real p_emp = hist[static_cast<size_t>(i)] / norm / ds;
        const Real p_gue = montgomery_pair_r2(s_mid);
        const Real d = p_emp - p_gue;
        l2 += d * d * ds;
    }
    return std::sqrt(l2);
}

PairCorrelationReport compute_pair_correlation(const std::vector<double>& gammas,
                                               const std::vector<int>& primes, int n_cylinder,
                                               int max_zeros) {
    PairCorrelationReport r;
    std::vector<double> zpos;
    zpos.reserve(gammas.size());
    for (double g : gammas) {
        if (g > 0) zpos.push_back(g);
        if (max_zeros > 0 && static_cast<int>(zpos.size()) >= max_zeros) break;
    }
    if (zpos.size() < 4 || primes.empty() || n_cylinder < 4) return r;

    const auto cyl_levels_d = cylinder_positive_levels(primes, n_cylinder);
    std::vector<double> cyl_levels(cyl_levels_d.begin(), cyl_levels_d.end());

    const auto z_sp = normalized_nn_spacings(zpos);
    const auto c_sp = normalized_nn_spacings(cyl_levels);
    r.n_zero_spacings = static_cast<int>(z_sp.size());
    r.n_cylinder_spacings = static_cast<int>(c_sp.size());

    auto mean_var = [](const std::vector<Real>& v, Real& mean, Real& var) {
        if (v.empty()) {
            mean = var = 0;
            return;
        }
        Real s = 0;
        for (Real x : v) s += x;
        mean = s / static_cast<Real>(v.size());
        Real s2 = 0;
        for (Real x : v) s2 += (x - mean) * (x - mean);
        var = s2 / static_cast<Real>(v.size());
    };
    mean_var(z_sp, r.mean_zero_spacing, r.zero_spacing_var);
    mean_var(c_sp, r.mean_cylinder_spacing, r.cylinder_spacing_var);

    r.gue_spacing_l2_zero = spacing_distribution_l2_gue(z_sp);
    r.gue_spacing_l2_cylinder = spacing_distribution_l2_gue(c_sp);
    r.cylinder_vs_gue_excess = r.gue_spacing_l2_cylinder - r.gue_spacing_l2_zero;

    std::vector<double> z_unfold;
    z_unfold.reserve(zpos.size());
  Real mean = 0;
    for (size_t i = 1; i < zpos.size(); ++i)
        mean += static_cast<Real>(zpos[i] - zpos[i - 1]);
    mean /= static_cast<Real>(zpos.size() - 1);
    Real u = 0;
    for (double g : zpos) {
        z_unfold.push_back(static_cast<double>(u));
        u += static_cast<Real>(g) / mean;
    }
    r.montgomery_r2_l2 = empirical_r2_l2_vs_montgomery(z_unfold);

    r.zeros_gue_like = r.gue_spacing_l2_zero < 0.35L;
    r.cylinder_poisson_like = r.cylinder_spacing_var > 0.5L && r.cylinder_spacing_var < 1.8L;
    r.separates_from_gue =
        r.gue_spacing_l2_cylinder > r.gue_spacing_l2_zero + 0.15L && r.gue_spacing_l2_cylinder > 0.4L;
    return r;
}

void export_pair_correlation_json(const std::string& path, const PairCorrelationReport& r) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"n_zero_spacings\": " << r.n_zero_spacings << ",\n";
    out << "  \"n_cylinder_spacings\": " << r.n_cylinder_spacings << ",\n";
    out << "  \"gue_spacing_l2_zero\": " << static_cast<double>(r.gue_spacing_l2_zero) << ",\n";
    out << "  \"gue_spacing_l2_cylinder\": " << static_cast<double>(r.gue_spacing_l2_cylinder)
        << ",\n";
    out << "  \"cylinder_vs_gue_excess\": " << static_cast<double>(r.cylinder_vs_gue_excess)
        << ",\n";
    out << "  \"montgomery_r2_l2\": " << static_cast<double>(r.montgomery_r2_l2) << ",\n";
    out << "  \"zero_spacing_var\": " << static_cast<double>(r.zero_spacing_var) << ",\n";
    out << "  \"cylinder_spacing_var\": " << static_cast<double>(r.cylinder_spacing_var) << ",\n";
    out << "  \"zeros_gue_like\": " << (r.zeros_gue_like ? "true" : "false") << ",\n";
    out << "  \"cylinder_poisson_like\": " << (r.cylinder_poisson_like ? "true" : "false")
        << ",\n";
    out << "  \"separates_from_gue\": " << (r.separates_from_gue ? "true" : "false") << "\n";
    out << "}\n";
}

}  // namespace Marshal::Analysis

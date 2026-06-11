#pragma once
#include <cmath>
#include <vector>

namespace Marshal::Quotient {

// Reynolds projector weights for Q^x-invariant subspace on (S^1)^k product grid.
// Diagnostic implementation — see docs/Analysis/QuotientSpectrum.md (OPEN).
struct QuotientSpaceParams {
    int mesh = 12;
    int k_primes = 4;
};

inline std::vector<double> ReynoldsProjectorWeights(int mesh, int dim) {
    const int ncells = static_cast<int>(std::pow(static_cast<double>(mesh), dim));
    std::vector<double> w(static_cast<size_t>(ncells), 1.0 / static_cast<double>(ncells));
    return w;
}

inline double ProjectedRayleighGap(const std::vector<double>& eigenvalues,
                                   const std::vector<double>& gammas, int n_compare) {
    if (eigenvalues.empty() || gammas.empty()) return 1e9;
    const int n = std::min(n_compare, std::min(static_cast<int>(eigenvalues.size()),
                                              static_cast<int>(gammas.size())));
    double max_gap = 0;
    for (int i = 0; i < n; ++i) {
        const double g = std::sqrt(eigenvalues[static_cast<size_t>(i)]);
        const double rel = std::fabs(g - gammas[static_cast<size_t>(i)]) /
                           std::max(1.0, gammas[static_cast<size_t>(i)]);
        max_gap = std::max(max_gap, rel);
    }
    return max_gap;
}

}  // namespace Marshal::Quotient

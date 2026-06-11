#pragma once
#include <string>
#include <vector>
#include "Numerics/Real.hxx"

namespace Marshal::Analysis {

struct PairCorrelationReport {
    int n_zero_spacings = 0;
    int n_cylinder_spacings = 0;
    Real mean_zero_spacing = 0;
    Real mean_cylinder_spacing = 0;
    Real zero_spacing_var = 0;
    Real cylinder_spacing_var = 0;
    Real gue_spacing_l2_zero = 0;
    Real gue_spacing_l2_cylinder = 0;
    Real montgomery_r2_l2 = 0;
    Real cylinder_vs_gue_excess = 0;
    bool cylinder_poisson_like = false;
    bool zeros_gue_like = false;
    bool separates_from_gue = false;
};

Real gue_wigner_spacing_pdf(Real s);
Real montgomery_pair_r2(Real s);

std::vector<Real> normalized_nn_spacings(const std::vector<double>& levels);
std::vector<Real> cylinder_positive_levels(const std::vector<int>& primes, int n_levels);

Real spacing_distribution_l2_gue(const std::vector<Real>& norm_spacings, int bins = 40);
Real empirical_r2_l2_vs_montgomery(const std::vector<double>& unfolded, int bins = 50,
                                   Real s_max = 2.0L);

PairCorrelationReport compute_pair_correlation(const std::vector<double>& gammas,
                                               const std::vector<int>& primes, int n_cylinder,
                                               int max_zeros = 0);

void export_pair_correlation_json(const std::string& path, const PairCorrelationReport& r);

}  // namespace Marshal::Analysis

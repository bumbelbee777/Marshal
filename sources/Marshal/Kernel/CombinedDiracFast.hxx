#pragma once

#include "../Heat/ArchimedeanBoundary.hxx"
#include "../Heat/BerryKeatingOperator.hxx"
#include "../Heat/ConnesCouplingMode.hxx"
#include "../Numerics/Real.hxx"

#include <cstddef>
#include <vector>

namespace Marshal::Kernel {

struct CombinedDiracFastSpec {
    Marshal::Heat::BerryKeatingSpec bk;
    Real coupling_lambda = 0.5L;
    Marshal::Heat::ConnesCouplingMode coupling_mode = Marshal::Heat::ConnesCouplingMode::LogLadder;
    int kmax = 12;
    int arch_cap = 80;
    int combined_cap = 400;
    int sweep_steps = 24;
};

struct CombinedDiracCandidateResult {
    Real theta = 0;
    int boundary = 0;
    Real combined_action = 0;
    Real spectrum_rmse = 0;
    Real gue_spacing_l2 = 0;
    int n_modes = 0;
};

struct CombinedDiracConvergenceRow {
    int combined_cap = 0;
    int n_modes = 0;
    Real spectrum_rmse = 0;
    Real combined_action = 0;
    double elapsed_ms = 0;
};

/// Precomputed prime block, arch ladders, bridge indices, thread scratch (tile-aware).
class CombinedDiracWorkspace {
public:
    bool prepare(const CombinedDiracFastSpec& spec, const std::vector<int>& primes);
    bool prepare_with_thetas(const CombinedDiracFastSpec& spec, const std::vector<int>& primes,
                             const std::vector<Real>& thetas);

    int n_total() const { return n_total_; }
    int n_arch() const { return n_arch_; }
    int n_prime() const { return n_prime_; }
    int sweep_steps() const { return sweep_steps_; }
    Real theta_at(int idx) const;

    CombinedDiracCandidateResult eval_candidate(int theta_idx, int boundary_idx,
                                               const Marshal::Heat::ArchimedeanBoundarySpec& arch,
                                               Real scale_base, int n_scales,
                                               const std::vector<Real>& gammas_ld,
                                               int thread_id) const;

    CombinedDiracCandidateResult eval_at_theta(Real theta, int boundary_idx,
                                               const Marshal::Heat::ArchimedeanBoundarySpec& arch,
                                               Real scale_base, int n_scales,
                                               const std::vector<Real>& gammas_ld,
                                               int thread_id) const;

    CombinedDiracConvergenceRow probe_cap(int cap, int theta_idx, int boundary_idx,
                                          const Marshal::Heat::ArchimedeanBoundarySpec& arch,
                                          Real scale_base, int n_scales,
                                          const std::vector<Real>& gammas_ld,
                                          int thread_id) const;

private:
    int n_total_ = 0;
    int n_arch_ = 0;
    int n_prime_ = 0;
    int sweep_steps_ = 24;
    std::vector<Real> theta_values_;
    double coupling_ = 0;
    CombinedDiracFastSpec spec_{};
    std::vector<int> primes_;

    std::vector<double> prime_diag_;
    std::vector<double> prime_off_;
    std::vector<double> prime_u_;
    std::vector<double> prime_w_;

    std::vector<std::vector<double>> arch_ladder_;
    std::vector<std::vector<int>> bridge_j_;
    std::vector<std::vector<double>> bridge_base_;

    mutable std::vector<std::vector<double>> thread_H_;

    void build_arch_for_theta(Real theta, std::vector<double>& al, std::vector<int>& bj,
                              std::vector<double>& bb) const;

    CombinedDiracCandidateResult eval_from_ladder(
        const std::vector<double>& al, const std::vector<int>& bj, const std::vector<double>& bb,
        Real theta, int boundary_idx, const Marshal::Heat::ArchimedeanBoundarySpec& arch,
        Real scale_base, int n_scales, const std::vector<Real>& gammas_ld, int thread_id) const;
};

/// LUT-backed SIMD multi-scale heat trace on eigenvalue vector (double lanes).
double FusedSpectralActionHeat(const double* evals, int n, double scale_base, int n_scales);

void SymmetricEigenvalues(int n, const double* A, double* evals_out);

void SymmetricEigenvaluesInplace(int n, double* A, double* evals_out);

CombinedDiracConvergenceRow probe_combined_cap(const CombinedDiracFastSpec& base_spec,
                                               const std::vector<int>& primes, int cap,
                                               int theta_idx, int boundary_idx,
                                               const Marshal::Heat::ArchimedeanBoundarySpec& arch,
                                               Real scale_base, int n_scales,
                                               const std::vector<Real>& gammas_ld);

}  // namespace Marshal::Kernel

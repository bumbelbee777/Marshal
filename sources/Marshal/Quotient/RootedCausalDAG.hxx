#pragma once

#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Quotient {

/// Parameters for the rooted causal DAG on adele-class truncation.
struct RootedCausalDAGParams {
    int mesh = 8;
    int k_primes = 4;
    int max_scale_power = 2;
    Real coupling_lambda = 0.5L;
};

/// One rung of the DAG limit ladder.
struct RootedCausalDAGPoint {
    int mesh = 0;
    int k_primes = 0;
    int n_vertices = 0;
    int n_edges = 0;
    Real dag_spectrum_rmse = 0;
    Real blended_rmse = 0;
    Real dag_weight = 0;
};

/// Full limit report for Lean cert emission.
struct RootedCausalDAGReport {
    std::string proof_status = "OPEN";
    std::string limit_verdict;
    bool monotone_rmse_decrease = false;
    bool lean_emit_ready = false;
    Real final_blended_rmse = 0;
    Real resolvent_gap = 0;
    std::vector<RootedCausalDAGPoint> points;
};

/// Build weighted adjacency row-major (n x n) for the scaling DAG.
std::vector<Real> build_dag_adjacency(const RootedCausalDAGParams& params,
                                      const std::vector<int>& primes, int& n_vertices,
                                      int& n_edges);

/// Laplacian eigenvalues (smallest modes) via Jacobi on L = D - A.
std::vector<Real> dag_laplacian_eigenvalues(const std::vector<Real>& adjacency, int n,
                                              int n_modes);

/// RMSE of sqrt(eigenvalues) vs Riemann ordinates.
Real spectrum_rmse_vs_gammas(const std::vector<Real>& eigenvalues_sq,
                             const std::vector<Real>& gammas, int n_compare);

/// Blend DAG + crossed-product eigenvalue squares; dag_weight in [0,1].
std::vector<Real> blend_spectra(const std::vector<Real>& dag_eigs_sq,
                                const std::vector<Real>& crossed_eigs_sq, Real dag_weight);

/// Run mesh/prime ladder; dag_weight decreases as resolution increases.
RootedCausalDAGReport run_rooted_dag_limit(const RootedCausalDAGParams& base,
                                           const std::vector<int>& primes,
                                           const std::vector<Real>& gammas,
                                           const std::vector<int>& mesh_ladder);

}  // namespace Marshal::Quotient

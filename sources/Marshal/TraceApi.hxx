#pragma once
#include <vector>
#include "Config.hxx"
#include "Heat/ArchimedeanBoundary.hxx"
#include "Numerics/TestFunctions.hxx"
#include "Heat/PrimeCatalog.hxx"

namespace Marshal {

struct TraceResult {
    Real lhs = 0;
    Real rhs = 0;
    Real poles = 0;
    Real arch = 0;
    Real prime = 0;
    Real heat_prime_ab = 0;
    Real trivial = 0;
    Real residual_kahan = 0;
    Real residual() const;
};

TraceResult EvaluateTrace(const TestFunction& tf, Real sigma,
                          const std::vector<double>& gammas,
                          const std::vector<Real>& gammas_ld,
                          const Heat::PrimeCatalog& cat, ZeroKernel zk, SimdLevel simd,
                          Real eps, bool include_trivial, bool precision_mode = false,
                          int arch_pts = 400000, bool quick_sinc2_arch = false,
                          const Heat::ArchimedeanBoundarySpec* arch_spec = nullptr);

// Prefix evaluation: sum only the first n_zeros ordinates (no vector copy).
TraceResult EvaluateTracePrefix(const TestFunction& tf, Real sigma, const double* gammas,
                                size_t n_zeros, const Real* gammas_ld, size_t n_zeros_ld,
                                const Heat::PrimeCatalog& cat, ZeroKernel zk, SimdLevel simd,
                                Real eps, bool include_trivial, bool precision_mode = false,
                                int arch_pts = 400000, bool quick_sinc2_arch = false,
                                const Heat::ArchimedeanBoundarySpec* arch_spec = nullptr);

struct ArchSinc2AuditPoint {
    Real L = 0;
    int n_pts = 0;
    Real arch = 0;
    Real richardson_est = 0;
};

std::vector<ArchSinc2AuditPoint> AuditArchSinc2(Real T, Real kappa, SimdLevel simd,
                                                bool precision_mode);

Real ArchimedeanBaselineForTestFunction(const TestFunction& tf, Real sigma, SimdLevel simd,
                                        bool precision_mode, int arch_pts, Real eps,
                                        bool quick_sinc2 = false);

Real ArchSinc2Adaptive(Real T, Real kappa, SimdLevel simd, bool precision_mode, int arch_pts,
                     Real eps);

struct ArchSinc2ConvergeResult {
    Real T = 0;
    Real kappa = 0;
    Real arch = 0;
    Real arch_target = 0;
    int n_pts_final = 0;
    Real L_final = 0;
    bool converged = false;
    std::vector<ArchSinc2AuditPoint> ladder;
};

ArchSinc2ConvergeResult ArchSinc2Converge(Real T, Real kappa, Real target, SimdLevel simd,
                                          bool precision_mode, int arch_pts_max);

inline Real SigmaTrace(const Config& cfg) {
    return cfg.sigma_trace > 0 ? cfg.sigma_trace : cfg.sigma;
}

}  // namespace Marshal

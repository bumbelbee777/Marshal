#pragma once
#include <vector>
#include "Config.hxx"
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
                          int arch_pts = 400000, bool quick_sinc2_arch = false);

inline Real SigmaTrace(const Config& cfg) {
    return cfg.sigma_trace > 0 ? cfg.sigma_trace : cfg.sigma;
}

}  // namespace Marshal

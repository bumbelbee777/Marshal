#pragma once
#include <cstddef>
#include <vector>
#include "Config.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Induction::detail {

Real arch_quadrature_floor(Real sigma, SimdLevel simd, bool precision_mode, int arch_pts);
Real zero_sum_h(const TestFunction& tf, Real sigma,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    ZeroKernel zk, SimdLevel simd);
Real zero_sum_h_batched(const TestFunction& tf, Real sigma,
    const double* gammas, size_t n, SimdLevel simd);
Real zero_sum_h_ld_batched(const TestFunction& tf, const Real* gammas, size_t n);

}  // namespace Marshal::Induction::detail

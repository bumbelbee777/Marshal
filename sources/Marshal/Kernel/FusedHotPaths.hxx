#pragma once
#include "../Numerics/Real.hxx"
#include "SimdMicrokernels.hxx"

namespace Marshal::Kernel {

// Fused: sum_k exp(-t * (2*pi*n/log_p)^2) for n=1..nmax (Poisson heat trace modes).
Real FusedHeatTracePoisson(Real t, Real inv_log_p, int nmax);

// Fused: sum_k (log_p/ppow) * exp(-t*(k*log_p)^2) for prime trace block.
Real FusedPrimeBlock(Real t, Real log_p, int k_cap);

// Fused: 2 * sum_i exp(-t * gamma_i^2) over zero chunk [off, off+len).
Real FusedZeroGaussianSum(const Real* gamma_ld, size_t off, size_t len, Real t);

#if defined(MARSHAL_HAVE_AVX2)
// SIMD zero chunk when gamma stored as double.
// heat_t: exponent coefficient, sum 2*exp(-heat_t * gamma^2)
double FusedZeroGaussianSum4(const double* gamma, size_t n, double heat_t);
#endif

// Batch oracle LHS: out_lhs[j] = 2 * sum_i exp(-t[j] * gamma[i]^2).
void FusedZeroGaussianSumBatch(const double* gamma, size_t n,
                               const double* t_values, size_t nt,
                               double* out_lhs);

// Batch Poisson heat traces: out[p] = FusedHeatTracePoisson(t, inv_log_p[p], nmax).
void FusedHeatTracePoissonSoA(Real t, const double* inv_log_p, size_t n_primes,
                              int nmax, double* out);

// Batch prime blocks at fixed t: out[p] = FusedPrimeBlock(t, log_p[p], k_cap[p]).
void FusedPrimeBlockSoA(Real t, const double* log_p, const int* k_cap,
                            size_t n_primes, double* out);

}  // namespace Marshal::Kernel

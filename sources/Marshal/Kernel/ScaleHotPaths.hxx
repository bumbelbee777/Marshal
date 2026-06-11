#pragma once
#include <cstddef>
#include <vector>
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Kernel {

constexpr size_t kScalePrimeThreshold = 5000;
constexpr size_t kScaleZeroThreshold = 10000;

// Gauss h_hat prime block: sum_k (log_p/p^{k/2}) * 2 * h_hat(k log p), unnormalized.
Real FusedGaussPrimeHatBlock(Real sigma, Real log_p, int k_cap, Real eps);

void FusedGaussPrimeHatBlockSoA(Real sigma, const Real* log_p, const int* k_cap,
                                  size_t n_primes, Real eps, double* out);

// AB heat block at tau = 1/(2 sigma^2): sum_k (log_p/p^{k/2}) * exp(-u^2/(4 tau)).
Real FusedAbHeatBlock(Real tau, Real log_p, int k_cap, Real eps);

void FusedAbHeatBlockSoA(Real tau, const Real* log_p, const int* k_cap,
                         size_t n_primes, Real eps, double* out);

void AccumulateGaussPrimeBlocks(const Heat::PrimeCatalog& cat, Real sigma, Real tau,
                                Real eps, Real& prime_raw, Real& heat_raw);

// OpenMP-chunked AVX2 zero sum: 2 * sum exp(-t * gamma^2).
double FusedZeroGaussianSumScale(const double* gamma, size_t n, double heat_t);

void FusedZeroGaussianSumBatchScale(const double* gamma, size_t n,
                                    const double* t_values, size_t nt, double* out_lhs);

}  // namespace Marshal::Kernel

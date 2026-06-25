#pragma once
#include <cstddef>
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"
#include "ScaleHotPaths.hxx"

namespace Marshal::Kernel {

inline size_t EffectiveScalePrimeThreshold(bool force_scale) {
    return force_scale ? 0 : kScalePrimeThreshold;
}

inline size_t EffectiveScaleZeroThreshold(bool force_scale) {
    return force_scale ? 0 : kScaleZeroThreshold;
}

// Streaming SoA accumulation: Gauss scale path or fused AB + per-prime Weil blocks.
void AccumulatePrimeBlocksStreaming(const Heat::PrimeCatalog& cat, const TestFunction& tf,
                                    Real tau, Real eps, bool use_kahan, bool force_scale,
                                    Real& prime_out, Real& heat_out);

// Batch heat prime blocks across tau values (OpenMP over tau).
void FusedAbHeatBlockBatch(const Real* log_p, const int* k_cap, size_t n_primes, Real eps,
                           const double* tau_values, size_t ntau, double* out_row_major);

}  // namespace Marshal::Kernel

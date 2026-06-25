#include "ScaleHotPaths.hxx"

#include <algorithm>
#include "Kernel/FusedHotPaths.hxx"
#include "Kernel/PairwiseSum.hxx"
#include "Kernel/SimdMicrokernels.hxx"
#include "Kernel/SoaLayouts.hxx"
#include "Numerics/Real.hxx"
#include <cmath>
#ifdef _OPENMP
#include <omp.h>
#endif
#if defined(MARSHAL_HAVE_AVX2)
#include <immintrin.h>
#endif

namespace Marshal::Kernel {

static constexpr Real kSqrt2Pi = MARSHAL_LIT(2.506628274631000502415165145310982062);

Real FusedGaussPrimeHatBlock(Real sigma, Real log_p, int k_cap, Real eps) {
    const Real hscale = sigma * kSqrt2Pi;
    const Real inv_2ss = 0.5L * sigma * sigma;
#if defined(MARSHAL_HAVE_AVX2)
    const double coeff = -static_cast<double>(inv_2ss);
    const double scale = 2.0 * static_cast<double>(hscale);
    const double raw =
        PrimePowerExpSum(static_cast<double>(log_p), k_cap, coeff, static_cast<double>(eps));
    return static_cast<Real>(raw * scale);
#else
    Real sum = 0;
    Real ppow = MarshalExp(log_p * 0.5L);
    for (int k = 1; k <= k_cap; ++k) {
        const Real u = static_cast<Real>(k) * log_p;
        const Real term = (log_p / ppow) * 2.0L * hscale * MarshalExp(-inv_2ss * u * u);
        sum += term;
        if (term < eps) break;
        ppow *= MarshalExp(log_p * 0.5L);
    }
    return sum;
#endif
}

Real FusedAbHeatBlock(Real tau, Real log_p, int k_cap, Real eps) {
#if defined(MARSHAL_HAVE_AVX2)
    const double coeff = -1.0 / (4.0 * static_cast<double>(tau));
    return static_cast<Real>(PrimePowerExpSum(static_cast<double>(log_p), k_cap, coeff,
                                            static_cast<double>(eps)));
#else
    const Real inv4t = 1.0L / (4.0L * tau);
    Real sum = 0;
    Real ppow = MarshalExp(log_p * 0.5L);
    for (int k = 1; k <= k_cap; ++k) {
        const Real u = static_cast<Real>(k) * log_p;
        const Real term = (log_p / ppow) * MarshalExp(-u * u * inv4t);
        sum += term;
        if (term < eps) break;
        ppow *= MarshalExp(log_p * 0.5L);
    }
    return sum;
#endif
}

void FusedGaussPrimeHatBlockSoA(Real sigma, const Real* log_p, const int* k_cap,
                                size_t n_primes, Real eps, double* out) {
    if (!log_p || !k_cap || !out || n_primes == 0) return;
#ifdef _OPENMP
    #pragma omp parallel for schedule(static, 64) if(n_primes > 256)
#endif
    for (ptrdiff_t p = 0; p < static_cast<ptrdiff_t>(n_primes); ++p) {
        const size_t i = static_cast<size_t>(p);
        out[i] = static_cast<double>(
            FusedGaussPrimeHatBlock(sigma, log_p[i], k_cap[i], eps));
    }
}

void FusedAbHeatBlockSoA(Real tau, const Real* log_p, const int* k_cap,
                         size_t n_primes, Real eps, double* out) {
    if (!log_p || !k_cap || !out || n_primes == 0) return;
#ifdef _OPENMP
    #pragma omp parallel for schedule(static, 64) if(n_primes > 256)
#endif
    for (ptrdiff_t p = 0; p < static_cast<ptrdiff_t>(n_primes); ++p) {
        const size_t i = static_cast<size_t>(p);
        out[i] = static_cast<double>(
            FusedAbHeatBlock(tau, log_p[i], k_cap[i], eps));
    }
}

void AccumulateGaussPrimeBlocks(const Heat::PrimeCatalog& cat, Real sigma, Real tau,
                                Real eps, Real& prime_raw, Real& heat_raw) {
    const size_t n = cat.p.size();
    if (n == 0) {
        prime_raw = heat_raw = 0;
        return;
    }
    const int nbatches = static_cast<int>((n + kPrimeBatch - 1) / kPrimeBatch);
    std::vector<Real> wp;
    std::vector<Real> hp;
#ifdef _OPENMP
    wp.resize(static_cast<size_t>(omp_get_max_threads()), 0.0L);
    hp.resize(static_cast<size_t>(omp_get_max_threads()), 0.0L);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        Real wl = 0;
        Real hl = 0;
        #pragma omp for schedule(static, kPrimeBatch)
        for (int b = 0; b < nbatches; ++b) {
            const size_t i0 = static_cast<size_t>(b) * kPrimeBatch;
            const size_t i1 = std::min(i0 + kPrimeBatch, n);
            const size_t bn = i1 - i0;
            std::vector<double> wbuf(bn);
            std::vector<double> hbuf(bn);
            FusedGaussPrimeHatBlockSoA(sigma, cat.logp.data() + i0,
                cat.kmax_adaptive.data() + i0, bn, eps, wbuf.data());
            FusedAbHeatBlockSoA(tau, cat.logp.data() + i0,
                cat.kmax_adaptive.data() + i0, bn, eps, hbuf.data());
            for (size_t j = 0; j < bn; ++j) {
                wl += static_cast<Real>(wbuf[j]);
                hl += static_cast<Real>(hbuf[j]);
            }
        }
        wp[static_cast<size_t>(tid)] = wl;
        hp[static_cast<size_t>(tid)] = hl;
    }
    prime_raw = PairwiseSum(wp);
    heat_raw = PairwiseSum(hp);
#else
    std::vector<double> wbuf(n);
    std::vector<double> hbuf(n);
    FusedGaussPrimeHatBlockSoA(sigma, cat.logp.data(), cat.kmax_adaptive.data(), n, eps, wbuf.data());
    FusedAbHeatBlockSoA(tau, cat.logp.data(), cat.kmax_adaptive.data(), n, eps, hbuf.data());
    prime_raw = 0;
    heat_raw = 0;
    for (size_t i = 0; i < n; ++i) {
        prime_raw += static_cast<Real>(wbuf[i]);
        heat_raw += static_cast<Real>(hbuf[i]);
    }
#endif
}

#if defined(MARSHAL_HAVE_AVX2)
double FusedZeroGaussianSumScale(const double* gamma, size_t n, double heat_t) {
    if (!gamma || n == 0) return 0;
    const size_t nchunks = (n + kZeroChunk - 1) / kZeroChunk;
    std::vector<double> parts;
#ifdef _OPENMP
    parts.resize(static_cast<size_t>(omp_get_max_threads()), 0.0);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double local = 0;
        #pragma omp for schedule(static)
        for (int c = 0; c < static_cast<int>(nchunks); ++c) {
            const size_t off = static_cast<size_t>(c) * kZeroChunk;
            const size_t len = std::min(static_cast<size_t>(kZeroChunk), n - off);
            local += FusedZeroGaussianSum4(gamma + off, len, heat_t) * 0.5;
        }
        parts[static_cast<size_t>(tid)] = local;
    }
    double sum = 0;
    for (double v : parts) sum += v;
    return 2.0 * sum;
#else
    return FusedZeroGaussianSum4(gamma, n, heat_t);
#endif
}
#else
double FusedZeroGaussianSumScale(const double* gamma, size_t n, double heat_t) {
    double sum = 0;
#ifdef _OPENMP
    #pragma omp parallel for reduction(+:sum) schedule(static, 4096) if(n > 8192)
#endif
    for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(n); ++i)
        sum += std::exp(-heat_t * gamma[static_cast<size_t>(i)] * gamma[static_cast<size_t>(i)]);
    return 2.0 * sum;
}
#endif

void FusedZeroGaussianSumBatchScale(const double* gamma, size_t n,
                                    const double* t_values, size_t nt, double* out_lhs) {
    if (!gamma || !t_values || !out_lhs || n == 0 || nt == 0) return;
    if (n < kScaleZeroThreshold) {
        FusedZeroGaussianSumBatch(gamma, n, t_values, nt, out_lhs);
        return;
    }
#ifdef _OPENMP
    #pragma omp parallel for schedule(static) if(nt > 4)
#endif
    for (ptrdiff_t j = 0; j < static_cast<ptrdiff_t>(nt); ++j) {
        const double t = t_values[static_cast<size_t>(j)];
        out_lhs[static_cast<size_t>(j)] = FusedZeroGaussianSumScale(gamma, n, t);
    }
}

}  // namespace Marshal::Kernel

#include "Induction.hxx"
#include "Heat/Common.hxx"
#include "Compat.hxx"
#include "Cert/Schema.hxx"
#include "Cert/Verdict.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Quotient/QuotientToy.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Induction {
namespace detail {

constexpr Real kEulerGamma  = 0.577215664901532860606512090082402431L;
#include "psi_lut.inc"
#include "gh64.inc"
#include "gh128.inc"
#include "gh256.inc"
#include "gh512.inc"
#include "gh1024.inc"

inline Real h_gauss(Real t, Real sigma) noexcept {
    return expl(-t * t / (2.0L * sigma * sigma));
}

inline Real h_hat(Real u, Real sigma) noexcept {
    return sigma * kSqrt2Pi * expl(-0.5L * sigma * sigma * u * u);
}


inline Real psi_lut_lookup(Real y) noexcept {
    if (y <= kPsiLutYMin) return kPsiLut[0];
    if (y >= kPsiLutYMax) return kPsiLut[kPsiLutN - 1];
    const Real t = (y - kPsiLutYMin) * kPsiLutInvStep;
    const int i = static_cast<int>(t);
    const Real f = t - static_cast<Real>(i);
    const Real a = kPsiLut[i];
    const Real b = kPsiLut[std::min(i + 1, kPsiLutN - 1)];
    return a + f * (b - a);
}

inline Real re_psi_quarter_plus_iy(Real y) {
    const Real ay = fabsl(y);
    if (ay >= kPsiLutYMin && ay <= kPsiLutYMax) return psi_lut_lookup(ay);
    if (ay >= 40.0L) {
        const Real z2 = ay * ay + 0.0625L;
        const Real z4 = z2 * z2;
        const Real z6 = z4 * z2;
        return 0.5L * logl(z2) - 1.0L / (8.0L * z2) + 1.0L / (48.0L * z4)
             - 1.0L / (192.0L * z6);
    }
    Real sum = -kEulerGamma;
    const Real y2 = ay * ay;
    for (int n = 0; n < 400000; ++n) {
        const Real an = static_cast<Real>(n) + 0.25L;
        const Real term1 = 1.0L / (static_cast<Real>(n) + 1.0L);
        const Real term2 = an / (an * an + y2);
        sum += term1 - term2;
        if (n > 12000 && fabsl(term1 - term2) < 1e-28L) break;
    }
    return sum;
}

// =============================================================================
// Archimedean (GH Richardson + Simpson fallback)
// =============================================================================

Real arch_psi_series(Real y, bool high_precision = false) {
    if (high_precision) return re_psi_quarter_plus_iy(y);
    const Real ay = fabsl(y);
    const Real y2 = ay * ay;
    Real sum = -kEulerGamma;
    for (int k = 0; k < 400000; ++k) {
        const Real ak = static_cast<Real>(k) + 0.25L;
        const Real t1 = 1.0L / (static_cast<Real>(k) + 1.0L);
        const Real t2 = ak / (ak * ak + y2);
        sum += t1 - t2;
        if (k > 8000 && fabsl(t1 - t2) < 1e-22L) break;
    }
    return sum;
}

Real arch_gauss_hermite_n(Real sigma, const Real* x, const Real* w, int n,
                                 bool precise_psi = false) {
    const Real scale = sigma * sqrtl(2.0L) / (2.0L * kPi);
    const Real inv_sqrt2 = 1.0L / sqrtl(2.0L);
    const Real log_pi = logl(kPi);
    Heat::Kahan acc;
    if (precise_psi) {
        #ifdef _OPENMP
        #pragma omp parallel
        {
            Heat::Kahan local;
            #pragma omp for
            for (int i = 0; i < n; ++i) {
                const Real y = x[i] * sigma * inv_sqrt2;
                local.add(w[i] * (re_psi_quarter_plus_iy(y) - log_pi));
            }
            #pragma omp critical
            { acc.add(local.total()); }
        }
        #else
        for (int i = 0; i < n; ++i) {
            const Real y = x[i] * sigma * inv_sqrt2;
            acc.add(w[i] * (re_psi_quarter_plus_iy(y) - log_pi));
        }
        #endif
    } else {
        #ifdef _OPENMP
        #pragma omp parallel
        {
            Heat::Kahan local;
            #pragma omp for
            for (int i = 0; i < n; ++i) {
                const Real y = x[i] * sigma * inv_sqrt2;
                local.add(w[i] * (re_psi_quarter_plus_iy(y) - log_pi));
            }
            #pragma omp critical
            { acc.add(local.total()); }
        }
        #else
        for (int i = 0; i < n; ++i) {
            const Real y = x[i] * sigma * inv_sqrt2;
            acc.add(w[i] * (re_psi_quarter_plus_iy(y) - log_pi));
        }
        #endif
    }
    return scale * acc.total();
}

Real arch_simpson_check(Real sigma, int N, SimdLevel simd, bool precise_psi = false);

Real arch_gauss_richardson_1024(Real sigma, Real& drift_out, bool precise_psi = false) {
    const Real a64   = arch_gauss_hermite_n(sigma, kGHX, kGHW, kGHN, precise_psi);
    const Real a128  = arch_gauss_hermite_n(sigma, kGH128X, kGH128W, kGH128N, precise_psi);
    const Real a256  = arch_gauss_hermite_n(sigma, kGH256X, kGH256W, kGH256N, precise_psi);
    const Real a512  = arch_gauss_hermite_n(sigma, kGH512X, kGH512W, kGH512N, precise_psi);
    const Real a1024 = arch_gauss_hermite_n(sigma, kGH1024X, kGH1024W, kGH1024N, precise_psi);
    const Real e512_256  = a512 - a256;
    const Real e256_128  = a256 - a128;
    const Real e128_64   = a128 - a64;
    const Real e1024_512 = a1024 - a512;
    drift_out = std::max({fabsl(e1024_512), fabsl(e512_256), fabsl(e256_128)});
    return a1024 + e1024_512 / 3.0L
         + (e1024_512 - e512_256) / 15.0L
         + (e1024_512 - 2.0L * e512_256 + e256_128) / 105.0L
         + (e1024_512 - 3.0L * e512_256 + 3.0L * e256_128 - e128_64) / 945.0L;
}

Real arch_simpson_adaptive(Real sigma, int N0, int N_max, Real eps, SimdLevel simd,
                                  bool precise_psi = false) {
    int N = N0;
    Real prev = arch_simpson_check(sigma, N, simd, precise_psi);
    while (N < N_max) {
        const int N2 = std::min(2 * N - 1, N_max);
        const Real next = arch_simpson_check(sigma, N2, simd, precise_psi);
        const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
        if (rel < eps) return next;
        prev = next;
        N = N2;
    }
    return prev;
}

Real archimedean_exact(Real sigma, SimdLevel simd, bool precision_mode,
                       int arch_pts, Real eps) {
    Real drift = 0;
    const Real gh = arch_gauss_richardson_1024(sigma, drift, precision_mode);
    const Real gh_tol = precision_mode ? 1e-14L : 1e-12L;
    if (drift < gh_tol) return gh;
    const Real sim_tol = precision_mode ? 1e-16L : std::max(eps, 1e-14L);
    const int N0 = sigma >= 4.0L ? 128001 : 64001;
    return arch_simpson_adaptive(sigma, N0, arch_pts, sim_tol, simd, precision_mode);
}

Real arch_quadrature_floor(Real sigma, SimdLevel simd, bool precision_mode, int arch_pts) {
    Real drift = 0;
    arch_gauss_richardson_1024(sigma, drift, precision_mode);
    if (drift < 1e-12L) return drift;
    const Real a1 = arch_simpson_check(sigma, 128001, simd, precision_mode);
    const Real a2 = arch_simpson_check(sigma, std::min(512001, arch_pts), simd, precision_mode);
    return fabsl(a2 - a1);
}

// =============================================================================
// Zero sum (fused, thread-array reduction)
// =============================================================================

Real zero_sum_gauss_avx2(Real sigma, const double* gammas, size_t n) {
#if defined(WEIL_HAVE_AVX2)
    const double inv_2ss = 1.0 / (2.0 * static_cast<double>(sigma * sigma));
    const int nchunks = static_cast<int>((n + kZeroChunk - 1) / kZeroChunk);
    std::vector<Real> parts;
    #ifdef _OPENMP
    parts.resize(omp_get_max_threads(), 0.0L);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        Real local = 0.0L;
        #pragma omp for schedule(static)
        for (int c = 0; c < nchunks; ++c) {
            const size_t off = static_cast<size_t>(c) * kZeroChunk;
            const size_t len = std::min(static_cast<size_t>(kZeroChunk), n - off);
            const double* src = gammas + off;
            size_t i = 0;
            for (; i + 4 <= len; i += 4) {
                __m256d g = _mm256_loadu_pd(src + i);
                __m256d e = exp_neg_sq4_avx2(g, inv_2ss);
                alignas(32) double t[4];
                _mm256_storeu_pd(t, e);
                local += static_cast<Real>(t[0] + t[1] + t[2] + t[3]);
            }
            for (; i < len; ++i)
                local += static_cast<Real>(std::exp(-inv_2ss * src[i] * src[i]));
        }
        parts[static_cast<size_t>(tid)] = local;
    }
    #else
    Real local = 0.0L;
    for (size_t off = 0; off < n; off += kZeroChunk) {
        const size_t len = std::min(static_cast<size_t>(kZeroChunk), n - off);
        const double* src = gammas + off;
        for (size_t i = 0; i + 4 <= len; i += 4) {
            __m256d g = _mm256_loadu_pd(src + i);
            __m256d e = exp_neg_sq4_avx2(g, inv_2ss);
            alignas(32) double t[4];
            _mm256_storeu_pd(t, e);
            local += static_cast<Real>(t[0] + t[1] + t[2] + t[3]);
        }
        for (size_t i = (len / 4) * 4; i < len; ++i)
            local += static_cast<Real>(std::exp(-inv_2ss * src[i] * src[i]));
    }
    parts.push_back(local);
    #endif
    return 2.0L * pairwise_sum(parts);
#else
    (void)sigma; (void)gammas; (void)n;
    return 0.0L;
#endif
}

Real zero_sum_h_ld_batched(const TestFunction& tf, const Real* gammas, size_t n) {
    if (n == 0) return 0.0L;
    const int nchunks = static_cast<int>((n + kZeroChunk - 1) / kZeroChunk);
    std::vector<Real> parts;
    #ifdef _OPENMP
    parts.resize(omp_get_max_threads(), 0.0L);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        Heat::Kahan local;
        #pragma omp for schedule(static)
        for (int c = 0; c < nchunks; ++c) {
            const size_t off = static_cast<size_t>(c) * kZeroChunk;
            const size_t len = std::min(static_cast<size_t>(kZeroChunk), n - off);
            for (size_t i = 0; i < len; ++i)
                local.add(tf.h(gammas[off + i]));
        }
        parts[static_cast<size_t>(tid)] = local.total();
    }
    #else
    {
        Heat::Kahan local;
        for (size_t i = 0; i < n; ++i)
            local.add(tf.h(gammas[i]));
        parts.push_back(local.total());
    }
    #endif
    Heat::Kahan merge;
    for (Real p : parts) merge.add(p);
    return 2.0L * merge.total();
}

Real zero_sum_h_batched(const TestFunction& tf, Real sigma,
                               const double* gammas, size_t n, SimdLevel simd) {
    if (n == 0) return 0.0L;
    if (std::string(tf.name()) == "gauss" && simd == SimdLevel::AVX2)
        return zero_sum_gauss_avx2(sigma, gammas, n);
    const int nchunks = static_cast<int>((n + kZeroChunk - 1) / kZeroChunk);
    std::vector<Real> parts;
    #ifdef _OPENMP
    parts.resize(omp_get_max_threads(), 0.0L);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        Real local = 0.0L;
        #pragma omp for schedule(static)
        for (int c = 0; c < nchunks; ++c) {
            const size_t off = static_cast<size_t>(c) * kZeroChunk;
            const size_t len = std::min(static_cast<size_t>(kZeroChunk), n - off);
            for (size_t i = 0; i < len; ++i)
                local += tf.h(static_cast<Real>(gammas[off + i]));
        }
        parts[static_cast<size_t>(tid)] = local;
    }
    #else
    parts.push_back(0.0L);
    for (size_t i = 0; i < n; ++i)
        parts[0] += tf.h(static_cast<Real>(gammas[i]));
    #endif
    return 2.0L * pairwise_sum(parts);
}

Real zero_sum_h(const TestFunction& tf, Real sigma,
                       const std::vector<double>& gammas,
                       const std::vector<Real>& gammas_ld,
                       ZeroKernel zk, SimdLevel simd) {
    if (zk == ZeroKernel::LongDouble && !gammas_ld.empty())
        return zero_sum_h_ld_batched(tf, gammas_ld.data(), gammas_ld.size());
    return zero_sum_h_batched(tf, sigma, gammas.data(), gammas.size(), simd);
}

Real trivial_zero_sum(const TestFunction& tf, int K = 500) {
    Real s = 0.0L;
    for (int m = 0; m < K; ++m)
        s += tf.h(static_cast<Real>(2 * m + 1));
    return 2.0L * s;
}

// =============================================================================
// Formula terms
// =============================================================================

Real pole_terms(const TestFunction& tf, Real sigma) {
    if (std::string(tf.name()) == "gauss")
        return 2.0L * expl(1.0L / (8.0L * sigma * sigma));
    return 0.0L;
}

Real arch_simpson(Real sigma, int N, SimdLevel, bool precise_psi = false) {
    const Real tau = TauFromSigma(sigma);
    const Real L = 10.0L / sqrtl(tau);
    const Real dx = 2.0L * L / static_cast<Real>(N - 1);
    const Real log_pi = logl(kPi);
    Heat::Kahan acc;
    #ifdef _OPENMP
    #pragma omp parallel
    {
        Heat::Kahan local;
        #pragma omp for
        for (int i = 0; i < N; ++i) {
            const Real x = -L + static_cast<Real>(i) * dx;
            const Real y = x * 0.5L;
            const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
            const Real f = h_gauss(x, sigma) * (psi - log_pi);
            Real w = (i == 0 || i == N - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
            local.add(w * f);
        }
        #pragma omp critical
        { acc.add(local.total()); }
    }
    #else
    for (int i = 0; i < N; ++i) {
        const Real x = -L + static_cast<Real>(i) * dx;
        const Real y = x * 0.5L;
        const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
        const Real f = h_gauss(x, sigma) * (psi - log_pi);
        Real w = (i == 0 || i == N - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
        acc.add(w * f);
    }
    #endif
    return (dx / 3.0L) * acc.total() / (2.0L * kPi);
}

Real arch_simpson_check(Real sigma, int N, SimdLevel simd, bool precise_psi) {
    const Real a1 = arch_simpson(sigma, N, simd, precise_psi);
    const Real a2 = arch_simpson(sigma, 2 * N - 1, simd, precise_psi);
    return a2 + (a2 - a1) / 15.0L;
}

// =============================================================================
// Zero loader
// =============================================================================

}  // namespace detail
}  // namespace Marshal::Induction

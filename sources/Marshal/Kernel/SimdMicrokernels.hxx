#pragma once
// Branchless LUT-backed SIMD exp primitives. Domain: [kExpDomainLo, kExpDomainHi].
#include "../../Generated/exp_coeffs.inc"

#if defined(MARSHAL_HAVE_AVX2)
#include <cmath>
#include <immintrin.h>
#endif

namespace Marshal::Kernel {

#if defined(MARSHAL_HAVE_AVX2)

inline __m256d ExpPolyEval4(__m256d x) {
    __m256d p = _mm256_set1_pd(kExpPolyCoeff[0]);
    for (int i = 1; i <= kExpPolyDegree; ++i)
        p = _mm256_add_pd(_mm256_mul_pd(p, x), _mm256_set1_pd(kExpPolyCoeff[i]));
    return p;
}

inline __m256d Exp4(__m256d x) {
    const __m256d lo = _mm256_set1_pd(kExpDomainLo);
    const __m256d hi = _mm256_set1_pd(kExpDomainHi);
    const __m256d zero = _mm256_setzero_pd();
    x = _mm256_min_pd(_mm256_max_pd(x, lo), hi);
    __m256d r = ExpPolyEval4(x);
    const __m256d mask = _mm256_cmp_pd(x, lo, _CMP_LE_OQ);
    return _mm256_blendv_pd(r, zero, mask);
}

inline double Exp1(double x) {
    const __m256d v = Exp4(_mm256_set1_pd(x));
    alignas(32) double out[4];
    _mm256_store_pd(out, v);
    return out[0];
}

inline __m256d ExpNegSq4(__m256d g, double inv_2ss) {
    const __m256d c = _mm256_set1_pd(-inv_2ss);
    __m256d v = _mm256_mul_pd(g, g);
    v = _mm256_mul_pd(v, c);
    return Exp4(v);
}

inline double HorizontalSum4(__m256d v) {
    const __m128d lo = _mm256_castpd256_pd128(v);
    const __m128d hi = _mm256_extractf128_pd(v, 1);
    __m128d s = _mm_add_pd(lo, hi);
    s = _mm_hadd_pd(s, s);
    return _mm_cvtsd_f64(s);
}

// Fused exp(-c * x^2) reduction over 8 lanes (two AVX registers).
inline double ExpNegSqReduce8(const double* x, double c) {
    __m256d acc = _mm256_setzero_pd();
    acc = _mm256_add_pd(acc, ExpNegSq4(_mm256_loadu_pd(x), c));
    acc = _mm256_add_pd(acc, ExpNegSq4(_mm256_loadu_pd(x + 4), c));
    return HorizontalSum4(acc);
}

// Vectorized Poisson mode sum: sum_{n=1..nmax} exp(-t * (2*pi*n*inv_log_p)^2).
inline double ExpPoissonModesSum(double t, double inv_log_p, int nmax) {
    if (nmax <= 0) return 0.0;
    const double two_pi = 6.283185307179586 * inv_log_p;
    const __m256d t_v = _mm256_set1_pd(-t);
    const __m256d step = _mm256_set1_pd(4.0);
    __m256d nvec = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
    __m256d acc = _mm256_setzero_pd();
    int n = 1;
    for (; n + 3 <= nmax; n += 4) {
        __m256d lam = _mm256_mul_pd(_mm256_set1_pd(two_pi), nvec);
        __m256d lam2 = _mm256_mul_pd(lam, lam);
        acc = _mm256_add_pd(acc, Exp4(_mm256_mul_pd(lam2, t_v)));
        nvec = _mm256_add_pd(nvec, step);
    }
    double sum = HorizontalSum4(acc);
    for (; n <= nmax; ++n) {
        const double lam = two_pi * static_cast<double>(n);
        sum += Exp1(-t * lam * lam);
    }
    return sum;
}

// Sum_{k=1..k_cap} (log_p / ppow_k) * exp(-coeff * (k*log_p)^2); ppow_k = exp(log_p/2 * k).
inline double PrimePowerExpSum(double log_p, int k_cap, double coeff_neg, double eps,
                               double log_p_over_2pi = 0.0, bool divide_2pi = false) {
    const double lp = log_p;
    const __m256d lp_v = _mm256_set1_pd(lp);
    const __m256d coeff_v = _mm256_set1_pd(coeff_neg);
    double sum = 0.0;
    int k = 1;
    for (; k + 3 <= k_cap; k += 4) {
        alignas(32) double karr[4] = {static_cast<double>(k), static_cast<double>(k + 1),
                                        static_cast<double>(k + 2), static_cast<double>(k + 3)};
        __m256d kvec = _mm256_load_pd(karr);
        __m256d u = _mm256_mul_pd(kvec, lp_v);
        __m256d e = Exp4(_mm256_mul_pd(_mm256_mul_pd(u, u), coeff_v));
        alignas(32) double lane[4];
        _mm256_store_pd(lane, e);
        for (int j = 0; j < 4; ++j) {
            const double ppow = std::exp(0.5 * lp * karr[j]);
            const double term = (lp / ppow) * lane[j];
            sum += term;
            if (term < eps) return divide_2pi ? sum / log_p_over_2pi : sum;
        }
    }
    for (; k <= k_cap; ++k) {
        const double u = static_cast<double>(k) * lp;
        const double ppow = std::exp(0.5 * lp * static_cast<double>(k));
        const double term = (lp / ppow) * Exp1(coeff_neg * u * u);
        sum += term;
        if (term < eps) break;
    }
    return divide_2pi ? sum / log_p_over_2pi : sum;
}

#else

inline double Exp1(double x) {
    if (x <= kExpDomainLo) return 0.0;
    if (x > kExpDomainHi) x = kExpDomainHi;
    double p = kExpPolyCoeff[0];
    for (int i = 1; i <= kExpPolyDegree; ++i)
        p = p * x + kExpPolyCoeff[i];
    return p;
}

#endif

}  // namespace Marshal::Kernel

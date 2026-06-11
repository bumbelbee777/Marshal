#pragma once
// Branchless LUT-backed SIMD exp primitives. Domain: [kExpDomainLo, kExpDomainHi].
#include "../../Generated/exp_coeffs.inc"

#if defined(MARSHAL_HAVE_AVX2)
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

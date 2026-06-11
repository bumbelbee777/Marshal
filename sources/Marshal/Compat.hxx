#pragma once
// Bridge legacy Weil names to Marshal modules during consolidation.
#include "Numerics/Real.hxx"
#include "Numerics/Support.hxx"
#include "Kernel/PairwiseSum.hxx"
#include "Kernel/SoaLayouts.hxx"

#if defined(MARSHAL_HAVE_AVX2)
#include "Kernel/SimdMicrokernels.hxx"
#endif

using Real = ::Real;
using Marshal::Kernel::kPrimeBatch;
using Marshal::Kernel::kZeroChunk;
using Marshal::Numerics::AdaptiveKtheta;
using Marshal::Numerics::AdaptiveNmax;
using Marshal::Numerics::BoundArchQuadrature;
using Marshal::Numerics::BoundOmittedPrimeTail;
using Marshal::Numerics::BoundOmittedPrimeTailUniform;
using Marshal::Numerics::BoundPrimeTailGauss;
using Marshal::Numerics::BoundTotalGauss;
using Marshal::Numerics::BoundZeroTailGauss;
using Marshal::Numerics::ClassifyTailBound;
using Marshal::Numerics::ConvergenceRegime;
using Marshal::Numerics::Crc32Bytes;
using Marshal::Numerics::EvaluateOmittedPrimeTail;
using Marshal::Numerics::kM3TailConstant;
using Marshal::Numerics::ParseZeroLineFc;
using Marshal::Numerics::ParseZeroLineLd;
using Marshal::Numerics::ValidateConfig;

inline Real pairwise_sum(const std::vector<Real>& v) {
    return Marshal::Kernel::PairwiseSum(v);
}
inline uint32_t crc32_bytes(const void* d, size_t n) {
    return Marshal::Numerics::Crc32Bytes(d, n);
}
inline bool validate_config(Real s, int pl, bool sw, Real smn, Real smx, int st,
                            size_t mz, int km, int nm, int kt, std::string& e) {
    return ValidateConfig(s, pl, sw, smn, smx, st, mz, km, nm, kt, e);
}
inline bool parse_zero_line_fc(const char* b, const char* e, double& v) {
    return ParseZeroLineFc(b, e, v);
}
inline bool parse_zero_line_ld(const char* b, const char* e, Real& v) {
    return ParseZeroLineLd(b, e, v);
}
inline Real bound_omitted_prime_tail(Real t, Real P) { return BoundOmittedPrimeTail(t, P); }
inline Real bound_omitted_prime_tail_uniform(Real a, Real b, Real P) {
    return BoundOmittedPrimeTailUniform(a, b, P);
}
inline Real evaluate_omitted_prime_tail(Real t, Real P, const int* pr, size_t n, int k) {
    return EvaluateOmittedPrimeTail(t, P, pr, n, k);
}
inline int adaptive_nmax(Real a, Real b, Real c) { return AdaptiveNmax(a, b, c); }
inline int adaptive_ktheta(Real a, Real b, Real c) { return AdaptiveKtheta(a, b, c); }
inline Real bound_prime_tail_gauss(Real s, Real p) { return BoundPrimeTailGauss(s, p); }
inline Real bound_zero_tail_gauss(Real s, Real g) { return BoundZeroTailGauss(s, g); }
inline Real bound_arch_quadrature(Real s, int o) { return BoundArchQuadrature(s, o); }
inline Real bound_total_gauss(Real s, Real p, Real g, int o) { return BoundTotalGauss(s, p, g, o); }
inline const char* weil_real_name() { return MarshalRealName(); }
inline int weil_real_bits() { return MarshalRealBits(); }

#if defined(MARSHAL_HAVE_AVX2)
#define WEIL_HAVE_AVX2 1
#include <immintrin.h>
inline double expd1_avx2(double x) { return Marshal::Kernel::Exp1(x); }
inline __m256d exp_neg_sq4_avx2(__m256d g, double inv_2ss) {
    return Marshal::Kernel::ExpNegSq4(g, inv_2ss);
}
#endif

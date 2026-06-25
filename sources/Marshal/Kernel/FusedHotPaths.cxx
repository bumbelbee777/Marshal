#include "FusedHotPaths.hxx"
#include <cmath>
#include <cstddef>
#if defined(MARSHAL_HAVE_AVX2)
#include <immintrin.h>
#endif

namespace Marshal::Kernel {

static constexpr Real kPi = MARSHAL_LIT(3.141592653589793238462643383279502884);

Real FusedHeatTracePoisson(Real t, Real inv_log_p, int nmax) {
#if defined(MARSHAL_HAVE_AVX2)
    if (nmax > 0) {
        return static_cast<Real>(ExpPoissonModesSum(static_cast<double>(t),
                                                   static_cast<double>(inv_log_p), nmax));
    }
    return 0;
#else
    Real sum = 0;
    for (int n = 1; n <= nmax; ++n) {
        const Real lam = 2.0L * kPi * static_cast<Real>(n) * inv_log_p;
        sum += MarshalExp(-t * lam * lam);
    }
    return sum;
#endif
}

Real FusedPrimeBlock(Real t, Real log_p, int k_cap) {
#if defined(MARSHAL_HAVE_AVX2)
    const double coeff = -static_cast<double>(t);
    const double sum = PrimePowerExpSum(static_cast<double>(log_p), k_cap, coeff, 0.0,
                                        static_cast<double>(2.0L * kPi), true);
    return static_cast<Real>(sum);
#else
    Real sum = 0;
    Real ppow = MarshalExp(log_p * 0.5L);
    for (int k = 1; k <= k_cap; ++k) {
        const Real u = static_cast<Real>(k) * log_p;
        sum += (log_p / ppow) * MarshalExp(-t * u * u);
        ppow *= MarshalExp(log_p * 0.5L);
    }
    return sum / (2.0L * kPi);
#endif
}

Real FusedZeroGaussianSum(const Real* gamma_ld, size_t off, size_t len, Real t) {
    Real sum = 0;
    for (size_t i = off; i < off + len; ++i) {
        const Real g = gamma_ld[i];
        sum += MarshalExp(-t * g * g);
    }
    return 2.0L * sum;
}

void FusedZeroGaussianSumBatch(const double* gamma, size_t n,
                               const double* t_values, size_t nt,
                               double* out_lhs) {
    if (!gamma || !t_values || !out_lhs || n == 0 || nt == 0) return;
#ifdef _OPENMP
    #pragma omp parallel for schedule(static) if(nt > 8)
#endif
    for (ptrdiff_t j = 0; j < static_cast<ptrdiff_t>(nt); ++j) {
        const double heat_t = t_values[static_cast<size_t>(j)];
#if defined(MARSHAL_HAVE_AVX2)
        out_lhs[static_cast<size_t>(j)] = FusedZeroGaussianSum4(gamma, n, heat_t);
#else
        double sum = 0;
        for (size_t i = 0; i < n; ++i)
            sum += std::exp(-heat_t * gamma[i] * gamma[i]);
        out_lhs[static_cast<size_t>(j)] = 2.0 * sum;
#endif
    }
}

void FusedHeatTracePoissonSoA(Real t, const double* inv_log_p, size_t n_primes,
                              int nmax, double* out) {
    if (!inv_log_p || !out || n_primes == 0) return;
#ifdef _OPENMP
    #pragma omp parallel for schedule(static, 64) if(n_primes > 64)
#endif
    for (ptrdiff_t p = 0; p < static_cast<ptrdiff_t>(n_primes); ++p) {
        out[static_cast<size_t>(p)] = static_cast<double>(
            FusedHeatTracePoisson(t, static_cast<Real>(inv_log_p[static_cast<size_t>(p)]), nmax));
    }
}

void FusedPrimeBlockSoA(Real t, const double* log_p, const int* k_cap,
                            size_t n_primes, double* out) {
    if (!log_p || !k_cap || !out || n_primes == 0) return;
#ifdef _OPENMP
    #pragma omp parallel for schedule(static, 64) if(n_primes > 64)
#endif
    for (ptrdiff_t p = 0; p < static_cast<ptrdiff_t>(n_primes); ++p) {
        const size_t i = static_cast<size_t>(p);
        out[i] = static_cast<double>(
            FusedPrimeBlock(t, static_cast<Real>(log_p[i]), k_cap[i]));
    }
}

#if defined(MARSHAL_HAVE_AVX2)
double FusedZeroGaussianSum4(const double* gamma, size_t n, double heat_t) {
    __m256d acc = _mm256_setzero_pd();
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        acc = _mm256_add_pd(acc, ExpNegSq4(_mm256_loadu_pd(gamma + i), heat_t));
        acc = _mm256_add_pd(acc, ExpNegSq4(_mm256_loadu_pd(gamma + i + 4), heat_t));
    }
    double sum = HorizontalSum4(acc);
    for (; i + 4 <= n; i += 4)
        sum += HorizontalSum4(ExpNegSq4(_mm256_loadu_pd(gamma + i), heat_t));
    for (; i < n; ++i)
        sum += Exp1(-heat_t * gamma[i] * gamma[i]);
    return 2.0 * sum;
}
#endif

}  // namespace Marshal::Kernel

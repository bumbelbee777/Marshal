#include "TraceApi.hxx"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "Compat.hxx"
#include "Heat/Common.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Kernel/ScaleHotPaths.hxx"
#include "Kernel/SoaStreaming.hxx"
#include "Numerics/TestFunctions.hxx"

namespace {

using Marshal::Heat::HeatCylinderOp;
using Marshal::Heat::Kahan;

constexpr Real kEulerGamma = 0.577215664901532860606512090082402431L;

#include "psi_lut.inc"
#include "gh64.inc"
#include "gh128.inc"
#include "gh256.inc"
#include "gh512.inc"
#include "gh1024.inc"

inline Real h_gauss(Real t, Real sigma) noexcept {
    return expl(-t * t / (2.0L * sigma * sigma));
}

inline Real tau_from_sigma(Real sigma) noexcept {
    return 1.0L / (2.0L * sigma * sigma);
}

inline Real psi_lut_lookup(Real y) noexcept {
    if (y <= kPsiLutYMin) {
        return kPsiLut[0];
    }
    if (y >= kPsiLutYMax) {
        return kPsiLut[kPsiLutN - 1];
    }
    const Real t = (y - kPsiLutYMin) * kPsiLutInvStep;
    const int i = static_cast<int>(t);
    const Real f = t - static_cast<Real>(i);
    const Real a = kPsiLut[i];
    const Real b = kPsiLut[std::min(i + 1, kPsiLutN - 1)];
    return a + f * (b - a);
}

Real re_psi_quarter_plus_iy(Real y) {
    const Real ay = fabsl(y);
    if (ay >= kPsiLutYMin && ay <= kPsiLutYMax) {
        return psi_lut_lookup(ay);
    }
    if (ay >= 40.0L) {
        const Real z2 = ay * ay + 0.0625L;
        const Real z4 = z2 * z2;
        const Real z6 = z4 * z2;
        return 0.5L * logl(z2) - 1.0L / (8.0L * z2) + 1.0L / (48.0L * z4) - 1.0L / (192.0L * z6);
    }
    Real sum = -kEulerGamma;
    const Real y2 = ay * ay;
    for (int n = 0; n < 400000; ++n) {
        const Real an = static_cast<Real>(n) + 0.25L;
        const Real term1 = 1.0L / (static_cast<Real>(n) + 1.0L);
        const Real term2 = an / (an * an + y2);
        sum += term1 - term2;
        if (n > 12000 && fabsl(term1 - term2) < 1e-28L) {
            break;
        }
    }
    return sum;
}

Real arch_psi_series(Real y, bool high_precision = false) {
    if (high_precision) {
        return re_psi_quarter_plus_iy(y);
    }
    const Real ay = fabsl(y);
    const Real y2 = ay * ay;
    Real sum = -kEulerGamma;
    for (int k = 0; k < 400000; ++k) {
        const Real ak = static_cast<Real>(k) + 0.25L;
        const Real t1 = 1.0L / (static_cast<Real>(k) + 1.0L);
        const Real t2 = ak / (ak * ak + y2);
        sum += t1 - t2;
        if (k > 8000 && fabsl(t1 - t2) < 1e-22L) {
            break;
        }
    }
    return sum;
}

Real arch_gauss_hermite_n(Real sigma, const Real* x, const Real* w, int n, bool precise_psi = false) {
    const Real scale = sigma * sqrtl(2.0L) / (2.0L * Marshal::Heat::kPi);
    const Real inv_sqrt2 = 1.0L / sqrtl(2.0L);
    const Real log_pi = logl(Marshal::Heat::kPi);
    Kahan acc;
#ifdef _OPENMP
    #pragma omp parallel
    {
        Kahan local;
        #pragma omp for
        for (int i = 0; i < n; ++i) {
            const Real y = x[i] * sigma * inv_sqrt2;
            const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
            local.add(w[i] * (psi - log_pi));
        }
        #pragma omp critical
        { acc.add(local.total()); }
    }
#else
    for (int i = 0; i < n; ++i) {
        const Real y = x[i] * sigma * inv_sqrt2;
        const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
        acc.add(w[i] * (psi - log_pi));
    }
#endif
    return scale * acc.total();
}

Real arch_simpson(Real sigma, int n_pts, Marshal::SimdLevel /*simd*/, bool precise_psi = false) {
    const Real tau = tau_from_sigma(sigma);
    const Real L = 10.0L / sqrtl(tau);
    const Real dx = 2.0L * L / static_cast<Real>(n_pts - 1);
    const Real log_pi = logl(Marshal::Heat::kPi);
    Kahan acc;
#ifdef _OPENMP
    #pragma omp parallel
    {
        Kahan local;
        #pragma omp for
        for (int i = 0; i < n_pts; ++i) {
            const Real x = -L + static_cast<Real>(i) * dx;
            const Real y = x * 0.5L;
            const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
            const Real f = h_gauss(x, sigma) * (psi - log_pi);
            Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
            local.add(w * f);
        }
        #pragma omp critical
        { acc.add(local.total()); }
    }
#else
    for (int i = 0; i < n_pts; ++i) {
        const Real x = -L + static_cast<Real>(i) * dx;
        const Real y = x * 0.5L;
        const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
        const Real f = h_gauss(x, sigma) * (psi - log_pi);
        Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
        acc.add(w * f);
    }
#endif
    return (dx / 3.0L) * acc.total() / (2.0L * Marshal::Heat::kPi);
}

Real arch_simpson_check(Real sigma, int n_pts, Marshal::SimdLevel simd, bool precise_psi) {
    const Real a1 = arch_simpson(sigma, n_pts, simd, precise_psi);
    const Real a2 = arch_simpson(sigma, 2 * n_pts - 1, simd, precise_psi);
    return a2 + (a2 - a1) / 15.0L;
}

Real arch_gauss_richardson_1024(Real sigma, Real& drift_out, bool precise_psi = false) {
    const Real a64 = arch_gauss_hermite_n(sigma, kGHX, kGHW, kGHN, precise_psi);
    const Real a128 = arch_gauss_hermite_n(sigma, kGH128X, kGH128W, kGH128N, precise_psi);
    const Real a256 = arch_gauss_hermite_n(sigma, kGH256X, kGH256W, kGH256N, precise_psi);
    const Real a512 = arch_gauss_hermite_n(sigma, kGH512X, kGH512W, kGH512N, precise_psi);
    const Real a1024 = arch_gauss_hermite_n(sigma, kGH1024X, kGH1024W, kGH1024N, precise_psi);
    const Real e512_256 = a512 - a256;
    const Real e256_128 = a256 - a128;
    const Real e128_64 = a128 - a64;
    const Real e1024_512 = a1024 - a512;
    drift_out = std::max({fabsl(e1024_512), fabsl(e512_256), fabsl(e256_128)});
    return a1024 + e1024_512 / 3.0L + (e1024_512 - e512_256) / 15.0L +
           (e1024_512 - 2.0L * e512_256 + e256_128) / 105.0L +
           (e1024_512 - 3.0L * e512_256 + 3.0L * e256_128 - e128_64) / 945.0L;
}

Real arch_simpson_adaptive(Real sigma, int n0, int n_max, Real eps, Marshal::SimdLevel simd,
                           bool precise_psi = false) {
    int n_pts = n0;
    Real prev = arch_simpson_check(sigma, n_pts, simd, precise_psi);
    while (n_pts < n_max) {
        const int n2 = std::min(2 * n_pts - 1, n_max);
        const Real next = arch_simpson_check(sigma, n2, simd, precise_psi);
        const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
        if (rel < eps) {
            return next;
        }
        prev = next;
        n_pts = n2;
    }
    return prev;
}

Real archimedean_exact(Real sigma, Marshal::SimdLevel simd, bool precision_mode, int arch_pts,
                       Real eps) {
    Real drift = 0;
    const Real gh = arch_gauss_richardson_1024(sigma, drift, precision_mode);
    const Real gh_tol = precision_mode ? 1e-14L : 1e-12L;
    if (drift < gh_tol) {
        return gh;
    }
    const Real sim_tol = precision_mode ? 1e-16L : std::max(eps, 1e-14L);
    const int n0 = sigma >= 4.0L ? 128001 : 64001;
    return arch_simpson_adaptive(sigma, n0, arch_pts, sim_tol, simd, precision_mode);
}

// Arch sinc²: (1/2π) ∫ h_{κ,T}(t) (Re ψ(1/4+it/2) − log π) dt,  h_{κ,T}(t)=sinc²(κt/T).
inline Real h_sinc2_at(Real t, Real T, Real kappa = 1.0L) {
    if (T <= 0) return 0.0L;
    const Real x = kappa * t / T;
    if (MarshalFabs(x) < 1e-30L) return 1.0L;
    const Real s = MarshalSin(x) / x;
    return s * s;
}

Real arch_sinc2_simpson(Real T, Real kappa, Real L, int n_pts, Marshal::SimdLevel /*simd*/,
                        bool precise_psi = false) {
    const Real dx = 2.0L * L / static_cast<Real>(n_pts - 1);
    const Real log_pi = logl(Marshal::Heat::kPi);
    Kahan acc;
#ifdef _OPENMP
    #pragma omp parallel
    {
        Kahan local;
        #pragma omp for
        for (int i = 0; i < n_pts; ++i) {
            const Real t = -L + static_cast<Real>(i) * dx;
            const Real y = t * 0.5L;
            const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
            const Real f = h_sinc2_at(t, T, kappa) * (psi - log_pi);
            Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
            local.add(w * f);
        }
        #pragma omp critical
        { acc.add(local.total()); }
    }
#else
    for (int i = 0; i < n_pts; ++i) {
        const Real t = -L + static_cast<Real>(i) * dx;
        const Real y = t * 0.5L;
        const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
        const Real f = h_sinc2_at(t, T, kappa) * (psi - log_pi);
        Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
        acc.add(w * f);
    }
#endif
    return (dx / 3.0L) * acc.total() / (2.0L * Marshal::Heat::kPi);
}

Real arch_sinc2_simpson_check(Real T, Real kappa, Real L, int n_pts, Marshal::SimdLevel simd,
                              bool precise_psi) {
    const Real a1 = arch_sinc2_simpson(T, kappa, L, n_pts, simd, precise_psi);
    const Real a2 = arch_sinc2_simpson(T, kappa, L, 2 * n_pts - 1, simd, precise_psi);
    return a2 + (a2 - a1) / 15.0L;
}

Real arch_sinc2_at_window(Real T, Real kappa, Real L, Marshal::SimdLevel simd,
                          bool precision_mode, int arch_pts, Real eps) {
    const Real sim_tol = precision_mode ? 1e-16L : std::max(eps, 1e-14L);
    const bool precise_psi = precision_mode;
    int n_pts = 64001;
    Real prev = arch_sinc2_simpson_check(T, kappa, L, n_pts, simd, precise_psi);
    while (n_pts < arch_pts) {
        const int n2 = std::min(2 * n_pts - 1, arch_pts);
        const Real next = arch_sinc2_simpson_check(T, kappa, L, n2, simd, precise_psi);
        const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
        if (rel < sim_tol) {
            return next;
        }
        prev = next;
        n_pts = n2;
    }
    return prev;
}

Real arch_sinc2_adaptive(Real T, Real kappa, Marshal::SimdLevel simd, bool precision_mode,
                         int arch_pts, Real eps) {
    const Real L_tol = precision_mode ? 1e-14L : 1e-12L;
    const Real L_scale = precision_mode ? 128.0L : 80.0L;
    const int L_iters = precision_mode ? 7 : 5;
    Real L = std::max(L_scale * T / kappa, 120.0L);
    Real prev = arch_sinc2_at_window(T, kappa, L, simd, precision_mode, arch_pts, eps);
    for (int k = 0; k < L_iters; ++k) {
        L *= 2.0L;
        const Real next = arch_sinc2_at_window(T, kappa, L, simd, precision_mode, arch_pts, eps);
        const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
        if (rel < L_tol) {
            return next;
        }
        prev = next;
    }
    return prev;
}

void sinc2_arch_params(const TestFunction& tf, Real sigma, Real& T, Real& kappa) {
    T = sigma;
    kappa = 1.0L;
    if (const auto* s = dynamic_cast<const Sinc2Test*>(&tf)) {
        T = s->T;
        kappa = s->kappa;
    }
}

Real arch_generic_simpson(const TestFunction& tf, Real L, int n_pts, Marshal::SimdLevel /*simd*/,
                          bool precise_psi = false) {
    const Real dx = 2.0L * L / static_cast<Real>(n_pts - 1);
    const Real log_pi = logl(Marshal::Heat::kPi);
    Kahan acc;
#ifdef _OPENMP
#pragma omp parallel
    {
        Kahan local;
#pragma omp for
        for (int i = 0; i < n_pts; ++i) {
            const Real t = -L + static_cast<Real>(i) * dx;
            const Real y = t * 0.5L;
            const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
            const Real f = tf.h(t) * (psi - log_pi);
            const Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
            local.add(w * f);
        }
#pragma omp critical
        { acc.add(local.total()); }
    }
#else
    for (int i = 0; i < n_pts; ++i) {
        const Real t = -L + static_cast<Real>(i) * dx;
        const Real y = t * 0.5L;
        const Real psi = precise_psi ? arch_psi_series(y, true) : re_psi_quarter_plus_iy(y);
        const Real f = tf.h(t) * (psi - log_pi);
        const Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
        acc.add(w * f);
    }
#endif
    return (dx / 3.0L) * acc.total() / (2.0L * Marshal::Heat::kPi);
}

Real arch_generic_simpson_check(const TestFunction& tf, Real L, int n_pts,
                                Marshal::SimdLevel simd, bool precise_psi) {
    const Real a1 = arch_generic_simpson(tf, L, n_pts, simd, precise_psi);
    const Real a2 = arch_generic_simpson(tf, L, 2 * n_pts - 1, simd, precise_psi);
    return a2 + (a2 - a1) / 15.0L;
}

Real arch_generic_adaptive(const TestFunction& tf, Real L0, Marshal::SimdLevel simd,
                           bool precision_mode, int arch_pts, Real eps) {
    const Real sim_tol = precision_mode ? 1e-16L : std::max(eps, 1e-14L);
    const bool precise_psi = precision_mode;
    Real L = L0;
    Real prev = arch_generic_simpson_check(tf, L, 64001, simd, precise_psi);
    for (int k = 0; k < 6; ++k) {
        int n_pts = 64001;
        while (n_pts < arch_pts) {
            const int n2 = std::min(2 * n_pts - 1, arch_pts);
            const Real next = arch_generic_simpson_check(tf, L, n2, simd, precise_psi);
            const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
            if (rel < sim_tol) return next;
            prev = next;
            n_pts = n2;
        }
        L *= 2.0L;
        prev = arch_generic_simpson_check(tf, L, 64001, simd, precise_psi);
    }
    return prev;
}

Real archimedean_for_tf(const TestFunction& tf, Real sigma, Marshal::SimdLevel simd,
                        bool precision_mode, int arch_pts, Real eps, bool quick_sinc2) {
    if (std::string(tf.name()) == "sinc2") {
        Real T = sigma;
        Real kappa = 1.0L;
        sinc2_arch_params(tf, sigma, T, kappa);
        const bool precise_psi = precision_mode;
        if (quick_sinc2) {
            const Real L = std::max(240.0L * T / kappa, 240.0L);
            return arch_sinc2_simpson_check(T, kappa, L, 64001, simd, precise_psi);
        }
        return arch_sinc2_adaptive(T, kappa, simd, precision_mode, arch_pts, eps);
    }
    if (std::string(tf.name()) == "gauss") {
        return archimedean_exact(sigma, simd, precision_mode, arch_pts, eps);
    }
    Real L0 = tf.support_radius();
    if (const auto* b = dynamic_cast<const BumpTest*>(&tf)) {
        (void)b;
        L0 = std::max(L0, static_cast<Real>(8.0L));
    }
    if (const auto* r = dynamic_cast<const RationalTest*>(&tf)) {
        L0 = std::max(L0, 30.0L * r->a);
    }
    if (const auto* lp = dynamic_cast<const LaplaceTest*>(&tf)) {
        L0 = std::max(L0, 400.0L / lp->a);
    }
    return arch_generic_adaptive(tf, L0, simd, precision_mode, arch_pts, eps);
}

Real zero_sum_h_ld_batched(const TestFunction& tf, const Real* gammas, size_t n) {
    if (n == 0) {
        return 0.0L;
    }
    const int nchunks = static_cast<int>((n + kZeroChunk - 1) / kZeroChunk);
    std::vector<Real> parts;
#ifdef _OPENMP
    parts.resize(omp_get_max_threads(), 0.0L);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        Kahan local;
        #pragma omp for schedule(static)
        for (int c = 0; c < nchunks; ++c) {
            const size_t off = static_cast<size_t>(c) * kZeroChunk;
            const size_t len = std::min(static_cast<size_t>(kZeroChunk), n - off);
            for (size_t i = 0; i < len; ++i) {
                local.add(tf.h(gammas[off + i]));
            }
        }
        parts[static_cast<size_t>(tid)] = local.total();
    }
#else
    {
        Kahan local;
        for (size_t i = 0; i < n; ++i) {
            local.add(tf.h(gammas[i]));
        }
        parts.push_back(local.total());
    }
#endif
    Kahan merge;
    for (Real p : parts) {
        merge.add(p);
    }
    return 2.0L * merge.total();
}

Real zero_sum_h_batched(const TestFunction& tf, Real sigma, const double* gammas, size_t n,
                        Marshal::SimdLevel simd) {
    if (n == 0) {
        return 0.0L;
    }
    if (std::string(tf.name()) == "gauss" && simd == Marshal::SimdLevel::AVX2) {
        const double heat_t = 1.0 / (2.0 * static_cast<double>(sigma * sigma));
        return static_cast<Real>(
            Marshal::Kernel::FusedZeroGaussianSumScale(gammas, n, heat_t));
    }
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
            for (size_t i = 0; i < len; ++i) {
                local += tf.h(static_cast<Real>(gammas[off + i]));
            }
        }
        parts[static_cast<size_t>(tid)] = local;
    }
#else
    parts.push_back(0.0L);
    for (size_t i = 0; i < n; ++i) {
        parts[0] += tf.h(static_cast<Real>(gammas[i]));
    }
#endif
    return 2.0L * pairwise_sum(parts);
}

Real zero_sum_h(const TestFunction& tf, Real sigma, const std::vector<double>& gammas,
                const std::vector<Real>& gammas_ld, Marshal::ZeroKernel zk, Marshal::SimdLevel simd,
                bool force_ld = false) {
    if (force_ld || (zk == Marshal::ZeroKernel::LongDouble && !gammas_ld.empty())) {
        return zero_sum_h_ld_batched(tf, gammas_ld.data(), gammas_ld.size());
    }
    return zero_sum_h_batched(tf, sigma, gammas.data(), gammas.size(), simd);
}

Real zero_sum_h_prefix(const TestFunction& tf, Real sigma, const double* gammas, size_t n,
                       const Real* gammas_ld, size_t n_ld, Marshal::ZeroKernel zk,
                       Marshal::SimdLevel simd, bool force_ld = false) {
    if (force_ld || (zk == Marshal::ZeroKernel::LongDouble && gammas_ld && n_ld > 0)) {
        const size_t use = std::min(n, n_ld);
        return zero_sum_h_ld_batched(tf, gammas_ld, use);
    }
    return zero_sum_h_batched(tf, sigma, gammas, n, simd);
}

Real trivial_zero_sum(const TestFunction& tf, int k = 500) {
    Real s = 0.0L;
    for (int m = 0; m < k; ++m) {
        s += tf.h(static_cast<Real>(2 * m + 1));
    }
    return 2.0L * s;
}

Real pole_terms(const TestFunction& tf, Real sigma) {
    if (std::string(tf.name()) == "gauss") {
        return 2.0L * expl(1.0L / (8.0L * sigma * sigma));
    }
    return 2.0L * tf.h(0.0L);
}

void accumulate_prime_blocks(const Marshal::Heat::PrimeCatalog& cat, const TestFunction& tf,
                             Real tau, Real /*link_n*/, Real eps, Real& prime_out, Real& heat_out,
                             bool use_kahan = false, bool force_scale = false) {
    Marshal::Kernel::AccumulatePrimeBlocksStreaming(cat, tf, tau, eps, use_kahan, force_scale,
                                                    prime_out, heat_out);
}

}  // namespace

namespace Marshal {

TraceResult EvaluateTrace(const TestFunction& tf, Real sigma, const std::vector<double>& gammas,
                        const std::vector<Real>& gammas_ld, const Heat::PrimeCatalog& cat,
                        ZeroKernel zk, SimdLevel simd, Real eps, bool include_trivial,
                        bool precision_mode, int arch_pts, bool quick_sinc2_arch,
                        const Heat::ArchimedeanBoundarySpec* arch_spec, bool scale_mode) {
    TraceResult r;
    const Real tau = tau_from_sigma(sigma);
    const Real link_n = sigma * sqrtl(2.0L / Marshal::Heat::kPi);

    r.lhs = zero_sum_h(tf, sigma, gammas, gammas_ld, zk, precision_mode ? SimdLevel::Scalar : simd,
                       precision_mode);
    if (include_trivial) {
        r.lhs += trivial_zero_sum(tf);
    }
    r.poles = pole_terms(tf, sigma);
    if (arch_spec)
        r.arch = Heat::archimedean_for_tf_bounded(tf, sigma, *arch_spec, simd, precision_mode,
                                                  arch_pts, eps, quick_sinc2_arch);
    else
        r.arch = archimedean_for_tf(tf, sigma, simd, precision_mode, arch_pts, eps, quick_sinc2_arch);

    Real prime_raw = 0;
    Real heat_raw = 0;
    accumulate_prime_blocks(cat, tf, tau, link_n, eps, prime_raw, heat_raw, precision_mode,
                           scale_mode);
    r.prime = prime_raw / (2.0L * Marshal::Heat::kPi);
    r.heat_prime_ab = heat_raw * link_n;
    Kahan geom;
    geom.add(r.poles);
    geom.add(r.arch);
    r.rhs = geom.total() - r.prime;
    Kahan bal;
    bal.add(r.lhs);
    bal.add(-r.poles);
    bal.add(-r.arch);
    bal.add(r.prime);
    r.residual_kahan = bal.total();
    r.trivial = include_trivial ? trivial_zero_sum(tf) : 0;
    return r;
}

TraceResult EvaluateTracePrefix(const TestFunction& tf, Real sigma, const double* gammas,
                                size_t n_zeros, const Real* gammas_ld, size_t n_zeros_ld,
                                const Heat::PrimeCatalog& cat, ZeroKernel zk, SimdLevel simd,
                                Real eps, bool include_trivial, bool precision_mode, int arch_pts,
                                bool quick_sinc2_arch,
                                const Heat::ArchimedeanBoundarySpec* arch_spec,
                                bool scale_mode) {
    TraceResult r;
    const Real tau = tau_from_sigma(sigma);
    const Real link_n = sigma * sqrtl(2.0L / Marshal::Heat::kPi);

    r.lhs = zero_sum_h_prefix(tf, sigma, gammas, n_zeros, gammas_ld, n_zeros_ld, zk,
                              precision_mode ? SimdLevel::Scalar : simd, precision_mode);
    if (include_trivial) r.lhs += trivial_zero_sum(tf);
    r.poles = pole_terms(tf, sigma);
    if (arch_spec)
        r.arch = Heat::archimedean_for_tf_bounded(tf, sigma, *arch_spec, simd, precision_mode,
                                                  arch_pts, eps, quick_sinc2_arch);
    else
        r.arch = archimedean_for_tf(tf, sigma, simd, precision_mode, arch_pts, eps, quick_sinc2_arch);

    Real prime_raw = 0;
    Real heat_raw = 0;
    accumulate_prime_blocks(cat, tf, tau, link_n, eps, prime_raw, heat_raw, precision_mode,
                           scale_mode);
    r.prime = prime_raw / (2.0L * Marshal::Heat::kPi);
    r.heat_prime_ab = heat_raw * link_n;
    Kahan geom;
    geom.add(r.poles);
    geom.add(r.arch);
    r.rhs = geom.total() - r.prime;
    Kahan bal;
    bal.add(r.lhs);
    bal.add(-r.poles);
    bal.add(-r.arch);
    bal.add(r.prime);
    r.residual_kahan = bal.total();
    r.trivial = include_trivial ? trivial_zero_sum(tf) : 0;
    return r;
}

Real ArchSinc2Adaptive(Real T, Real kappa, SimdLevel simd, bool precision_mode, int arch_pts,
                       Real eps) {
    return arch_sinc2_adaptive(T, kappa, simd, precision_mode, arch_pts, eps);
}

std::vector<ArchSinc2AuditPoint> AuditArchSinc2(Real T, Real kappa, SimdLevel simd,
                                                bool precision_mode) {
    std::vector<ArchSinc2AuditPoint> out;
    const bool precise_psi = precision_mode;
    const Real hw = T / kappa;
    const Real L0 = std::max(120.0L, 8.0L * hw);
    const Real L_vals[] = {L0, std::max(L0, 16.0L * hw), std::max(L0, 32.0L * hw),
                           std::max(L0, 64.0L * hw), std::max(L0, 128.0L * hw)};
    const int n_vals[] = {64001, 128001, 256001, 512001, 1024001};
    for (Real L : L_vals) {
        for (int n_pts : n_vals) {
            ArchSinc2AuditPoint pt;
            pt.L = L;
            pt.n_pts = n_pts;
            pt.arch = arch_sinc2_simpson(T, kappa, L, n_pts, simd, precise_psi);
            pt.richardson_est = arch_sinc2_simpson_check(T, kappa, L, n_pts, simd, precise_psi);
            out.push_back(pt);
        }
    }
    return out;
}

ArchSinc2ConvergeResult ArchSinc2Converge(Real T, Real kappa, Real target, SimdLevel simd,
                                          bool precision_mode, int arch_pts_max) {
    ArchSinc2ConvergeResult result;
    result.T = T;
    result.kappa = kappa;
    result.arch_target = target;
    const bool precise_psi = precision_mode;
    const Real hw = T / kappa;
    Real L = std::max(120.0L, 8.0L * hw);
    const int n_ladder[] = {64001, 128001, 256001, 512001, 1024001, 2048001};
    Real best = 0;
    int best_n = 64001;
    Real best_L = L;
    bool converged = false;

    for (int pass = 0; pass < 4 && !converged; ++pass) {
        Real prev = 0;
        for (int n_pts : n_ladder) {
            if (n_pts > arch_pts_max) break;
            const Real arch = arch_sinc2_simpson_check(T, kappa, L, n_pts, simd, precise_psi);
            ArchSinc2AuditPoint pt;
            pt.L = L;
            pt.n_pts = n_pts;
            pt.arch = arch_sinc2_simpson(T, kappa, L, n_pts, simd, precise_psi);
            pt.richardson_est = arch;
            result.ladder.push_back(pt);
            if (prev != 0 && fabsl(arch - prev) < target) {
                best = arch;
                best_n = n_pts;
                best_L = L;
                converged = true;
                break;
            }
            prev = arch;
            best = arch;
            best_n = n_pts;
            best_L = L;
        }
        if (!converged) L *= 2.0L;
    }

    result.arch = best;
    result.n_pts_final = best_n;
    result.L_final = best_L;
    result.converged = converged;
    return result;
}

Real ArchimedeanBaselineForTestFunction(const TestFunction& tf, Real sigma, SimdLevel simd,
                                        bool precision_mode, int arch_pts, Real eps,
                                        bool quick_sinc2) {
    return archimedean_for_tf(tf, sigma, simd, precision_mode, arch_pts, eps, quick_sinc2);
}

}  // namespace Marshal

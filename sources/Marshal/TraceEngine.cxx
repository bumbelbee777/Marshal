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

inline Real h_sinc2_at(Real t, Real T) {
    if (T <= 0) return 0.0L;
    const Real x = t / T;
    if (std::fabsl(x) < 1e-30L) return 1.0L;
    const Real s = std::sinl(x) / x;
    return s * s;
}

Real arch_sinc2_simpson(Real T, Real L, int n_pts, Marshal::SimdLevel /*simd*/,
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
            const Real f = h_sinc2_at(t, T) * (psi - log_pi);
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
        const Real f = h_sinc2_at(t, T) * (psi - log_pi);
        Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
        acc.add(w * f);
    }
#endif
    return (dx / 3.0L) * acc.total() / (2.0L * Marshal::Heat::kPi);
}

Real arch_sinc2_simpson_check(Real T, Real L, int n_pts, Marshal::SimdLevel simd,
                              bool precise_psi) {
    const Real a1 = arch_sinc2_simpson(T, L, n_pts, simd, precise_psi);
    const Real a2 = arch_sinc2_simpson(T, L, 2 * n_pts - 1, simd, precise_psi);
    return a2 + (a2 - a1) / 15.0L;
}

Real arch_sinc2_at_window(Real T, Real L, Marshal::SimdLevel simd, bool precision_mode,
                          int arch_pts, Real eps) {
    const Real sim_tol = precision_mode ? 1e-16L : std::max(eps, 1e-14L);
    int n_pts = 64001;
    Real prev = arch_sinc2_simpson_check(T, L, n_pts, simd, precision_mode);
    while (n_pts < arch_pts) {
        const int n2 = std::min(2 * n_pts - 1, arch_pts);
        const Real next = arch_sinc2_simpson_check(T, L, n2, simd, precision_mode);
        const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
        if (rel < sim_tol) {
            return next;
        }
        prev = next;
        n_pts = n2;
    }
    return prev;
}

Real arch_sinc2_adaptive(Real T, Marshal::SimdLevel simd, bool precision_mode, int arch_pts,
                         Real eps) {
    const Real L_tol = precision_mode ? 1e-14L : 1e-12L;
    Real L = std::max(80.0L * T, 120.0L);
    Real prev = arch_sinc2_at_window(T, L, simd, precision_mode, arch_pts, eps);
    for (int k = 0; k < 5; ++k) {
        L *= 2.0L;
        const Real next = arch_sinc2_at_window(T, L, simd, precision_mode, arch_pts, eps);
        const Real rel = fabsl(next - prev) / std::max(1.0L, fabsl(next));
        if (rel < L_tol) {
            return next;
        }
        prev = next;
    }
    return prev;
}

Real archimedean_for_tf(const TestFunction& tf, Real sigma, Marshal::SimdLevel simd,
                        bool precision_mode, int arch_pts, Real eps, bool quick_sinc2) {
    if (std::string(tf.name()) == "sinc2") {
        if (quick_sinc2) {
            const Real L = std::max(240.0L * sigma, 240.0L);
            return arch_sinc2_simpson_check(sigma, L, 64001, simd, precision_mode);
        }
        return arch_sinc2_adaptive(sigma, simd, precision_mode, arch_pts, eps);
    }
    return archimedean_exact(sigma, simd, precision_mode, arch_pts, eps);
}

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
            for (; i < len; ++i) {
                local += static_cast<Real>(std::exp(-inv_2ss * src[i] * src[i]));
            }
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
        for (size_t i = (len / 4) * 4; i < len; ++i) {
            local += static_cast<Real>(std::exp(-inv_2ss * src[i] * src[i]));
        }
    }
    parts.push_back(local);
#endif
    return 2.0L * pairwise_sum(parts);
#else
    (void)sigma;
    (void)gammas;
    (void)n;
    return 0.0L;
#endif
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
        return zero_sum_gauss_avx2(sigma, gammas, n);
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
                             bool use_kahan = false) {
    const size_t n = cat.p.size();
    const int nbatches = static_cast<int>((n + kPrimeBatch - 1) / kPrimeBatch);
    std::vector<Real> wp;
    std::vector<Real> hp;
#ifdef _OPENMP
    wp.resize(omp_get_max_threads(), 0.0L);
    hp.resize(omp_get_max_threads(), 0.0L);
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        Kahan wl;
        Kahan hl;
        Real wl_plain = 0.0L;
        Real hl_plain = 0.0L;
        #pragma omp for schedule(static, kPrimeBatch)
        for (int b = 0; b < nbatches; ++b) {
            const size_t i0 = static_cast<size_t>(b) * kPrimeBatch;
            const size_t i1 = std::min(i0 + kPrimeBatch, n);
            for (size_t i = i0; i < i1; ++i) {
                HeatCylinderOp op(cat, i);
                const int km = cat.kmax_adaptive[i];
                const Real w = op.prime_block_raw(tf, km, eps);
                const Real h = op.ab_heat_block(tau, km, eps);
                if (use_kahan) {
                    wl.add(w);
                    hl.add(h);
                } else {
                    wl_plain += w;
                    hl_plain += h;
                }
            }
        }
        wp[static_cast<size_t>(tid)] = use_kahan ? wl.total() : wl_plain;
        hp[static_cast<size_t>(tid)] = use_kahan ? hl.total() : hl_plain;
    }
    if (use_kahan) {
        Kahan wm;
        Kahan hm;
        for (Real v : wp) {
            wm.add(v);
        }
        for (Real v : hp) {
            hm.add(v);
        }
        prime_out = wm.total();
        heat_out = hm.total();
    } else {
        prime_out = pairwise_sum(wp);
        heat_out = pairwise_sum(hp);
    }
#else
    if (use_kahan) {
        Kahan wl;
        Kahan hl;
        for (size_t i = 0; i < n; ++i) {
            HeatCylinderOp op(cat, i);
            const int km = cat.kmax_adaptive[i];
            wl.add(op.prime_block_raw(tf, km, eps));
            hl.add(op.ab_heat_block(tau, km, eps));
        }
        prime_out = wl.total();
        heat_out = hl.total();
    } else {
        for (size_t i = 0; i < n; ++i) {
            HeatCylinderOp op(cat, i);
            const int km = cat.kmax_adaptive[i];
            prime_out += op.prime_block_raw(tf, km, eps);
            heat_out += op.ab_heat_block(tau, km, eps);
        }
    }
#endif
}

}  // namespace

namespace Marshal {

TraceResult EvaluateTrace(const TestFunction& tf, Real sigma, const std::vector<double>& gammas,
                        const std::vector<Real>& gammas_ld, const Heat::PrimeCatalog& cat,
                        ZeroKernel zk, SimdLevel simd, Real eps, bool include_trivial,
                        bool precision_mode, int arch_pts, bool quick_sinc2_arch) {
    TraceResult r;
    const Real tau = tau_from_sigma(sigma);
    const Real link_n = sigma * sqrtl(2.0L / Marshal::Heat::kPi);

    r.lhs = zero_sum_h(tf, sigma, gammas, gammas_ld, zk, precision_mode ? SimdLevel::Scalar : simd,
                       precision_mode);
    if (include_trivial) {
        r.lhs += trivial_zero_sum(tf);
    }
    r.poles = pole_terms(tf, sigma);
    r.arch = archimedean_for_tf(tf, sigma, simd, precision_mode, arch_pts, eps, quick_sinc2_arch);

    Real prime_raw = 0;
    Real heat_raw = 0;
    const size_t scale_thr = Marshal::Kernel::kScalePrimeThreshold;
    const bool scale_gauss = (std::string(tf.name()) == "gauss")
        && cat.p.size() >= scale_thr;
    if (scale_gauss) {
        Marshal::Kernel::AccumulateGaussPrimeBlocks(cat, sigma, tau, eps, prime_raw, heat_raw);
    } else {
        accumulate_prime_blocks(cat, tf, tau, link_n, eps, prime_raw, heat_raw, precision_mode);
    }
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

}  // namespace Marshal

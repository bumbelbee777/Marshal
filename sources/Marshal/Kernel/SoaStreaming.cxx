#include "SoaStreaming.hxx"

#include <algorithm>
#include <string>
#include <vector>

#include "FusedHotPaths.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "PairwiseSum.hxx"
#include "ScaleHotPaths.hxx"
#include "SoaLayouts.hxx"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Kernel {

void FusedAbHeatBlockBatch(const Real* log_p, const int* k_cap, size_t n_primes, Real eps,
                           const double* tau_values, size_t ntau, double* out_row_major) {
    if (!log_p || !k_cap || !tau_values || !out_row_major || n_primes == 0 || ntau == 0) return;
#ifdef _OPENMP
    #pragma omp parallel for schedule(static) if(ntau > 2)
#endif
    for (ptrdiff_t j = 0; j < static_cast<ptrdiff_t>(ntau); ++j) {
        const Real tau = static_cast<Real>(tau_values[static_cast<size_t>(j)]);
        double* row = out_row_major + static_cast<size_t>(j) * n_primes;
        FusedAbHeatBlockSoA(tau, log_p, k_cap, n_primes, eps, row);
    }
}

void AccumulatePrimeBlocksStreaming(const Heat::PrimeCatalog& cat, const TestFunction& tf,
                                    Real tau, Real eps, bool /*use_kahan*/, bool force_scale,
                                    Real& prime_out, Real& heat_out) {
    const size_t n = cat.p.size();
    if (n == 0) {
        prime_out = 0;
        heat_out = 0;
        return;
    }

    const bool is_gauss = (std::string(tf.name()) == "gauss");
    const size_t scale_thr = EffectiveScalePrimeThreshold(force_scale);
    if (is_gauss && n >= scale_thr) {
        const Real sigma = std::sqrt(1.0L / (2.0L * tau));
        AccumulateGaussPrimeBlocks(cat, sigma, tau, eps, prime_out, heat_out);
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
            std::vector<double> hbuf(bn);
            FusedAbHeatBlockSoA(tau, cat.logp.data() + i0, cat.kmax_adaptive.data() + i0, bn, eps,
                                hbuf.data());
            Real wl_batch = 0;
            Real hl_batch = 0;
            for (size_t j = 0; j < bn; ++j) {
                const size_t idx = i0 + j;
                Heat::HeatCylinderOp op(cat, idx);
                wl_batch += op.prime_block_raw(tf, cat.kmax_adaptive[idx], eps);
                hl_batch += static_cast<Real>(hbuf[j]);
            }
            wl += wl_batch;
            hl += hl_batch;
        }
        wp[static_cast<size_t>(tid)] = wl;
        hp[static_cast<size_t>(tid)] = hl;
    }
    prime_out = PairwiseSum(wp);
    heat_out = PairwiseSum(hp);
#else
    std::vector<double> hbuf(n);
    FusedAbHeatBlockSoA(tau, cat.logp.data(), cat.kmax_adaptive.data(), n, eps, hbuf.data());
    prime_out = 0;
    heat_out = 0;
    for (size_t i = 0; i < n; ++i) {
        Heat::HeatCylinderOp op(cat, i);
        prime_out += op.prime_block_raw(tf, cat.kmax_adaptive[i], eps);
        heat_out += static_cast<Real>(hbuf[i]);
    }
#endif
}

}  // namespace Marshal::Kernel

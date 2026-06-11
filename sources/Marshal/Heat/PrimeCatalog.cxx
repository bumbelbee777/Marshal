#include "PrimeCatalog.hxx"

#include "Numerics/Support.hxx"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Heat {

void PrimeCatalog::set_primes(const std::vector<int>& primes) {
    const size_t n = primes.size();
    p.resize(n);
    logp.resize(n);
    inv_logp.resize(n);
    sqrtp.resize(n);
    kmax_adaptive.resize(n);
    for (size_t i = 0; i < n; ++i) {
        const int pr = primes[static_cast<size_t>(i)];
        const Real lp = std::log(static_cast<Real>(pr));
        p[static_cast<size_t>(i)] = pr;
        logp[static_cast<size_t>(i)] = lp;
        inv_logp[static_cast<size_t>(i)] = 1.0L / lp;
        sqrtp[static_cast<size_t>(i)] = std::sqrt(static_cast<Real>(pr));
    }
}

int PrimeCatalog::adaptive_kmax(int pr, Real lp, const TestFunction& tf, Real tau,
                                int k_cap, Real eps) {
    Real ppow = std::sqrt(static_cast<Real>(pr));
    for (int k = 1; k <= k_cap; ++k) {
        const Real u = static_cast<Real>(k) * lp;
        const Real weil_term = (lp / ppow) * 2.0L * tf.h_hat(u);
        const Real ab_term = (lp / ppow) * std::exp(-u * u / (4.0L * tau));
        if (weil_term < eps && ab_term < eps) {
            return k;
        }
        ppow *= std::sqrt(static_cast<Real>(pr));
    }
    return k_cap;
}

void PrimeCatalog::rebuild_adaptive(const TestFunction& tf, Real tau, int k_cap, Real eps) {
    const size_t n = p.size();
#ifdef _OPENMP
    #pragma omp parallel for schedule(static)
#endif
    for (int i = 0; i < static_cast<int>(n); ++i) {
        kmax_adaptive[static_cast<size_t>(i)] =
            adaptive_kmax(p[static_cast<size_t>(i)], logp[static_cast<size_t>(i)],
                          tf, tau, k_cap, eps);
    }
}

void PrimeCatalog::build(const std::vector<int>& primes, const TestFunction& tf, Real tau,
                         int k_cap, Real eps) {
    set_primes(primes);
    rebuild_adaptive(tf, tau, k_cap, eps);
}

void PrimeCatalog::truncate_to_pmax(int p_max) {
    size_t n = 0;
    while (n < p.size() && p[n] <= p_max) {
        ++n;
    }
    p.resize(n);
    logp.resize(n);
    inv_logp.resize(n);
    sqrtp.resize(n);
    kmax_adaptive.resize(n);
}

int PrimeCatalog::pmax() const { return p.empty() ? 0 : p.back(); }

size_t PrimeCatalog::count() const { return p.size(); }

}  // namespace Marshal::Heat

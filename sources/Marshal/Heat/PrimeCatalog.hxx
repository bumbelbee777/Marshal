#pragma once
#include <vector>
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Heat {

struct PrimeCatalog {
    std::vector<int> p;
    std::vector<Real> logp;
    std::vector<Real> inv_logp;
    std::vector<Real> sqrtp;
    std::vector<int> kmax_adaptive;

    void set_primes(const std::vector<int>& primes);
    void rebuild_adaptive(const TestFunction& tf, Real tau, int k_cap, Real eps);
    void build(const std::vector<int>& primes, const TestFunction& tf, Real tau,
               int k_cap, Real eps);
    static int adaptive_kmax(int pr, Real lp, const TestFunction& tf, Real tau,
                             int k_cap, Real eps);
    void truncate_to_pmax(int p_max);
    int pmax() const;
    size_t count() const;
};

}  // namespace Marshal::Heat

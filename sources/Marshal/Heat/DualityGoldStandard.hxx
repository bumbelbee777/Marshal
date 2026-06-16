#pragma once
#include <string>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct DualityGoldStandardResult {
    Real a = 1;
    Real lhs_zero_sum = 0;
    Real rhs_arch = 0;
    Real rhs_poles = 0;
    Real rhs_prime = 0;
    Real rhs_total = 0;
    Real residual = 0;
    Real t1_gap = 0;
    bool t1_pass = false;
    bool pass = false;
    size_t n_zeros = 0;
    int n_primes = 0;
};

DualityGoldStandardResult run_duality_gold_standard(
    const Config& cfg, const double* gammas, size_t n_zeros, const Real* gammas_ld,
    size_t n_ld, PrimeCatalog& cat, const std::vector<int>& primes);

bool export_duality_gold_standard_json(const std::string& path, const DualityGoldStandardResult& r);

}  // namespace Marshal::Heat

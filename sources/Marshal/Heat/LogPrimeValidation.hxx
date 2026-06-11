#pragma once

#include <string>
#include <vector>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Heat {

struct PrimeLadderPoint {
    int p_max = 0;
    int n_primes = 0;
    Real p_weight_sinc2 = 0;
    Real weil_prime_sinc2 = 0;
};

struct Sinc2TSweepPoint {
    Real T = 0;
    Real zero_sum = 0;
    Real arch_plus_poles = 0;
    Real prime = 0;
    Real weil_residual = 0;
    Real h_at_gamma1 = 0;
};

struct LogPrimeValidationReport {
    bool t1_pass = false;
    bool weil_identity_pass = false;
    bool gauss_weil_identity_pass = false;
    Real t1_weil_vs_marshal_gap = 0;
    Real weil_identity_residual = 0;
    Real gauss_weil_identity_residual = 0;
    Real zero_sum = 0;
    Real marshal_prime = 0;
    Real hlog_weil_prime = 0;
    Real arch_plus_poles = 0;
    Real p_weight_sinc2_residual = 0;
    Real weil_prime_sinc2_residual = 0;
    Real p_weight_vs_zero_pct = 0;
    int n_primes_total = 0;
    int n_primes_global = 0;
    Real t2_cylinder_baseline = 12.67L;
    int t3_count_at_T = 0;
    Real t3_zero_count = 0;
    Real t4_best_sinc2 = 0;
    Real t4_best_lambda = 0;
    Real t5_sinc2_drift_halving = 0;
    Real t6_crossed_sinc2 = 0;
    Real sinc2_best_T = 0;
    Real sinc2_best_residual = 0;
    Real sinc2_best_zero_sum = 0;
    std::vector<PrimeLadderPoint> prime_ladder;
    std::vector<Sinc2TSweepPoint> sinc2_T_sweep;
};

LogPrimeValidationReport run_log_prime_validation(const Config& cfg,
                                                  const TestFunction& tf,
                                                  const std::vector<double>& gammas,
                                                  const std::vector<Real>& gammas_ld,
                                                  PrimeCatalog& cat,
                                                  const std::vector<int>& primes);

bool export_log_prime_validation_json(const std::string& path,
                                      const LogPrimeValidationReport& r);

}  // namespace Marshal::Heat

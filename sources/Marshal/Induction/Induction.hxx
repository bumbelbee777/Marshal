#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Analysis/ConvergenceStudy.hxx"
#include "Analysis/PairCorrelation.hxx"
#include "AnaVM/AnaFormal.hxx"
#include "AnaVM/FormalCalibration.hxx"
#include "Config.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"
#include "Cert/Verdict.hxx"
#include "MeasureLimitSweep.hxx"
#include "TraceApi.hxx"

namespace Marshal::Induction {

inline Real TauFromSigma(Real sigma) noexcept { return 1.0L / (2.0L * sigma * sigma); }

size_t CountEffectiveZeros(Real sigma, const std::vector<double>& gammas, Real thresh = 1e-16L);

std::unique_ptr<TestFunction> MakeTestFunction(const Config& cfg);
TraceResult RunEvaluate(const Config& cfg, const TestFunction& tf,
                        const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
                        const Heat::PrimeCatalog& cat);
Analysis::ResidualBudget ComputeResidualBudget(const TestFunction& tf, Real sigma,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat, ZeroKernel zk, SimdLevel simd,
    bool precision_mode, int arch_pts);

void RunHeatInduction(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);
void RunResidualScaling(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat, const std::vector<int>& primes);
void RunHpAnsatz(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat, const std::vector<int>& primes);

struct SpectralHpReport {
    Real trace_oracle_lhs = 0;
    Real trace_formula_residual = 0;
    Real max_eigenvalue_gap = 0;
    Real mean_eigenvalue_gap = 0;
    Real direct_sum_max_gap = 0;
    Real direct_sum_mean_gap = 0;
    Real matched_cylinder_max_gap = 0;
    Real matched_cylinder_mean_gap = 0;
    Real matched_sq_max_gap = 0;
    Real matched_sq_mean_gap = 0;
    Real fixed_mode_max_gap = 0;
    Real fixed_mode_mean_gap = 0;
    Real compact_sinc2_residual = 0;
    Real compact_sinc2_T = 0;
    bool compact_sinc2_mismatch_proved = false;
    Real quotient_max_gap = 0;
    Real quotient_mean_gap = 0;
    Real quotient_sq_max_gap = 0;
    Real quotient_sq_mean_gap = 0;
    Real exponent_gap_max = 0;
    Real exponent_gap_mean = 0;
    Real exponent_sq_gap_max = 0;
    Real exponent_sq_gap_mean = 0;
    Real compact_sinc2_quotient_lhs_residual = 0;
    Real fixed_quotient_gap_max = 0;
    Real fixed_quotient_sq_gap_max = 0;
    std::vector<std::string> expect_warnings;
    std::string anavm_program;
    std::string anavm_rule_id;
    bool anavm_scaffold = false;
    bool anavm_placeholder = false;
    bool anavm_falsify_sinc2 = false;
    Real max_gap_in_spacings = 0;
    Real mean_gap_in_spacings = 0;
    int n_pairs = 0;
    int quotient_mesh = 0;
    int quotient_k_primes = 0;
    int quotient_ncells = 0;
    size_t n_effective_zeros = 0;
    bool quotient_skipped = false;
    bool quotient_converged = false;
    int quotient_k_prev = 0;
    Real quotient_gap_prev_k = 0;
    std::string quotient_method = "continuum_haar_rayleigh";
    std::string quotient_k_selection = "fixed";
    bool lhs_underflow = false;
    bool trace_proved = false;
    bool spectrum_approximated = false;
    bool spectrum_identified = false;
    bool locked_spectrum_pass = false;
    bool prony_spectrum_pass = false;
    Real locked_spectrum_max_gap = 0;
    Real locked_spectrum_mean_gap = 0;
    Real prony_spectrum_max_gap = 0;
    Real prony_spectrum_mean_gap = 0;
    bool spec_trace_pass = false;
    bool spec_h_equals_gamma_n = false;
    bool spectral_mismatch = false;
    Heat::HeatTraceSweepResult heat_sweep;
    bool pair_correlation_ran = false;
    Analysis::PairCorrelationReport pair_correlation;
};

struct PairCorrelationRunResult {
    bool ok = false;
    Analysis::PairCorrelationReport report;
};

struct FormalAnalyticsRunResult {
    bool ok = false;
    AnaVM::FormalAnalyticsResult result;
    AnaVM::FormalCalibration calibration;
};

struct HpProofVerdict {
    bool local_self_adjoint = true;
    bool base_case = true;
    bool tier1_all = true;
    bool inductive_step = true;
    bool local_prime_heat = true;
    bool global_balance = true;
    bool spectrum_symmetry = true;
    bool local_hp_proved = false;
    bool spec_trace_pass = false;
    bool lhs_underflow = false;
    bool spectral_mismatch = false;
    Real max_eigenvalue_gap = 0;
    Real max_local_err = 0;
    Real global_residual = 0;
    Real ladder_max_err = 0;
    Real local_prime = 0;
    Real local_heat = 0;
    Real local_prime_heat_err = 0;
    Real local_geometric_residual = 0;
    size_t local_prime_count = 0;
    int p_max_local = 0;
    int tier1_failures = 0;
    int failures = 0;
    Real trace_formula_residual = 0;
    Real proof_eps_used = 0;
    bool machine_zero_pass = false;
    Real residual_fp_delta = 0;
};

SpectralHpReport ComputeSpectralHp(const TraceResult& global,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat, int N, Real sigma, Real proof_eps, const Config& cfg);

Cert::HpVerdictResult ComputeVerdict(const HpProofVerdict& v, const SpectralHpReport& spec);

HpProofVerdict RunHpProofInduction(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat);

void ExportTraceJson(const std::string& path, const Config& cfg, const TestFunction& tf,
    const TraceResult& r, const Heat::PrimeCatalog& cat, const Analysis::ResidualBudget& b, size_t n_zeros);
void ExportInductionJson(const std::string& path, const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);
void ExportSpectralJson(const std::string& path, int n, const Heat::PrimeCatalog& cat,
    const std::vector<double>& gammas);

bool ParseConfig(int argc, char** argv, Config& cfg, std::string& err);
void PrintUsage(const char* prog);
void PrintResult(Real sigma, const TraceResult& r);
void RunSweep(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat, const std::vector<int>& primes);
void RunMachineTest(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);
void RunCompactTest(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat);
void RunOperatorCandidates(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat);
PairCorrelationRunResult RunPairCorrelation(const Config& cfg, const std::vector<double>& gammas,
                                            const std::vector<int>& primes);
FormalAnalyticsRunResult RunFormalAnalytics(const Config& cfg, const std::vector<double>& gammas,
                                            const std::vector<int>& primes,
                                            const Analysis::PairCorrelationReport* pair);

}  // namespace Marshal::Induction

#pragma once
#include <cstddef>
#include <string>
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal {

enum class SimdLevel { Scalar, AVX2 };
enum class ZeroKernel { Float, LongDouble };

inline constexpr Real kDefaultSigma = 2.23606797749978969641L;

SimdLevel DetectSimd();
const char* SimdName(SimdLevel l);
const char* ZeroKernelName(ZeroKernel k);

struct AnaVmSnapshot {
    bool loaded = false;
    std::string id;
    std::string rule_id;
    std::string derived_omega;
    std::string derived_lambda;
    std::string lean_module;
    bool placeholder = false;
    bool scaffold = false;
    bool falsify_sinc2 = false;
    bool trace_lhs_quotient = false;
};

struct Config {
    Real sigma = kDefaultSigma;
    Real sigma_trace = 0;
    bool fast_mode = false;
    bool proof_mode = false;
    bool scale_mode = false;
    bool skip_quotient_prev = false;
    int induction_export_max = 0;
    bool do_sweep = false;
    Real sweep_min = 0.5;
    Real sweep_max = 4.0;
    int sweep_steps = 20;
    bool induction = false;
    bool ansatz = false;
    bool residual_scaling = false;
    bool machine_test = false;
    bool sign_check = false;
    bool compact_test = false;
    bool trivial_zeros = false;
    bool deterministic = false;
    bool checksum = false;
    std::string zeros_path = "tests/Fixtures/Zeros/odlyzko_zeros100k.txt";
    std::string csv_path;
    std::string export_trace_path;
    std::string export_induction_path;
    size_t max_zeros = 0;
    int prime_limit = 5000000;
    int kmax = 20;
    int nmax = 500;
    int ktheta = 200;
    int arch_pts = 400000;
    int threads = 0;
    int spectral_compare_n = 0;
    int quotient_mesh = 0;
    int quotient_primes = 0;
    int quotient_max_cells = 8000000;
    Real spec_max_gap = 1.0L;
    Real spec_mean_gap = 0.35L;
    Real spec_max_gap_spacings = 1.0L;
    int heat_sweep_n = 50;
    Real heat_sweep_t_min = 0;
    Real heat_sweep_t_max = 0;
    Real heat_sweep_tol = 1e-6L;
    Real eps = 1e-30L;
    Real s_euler = 0.5L;
    Real test_param = 0.0L;
    bool use_cache = true;
    bool hp_proof = false;
    bool precision_mode = false;
    int local_prime_count = 0;
    Real tier1_tol = 1e-10L;
    std::string export_hp_cert_path;
    std::string anavm_program;
    bool anavm_check = false;
    AnaVmSnapshot anavm;
    bool measure_limit_sweep = false;
    bool operator_candidates = false;
    bool pair_correlation = false;
    bool formal_analytics = false;
    int pair_correlation_n_cylinder = 0;
    int pair_correlation_max_zeros = 0;
    Real formal_counting_window = 100.0L;
    std::string export_formal_cal_path;
    std::string export_formal_analytics_path;
    std::string export_pair_correlation_path;
    bool log_prime_validation = false;
    bool log_prime_catalog = false;
    int log_prime_global_cap = 0;
    std::string export_log_prime_validation_path = "docs/generated/log_prime_validation.json";
    bool suggest_next = false;
    std::string lemma_manifest_path = "docs/Analysis/LemmaManifest.json";
    std::string ansatz_registry_path = "docs/Analysis/AnsatzRegistry.json";
    std::string export_next_actions_path = "build/cert/next_actions.json";
    TestKind test_kind = TestKind::Gauss;
    ZeroKernel zero_kernel = ZeroKernel::LongDouble;
    SimdLevel simd = DetectSimd();
};

}  // namespace Marshal

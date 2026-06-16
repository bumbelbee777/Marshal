#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "AnaVM/MrsTypes.hxx"
#include "Heat/ConnesCouplingMode.hxx"
#include "Investigation/InvestigationTypes.hxx"
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
    std::vector<AnaVM::FormalRef> lemma_refs;
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
    Real sinc2_kappa = 0.0L;
    bool arch_sinc2_audit = false;
    bool weil_convergence_study = false;
    std::string export_arch_sinc2_audit_path = "docs/generated/arch_sinc2_audit.json";
    std::string export_weil_convergence_path = "docs/generated/weil_convergence_gamma1.json";
    std::string export_connes_study_path = "docs/generated/connes_crossed_product_study.json";
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
    bool marshal_wedge_analytic_validation = false;
    bool xi_hadamard_proof_validation = false;
    int pair_correlation_n_cylinder = 0;
    int pair_correlation_max_zeros = 0;
    Real formal_counting_window = 100.0L;
    std::string export_formal_cal_path;
    std::string export_formal_analytics_path;
    std::string export_wedge_analytic_path =
        "docs/generated/marshal_wedge_analytic_cpp.json";
    std::string export_xi_hadamard_proof_path =
        "docs/generated/anavm_xi_hadamard_proof.json";
    std::string export_xi_hadamard_proof_graph_path =
        "docs/generated/anavm_xi_hadamard_proof_graph.json";
    /// Empty unless explicitly passed on CLI — Lean codegen is not the primary RH track.
    std::string export_xi_hadamard_lean_cert_path;
    std::string export_xi_hadamard_canonical_lean_path;
    std::string export_xi_hadamard_rh_closure_lean_path;
    std::string export_pair_correlation_path;
    bool log_prime_validation = false;
    bool log_prime_catalog = false;
    int log_prime_global_cap = 0;
    std::string export_log_prime_validation_path = "docs/generated/log_prime_validation.json";
    bool ntz_generate = false;
    std::string ntz_input;
    std::string ntz_output;
    std::string ntz_cache_path;
    std::string ntz_report_path = "tests/Fixtures/Zeros/NtzReport.json";
    std::string ntz_shard_dir;
    size_t ntz_count = 0;
    size_t ntz_offset = 0;
    size_t ntz_pad_to = 0;
    int ntz_batch_size = 256;
    double ntz_tol = 1e-14;
    bool ntz_refine = false;
    bool zeros_ingest = false;
    std::string zeros_ingest_input;
    std::string zeros_ingest_cache = "build/cache/zeros2m.zerocache";
    std::string zeros_ingest_shard_dir;
    size_t zeros_ingest_count = 0;
    bool arch_sinc2_converge = false;
    Real arch_target = 1e-12L;
    bool duality_gold_standard = false;
    Real duality_a = 1.0L;
    std::string export_duality_gold_path = "docs/generated/duality_gold_standard.json";
    bool connes_crossed_validation = false;
    bool connes_expect_spectrum_identified = false;
    bool diagnostics_expect_finite_spectrum_mismatch = false;
    Heat::ConnesCouplingMode connes_coupling_mode = Heat::ConnesCouplingMode::LogLadder;
    Real connes_coupling_lambda = 0;
    Real connes_spectrum_rmse_max = 1.0L;
    Real connes_spectrum_max_gap_max = 1.0L;
    std::vector<Real> connes_lambda_sweep;
    std::vector<int> connes_prime_ladder;
    std::string export_connes_crossed_path = "docs/generated/connes_spectrum_validation.json";
    bool completion_validation = false;
    bool archimedean_boundary_sweep = false;
    bool archimedean_sweep_all = true;
    bool spectral_determinant_validation = false;
    bool spectral_det_boundary_sweep = false;
    bool assembly_search = false;
    bool skip_archimedean_sweep = false;
    Real completion_tolerance_override = 0;
    bool height_a_override_set = false;
    bool height_b_override_set = false;
    int adelic_max_primes_override = 0;
    Real height_a_override = 1.0L;
    Real height_b_override = 0.0L;
    AnaVM::MrsCompletion completion;
    AnaVM::MrsAdelicCauchy adelic_cauchy;
    AnaVM::MrsArchimedean archimedean;
    AnaVM::MrsHeightMap height_map;
    AnaVM::MrsSpectralDeterminant spectral_determinant;
    AnaVM::MrsAssemblySearch assembly_search_spec;
    AnaVM::MrsSemiclassical semiclassical;
    AnaVM::MrsSelfAdjointExtension self_adjoint_extension;
    AnaVM::MrsSpectralAction spectral_action;
    AnaVM::MrsDiscretizationLimit discretization_limit;
    AnaVM::MrsFormalTarget formal_target;
    AnaVM::MrsBoundAudit bound_audit;
    AnaVM::MrsGenusOneLogBounds genus_one_log_bounds;
    bool self_adjoint_extension_sweep = false;
    bool spectral_action_validation = false;
    bool global_dirac_limit_validation = false;
    bool analytic_lemma_demo = false;
    bool trace_formula_gate = false;
    bool berry_keating_validation = false;
    bool gln_ladder_validation = false;
    bool gl2_ellipse_heegner_validation = false;
    bool bsd_proof_engine = false;
    bool hodge_proof_engine = false;
    bool goldbach_proof_engine = false;
    bool mrs_ladder_proof_engine = false;
    bool analytic_construction_validation = false;
    bool spectral_discreteness_check = false;
    Real diagnostics_weil_residual_max = 0;
    Real diagnostics_spectrum_rmse_max = 0;
    Real diagnostics_spectrum_max_gap_max = 0;
    std::string export_self_adjoint_ext_path =
        "docs/generated/self_adjoint_extension_sweep.json";
    std::string export_trace_formula_gate_path = "docs/generated/trace_formula_gate.json";
    std::string export_berry_keating_path = "docs/generated/berry_keating_validation.json";
    std::string export_gln_ladder_path = "docs/generated/marshal_gln_ladder_sweep.json";
    std::string export_hodge_k3_path = "docs/generated/marshal_hodge_k3_demo.json";
    std::string export_gl2_ellipse_heegner_path = "docs/generated/marshal_gl2_ellipse_heegner.json";
    std::string export_bsd_proof_path = "docs/generated/anavm_bsd_proof.json";
    std::string export_hodge_proof_path = "docs/generated/anavm_hodge_proof.json";
    std::string export_goldbach_proof_path = "docs/generated/anavm_goldbach_proof.json";
    std::string export_mrs_ladder_audit_path = "docs/generated/mrs_ladder_proof_audit.json";
    std::string export_mrs_ladder_closure_path = "docs/generated/mrs_ladder_closure.json";
    std::string export_analytic_construction_path =
        "docs/generated/analytic_construction.json";
    bool export_analytic_construction_user_set = false;
    std::string export_completion_path = "docs/generated/completion_validation.json";
    std::string export_archimedean_path = "docs/generated/archimedean_boundary_sweep.json";
    std::string export_spectral_det_path = "docs/generated/spectral_determinant.json";
    std::string export_spectral_action_path = "docs/generated/spectral_action_selection.json";
    bool export_spectral_action_user_set = false;
    std::string export_global_dirac_limit_path = "docs/generated/global_dirac_limit.json";
    bool export_global_dirac_limit_user_set = false;
    std::string export_analytic_lemma_demo_path = "docs/generated/analytic_lemma_demo.json";
    bool export_analytic_lemma_demo_user_set = false;

    bool investigation_run = false;
    bool investigation_quick = false;
    std::string investigation_id = "theorem_ab";
    std::string investigation_cert_root;
    std::string investigation_diag_id;
    Investigation::InvestigationSpec investigation_spec;

    std::string export_assembly_path = "docs/generated/assembly_search.json";
    std::string assembly_point_path;
    bool suggest_next = false;
    std::string lemma_manifest_path = "docs/Analysis/LemmaManifest.json";
    std::string ansatz_registry_path = "docs/Analysis/AnsatzRegistry.json";
    std::string export_next_actions_path = "build/cert/next_actions.json";
    TestKind test_kind = TestKind::Gauss;
    ZeroKernel zero_kernel = ZeroKernel::LongDouble;
    SimdLevel simd = DetectSimd();
};

}  // namespace Marshal

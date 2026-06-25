#pragma once

#include <string>

#include <vector>



namespace Marshal::AnaVM {



struct SourceSpan {

    std::string file;

    int line = 0;

    int col = 0;

};



enum class SymTier { Production, Scaffold };



enum class HeatCoupling { None, Poisson, BerryKeating, ConnesHeat };



enum class SymRuleStatus { Implemented, Partial, Placeholder };



enum class CompletionMethod { Cauchy, InductiveLimit, CrossedProduct };

enum class CompletionConstraint { Exact, Asymptotic };

enum class CompletionNorm { L2, Sup, Resolvent };

enum class AdelicMetric { Real, Adelic, Mixed };



enum class ArchimedeanType { Torus, RealLine, HalfLine };

enum class ArchimedeanBoundary { Dirichlet, Neumann, Periodic, BerryKeating };

enum class ArchimedeanCutoff { PlanckScale, ParameterLambda };

enum class HeightMapType { Log, Power, Custom };

enum class SpectralDetTarget { RiemannXi, Custom };

enum class SpectralDetMethod { HeatKernel, ZetaRegularization };

enum class ExtensionFamily { U1, U2, Unknown };

enum class SemiclassicalLadder { Wkb, Unknown };

enum class TraceFormulaStatus { Proved, Open, Unknown };

enum class TraceFormulaMode { WeilFull, Custom };



struct MrsError {

    std::string code;

    SourceSpan span;

    std::string message;

    std::string hint;

};



struct RescaleSpec {

    bool has_derivative_scale = false;

    std::string derivative_factor;  // e.g. "log(p)"

};



struct OnBlock {

    std::string space;

    std::string omega_expr;

    std::string lambda_expr;

    SourceSpan span;

};



struct FormalRef {

    std::string lemma_id;

    std::string status;  // open | proved | falsified

};



struct MrsCrossedProduct {

    std::string coupling = "log_ladder";

    double lambda = 0;

    double lambda_default = 0.1;

    int kmax = 0;

    std::vector<int> prime_ladder;

};



struct MrsCompletion {

    bool present = false;

    CompletionMethod method = CompletionMethod::Cauchy;

    CompletionConstraint constraint = CompletionConstraint::Asymptotic;

    double tolerance = 1e-6;

    CompletionNorm norm = CompletionNorm::L2;

};



struct MrsAdelicCauchy {

    bool present = false;

    AdelicMetric metric = AdelicMetric::Mixed;

    std::string search = "diophantine_approximation";

    int max_denominator = 1000;

    int max_primes = 100;

    bool include_raw_ladder = false;

    int target_zeros = 100;

};

struct MrsHeightMap {

    bool present = false;

    HeightMapType type = HeightMapType::Log;

    std::string formula;

    double a = 1.0;

    double b = 0.0;

    double alpha = 1.0;

};

struct MrsFormalTarget {

    bool present = false;

    std::string lemma;

    std::string approach;

    std::string proof_status = "open";

};

/// Numeric bound tolerances for AnaVM audits and Lean cert pins (GL(n)/RH extensible).
struct MrsBoundAudit {

    bool present = false;

    double grid_rel_gap_ub = 0.03;

    double grid_mult_dev_ub = 0.03;

    double tail_bound_decades_ub = 0.15;

    double ident_gap_decades_ub = 0.15;

    double holomorphy_uniform_gap_ub = 0.01;

    double log_partial_sum_ub = 8.0;

    double log_majorant_c = 1.05;

    double l_function_grid_rel_gap_ub = 0.03;

    double sha_resolvent_gap_ub = 2.0;

    double kernel_tolerance = 1e-6;

    double hodge_h11_target = 20;

    double major_arc_threshold = 0.45;

    double minor_arc_ub = 0.01;

    double goldbach_n0 = 4;

    double goldbach_effective_n_max = 10000;

    double goldbach_extension_ratio_lb = 10.0;

    double bsd_extension_ratio_lb = 2.0;

    double bsd_formula_rel_gap_ub = 0.05;

    double bsd_millennium_extension_ratio_lb = 2.0;

    double hodge_extension_ratio_lb = 1.0;

    double hodge_millennium_extension_ratio_lb = 1.0;

    double hodge_millennium_pp_target = 22;

    double rooted_rmse_ub = 0.001;

    double gauge_over_gravity_lb = 1.0;

    double holy_anchor_t = 3.141592653589793;

    double ym_mass_gap_lb = 2.0;

    double ym_extension_ratio_lb = 1.0;

    double ym_millennium_extension_ratio_lb = 1.0;

    double holy_stationarity_residual_ub = 50.0;

    double ym_lattice_beta = 5.7;

    double ym_lattice_volume = 64;

};

/// Genus-1 log summability audit (ported from Mathlib `GenusOneLogBounds` to AnaVM/MRS).
struct MrsGenusOneLogBounds {

    bool present = false;

    double small_z_threshold = 0.5;

    double head_majorant_margin = 0.01;

    std::string lemma = "marshal_genus_one_log_summability_closed";

    std::string program_id;

};

struct MrsDiscretizationLimit {

    bool present = false;

    std::string kind = "global_connes_dirac";

    std::vector<int> caps;

    std::string metric = "spectrum_rmse";

    std::string limit_target = "riemann_zero_heights";

    std::string formal_status = "open";

};

struct MrsSpectralAction {

    bool present = false;

    std::string selection = "min_action_subject_to_t1";

    /// combined_crossed_product — BK arch ⊗ coupled log-prime generator (v1 target)
    std::string action_proxy = "combined_crossed_product";

    double t1_tolerance = 1e-6;

    double weil_residual_max = 1.0;

    double heat_scale_base = 1.0;

    int heat_scales = 3;

    double coupling_lambda = 1.0;

    int mode_kmax = 40;

    double arch_action_weight = 1.0;

    double gue_spacing_l2_max = 0.4;

};

struct MrsSpectralDeterminant {

    bool present = false;

    SpectralDetTarget target = SpectralDetTarget::RiemannXi;

    SpectralDetMethod method = SpectralDetMethod::HeatKernel;

};

struct MrsAssemblySearch {

    bool present = false;

    int quick_zeros = 5000;

    int quick_primes = 10000;

    int full_zeros = 100000;

    int top_k = 10;

};

struct MrsLocal {

    bool present = false;

    bool spectral_triple = false;

    std::string hamiltonian_expr;

};

struct MrsQuotient {

    bool present = false;

    std::string kind;  // sunit | gl1 | custom

    std::string generators;

};

struct MrsSelfAdjointExtension {

    bool present = false;

    int deficiency_n = 1;

    int deficiency_m = 1;

    ExtensionFamily family = ExtensionFamily::U1;

    int sweep_steps = 24;

};

struct MrsTraceFormula {

    bool present = false;

    TraceFormulaStatus status = TraceFormulaStatus::Unknown;

    TraceFormulaMode mode = TraceFormulaMode::WeilFull;

};

struct MrsSemiclassical {

    bool present = false;

    ArchimedeanCutoff cutoff = ArchimedeanCutoff::PlanckScale;

    SemiclassicalLadder ladder = SemiclassicalLadder::Wkb;

    double log_span = 6.0;

    int max_modes = 500;

    bool height_renormalize_log_n = true;

};



struct MrsArchimedean {

    bool present = false;

    ArchimedeanType type = ArchimedeanType::RealLine;

    ArchimedeanBoundary boundary = ArchimedeanBoundary::BerryKeating;

    ArchimedeanCutoff cutoff = ArchimedeanCutoff::PlanckScale;

    double lambda_cutoff = 1.0;

};



struct MrsDiagnostics {

    bool local_weil_t1 = false;

    bool connes_crossed = false;

    bool archimedean_sweep = false;

    bool assembly_grid = false;

    std::string weil_identity;

    std::string trace_test;

    double test_param = 0;

    double sinc2_kappa = 0;

    double weil_residual_max = 0;

    double spectrum_rmse_max = 0;

    double spectrum_max_gap_max = 0;

    double sinc2_spectral_gap_max = 0;

    bool expect_spectrum_identified = false;

    bool expect_finite_spectrum_mismatch = false;

    bool self_adjoint_extension_sweep = false;

    bool trace_formula_gate = false;

    bool spectral_discreteness = false;

    bool spectrum_vs_zeros = false;

    bool spectral_action_selection = false;

    bool global_dirac_limit = false;

    bool analytic_lemma_demo = false;

    bool investigation_suite = false;

    bool hurwitz_spectral_action = false;

    bool theorem_b_scaffold = false;

    bool wedge_analytics = false;

    bool xi_hadamard_proof = false;

    std::string investigation_id;

};



struct MrsInvestigation {

    bool present = false;

    bool quick = false;

    std::string id = "theorem_ab";

    std::string cert_root;

    double fixed_theta = 5.76;

    std::string fixed_boundary = "periodic";

    double t1_tolerance = 1e-6;

    double heat_scale_base = 1.0;

    int heat_scales = 3;

    double coupling_lambda = 0.5;

    int combined_cap = 400;

    int arch_cap = 80;

    int mode_kmax = 12;

    double curvature_center = 5.76;

    double curvature_range = 1.0;

    int curvature_steps = 200;

    double topology_min = 0;

    double topology_max = 6.283185307179586;

    int topology_steps = 400;

    std::vector<double> heat_t_values;

    std::vector<int> prime_limit_values;

    bool run_curvature = true;

    bool run_topology = true;

    bool run_heat_trace = true;

    bool run_spacing = true;

    bool run_continuum = true;

};



struct MrsProgram {

    std::string id;

    std::string source_path;

    bool uses_gamma = false;

    SymTier sym_tier = SymTier::Production;

    HeatCoupling heat_coupling = HeatCoupling::Poisson;

    std::string kind = "operator";

    RescaleSpec rescale;

    std::vector<OnBlock> on_blocks;

    std::vector<std::string> expects;

    std::vector<FormalRef> lemma_refs;

    bool falsify_sinc2 = false;

    bool trace_lhs_quotient = false;

    bool pair_correlation_gue = false;

    bool formal_analytics = false;

    bool wedge_analytics = false;

    bool xi_hadamard_proof = false;

    bool placeholder = false;

    MrsCrossedProduct crossed_product;

    MrsCompletion completion;

    MrsAdelicCauchy adelic_cauchy;

    MrsArchimedean archimedean;

    MrsHeightMap height_map;

    MrsSpectralDeterminant spectral_determinant;

    MrsSpectralAction spectral_action;

    MrsDiscretizationLimit discretization_limit;

    MrsFormalTarget formal_target;

    MrsBoundAudit bound_audit;

    MrsGenusOneLogBounds genus_one_log_bounds;

    MrsAssemblySearch assembly_search;

    MrsLocal local;

    MrsQuotient quotient;

    MrsSelfAdjointExtension self_adjoint_extension;

    MrsTraceFormula trace_formula;

    MrsSemiclassical semiclassical;

    std::string rule_id;

    std::string derived_omega;

    std::string derived_lambda;

    std::string required_omega;

    bool weil_ok = true;

    MrsDiagnostics diagnostics;

    MrsInvestigation investigation;

};



}  // namespace Marshal::AnaVM


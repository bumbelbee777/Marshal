#include "Induction.hxx"
#include "Heat/ConnesCouplingMode.hxx"
#include "Heat/Common.hxx"
#include "Compat.hxx"
#include "Cert/Schema.hxx"
#include "Cert/Verdict.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Quotient/QuotientToy.hxx"
#include "SpectralDiagnostic.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Induction {

void PrintUsage(const char* prog) {
    std::cerr
        << "Usage: " << prog << " [options]\n"
        << "  --sigma S              Gaussian width (default " << kDefaultSigma << ")\n"
        << "  --test gauss|sinc2|bump|rational|laplace\n"
        << "  --test-param X         sinc2 T, bump/rational scale\n"
        << "  --sinc2-kappa K        sinc2 frequency stretch (0=auto)\n"
        << "  --arch-sinc2-audit     arch quadrature L x n_pts sweep\n"
        << "  --export-arch-sinc2 F  arch_sinc2_audit.json output\n"
        << "  --weil-convergence-study  zero/prime truncation ladders at T=gamma1\n"
        << "  --export-weil-convergence F  weil_convergence_gamma1.json output\n"
        << "  --export-connes-study F  connes_crossed_product_study.json output\n"
        << "  --connes-crossed-validation  spectrum RMSE vs gamma_n ladder\n"
        << "  --export-connes-crossed F  connes_spectrum_validation.json output\n"
        << "  --completion-validation      adelic Cauchy completion vs zeros\n"
        << "  --export-completion F        completion_validation.json output\n"
        << "  --spectral-determinant         spectral det vs completed xi (Laplace h)\n"
        << "  --spectral-det-sweep           sweep arch boundaries for xi_det_gap\n"
        << "  --export-spectral-det F        spectral_determinant.json output\n"
        << "  --adelic-max-primes N          override adelic_cauchy max_primes\n"
        << "  --assembly-search              tiered assembly parameter grid\n"
        << "  --export-assembly F            assembly_search.json output\n"
        << "  --assembly-point F             single grid point JSON for workers\n"
        << "  --completion-tolerance E       override completion/adelic epsilon\n"
        << "  --height-a A                   override height_map.a\n"
        << "  --height-b B                   override height_map.b\n"
        << "  --skip-archimedean-sweep       skip archimedean matrix (faster sweeps)\n"
        << "  --archimedean-boundary-sweep archimedean boundary test-function matrix\n"
        << "  --export-archimedean F       archimedean_boundary_sweep.json output\n"
        << "  --archimedean-sweep-all      sweep all boundary specs (default on)\n"
        << "  --no-archimedean-sweep-all   single .mrs archimedean spec only\n"
        << "  --connes-coupling MODE     log_ladder|prime_power (default log_ladder)\n"
        << "  --connes-lambda L          default crossed-product coupling strength\n"
        << "  --export-trace FILE    JSON trace output\n"
        << "  --export-induction FILE  prime ladder JSON\n"
        << "  --export-hp-cert FILE    local HP proof certificate JSON\n"
        << "  --local-primes N         prove HP for first N primes (0=all)\n"
        << "  --local-cylinder-tol T   Local-cylinder identity tolerance (default 1e-10)\n"
        << "  --tier1-tol T            Alias for --local-cylinder-tol\n"
        << "  --spectral-compare N   compare Spec(H) vs zeros\n"
        << "  --compact-test         compact h_hat falsification test\n"
        << "  --sign-check           write trace and exit\n"
        << "  --trivial-zeros        add trivial zero contribution to LHS\n"
        << "  --deterministic        fixed scheduling\n"
        << "  --threads N            OpenMP thread count\n"
        << "  --checksum             print FNV checksum\n"
        << "  --zeros PATH           zero file\n"
        << "  --prime-limit N        sieve limit\n"
        << "  --hp-proof             controlled verification scaffold\n"
        << "  --precision            LD zeros + gh1024 arch + tight eps\n"
        << "  --sigma-trace S        sigma for trace-identity heat sweep (default: --sigma)\n"
        << "  --quotient-mesh M      spectrum-diagnostic product grid mesh (0=auto)\n"
        << "  --quotient-primes K    spectrum-diagnostic prime axes (0=auto)\n"
        << "  --quotient-max-cells N spectrum-diagnostic cell budget (default 8M)\n"
        << "  --spec-max-gap G       quotient diagnostic gap threshold (default 1)\n"
        << "  --spec-mean-gap G      quotient diagnostic mean gap threshold (default 0.5)\n"
        << "  --ansatz               HP ansatz + scaling + induction\n"
        << "  --fast                 skip heavy convergence / sweep rebuilds\n"
        << "  --proof                lite regime-valid convergence sweep (--fast compatible)\n"
        << "  --scale                SoA hot paths for large prime catalogs\n"
        << "  --simd scalar|avx2\n"
        << "  --anavm PATH           AnaVM operator program (.mrs)\n"
        << "  --anavm-check          compile .mrs only, then exit\n"
        << "  --measure-limit-sweep  sinc² residual vs prime_limit ladder (conjecture D)\n"
        << "  --operator-candidates  trait inference + ranked operator candidate sweep\n"
        << "  --pair-correlation     cylinder vs zero spacing / Montgomery GUE metrics\n"
        << "  --formal-analytics     AnaVM built-in analytic gates (no Lean per check)\n"
        << "  --pair-cyl-levels N    cylinder levels for pair correlation (0=auto)\n"
        << "  --pair-max-zeros N     max zeros for pair correlation (0=auto)\n"
        << "  --counting-window T    formal analytics counting window (default 100)\n"
        << "  --export-pair-cor F    export pair_correlation.json\n"
        << "  --export-formal-analytics F  export formal_analytics.json\n"
        << "  --wedge-analytic           genus-1 log tail gate (Marshal wedge spine)\n"
        << "  --export-wedge-analytic F  marshal_wedge_analytic_cpp.json output\n"
        << "  --export-formal-cal F  formal calibration / measure-limit / candidates JSON\n"
        << "  --log-prime-validation  H_log weighted trace validation suite (T1-T6)\n"
        << "  --log-prime-catalog     full-scale validation + induction ladder\n"
        << "  --log-prime-cap N       max primes for global sinc2 (0=all)\n"
        << "  --export-log-prime F    log_prime_validation.json output\n"
        << "  --suggest-next         inference engine: emit build/cert/next_actions.json\n"
        << "  --lemma-manifest F     LemmaManifest.json path (default docs/Analysis/...)\n"
        << "  --ansatz-registry F    AnsatzRegistry.json path\n"
        << "  --export-next-actions F  next_actions.json output path\n"
        << "  --ntz-generate           native Riemann-Siegel zero refinement (no mpmath)\n"
        << "  --ntz-input PATH         coarse zero file (mmap)\n"
        << "  --ntz-output PATH        refined text output\n"
        << "  --ntz-cache PATH         binary .zerocache output\n"
        << "  --ntz-report PATH        NtzReport.json\n"
        << "  --ntz-shard-dir PATH     temp shard directory\n"
        << "  --ntz-count N            zeros to refine (0=all from input)\n"
        << "  --ntz-offset N           skip first N input lines\n"
        << "  --ntz-pad-to N           append coarse tail to merged output\n"
        << "  --ntz-batch N            OpenMP batch size (default 256)\n"
        << "  --ntz-tol T              Newton tolerance (default 1e-14)\n"
        << "  --ntz-refine             Newton-refine coarse seeds (default: mmap passthrough)\n"
        << "  --zeros-ingest            fast mmap text -> .zerocache (no mpmath)\n"
        << "  --zeros-input PATH        ingest source (alias --ntz-input)\n"
        << "  --zeros-cache PATH        ingest output cache\n"
        << "  --zeros-count N           lines to ingest (0=all)\n"
        << "  --arch-sinc2-converge     arch quadrature until |delta|<target\n"
        << "  --arch-target T           arch convergence tolerance (default 1e-12)\n"
        << "  --duality-gold-standard   Laplace h(t)=exp(-a|t|) gold cert\n"
        << "  --duality-a A             Laplace parameter (default 1)\n"
        << "  --export-duality-gold F   duality_gold_standard.json\n"
        << "  --self-adjoint-ext-sweep  boundary-parameter extension sweep\n"
        << "  --export-self-adjoint-ext F  self_adjoint_extension_sweep.json\n"
        << "  --trace-formula-gate      Weil trace identity gate (Laplace/Gauss/sinc2)\n"
        << "  --export-trace-formula-gate F  trace_formula_gate.json\n"
        << "  --berry-keating-validation  BK WKB ladder + extension sweep\n"
        << "  --export-berry-keating F    berry_keating_validation.json\n"
        << "  --gln-ladder-validation     GL(n) rank 1-4 ladder sweep\n"
        << "  --export-gln-ladder F       marshal_gln_ladder_sweep.json\n"
        << "  --export-hodge-k3 F         marshal_hodge_k3_demo.json\n"
        << "  --analytic-construction     4-step Connes analytic pipeline\n"
        << "  --spectral-discreteness-check  alias for --analytic-construction (shape gate)\n"
        << "  --export-analytic-construction F  analytic_construction.json\n"
        << "  --spectral-action-validation  extension selection via heat-kernel action proxy\n"
        << "  --export-spectral-action F      spectral_action_selection.json\n"
        << "  --global-dirac-limit-validation formal discretization limit ladder\n"
        << "  --export-global-dirac-limit F   global_dirac_limit.json\n"
        << "  --analytic-lemma-demo           per-lemma analytic demonstration cert\n"
        << "  --export-analytic-lemma-demo F  analytic_lemma_demo.json\n"
        << "  --investigation ID              run investigation suite (programs/investigations/ID.mrs)\n"
        << "  --investigation-mrs F           run investigation from explicit MRS path\n"
        << "  --diag DIAG_ID                  run single diagnostic from investigation spec\n"
        << "  --cert-root DIR                 investigation cert output root\n"
        << "  --quick                         investigation quick preset (fewer steps)\n"
        << "  --fixed-theta F                 investigation fixed theta (default 5.76)\n"
        << "  --t1-tolerance F                investigation T1 tolerance\n";
}

bool ParseConfig(int argc, char** argv, Config& cfg, std::string& err) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto need = [&](const char* name) -> std::string {
            if (i + 1 >= argc) { err = std::string("Missing value for ") + name; return {}; }
            return argv[++i];
        };
        if (arg == "--sigma")           cfg.sigma = std::stold(need("--sigma"));
        else if (arg == "--sigma-trace" || arg == "--sigma-weil") cfg.sigma_trace = std::stold(need(arg.c_str()));
        else if (arg == "--quotient-mesh") cfg.quotient_mesh = std::stoi(need("--quotient-mesh"));
        else if (arg == "--quotient-primes") cfg.quotient_primes = std::stoi(need("--quotient-primes"));
        else if (arg == "--quotient-max-cells") cfg.quotient_max_cells = std::stoi(need("--quotient-max-cells"));
        else if (arg == "--spec-max-gap") cfg.spec_max_gap = std::stold(need("--spec-max-gap"));
        else if (arg == "--spec-mean-gap") cfg.spec_mean_gap = std::stold(need("--spec-mean-gap"));
        else if (arg == "--heat-sweep-n") cfg.heat_sweep_n = std::stoi(need("--heat-sweep-n"));
        else if (arg == "--heat-sweep-t-min") cfg.heat_sweep_t_min = std::stold(need("--heat-sweep-t-min"));
        else if (arg == "--heat-sweep-t-max") cfg.heat_sweep_t_max = std::stold(need("--heat-sweep-t-max"));
        else if (arg == "--heat-sweep-tol") cfg.heat_sweep_tol = std::stold(need("--heat-sweep-tol"));
        else if (arg == "--test")       cfg.test_kind = parse_test_kind(need("--test"));
        else if (arg == "--test-param") cfg.test_param = std::stold(need("--test-param"));
        else if (arg == "--sinc2-kappa") cfg.sinc2_kappa = std::stold(need("--sinc2-kappa"));
        else if (arg == "--arch-sinc2-audit") cfg.arch_sinc2_audit = true;
        else if (arg == "--export-arch-sinc2")
            cfg.export_arch_sinc2_audit_path = need("--export-arch-sinc2");
        else if (arg == "--weil-convergence-study") cfg.weil_convergence_study = true;
        else if (arg == "--export-weil-convergence")
            cfg.export_weil_convergence_path = need("--export-weil-convergence");
        else if (arg == "--export-connes-study")
            cfg.export_connes_study_path = need("--export-connes-study");
        else if (arg == "--connes-crossed-validation") cfg.connes_crossed_validation = true;
        else if (arg == "--export-connes-crossed")
            cfg.export_connes_crossed_path = need("--export-connes-crossed");
        else if (arg == "--completion-validation") cfg.completion_validation = true;
        else if (arg == "--export-completion")
            cfg.export_completion_path = need("--export-completion");
        else if (arg == "--spectral-determinant") cfg.spectral_determinant_validation = true;
        else if (arg == "--spectral-det-sweep") {
            cfg.spectral_determinant_validation = true;
            cfg.spectral_det_boundary_sweep = true;
        }
        else if (arg == "--adelic-max-primes")
            cfg.adelic_max_primes_override = std::stoi(need("--adelic-max-primes"));
        else if (arg == "--export-spectral-det")
            cfg.export_spectral_det_path = need("--export-spectral-det");
        else if (arg == "--assembly-search") cfg.assembly_search = true;
        else if (arg == "--export-assembly")
            cfg.export_assembly_path = need("--export-assembly");
        else if (arg == "--assembly-point") {
            cfg.assembly_point_path = need("--assembly-point");
            cfg.assembly_search = true;
        }
        else if (arg == "--completion-tolerance")
            cfg.completion_tolerance_override = std::stold(need("--completion-tolerance"));
        else if (arg == "--height-a") {
            cfg.height_a_override = std::stold(need("--height-a"));
            cfg.height_a_override_set = true;
        } else if (arg == "--height-b") {
            cfg.height_b_override = std::stold(need("--height-b"));
            cfg.height_b_override_set = true;
        }
        else if (arg == "--skip-archimedean-sweep") cfg.skip_archimedean_sweep = true;
        else if (arg == "--archimedean-boundary-sweep") cfg.archimedean_boundary_sweep = true;
        else if (arg == "--export-archimedean")
            cfg.export_archimedean_path = need("--export-archimedean");
        else if (arg == "--archimedean-sweep-all") cfg.archimedean_sweep_all = true;
        else if (arg == "--no-archimedean-sweep-all") cfg.archimedean_sweep_all = false;
        else if (arg == "--connes-coupling")
            cfg.connes_coupling_mode =
                Heat::parse_connes_coupling_mode(need("--connes-coupling"));
        else if (arg == "--connes-lambda")
            cfg.connes_coupling_lambda = std::stold(need("--connes-lambda"));
        else if (arg == "--export-trace") cfg.export_trace_path = need("--export-trace");
        else if (arg == "--export-induction") cfg.export_induction_path = need("--export-induction");
        else if (arg == "--export-hp-cert") cfg.export_hp_cert_path = need("--export-hp-cert");
        else if (arg == "--local-primes") cfg.local_prime_count = std::stoi(need("--local-primes"));
        else if (arg == "--local-cylinder-tol" || arg == "--tier1-tol")
            cfg.tier1_tol = std::stold(need(arg.c_str()));
        else if (arg == "--spectral-compare") cfg.spectral_compare_n = std::stoi(need("--spectral-compare"));
        else if (arg == "--compact-test") cfg.compact_test = true;
        else if (arg == "--sign-check") cfg.sign_check = true;
        else if (arg == "--trivial-zeros") cfg.trivial_zeros = true;
        else if (arg == "--deterministic") cfg.deterministic = true;
        else if (arg == "--checksum") cfg.checksum = true;
        else if (arg == "--threads") cfg.threads = std::stoi(need("--threads"));
        else if (arg == "--sweep") {
            cfg.do_sweep = true;
            cfg.sweep_min = std::stold(need("--sweep"));
            cfg.sweep_max = std::stold(need("--sweep"));
            cfg.sweep_steps = std::stoi(need("--sweep"));
        }
        else if (arg == "--csv")        cfg.csv_path = need("--csv");
        else if (arg == "--zeros")      cfg.zeros_path = need("--zeros");
        else if (arg == "--max-zeros")  cfg.max_zeros = static_cast<size_t>(std::stoull(need("--max-zeros")));
        else if (arg == "--prime-limit") cfg.prime_limit = std::stoi(need("--prime-limit"));
        else if (arg == "--kmax")       cfg.kmax = std::stoi(need("--kmax"));
        else if (arg == "--nmax")       cfg.nmax = std::stoi(need("--nmax"));
        else if (arg == "--ktheta")     cfg.ktheta = std::stoi(need("--ktheta"));
        else if (arg == "--eps")        cfg.eps = std::stold(need("--eps"));
        else if (arg == "--s-euler")    cfg.s_euler = std::stold(need("--s-euler"));
        else if (arg == "--no-cache")   cfg.use_cache = false;
        else if (arg == "--induction" || arg == "--induction-report" || arg == "--layer1")
            cfg.induction = true;
        else if (arg == "--ansatz") cfg.ansatz = true;
        else if (arg == "--residual-scaling") cfg.residual_scaling = true;
        else if (arg == "--machine-test" || arg == "--heat-verify") cfg.machine_test = true;
        else if (arg == "--fast") cfg.fast_mode = true;
        else if (arg == "--proof") cfg.proof_mode = true;
        else if (arg == "--scale") cfg.scale_mode = true;
        else if (arg == "--skip-quotient-prev") cfg.skip_quotient_prev = true;
        else if (arg == "--induction-export-max") cfg.induction_export_max = std::stoi(need("--induction-export-max"));
        else if (arg == "--hp-proof") cfg.hp_proof = true;
        else if (arg == "--precision") {
            cfg.precision_mode = true;
            cfg.zero_kernel = ZeroKernel::LongDouble;
            cfg.eps = 1e-45L;
            cfg.arch_pts = 1024001;
            cfg.heat_sweep_tol = 1e-7L;
            cfg.heat_sweep_n = 64;
            if (cfg.kmax < 40) cfg.kmax = 40;
        }
        else if (arg == "--float-zero") cfg.zero_kernel = ZeroKernel::Float;
        else if (arg == "--simd") {
            std::string s = need("--simd");
            cfg.simd = (s == "avx2") ? SimdLevel::AVX2 : SimdLevel::Scalar;
        }
        else if (arg == "--anavm") cfg.anavm_program = need("--anavm");
        else if (arg == "--anavm-check") cfg.anavm_check = true;
        else if (arg == "--measure-limit-sweep") cfg.measure_limit_sweep = true;
        else if (arg == "--operator-candidates") cfg.operator_candidates = true;
        else if (arg == "--pair-correlation") cfg.pair_correlation = true;
        else if (arg == "--formal-analytics") cfg.formal_analytics = true;
        else if (arg == "--pair-cyl-levels") cfg.pair_correlation_n_cylinder = std::stoi(need("--pair-cyl-levels"));
        else if (arg == "--pair-max-zeros") cfg.pair_correlation_max_zeros = std::stoi(need("--pair-max-zeros"));
        else if (arg == "--counting-window") cfg.formal_counting_window = std::stold(need("--counting-window"));
        else if (arg == "--export-pair-cor") cfg.export_pair_correlation_path = need("--export-pair-cor");
        else if (arg == "--export-formal-analytics")
            cfg.export_formal_analytics_path = need("--export-formal-analytics");
        else if (arg == "--wedge-analytic") cfg.marshal_wedge_analytic_validation = true;
        else if (arg == "--export-wedge-analytic")
            cfg.export_wedge_analytic_path = need("--export-wedge-analytic");
        else if (arg == "--xi-hadamard-proof") cfg.xi_hadamard_proof_validation = true;
        else if (arg == "--export-xi-hadamard-proof")
            cfg.export_xi_hadamard_proof_path = need("--export-xi-hadamard-proof");
        else if (arg == "--export-xi-hadamard-proof-graph")
            cfg.export_xi_hadamard_proof_graph_path = need("--export-xi-hadamard-proof-graph");
        else if (arg == "--export-xi-hadamard-lean-cert")
            cfg.export_xi_hadamard_lean_cert_path = need("--export-xi-hadamard-lean-cert");
        else if (arg == "--export-xi-hadamard-canonical-lean")
            cfg.export_xi_hadamard_canonical_lean_path = need("--export-xi-hadamard-canonical-lean");
        else if (arg == "--export-xi-hadamard-rh-closure-lean")
            cfg.export_xi_hadamard_rh_closure_lean_path = need("--export-xi-hadamard-rh-closure-lean");
        else if (arg == "--export-formal-cal") cfg.export_formal_cal_path = need("--export-formal-cal");
        else if (arg == "--log-prime-validation") cfg.log_prime_validation = true;
        else if (arg == "--log-prime-catalog") {
            cfg.log_prime_validation = true;
            cfg.log_prime_catalog = true;
        }
        else if (arg == "--log-prime-cap") cfg.log_prime_global_cap = std::stoi(need("--log-prime-cap"));
        else if (arg == "--export-log-prime") cfg.export_log_prime_validation_path = need("--export-log-prime");
        else if (arg == "--suggest-next") cfg.suggest_next = true;
        else if (arg == "--lemma-manifest") cfg.lemma_manifest_path = need("--lemma-manifest");
        else if (arg == "--ansatz-registry") cfg.ansatz_registry_path = need("--ansatz-registry");
        else if (arg == "--export-next-actions") cfg.export_next_actions_path = need("--export-next-actions");
        else if (arg == "--ntz-generate") cfg.ntz_generate = true;
        else if (arg == "--ntz-input") cfg.ntz_input = need("--ntz-input");
        else if (arg == "--ntz-output") cfg.ntz_output = need("--ntz-output");
        else if (arg == "--ntz-cache") cfg.ntz_cache_path = need("--ntz-cache");
        else if (arg == "--ntz-report") cfg.ntz_report_path = need("--ntz-report");
        else if (arg == "--ntz-shard-dir") cfg.ntz_shard_dir = need("--ntz-shard-dir");
        else if (arg == "--ntz-count")
            cfg.ntz_count = static_cast<size_t>(std::stoull(need("--ntz-count")));
        else if (arg == "--ntz-offset")
            cfg.ntz_offset = static_cast<size_t>(std::stoull(need("--ntz-offset")));
        else if (arg == "--ntz-pad-to")
            cfg.ntz_pad_to = static_cast<size_t>(std::stoull(need("--ntz-pad-to")));
        else if (arg == "--ntz-batch") cfg.ntz_batch_size = std::stoi(need("--ntz-batch"));
        else if (arg == "--ntz-tol") cfg.ntz_tol = std::stod(need("--ntz-tol"));
        else if (arg == "--ntz-refine") cfg.ntz_refine = true;
        else if (arg == "--zeros-ingest") cfg.zeros_ingest = true;
        else if (arg == "--zeros-input") cfg.zeros_ingest_input = need("--zeros-input");
        else if (arg == "--zeros-cache") cfg.zeros_ingest_cache = need("--zeros-cache");
        else if (arg == "--zeros-count")
            cfg.zeros_ingest_count = static_cast<size_t>(std::stoull(need("--zeros-count")));
        else if (arg == "--arch-sinc2-converge") cfg.arch_sinc2_converge = true;
        else if (arg == "--arch-target") cfg.arch_target = std::stold(need("--arch-target"));
        else if (arg == "--duality-gold-standard") cfg.duality_gold_standard = true;
        else if (arg == "--duality-a") cfg.duality_a = std::stold(need("--duality-a"));
        else if (arg == "--export-duality-gold")
            cfg.export_duality_gold_path = need("--export-duality-gold");
        else if (arg == "--self-adjoint-ext-sweep") cfg.self_adjoint_extension_sweep = true;
        else if (arg == "--export-self-adjoint-ext")
            cfg.export_self_adjoint_ext_path = need("--export-self-adjoint-ext");
        else if (arg == "--trace-formula-gate") cfg.trace_formula_gate = true;
        else if (arg == "--export-trace-formula-gate")
            cfg.export_trace_formula_gate_path = need("--export-trace-formula-gate");
        else if (arg == "--berry-keating-validation") cfg.berry_keating_validation = true;
        else if (arg == "--gln-ladder-validation") cfg.gln_ladder_validation = true;
        else if (arg == "--gl2-ellipse-heegner-validation") cfg.gl2_ellipse_heegner_validation = true;
        else if (arg == "--bsd-proof-engine") cfg.bsd_proof_engine = true;
        else if (arg == "--hodge-proof-engine") cfg.hodge_proof_engine = true;
        else if (arg == "--goldbach-proof-engine") cfg.goldbach_proof_engine = true;
        else if (arg == "--mrs-ladder-proof-engine") cfg.mrs_ladder_proof_engine = true;
        else if (arg == "--export-gln-ladder")
            cfg.export_gln_ladder_path = need("--export-gln-ladder");
        else if (arg == "--export-hodge-k3")
            cfg.export_hodge_k3_path = need("--export-hodge-k3");
        else if (arg == "--export-gl2-ellipse-heegner")
            cfg.export_gl2_ellipse_heegner_path = need("--export-gl2-ellipse-heegner");
        else if (arg == "--export-bsd-proof")
            cfg.export_bsd_proof_path = need("--export-bsd-proof");
        else if (arg == "--export-hodge-proof")
            cfg.export_hodge_proof_path = need("--export-hodge-proof");
        else if (arg == "--export-goldbach-proof")
            cfg.export_goldbach_proof_path = need("--export-goldbach-proof");
        else if (arg == "--export-mrs-ladder-audit")
            cfg.export_mrs_ladder_audit_path = need("--export-mrs-ladder-audit");
        else if (arg == "--export-mrs-ladder-closure")
            cfg.export_mrs_ladder_closure_path = need("--export-mrs-ladder-closure");
        else if (arg == "--export-berry-keating")
            cfg.export_berry_keating_path = need("--export-berry-keating");
        else if (arg == "--analytic-construction" || arg == "--spectral-discreteness-check")
            cfg.analytic_construction_validation = true;
        else if (arg == "--export-analytic-construction") {
            cfg.export_analytic_construction_path = need("--export-analytic-construction");
            cfg.export_analytic_construction_user_set = true;
        }
        else if (arg == "--spectral-action-validation") cfg.spectral_action_validation = true;
        else if (arg == "--global-dirac-limit-validation")
            cfg.global_dirac_limit_validation = true;
        else if (arg == "--export-spectral-action") {
            cfg.export_spectral_action_path = need("--export-spectral-action");
            cfg.export_spectral_action_user_set = true;
        } else if (arg == "--export-global-dirac-limit") {
            cfg.export_global_dirac_limit_path = need("--export-global-dirac-limit");
            cfg.export_global_dirac_limit_user_set = true;
        }
        else if (arg == "--analytic-lemma-demo") cfg.analytic_lemma_demo = true;
        else if (arg == "--export-analytic-lemma-demo") {
            cfg.export_analytic_lemma_demo_path = need("--export-analytic-lemma-demo");
            cfg.export_analytic_lemma_demo_user_set = true;
        }
        else if (arg == "--investigation") {
            cfg.investigation_run = true;
            cfg.investigation_id = need("--investigation");
            cfg.anavm_program = "programs/investigations/" + cfg.investigation_id + ".mrs";
        }
        else if (arg == "--investigation-mrs") {
            cfg.investigation_run = true;
            cfg.anavm_program = need("--investigation-mrs");
        }
        else if (arg == "--diag") {
            cfg.investigation_run = true;
            cfg.investigation_diag_id = need("--diag");
        }
        else if (arg == "--cert-root") cfg.investigation_cert_root = need("--cert-root");
        else if (arg == "--quick") cfg.investigation_quick = true;
        else if (arg == "--fixed-theta")
            cfg.investigation_spec.fixed_theta = std::stold(need("--fixed-theta"));
        else if (arg == "--t1-tolerance")
            cfg.investigation_spec.t1_tolerance = std::stold(need("--t1-tolerance"));
        else if (arg == "-h" || arg == "--help") { PrintUsage(argv[0]); std::exit(0); }
        else { err = std::string("Unknown: ") + arg; return false; }
        if (!err.empty()) return false;
    }
    return validate_config(cfg.sigma, cfg.prime_limit, cfg.do_sweep, cfg.sweep_min,
                           cfg.sweep_max, cfg.sweep_steps, cfg.max_zeros,
                           cfg.kmax, cfg.nmax, cfg.ktheta, err);
}

void PrintResult(Real sigma, const TraceResult& r) {
    std::cout << std::scientific << std::setprecision(12);
    std::cout << "sigma=" << std::fixed << std::setprecision(6)
              << static_cast<double>(sigma) << std::scientific << "\n";
    std::cout << "  Poles:              " << static_cast<double>(r.poles) << "\n";
    std::cout << "  Archimedean:        " << static_cast<double>(r.arch) << "\n";
    std::cout << "  Prime sum (h_hat):  " << static_cast<double>(r.prime) << "\n";
    std::cout << "  Heat AB (linked):   " << static_cast<double>(r.heat_prime_ab) << "\n";
    std::cout << "  Geometric RHS:      " << static_cast<double>(r.rhs) << "\n";
    std::cout << "  Spectral LHS:       " << static_cast<double>(r.lhs) << "\n";
    std::cout << "  Residual (LHS-RHS): " << static_cast<double>(r.residual()) << "\n";
    std::cout << "  |prime-heat|:       "
              << static_cast<double>(fabsl(r.prime - r.heat_prime_ab)) << "\n";
}

void RunSweep(const Config& cfg, const TestFunction& /*tf*/,
                      const std::vector<double>& gammas,
                      const std::vector<Real>& gammas_ld,
                      Heat::PrimeCatalog& cat, const std::vector<int>& primes) {
    std::ofstream csv;
    if (!cfg.csv_path.empty()) {
        csv.open(cfg.csv_path);
        csv << "sigma,LHS,RHS,residual,poles,arch,prime,heat_ab\n";
    }
    for (int step = 0; step <= cfg.sweep_steps; ++step) {
        const Real t = static_cast<Real>(step) / static_cast<Real>(cfg.sweep_steps);
        const Real sigma = cfg.sweep_min + t * (cfg.sweep_max - cfg.sweep_min);
        GaussTest gt(sigma);
        cat.set_primes(primes);
        cat.rebuild_adaptive(gt, TauFromSigma(sigma), cfg.kmax, cfg.eps);
        const SimdLevel simd = cfg.precision_mode ? SimdLevel::Scalar : cfg.simd;
        const TraceResult r = EvaluateTrace(gt, sigma, gammas, gammas_ld, cat,
                                      cfg.zero_kernel, simd, cfg.eps, false,
                                      cfg.precision_mode, cfg.arch_pts);
        const Real abs_res = fabsl(r.residual());
        std::cout << std::scientific << std::setprecision(9)
                  << static_cast<double>(sigma) << " "
                  << static_cast<double>(r.residual()) << " "
                  << static_cast<double>(abs_res) << " "
                  << static_cast<double>(r.lhs) << " "
                  << static_cast<double>(r.poles + r.arch - r.prime) << "\n";
        if (csv) csv << static_cast<double>(sigma) << ","
                     << static_cast<double>(r.lhs) << ","
                     << static_cast<double>(r.rhs) << ","
                     << static_cast<double>(r.residual()) << "\n";
    }
}

void RunMachineTest(const Config& cfg, const TestFunction& tf,
                             const std::vector<double>& gammas,
                             const std::vector<Real>& gammas_ld,
                             const Heat::PrimeCatalog& cat) {
    std::cout << "=== Precision diagnostic ===\n";
    const TraceResult r_ld = EvaluateTrace(tf, cfg.sigma, gammas, gammas_ld, cat,
                                     ZeroKernel::LongDouble, cfg.simd, cfg.eps, false);
    const TraceResult r_fp = EvaluateTrace(tf, cfg.sigma, gammas, gammas_ld, cat,
                                     ZeroKernel::Float, cfg.simd, cfg.eps, false);
    PrintResult(cfg.sigma, cfg.zero_kernel == ZeroKernel::LongDouble ? r_ld : r_fp);
    std::cout << "  Float vs LD residual delta: "
              << static_cast<double>(fabsl(r_ld.residual() - r_fp.residual())) << "\n";
}

}  // namespace Marshal::Induction

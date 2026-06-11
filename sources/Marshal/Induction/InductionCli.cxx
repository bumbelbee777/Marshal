#include "Induction.hxx"
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
        << "  --test gauss|sinc2|bump|rational\n"
        << "  --test-param X         sinc2 T, bump/rational scale\n"
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
        << "  --export-formal-cal F  formal calibration / measure-limit / candidates JSON\n"
        << "  --log-prime-validation  H_log weighted trace validation suite (T1-T6)\n"
        << "  --log-prime-catalog     full-scale validation + induction ladder\n"
        << "  --log-prime-cap N       max primes for global sinc2 (0=all)\n"
        << "  --export-log-prime F    log_prime_validation.json output\n"
        << "  --suggest-next         inference engine: emit build/cert/next_actions.json\n"
        << "  --lemma-manifest F     LemmaManifest.json path (default docs/Analysis/...)\n"
        << "  --ansatz-registry F    AnsatzRegistry.json path\n"
        << "  --export-next-actions F  next_actions.json output path\n";
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

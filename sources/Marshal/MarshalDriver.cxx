// MarshalDriver.cxx
#include <atomic>
#include <iostream>
#include <memory>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "Compat.hxx"
#include "Config.hxx"
#include "TraceApi.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Heat/PrimeCache.hxx"
#include "IO/ZeroLoader.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/FormalCalibration.hxx"
#include "Induction/Induction.hxx"
#include "Inference/InferenceEngine.hxx"
#include "Heat/LogPrimeValidation.hxx"

using Marshal::Config;
using Marshal::TraceResult;
using Marshal::ZeroKernel;
using Marshal::Analysis::ResidualBudget;
using Marshal::Heat::PrimeCatalog;

int main(int argc, char** argv) {
    Config cfg;
    std::string parse_err;
    if (!Marshal::Induction::ParseConfig(argc, argv, cfg, parse_err)) {
        if (!parse_err.empty()) std::cerr << parse_err << "\n";
        Marshal::Induction::PrintUsage(argv[0]);
        return 1;
    }

    if (!cfg.anavm_program.empty()) {
        const auto cr = Marshal::AnaVM::compile_program(cfg.anavm_program, cfg.anavm_check);
        if (!cr.ok) {
            Marshal::AnaVM::print_errors(cr.errors);
            return 1;
        }
        const auto cal = Marshal::AnaVM::build_formal_calibration(cr.program);
        cfg.anavm.loaded = true;
        cfg.anavm.id = cal.ansatz_id;
        cfg.anavm.rule_id = cal.rule_id;
        cfg.anavm.derived_omega = cal.derived_omega;
        cfg.anavm.derived_lambda = cal.derived_lambda;
        cfg.anavm.lean_module = cal.lean_module;
        cfg.anavm.placeholder = cal.placeholder;
        cfg.anavm.scaffold = cal.scaffold;
        cfg.anavm.falsify_sinc2 = cal.falsify_sinc2;
        cfg.anavm.trace_lhs_quotient = cal.trace_lhs_quotient;
        if (cr.program.pair_correlation_gue) cfg.pair_correlation = true;
        if (cr.program.formal_analytics) cfg.formal_analytics = true;
        std::cout << "AnaVM: compiled " << cr.program.id << " rule=" << cal.rule_id
                  << " omega=" << cal.derived_omega;
        if (cal.placeholder) std::cout << " [scaffold]";
        std::cout << "\n";
        if (!cfg.export_formal_cal_path.empty())
            Marshal::AnaVM::export_formal_calibration_json(cfg.export_formal_cal_path, cal);
        if (cfg.anavm_check) return 0;
    }

    if (cfg.suggest_next && !cfg.hp_proof) {
        Marshal::Inference::InferenceConfig ic;
        ic.manifest_path = cfg.lemma_manifest_path;
        ic.ansatz_registry_path = cfg.ansatz_registry_path;
        ic.cert_path = cfg.export_hp_cert_path;
        ic.export_next_actions_path = cfg.export_next_actions_path;
        std::string ierr;
        if (!Marshal::Inference::run_inference(ic, ierr)) {
            std::cerr << ierr << "\n";
            return 1;
        }
        return 0;
    }

    #ifdef _OPENMP
    if (cfg.threads > 0) omp_set_num_threads(cfg.threads);
    else omp_set_num_threads(omp_get_max_threads());
    #endif

    auto tf = Marshal::Induction::MakeTestFunction(cfg);
    const Real tau = Marshal::Induction::TauFromSigma(cfg.sigma);

    std::vector<double> gammas;
    std::vector<Real> gammas_ld;
    std::vector<int> primes;
    PrimeCatalog cat;
    std::atomic<bool> load_failed{false};

    #ifdef _OPENMP
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            std::vector<Real>* ld_ptr = (cfg.precision_mode && cfg.zero_kernel == ZeroKernel::LongDouble)
                ? &gammas_ld : nullptr;
            if (!LoadZerosFast(cfg.zeros_path, gammas, cfg.max_zeros, cfg.use_cache, ld_ptr))
                load_failed = true;
        }
        #pragma omp section
        {
            primes = Marshal::Heat::LoadOrSievePrimes(cfg.prime_limit);
            cat.set_primes(primes);
            cat.rebuild_adaptive(*tf, tau, cfg.kmax, cfg.eps);
        }
    }
    #else
    {
        std::vector<Real>* ld_ptr = (cfg.precision_mode && cfg.zero_kernel == ZeroKernel::LongDouble)
            ? &gammas_ld : nullptr;
        if (!LoadZerosFast(cfg.zeros_path, gammas, cfg.max_zeros, cfg.use_cache, ld_ptr))
            return 1;
    }
    primes = Marshal::Heat::LoadOrSievePrimes(cfg.prime_limit);
    cat.set_primes(primes);
    cat.rebuild_adaptive(*tf, tau, cfg.kmax, cfg.eps);
    #endif

    if (load_failed || gammas.empty()) return 1;

    if (cfg.zero_kernel == ZeroKernel::LongDouble && gammas_ld.empty())
        PromoteZerosLd(gammas, gammas_ld);

    std::cout << "Marshal trace — test: " << tf->name()
              << ", SIMD: " << SimdName(cfg.simd)
              << ", Real: " << MarshalRealName() << " (" << MarshalRealBits() << "-bit)\n";
    std::cout << "Zeros: " << gammas.size() << "  Primes: " << cat.p.size() << "\n\n";

    const TraceResult result = Marshal::Induction::RunEvaluate(cfg, *tf, gammas, gammas_ld, cat);
    const ResidualBudget budget = Marshal::Induction::ComputeResidualBudget(*tf, cfg.sigma, gammas, gammas_ld,
                                                          cat, cfg.zero_kernel, cfg.simd,
                                                          cfg.precision_mode, cfg.arch_pts);

    if (!cfg.export_trace_path.empty() || cfg.sign_check)
        Marshal::Induction::ExportTraceJson(cfg.export_trace_path.empty() ? "build/cert/sign_check.json"
                          : cfg.export_trace_path, cfg, *tf, result, cat, budget, gammas.size());

    if (cfg.spectral_compare_n > 0)
        Marshal::Induction::ExportSpectralJson("traces/spectral.json", cfg.spectral_compare_n, cat, gammas);

    if (!cfg.export_induction_path.empty())
        Marshal::Induction::ExportInductionJson(cfg.export_induction_path, cfg, *tf, gammas, gammas_ld, cat);

    if (cfg.compact_test) {
        Marshal::Induction::RunCompactTest(cfg, *tf, gammas, gammas_ld, cat);
        return 0;
    }
    if (cfg.sign_check) { Marshal::Induction::PrintResult(cfg.sigma, result); return 0; }
    if (cfg.operator_candidates) {
        Marshal::Induction::RunOperatorCandidates(cfg, *tf, gammas, gammas_ld, cat);
        return 0;
    }
    if (cfg.pair_correlation || cfg.formal_analytics) {
        const auto pc = cfg.pair_correlation
                            ? Marshal::Induction::RunPairCorrelation(cfg, gammas, primes)
                            : Marshal::Induction::PairCorrelationRunResult{};
        if (cfg.formal_analytics) {
            const Marshal::Analysis::PairCorrelationReport* pr =
                cfg.pair_correlation && pc.ok ? &pc.report : nullptr;
            auto fa = Marshal::Induction::RunFormalAnalytics(cfg, gammas, primes, pr);
            if (fa.ok && !cfg.export_formal_cal_path.empty())
                Marshal::AnaVM::export_formal_calibration_json(
                    cfg.export_formal_cal_path, fa.calibration);
        }
        return 0;
    }
    if (cfg.log_prime_validation) {
        const auto rep = Marshal::Heat::run_log_prime_validation(cfg, *tf, gammas, gammas_ld, cat, primes);
        if (!cfg.export_log_prime_validation_path.empty()) {
            if (!Marshal::Heat::export_log_prime_validation_json(cfg.export_log_prime_validation_path,
                                                                 rep)) {
                std::cerr << "Failed to write " << cfg.export_log_prime_validation_path << "\n";
                return 1;
            }
            std::cout << "Log-prime validation: " << cfg.export_log_prime_validation_path << "\n";
        }
        return (rep.t1_pass && rep.gauss_weil_identity_pass) ? 0 : 1;
    }
    if (cfg.measure_limit_sweep) {
        const auto ml = Marshal::Induction::RunMeasureLimitSweep(
            cfg, *tf, gammas, gammas_ld, cat, primes);
        if (!cfg.export_formal_cal_path.empty())
            Marshal::Induction::ExportMeasureLimitJson(cfg.export_formal_cal_path, ml);
        return 0;
    }
    if (cfg.hp_proof) {
        Marshal::Induction::RunHpProofInduction(cfg, *tf, gammas, gammas_ld, cat);
        if (cfg.suggest_next) {
            Marshal::Inference::InferenceConfig ic;
            ic.manifest_path = cfg.lemma_manifest_path;
            ic.ansatz_registry_path = cfg.ansatz_registry_path;
            ic.cert_path = cfg.export_hp_cert_path;
            ic.export_next_actions_path = cfg.export_next_actions_path;
            std::string ierr;
            if (!Marshal::Inference::run_inference(ic, ierr)) {
                std::cerr << ierr << "\n";
                return 1;
            }
        }
        return 0;
    }
    if (cfg.ansatz) { Marshal::Induction::RunHpAnsatz(cfg, *tf, gammas, gammas_ld, cat, primes); return 0; }
    if (cfg.residual_scaling) {
        Marshal::Induction::RunResidualScaling(cfg, *tf, gammas, gammas_ld, cat, primes); return 0;
    }
    if (cfg.induction) { Marshal::Induction::RunHeatInduction(cfg, *tf, gammas, gammas_ld, cat); return 0; }
    if (cfg.do_sweep) { Marshal::Induction::RunSweep(cfg, *tf, gammas, gammas_ld, cat, primes); return 0; }
    if (cfg.machine_test) { Marshal::Induction::RunMachineTest(cfg, *tf, gammas, gammas_ld, cat); return 0; }

    Marshal::Induction::PrintResult(cfg.sigma, result);
    if (cfg.checksum) {
        const uint64_t cs = static_cast<uint64_t>(result.lhs * 1e9)
                          ^ static_cast<uint64_t>(result.rhs * 1e9);
        std::cout << "checksum: " << cs << "\n";
    }
    return 0;
}
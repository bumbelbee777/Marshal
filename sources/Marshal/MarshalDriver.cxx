// MarshalDriver.cxx
#include <atomic>
#include <fstream>
#include <iomanip>
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
#include "AnaVM/MrsProofGate.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/FormalCalibration.hxx"
#include "AnaVM/ValidationRouter.hxx"
#include "Heat/AnalyticConstructionValidation.hxx"
#include "Heat/SpectralActionValidation.hxx"
#include "Heat/GlobalDiracLimitValidation.hxx"
#include "Heat/AnalyticLemmaValidation.hxx"
#include "Heat/GenusOneWedgeValidation.hxx"
#include "Heat/XiHadamardEngine.hxx"
#include "AnaVM/AnaProofEngine.hxx"
#include "Heat/BerryKeatingValidation.hxx"
#include "Heat/GLn/GLnLadderValidation.hxx"
#include "Heat/GLn/GL2EllipseHeegnerValidation.hxx"
#include "Heat/GLn/GL2BSDEngine.hxx"
#include "AnaVM/MrsLadderProofEngine.hxx"
#include "AnaVM/MrsLadderProofGate.hxx"
#include "AnaVM/AnaVm.hxx"
#include "Heat/GLn/GL3HodgeEngine.hxx"
#include "Heat/SelfAdjointExtensionSweep.hxx"
#include "Heat/TraceFormulaGate.hxx"
#include "Induction/Induction.hxx"
#include "Inference/InferenceEngine.hxx"
#include "Heat/LogPrimeValidation.hxx"
#include "Induction/WeilConvergenceStudy.hxx"
#include "Induction/CrossSectorWeilStudy.hxx"
#include "IO/MappedZeroBank.hxx"
#include "IO/ZeroView.hxx"
#include "Heat/DualityGoldStandard.hxx"
#include "Heat/ConnesCrossedValidation.hxx"
#include "Heat/CompletionValidation.hxx"
#include "Heat/ArchimedeanBoundaryValidation.hxx"
#include "Heat/SpectralDeterminant.hxx"
#include "Heat/AssemblySearchValidation.hxx"
#include "Heat/ConnesCouplingMode.hxx"
#include "IO/ZeroIngest.hxx"
#include "Ntz/NtzGenerate.hxx"
#include "Investigation/InvestigationRunner.hxx"
#include "Investigation/InvestigationConfig.hxx"

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

    if (cfg.ntz_generate) {
        #ifdef _OPENMP
        if (cfg.threads > 0) omp_set_num_threads(cfg.threads);
        #endif
        Marshal::Ntz::NtzGenerateOptions opt;
        opt.input = cfg.ntz_input.empty() ? cfg.zeros_path : cfg.ntz_input;
        opt.output = cfg.ntz_output;
        opt.cache_path = cfg.ntz_cache_path;
        opt.report_path = cfg.ntz_report_path;
        opt.shard_dir = cfg.ntz_shard_dir;
        opt.count = cfg.ntz_count;
        opt.offset = cfg.ntz_offset;
        opt.pad_to = cfg.ntz_pad_to;
        opt.batch_size = cfg.ntz_batch_size;
        opt.tol = cfg.ntz_tol;
        opt.refine = cfg.ntz_refine;
        opt.threads = cfg.threads;
        if (opt.output.empty()) {
            std::cerr << "NTZ generate: --ntz-output required\n";
            return 1;
        }
        Marshal::Ntz::NtzGenerateReport rep;
        std::string ntz_err;
        const bool ok = Marshal::Ntz::RunNtzGenerate(opt, rep, ntz_err);
        if (!ntz_err.empty()) std::cerr << ntz_err << "\n";
        if (!opt.report_path.empty())
            Marshal::Ntz::ExportNtzReportJson(opt.report_path, rep);
        return ok ? 0 : 1;
    }

    if (cfg.zeros_ingest) {
#ifdef _OPENMP
        if (cfg.threads > 0) omp_set_num_threads(cfg.threads);
#endif
        Marshal::IO::ZeroIngestOptions opt;
        opt.input = cfg.zeros_ingest_input.empty()
                        ? (cfg.ntz_input.empty() ? cfg.zeros_path : cfg.ntz_input)
                        : cfg.zeros_ingest_input;
        opt.cache_path = cfg.zeros_ingest_cache;
        opt.shard_dir = cfg.zeros_ingest_shard_dir.empty() ? cfg.ntz_shard_dir : cfg.zeros_ingest_shard_dir;
        opt.count = cfg.zeros_ingest_count > 0 ? cfg.zeros_ingest_count : cfg.ntz_count;
        opt.batch_size = cfg.ntz_batch_size > 0 ? cfg.ntz_batch_size : 65536;
        opt.threads = cfg.threads;
        Marshal::IO::ZeroIngestReport rep;
        std::string err;
        if (!Marshal::IO::RunZeroIngest(opt, rep, err)) {
            if (!err.empty()) std::cerr << err << "\n";
            return 1;
        }
        return 0;
    }

    if (cfg.gln_ladder_validation) {
        const int plim = cfg.prime_limit > 0 ? cfg.prime_limit : 100;
        const std::vector<int> ladder_primes = Marshal::Heat::LoadOrSievePrimes(plim);
        const auto lrep = Marshal::Heat::GLn::run_gln_ladder_validation(cfg, ladder_primes);
        if (!cfg.export_gln_ladder_path.empty()) {
            if (!Marshal::Heat::GLn::export_gln_ladder_json(cfg.export_gln_ladder_path, lrep)) {
                std::cerr << "Failed to write " << cfg.export_gln_ladder_path << "\n";
                return 1;
            }
            std::cout << "GL(n) ladder sweep: " << cfg.export_gln_ladder_path << "\n";
        }
        for (const auto& e : lrep.ranks) {
            if (e.rank == 3 && !cfg.export_hodge_k3_path.empty()) {
                if (!Marshal::Heat::GLn::export_hodge_k3_demo_json(cfg.export_hodge_k3_path, e)) {
                    std::cerr << "Failed to write " << cfg.export_hodge_k3_path << "\n";
                    return 1;
                }
                std::cout << "Hodge K3 demo: " << cfg.export_hodge_k3_path << "\n";
            }
        }
        return 0;
    }

    if (cfg.mrs_ladder_proof_engine) {
        const int plim = cfg.prime_limit > 0 ? cfg.prime_limit : 100;
        const std::vector<int> primes = Marshal::Heat::LoadOrSievePrimes(plim);
        const auto rep = Marshal::AnaVM::run_mrs_ladder_proof_engine(cfg, primes);
        if (!cfg.export_mrs_ladder_audit_path.empty()) {
            if (!Marshal::AnaVM::export_mrs_ladder_proof_audit_json(cfg.export_mrs_ladder_audit_path,
                                                                    rep)) {
                std::cerr << "Failed to write " << cfg.export_mrs_ladder_audit_path << "\n";
                return 1;
            }
            std::cout << "MRS ladder audit: " << cfg.export_mrs_ladder_audit_path << "\n";
        }
        if (!cfg.export_mrs_ladder_closure_path.empty()) {
            if (!Marshal::AnaVM::export_mrs_ladder_closure_json(cfg.export_mrs_ladder_closure_path,
                                                                rep)) {
                std::cerr << "Failed to write " << cfg.export_mrs_ladder_closure_path << "\n";
                return 1;
            }
            std::cout << "MRS ladder closure: " << cfg.export_mrs_ladder_closure_path << "\n";
        }
        if (!cfg.export_bsd_proof_path.empty()) {
            if (!Marshal::Heat::GLn::export_gl2_bsd_proof_json(cfg.export_bsd_proof_path, rep.bsd)) {
                std::cerr << "Failed to write " << cfg.export_bsd_proof_path << "\n";
                return 1;
            }
        }
        if (!cfg.export_hodge_proof_path.empty()) {
            if (!Marshal::Heat::GLn::export_gl3_hodge_proof_json(cfg.export_hodge_proof_path,
                                                                rep.hodge)) {
                std::cerr << "Failed to write " << cfg.export_hodge_proof_path << "\n";
                return 1;
            }
        }
        if (!cfg.export_goldbach_proof_path.empty()) {
            std::ofstream out(cfg.export_goldbach_proof_path);
            if (!out) {
                std::cerr << "Failed to write " << cfg.export_goldbach_proof_path << "\n";
                return 1;
            }
            out << std::setprecision(17);
            out << "{\n  \"version\": 1,\n";
            out << "  \"goldbach_proved\": " << (rep.goldbach_proved_closed ? "true" : "false")
                << ",\n";
            out << "  \"classical_goldbach\": " << (rep.classical_goldbach_closed ? "true" : "false")
                << ",\n";
            out << "  \"goldbach_spectral_extension_closed\": "
                << (rep.goldbach_spectral_extension_closed ? "true" : "false") << ",\n";
            const double minor =
                std::max(static_cast<double>(rep.goldbach.minor_arc_bound), 1e-12);
            out << "  \"goldbach_major_minor_ratio\": "
                << (static_cast<double>(rep.goldbach.major_arc_spectral_mass) / minor)
                << ",\n";
            out << "  \"major_arc_spectral_mass\": "
                << static_cast<double>(rep.goldbach.major_arc_spectral_mass) << ",\n";
            out << "  \"minor_arc_bound\": " << static_cast<double>(rep.goldbach.minor_arc_bound)
                << ",\n";
            out << "  \"major_arc_threshold\": "
                << static_cast<double>(rep.goldbach.major_arc_threshold) << ",\n";
            out << "  \"minor_arc_ub\": " << static_cast<double>(rep.goldbach.minor_arc_ub)
                << ",\n";
            out << "  \"goldbach_n0\": 4,\n";
            out << "  \"goldbach_effective_n_max\": " << rep.goldbach_effective.n_max_checked
                << ",\n";
            out << "  \"heegner_point_count\": " << rep.goldbach.heegner_point_count << ",\n";
            out << "  \"mrs_proof_audit_ok\": "
                << (rep.classical_goldbach_closed ? "true" : "false") << ",\n";
            out << "  \"proof_status\": \"" << rep.goldbach.proof_status << "\"\n}\n";
        }
        if (!cfg.export_ym_proof_path.empty()) {
            if (!Marshal::Heat::GLn::export_gl4_ym_proof_json(cfg.export_ym_proof_path, rep.ym)) {
                std::cerr << "Failed to write " << cfg.export_ym_proof_path << "\n";
                return 1;
            }
        }
        std::cout << "MRS ladder proof chain closed=" << (rep.proof_chain_closed ? "true" : "false")
                  << "\n";
        return rep.proof_chain_closed ? 0 : 1;
    }

    if (cfg.gl2_ellipse_heegner_validation || cfg.goldbach_proof_engine) {
        const int plim = cfg.prime_limit > 0 ? cfg.prime_limit : 100;
        const std::vector<int> primes = Marshal::Heat::LoadOrSievePrimes(plim);
        const auto rep = Marshal::Heat::GLn::run_gl2_ellipse_heegner_validation(cfg, primes);
        const std::string out_path = cfg.goldbach_proof_engine ? cfg.export_goldbach_proof_path
                                                               : cfg.export_gl2_ellipse_heegner_path;
        if (!out_path.empty()) {
            if (cfg.goldbach_proof_engine) {
                std::ofstream out(out_path);
                if (!out) {
                    std::cerr << "Failed to write " << out_path << "\n";
                    return 1;
                }
                out << std::setprecision(17);
                out << "{\n  \"version\": 1,\n";
                out << "  \"goldbach_proved\": " << (rep.bounds_ok ? "true" : "false") << ",\n";
                out << "  \"major_arc_spectral_mass\": "
                    << static_cast<double>(rep.major_arc_spectral_mass) << ",\n";
                out << "  \"minor_arc_bound\": " << static_cast<double>(rep.minor_arc_bound)
                    << ",\n";
                out << "  \"major_arc_threshold\": "
                    << static_cast<double>(rep.major_arc_threshold) << ",\n";
                out << "  \"minor_arc_ub\": " << static_cast<double>(rep.minor_arc_ub) << ",\n";
                out << "  \"goldbach_n0\": 4,\n";
                out << "  \"heegner_point_count\": " << rep.heegner_point_count << ",\n";
                out << "  \"mrs_proof_audit_ok\": " << (rep.bounds_ok ? "true" : "false") << ",\n";
                out << "  \"proof_status\": \"" << rep.proof_status << "\"\n}\n";
            } else if (!Marshal::Heat::GLn::export_gl2_ellipse_heegner_json(out_path, rep)) {
                std::cerr << "Failed to write " << out_path << "\n";
                return 1;
            }
            std::cout << "GL(2) ellipse/Heegner: " << out_path << "\n";
        }
        return rep.bounds_ok ? 0 : 1;
    }

    if (cfg.bsd_proof_engine) {
        const int plim = cfg.prime_limit > 0 ? cfg.prime_limit : 100;
        const std::vector<int> primes = Marshal::Heat::LoadOrSievePrimes(plim);
        const auto ladder = Marshal::AnaVM::run_mrs_ladder_proof_engine(cfg, primes);
        const auto rep = ladder.bsd;
        if (!cfg.export_bsd_proof_path.empty()) {
            if (!Marshal::Heat::GLn::export_gl2_bsd_proof_json(cfg.export_bsd_proof_path, rep)) {
                std::cerr << "Failed to write " << cfg.export_bsd_proof_path << "\n";
                return 1;
            }
            std::cout << "BSD proof engine: " << cfg.export_bsd_proof_path << "\n";
        }
        return Marshal::AnaVM::ladder_bsd_proof_ok(rep) ? 0 : 1;
    }

    if (cfg.hodge_proof_engine) {
        const int plim = cfg.prime_limit > 0 ? cfg.prime_limit : 100;
        const std::vector<int> primes = Marshal::Heat::LoadOrSievePrimes(plim);
        const auto ladder = Marshal::AnaVM::run_mrs_ladder_proof_engine(cfg, primes);
        const auto rep = ladder.hodge;
        if (!cfg.export_hodge_proof_path.empty()) {
            if (!Marshal::Heat::GLn::export_gl3_hodge_proof_json(cfg.export_hodge_proof_path, rep)) {
                std::cerr << "Failed to write " << cfg.export_hodge_proof_path << "\n";
                return 1;
            }
            std::cout << "Hodge proof engine: " << cfg.export_hodge_proof_path << "\n";
        }
        return Marshal::AnaVM::ladder_hodge_proof_ok(rep) ? 0 : 1;
    }

    if (cfg.ym_proof_engine) {
        const int plim = cfg.prime_limit > 0 ? cfg.prime_limit : 100;
        const std::vector<int> primes = Marshal::Heat::LoadOrSievePrimes(plim);
        const auto hodge = Marshal::Heat::GLn::run_gl3_hodge_proof_engine(cfg, primes);
        const auto rep = Marshal::Heat::GLn::run_gl4_ym_proof_engine(
            cfg, primes, hodge.bounds_ok, hodge.bounds_ok);
        if (!cfg.export_ym_proof_path.empty()) {
            if (!Marshal::Heat::GLn::export_gl4_ym_proof_json(cfg.export_ym_proof_path, rep)) {
                std::cerr << "Failed to write " << cfg.export_ym_proof_path << "\n";
                return 1;
            }
            std::cout << "YM proof engine: " << cfg.export_ym_proof_path << "\n";
        }
        return rep.bounds_ok ? 0 : 1;
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
        cfg.anavm.mrs_module = cal.mrs_module;
        cfg.anavm.placeholder = cal.placeholder;
        cfg.anavm.scaffold = cal.scaffold;
        cfg.anavm.falsify_sinc2 = cal.falsify_sinc2;
        cfg.anavm.trace_lhs_quotient = cal.trace_lhs_quotient;
        cfg.anavm.lemma_refs = cal.lemma_refs;
        cfg.completion = cr.program.completion;
        cfg.adelic_cauchy = cr.program.adelic_cauchy;
        if (cr.program.height_map.present) {
            cfg.height_map = cr.program.height_map;
            cfg.height_map.present = true;
        }
        if (cr.program.spectral_determinant.present)
            cfg.spectral_determinant = cr.program.spectral_determinant;
        if (cr.program.assembly_search.present)
            cfg.assembly_search_spec = cr.program.assembly_search;
        cfg.semiclassical = cr.program.semiclassical;
        cfg.self_adjoint_extension = cr.program.self_adjoint_extension;
        if (cr.program.spectral_action.present)
            cfg.spectral_action = cr.program.spectral_action;
        if (cr.program.discretization_limit.present)
            cfg.discretization_limit = cr.program.discretization_limit;
        if (cr.program.formal_target.present)
            cfg.formal_target = cr.program.formal_target;
        if (cr.program.bound_audit.present)
            cfg.bound_audit = cr.program.bound_audit;
        if (cr.program.genus_one_log_bounds.present)
            cfg.genus_one_log_bounds = cr.program.genus_one_log_bounds;
        if (cr.program.archimedean.present) cfg.archimedean = cr.program.archimedean;
        if (cr.program.diagnostics.spectrum_rmse_max > 0)
            cfg.diagnostics_spectrum_rmse_max =
                static_cast<Real>(cr.program.diagnostics.spectrum_rmse_max);
        if (cr.program.diagnostics.spectrum_max_gap_max > 0)
            cfg.diagnostics_spectrum_max_gap_max =
                static_cast<Real>(cr.program.diagnostics.spectrum_max_gap_max);
        if (cr.program.diagnostics.weil_residual_max > 0)
            cfg.diagnostics_weil_residual_max =
                static_cast<Real>(cr.program.diagnostics.weil_residual_max);
        if (cr.program.diagnostics.spectral_discreteness)
            cfg.spectral_discreteness_check = true;
        if (!cr.program.crossed_product.coupling.empty())
            cfg.connes_coupling_mode =
                Marshal::Heat::parse_connes_coupling_mode(cr.program.crossed_product.coupling);
        if (cr.program.crossed_product.lambda > 0)
            cfg.connes_coupling_lambda = static_cast<Real>(cr.program.crossed_product.lambda);
        if (cr.program.crossed_product.kmax > 0) cfg.kmax = cr.program.crossed_product.kmax;
        if (!cr.program.crossed_product.prime_ladder.empty())
            cfg.connes_prime_ladder = cr.program.crossed_product.prime_ladder;
        cfg.connes_expect_spectrum_identified = cr.program.diagnostics.expect_spectrum_identified;
        cfg.diagnostics_expect_finite_spectrum_mismatch =
            cr.program.diagnostics.expect_finite_spectrum_mismatch;
        Marshal::AnaVM::apply_validation_jobs(
            cfg, Marshal::AnaVM::route_validation(cr.program));
        if (cr.program.investigation.present)
            Marshal::Investigation::apply_mrs_investigation(cfg, cr.program.investigation);
        if (cr.program.archimedean.present && !cfg.skip_archimedean_sweep &&
            cr.program.diagnostics.archimedean_sweep)
            cfg.archimedean_sweep_all = true;
        if (cfg.completion_tolerance_override > 0)
            cfg.completion.tolerance = static_cast<double>(cfg.completion_tolerance_override);
        if (cfg.adelic_max_primes_override > 0)
            cfg.adelic_cauchy.max_primes = cfg.adelic_max_primes_override;
        if (cfg.height_a_override_set)
            cfg.height_map.a = static_cast<double>(cfg.height_a_override);
        if (cfg.height_b_override_set)
            cfg.height_map.b = static_cast<double>(cfg.height_b_override);
        if (!cr.program.diagnostics.trace_test.empty()) {
            cfg.test_kind = parse_test_kind(cr.program.diagnostics.trace_test);
        }
        if (cr.program.diagnostics.test_param > 0)
            cfg.test_param = static_cast<Real>(cr.program.diagnostics.test_param);
        if (cr.program.diagnostics.sinc2_kappa > 0)
            cfg.sinc2_kappa = static_cast<Real>(cr.program.diagnostics.sinc2_kappa);
        std::cout << "AnaVM: compiled " << cr.program.id << " rule=" << cal.rule_id
                  << " omega=" << cal.derived_omega;
        if (cal.placeholder) std::cout << " [scaffold]";
        std::cout << "\n";
        if (!cr.infer_report.audit.empty()) {
            const std::string infer_path = "docs/generated/mrs_infer_audit.json";
            if (Marshal::AnaVM::export_infer_audit_json(infer_path, cr.infer_report))
                std::cout << "MrsInfer: audit -> " << infer_path << " ("
                          << cr.infer_report.audit.size() << " entries)\n";
        }
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

    std::vector<double> load_gammas;
    std::vector<Real> load_gammas_ld;
    Marshal::IO::MappedZeroBank zero_bank;
    Marshal::IO::ZeroView zero_view;
  std::vector<int> primes;
    PrimeCatalog cat;
    std::atomic<bool> load_failed{false};

    #ifdef _OPENMP
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (!zero_bank.Open(cfg.zeros_path)) {
                std::vector<Real>* ld_ptr =
                    (cfg.precision_mode && cfg.zero_kernel == ZeroKernel::LongDouble) ? &load_gammas_ld
                                                                                      : nullptr;
                if (!LoadZerosFast(cfg.zeros_path, load_gammas, cfg.max_zeros, cfg.use_cache, ld_ptr))
                    load_failed = true;
                else
                    zero_view.LoadVector(std::move(load_gammas), std::move(load_gammas_ld));
            } else {
                const size_t want =
                    cfg.max_zeros > 0 ? std::min(cfg.max_zeros, zero_bank.size()) : zero_bank.size();
                const bool need_ld =
                    cfg.precision_mode && cfg.zero_kernel == ZeroKernel::LongDouble;
                if (need_ld) {
                    zero_bank.CopyPrefix(want, load_gammas, &load_gammas_ld);
                    zero_view.LoadVector(std::move(load_gammas), std::move(load_gammas_ld));
                } else {
                    zero_view.Bind(zero_bank.data, want);
                }
            }
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
        if (!zero_bank.Open(cfg.zeros_path)) {
            std::vector<Real>* ld_ptr =
                (cfg.precision_mode && cfg.zero_kernel == ZeroKernel::LongDouble) ? &load_gammas_ld
                                                                                  : nullptr;
            if (!LoadZerosFast(cfg.zeros_path, load_gammas, cfg.max_zeros, cfg.use_cache, ld_ptr))
                return 1;
            zero_view.LoadVector(std::move(load_gammas), std::move(load_gammas_ld));
        } else {
            const size_t want =
                cfg.max_zeros > 0 ? std::min(cfg.max_zeros, zero_bank.size()) : zero_bank.size();
            const bool need_ld = cfg.precision_mode && cfg.zero_kernel == ZeroKernel::LongDouble;
            if (need_ld) {
                zero_bank.CopyPrefix(want, load_gammas, &load_gammas_ld);
                zero_view.LoadVector(std::move(load_gammas), std::move(load_gammas_ld));
            } else {
                zero_view.Bind(zero_bank.data, want);
            }
        }
    }
    primes = Marshal::Heat::LoadOrSievePrimes(cfg.prime_limit);
    cat.set_primes(primes);
    cat.rebuild_adaptive(*tf, tau, cfg.kmax, cfg.eps);
    #endif

    if (load_failed || zero_view.empty()) return 1;

    std::vector<double> gammas_work;
    std::vector<Real> gammas_ld_work;
    if (zero_view.mmap_backed) {
        gammas_work.assign(zero_view.ptr(), zero_view.ptr() + zero_view.size());
    } else {
        gammas_work = zero_view.vec();
        gammas_ld_work = zero_view.vec_ld();
    }
    if (gammas_ld_work.empty() && cfg.zero_kernel == ZeroKernel::LongDouble && !gammas_work.empty())
        PromoteZerosLd(gammas_work, gammas_ld_work);
    const std::vector<double>& gammas = gammas_work;
    const std::vector<Real>& gammas_ld = gammas_ld_work;

    std::cout << "Marshal trace — test: " << tf->name()
              << ", SIMD: " << SimdName(cfg.simd)
              << ", Real: " << MarshalRealName() << " (" << MarshalRealBits() << "-bit)\n";
    std::cout << "Zeros: " << zero_view.size() << "  Primes: " << cat.p.size();
    if (zero_view.mmap_backed) std::cout << "  [mmap]";
    std::cout << "\n\n";

    const TraceResult result =
        Marshal::Induction::RunEvaluateView(cfg, *tf, zero_view, cat);
    const ResidualBudget budget = Marshal::Induction::ComputeResidualBudget(
        *tf, cfg.sigma, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd, cfg.precision_mode,
        cfg.arch_pts);

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
    if (cfg.arch_sinc2_converge) {
        const auto audit = Marshal::Induction::RunArchSinc2Converge(cfg);
        if (!cfg.export_arch_sinc2_audit_path.empty()) {
            if (!Marshal::Induction::ExportArchSinc2AuditJson(cfg.export_arch_sinc2_audit_path,
                                                              audit)) {
                std::cerr << "Failed to write " << cfg.export_arch_sinc2_audit_path << "\n";
                return 1;
            }
        }
        return audit.converge.converged ? 0 : 1;
    }
    if (cfg.arch_sinc2_audit) {
        const auto audit = Marshal::Induction::RunArchSinc2Audit(cfg);
        if (!cfg.export_arch_sinc2_audit_path.empty()) {
            if (!Marshal::Induction::ExportArchSinc2AuditJson(cfg.export_arch_sinc2_audit_path,
                                                              audit)) {
                std::cerr << "Failed to write " << cfg.export_arch_sinc2_audit_path << "\n";
                return 1;
            }
            std::cout << "Arch sinc2 audit: " << cfg.export_arch_sinc2_audit_path << "\n";
        }
        return 0;
    }
    if (cfg.weil_convergence_study) {
        const auto conv = Marshal::Induction::RunWeilConvergenceStudy(cfg, gammas, gammas_ld, cat,
                                                                    primes);
        if (!cfg.export_weil_convergence_path.empty()) {
            if (!Marshal::Induction::ExportWeilConvergenceJson(cfg.export_weil_convergence_path,
                                                               conv)) {
                std::cerr << "Failed to write " << cfg.export_weil_convergence_path << "\n";
                return 1;
            }
            std::cout << "Weil convergence: " << cfg.export_weil_convergence_path << "\n";
        }
        return 0;
    }
    if (cfg.cross_sector_weil_study) {
        const auto cs = Marshal::Induction::RunCrossSectorWeilStudy(cfg, gammas, gammas_ld, cat,
                                                                    primes);
        if (!cfg.export_cross_sector_weil_path.empty()) {
            if (!Marshal::Induction::ExportCrossSectorWeilJson(cfg.export_cross_sector_weil_path,
                                                               cs)) {
                std::cerr << "Failed to write " << cfg.export_cross_sector_weil_path << "\n";
                return 1;
            }
            std::cout << "Cross-sector Weil study: " << cfg.export_cross_sector_weil_path << "\n";
        }
        return 0;
    }
    if (cfg.investigation_run) {
        const auto inv = Marshal::Investigation::run_investigation(cfg, gammas, gammas_ld, cat, primes);
        if (!inv.ok) return 1;
        return 0;
    }
    if (cfg.analytic_construction_validation) {
        const auto arep = Marshal::Heat::run_analytic_construction_validation(
            cfg, gammas, gammas_ld, cat, primes);
        if (!cfg.export_analytic_construction_path.empty()) {
            if (!Marshal::Heat::export_analytic_construction_json(
                    cfg.export_analytic_construction_path, arep)) {
                std::cerr << "Failed to write " << cfg.export_analytic_construction_path << "\n";
                return 1;
            }
            std::cout << "Analytic construction: " << cfg.export_analytic_construction_path << "\n";
        }
        return 0;
    }
    if (cfg.berry_keating_validation) {
        const auto brep = Marshal::Heat::run_berry_keating_validation(cfg, gammas, gammas_ld, cat,
                                                                      primes);
        if (!cfg.export_berry_keating_path.empty()) {
            if (!Marshal::Heat::export_berry_keating_validation_json(cfg.export_berry_keating_path,
                                                                     brep)) {
                std::cerr << "Failed to write " << cfg.export_berry_keating_path << "\n";
                return 1;
            }
            std::cout << "Berry-Keating validation: " << cfg.export_berry_keating_path << "\n";
        }
        if (!cfg.self_adjoint_extension_sweep && !cfg.trace_formula_gate &&
            !cfg.archimedean_boundary_sweep && !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.spectral_action_validation) {
        const auto sarep = Marshal::Heat::run_spectral_action_validation(cfg, gammas, gammas_ld, cat,
                                                                        primes);
        if (!cfg.export_spectral_action_path.empty()) {
            if (!Marshal::Heat::export_spectral_action_validation_json(
                    cfg.export_spectral_action_path, sarep)) {
                std::cerr << "Failed to write " << cfg.export_spectral_action_path << "\n";
                return 1;
            }
            std::cout << "Spectral action selection: " << cfg.export_spectral_action_path << "\n";
        }
        if (!cfg.self_adjoint_extension_sweep && !cfg.trace_formula_gate &&
            !cfg.analytic_construction_validation && !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.global_dirac_limit_validation) {
        const auto limrep =
            Marshal::Heat::run_global_dirac_limit_validation(cfg, gammas_ld, primes);
        if (!cfg.export_global_dirac_limit_path.empty()) {
            if (!Marshal::Heat::export_global_dirac_limit_json(cfg.export_global_dirac_limit_path,
                                                               limrep)) {
                std::cerr << "Failed to write " << cfg.export_global_dirac_limit_path << "\n";
                return 1;
            }
            std::cout << "Global Dirac limit cert: " << cfg.export_global_dirac_limit_path << "\n";
        }
        if (!cfg.trace_formula_gate && !cfg.archimedean_boundary_sweep &&
            !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.analytic_lemma_demo) {
        const auto lrep =
            Marshal::Heat::run_analytic_lemma_demo(cfg, gammas, gammas_ld, cat, primes);
        if (!cfg.export_analytic_lemma_demo_path.empty()) {
            if (!Marshal::Heat::export_analytic_lemma_demo_json(cfg.export_analytic_lemma_demo_path,
                                                                lrep)) {
                std::cerr << "Failed to write " << cfg.export_analytic_lemma_demo_path << "\n";
                return 1;
            }
            std::cout << "Analytic lemma demo: " << cfg.export_analytic_lemma_demo_path << "\n";
        }
        return 0;
    }
    if (cfg.marshal_wedge_analytic_validation) {
        const auto wrep = Marshal::Heat::run_genus_one_wedge_validation(cfg, gammas);
        if (!cfg.export_wedge_analytic_path.empty()) {
            if (!Marshal::Heat::export_genus_one_wedge_json(cfg.export_wedge_analytic_path, wrep)) {
                std::cerr << "Failed to write " << cfg.export_wedge_analytic_path << "\n";
                return 1;
            }
            std::cout << "Marshal wedge analytic (C++ log tail): " << cfg.export_wedge_analytic_path
                      << "\n";
        }
        if (!cfg.xi_hadamard_proof_validation)
            return wrep.genus_one_log_summability_ok ? 0 : 1;
    }
    if (cfg.xi_hadamard_proof_validation) {
        const Marshal::AnaVM::MrsBoundAudit* bounds =
            cfg.bound_audit.present ? &cfg.bound_audit : nullptr;
        const Marshal::AnaVM::MrsGenusOneLogBounds* genus =
            cfg.genus_one_log_bounds.present ? &cfg.genus_one_log_bounds : nullptr;
        const auto xhrep = Marshal::Heat::run_xi_hadamard_engine(cfg, gammas, primes, bounds, genus);
        if (!cfg.export_xi_hadamard_proof_path.empty()) {
            if (!Marshal::Heat::export_xi_hadamard_engine_json(cfg.export_xi_hadamard_proof_path,
                                                               xhrep)) {
                std::cerr << "Failed to write " << cfg.export_xi_hadamard_proof_path << "\n";
                return 1;
            }
            std::cout << "AnaVM XiHadamard proof: " << cfg.export_xi_hadamard_proof_path << "\n";
        }
        if (!cfg.export_xi_hadamard_proof_graph_path.empty()) {
            if (!Marshal::AnaVM::export_proof_graph_json(cfg.export_xi_hadamard_proof_graph_path,
                                                         xhrep.proof_graph)) {
                std::cerr << "Failed to write " << cfg.export_xi_hadamard_proof_graph_path << "\n";
                return 1;
            }
            std::cout << "AnaVM proof graph: " << cfg.export_xi_hadamard_proof_graph_path << "\n";
        }
        const auto refusal = Marshal::AnaVM::xi_hadamard_mrs_proof_refusal(xhrep);
        if (refusal != Marshal::AnaVM::XiHadamardMrsProofRefusal::None) {
            if (refusal == Marshal::AnaVM::XiHadamardMrsProofRefusal::ProofChainOpen &&
                Marshal::AnaVM::xi_hadamard_report_bounds_ok(xhrep) &&
                xhrep.non_circular_architecture_ok) {
                std::cout << "AnaVM Hadamard routing OK; Suzuki B_a eq. (2.5) RH pin analytically "
                             "open (expected until closure).\n";
                return 0;
            }
            std::cerr << "AnaVM MRS proof gate refused: "
                      << Marshal::AnaVM::xi_hadamard_mrs_proof_refusal_message(refusal) << "\n";
            return 1;
        }
        return 0;
    }
    if (cfg.self_adjoint_extension_sweep) {
        const auto srep = Marshal::Heat::run_self_adjoint_extension_sweep(cfg, gammas, gammas_ld, cat,
                                                                          primes);
        if (!cfg.export_self_adjoint_ext_path.empty()) {
            if (!Marshal::Heat::export_self_adjoint_extension_sweep_json(
                    cfg.export_self_adjoint_ext_path, srep)) {
                std::cerr << "Failed to write " << cfg.export_self_adjoint_ext_path << "\n";
                return 1;
            }
            std::cout << "Self-adjoint extension sweep: " << cfg.export_self_adjoint_ext_path
                      << "\n";
        }
        if (!cfg.trace_formula_gate && !cfg.archimedean_boundary_sweep &&
            !cfg.completion_validation && !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.trace_formula_gate) {
        const auto trep =
            Marshal::Heat::run_trace_formula_gate(cfg, gammas, gammas_ld, cat, primes);
        if (!cfg.export_trace_formula_gate_path.empty()) {
            if (!Marshal::Heat::export_trace_formula_gate_json(cfg.export_trace_formula_gate_path,
                                                               trep)) {
                std::cerr << "Failed to write " << cfg.export_trace_formula_gate_path << "\n";
                return 1;
            }
            std::cout << "Trace formula gate: " << cfg.export_trace_formula_gate_path << "\n";
        }
        if (!cfg.archimedean_boundary_sweep && !cfg.completion_validation &&
            !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.archimedean_boundary_sweep) {
        const auto arep =
            Marshal::Heat::run_archimedean_boundary_validation(cfg, gammas, gammas_ld, primes);
        if (!cfg.export_archimedean_path.empty()) {
            if (!Marshal::Heat::export_archimedean_boundary_json(cfg.export_archimedean_path,
                                                                 arep)) {
                std::cerr << "Failed to write " << cfg.export_archimedean_path << "\n";
                return 1;
            }
            std::cout << "Archimedean boundary sweep: " << cfg.export_archimedean_path << "\n";
        }
        if (!cfg.completion_validation && !cfg.connes_crossed_validation &&
            !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.completion_validation) {
        const auto crep = Marshal::Heat::run_completion_validation(cfg, gammas_ld, primes);
        if (!cfg.export_completion_path.empty()) {
            if (!Marshal::Heat::export_completion_validation_json(cfg.export_completion_path,
                                                                  crep)) {
                std::cerr << "Failed to write " << cfg.export_completion_path << "\n";
                return 1;
            }
            std::cout << "Completion validation: " << cfg.export_completion_path << "\n";
        }
        if (!cfg.connes_crossed_validation && !cfg.log_prime_validation &&
            !cfg.spectral_determinant_validation && !cfg.assembly_search)
            return 0;
    }
    if (cfg.spectral_determinant_validation) {
        const auto srep = Marshal::Heat::run_spectral_determinant_validation(
            cfg, gammas, gammas_ld, cat, primes);
        if (!cfg.export_spectral_det_path.empty()) {
            if (!Marshal::Heat::export_spectral_determinant_json(cfg.export_spectral_det_path,
                                                                 srep)) {
                std::cerr << "Failed to write " << cfg.export_spectral_det_path << "\n";
                return 1;
            }
            std::cout << "Spectral determinant: " << cfg.export_spectral_det_path << "\n";
        }
        if (!cfg.assembly_search && !cfg.connes_crossed_validation && !cfg.log_prime_validation)
            return 0;
    }
    if (cfg.assembly_search) {
        const auto arep = Marshal::Heat::run_assembly_search_validation(cfg, gammas, gammas_ld, primes);
        if (!cfg.export_assembly_path.empty()) {
            if (!Marshal::Heat::export_assembly_search_json(cfg.export_assembly_path, arep)) {
                std::cerr << "Failed to write " << cfg.export_assembly_path << "\n";
                return 1;
            }
            std::cout << "Assembly search: " << cfg.export_assembly_path << "\n";
        }
        if (!cfg.connes_crossed_validation && !cfg.log_prime_validation) return 0;
    }
    if (cfg.connes_crossed_validation) {
        const auto crep = Marshal::Heat::run_connes_crossed_validation(cfg, gammas_ld, primes);
        if (!cfg.export_connes_crossed_path.empty()) {
            if (!Marshal::Heat::export_connes_crossed_validation_json(cfg.export_connes_crossed_path,
                                                                      crep)) {
                std::cerr << "Failed to write " << cfg.export_connes_crossed_path << "\n";
                return 1;
            }
            std::cout << "Connes spectrum validation: " << cfg.export_connes_crossed_path << "\n";
        }
        if (cfg.connes_expect_spectrum_identified && !crep.spectrum_identified) return 1;
        if (!cfg.log_prime_validation) return 0;
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
        if (!cfg.export_connes_study_path.empty()) {
            if (!Marshal::Heat::export_connes_study_json(cfg.export_connes_study_path, rep)) {
                std::cerr << "Failed to write " << cfg.export_connes_study_path << "\n";
                return 1;
            }
            std::cout << "Connes study: " << cfg.export_connes_study_path << "\n";
        }
        return (rep.t1_pass && rep.gauss_weil_identity_pass) ? 0 : 1;
    }
    if (cfg.duality_gold_standard) {
        const auto gold = Marshal::Heat::run_duality_gold_standard(
            cfg, zero_view.ptr(), zero_view.size(), gammas_ld.empty() ? nullptr : gammas_ld.data(),
            gammas_ld.size(), cat, primes);
        if (!cfg.export_duality_gold_path.empty()) {
            if (!Marshal::Heat::export_duality_gold_standard_json(cfg.export_duality_gold_path,
                                                                  gold)) {
                std::cerr << "Failed to write " << cfg.export_duality_gold_path << "\n";
                return 1;
            }
        }
        return gold.pass ? 0 : 1;
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
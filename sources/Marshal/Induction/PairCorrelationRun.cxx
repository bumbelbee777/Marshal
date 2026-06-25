#include "Induction.hxx"
#include "Analysis/PairCorrelation.hxx"
#include "AnaVM/AnaFormal.hxx"
#include "AnaVM/AnaVm.hxx"
#include "AnaVM/FormalCalibration.hxx"
#include "AnaVM/OperatorTraits.hxx"
#include <iostream>

namespace Marshal::Induction {
namespace {

int count_zeros_in_window(const std::vector<double>& gammas, Real T) {
    int n = 0;
    for (double g : gammas)
        if (g > 0 && static_cast<Real>(g) <= T) ++n;
    return n;
}

}  // namespace

PairCorrelationRunResult RunPairCorrelation(const Config& cfg, const std::vector<double>& gammas,
                                            const std::vector<int>& primes) {
    PairCorrelationRunResult out;
    if (gammas.size() < 4 || primes.empty()) return out;

    const int max_z = cfg.pair_correlation_max_zeros > 0
                          ? cfg.pair_correlation_max_zeros
                          : static_cast<int>(std::min(gammas.size(), size_t{5000}));
    const int n_cyl = cfg.pair_correlation_n_cylinder > 0
                          ? cfg.pair_correlation_n_cylinder
                          : max_z;

    out.report = Analysis::compute_pair_correlation(gammas, primes, n_cyl, max_z);
    out.ok = out.report.n_zero_spacings >= 3 && out.report.n_cylinder_spacings >= 3;

    std::cout << "\n=== Pair correlation (Montgomery GUE vs cylinder) ===\n";
    std::cout << "  zero spacings:     " << out.report.n_zero_spacings << "\n";
    std::cout << "  cylinder spacings: " << out.report.n_cylinder_spacings << "\n";
    std::cout << "  GUE spacing L² (zeros):     " << static_cast<double>(out.report.gue_spacing_l2_zero)
              << "\n";
    std::cout << "  GUE spacing L² (cylinder):  "
              << static_cast<double>(out.report.gue_spacing_l2_cylinder) << "\n";
    std::cout << "  cylinder excess vs zeros:   "
              << static_cast<double>(out.report.cylinder_vs_gue_excess) << "\n";
    std::cout << "  Montgomery R₂ L² (zeros):   "
              << static_cast<double>(out.report.montgomery_r2_l2) << "\n";
    std::cout << "  zeros GUE-like:      " << (out.report.zeros_gue_like ? "yes" : "no") << "\n";
    std::cout << "  cylinder Poisson-like: " << (out.report.cylinder_poisson_like ? "yes" : "no")
              << "\n";
    std::cout << "  separates from GUE:  " << (out.report.separates_from_gue ? "yes" : "no")
              << "\n";

    if (!cfg.export_pair_correlation_path.empty())
        Analysis::export_pair_correlation_json(cfg.export_pair_correlation_path, out.report);
    return out;
}

FormalAnalyticsRunResult RunFormalAnalytics(const Config& cfg, const std::vector<double>& gammas,
                                            const std::vector<int>& primes,
                                            const Analysis::PairCorrelationReport* pair) {
    FormalAnalyticsRunResult out;
    if (cfg.anavm_program.empty()) {
        std::cerr << "formal-analytics requires --anavm PATH\n";
        return out;
    }
    const auto cr = AnaVM::compile_program(cfg.anavm_program, false);
    if (!cr.ok) {
        AnaVM::print_errors(cr.errors);
        return out;
    }
    const auto traits = AnaVM::infer_traits(cr.program);
    const auto cal = AnaVM::build_formal_calibration(cr.program);
    const Real T = cfg.formal_counting_window > 0 ? cfg.formal_counting_window : 100.0L;
    const auto counting = AnaVM::compute_counting_analytics(primes, T, count_zeros_in_window(gammas, T));
    out.result = AnaVM::run_formal_analytics(cr.program, traits, counting, pair);
    out.calibration = cal;
    out.ok = true;

    std::cout << "\n=== AnaVM formal analytics ===\n";
    std::cout << "  ansatz: " << out.result.ansatz_id << "\n";
    std::cout << "  mrs_emit_ready: " << (out.result.mrs_emit_ready ? "yes" : "no") << "\n";
    std::cout << "  cylinder_class_excluded: "
              << (out.result.cylinder_class_excluded ? "yes" : "no") << "\n";
    std::cout << "  proved gates: " << out.result.proved_gates.size() << "\n";
    for (const auto& g : out.result.proved_gates) std::cout << "    + " << g << "\n";
    for (const auto& g : out.result.failed_gates) std::cout << "    - " << g << "\n";

    const std::string path = !cfg.export_formal_cal_path.empty()
                                 ? cfg.export_formal_cal_path
                                 : cfg.export_formal_analytics_path;
    if (!path.empty()) {
        out.calibration.mrs_emit_ready = out.result.mrs_emit_ready;
        AnaVM::export_formal_analytics_json(path, out.result, out.calibration);
    }
    return out;
}

}  // namespace Marshal::Induction

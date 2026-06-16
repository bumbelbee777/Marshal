#include "ArchimedeanBoundaryValidation.hxx"

#include "Heat/Common.hxx"
#include "Heat/LogPrimeGlobal.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Induction/Induction.hxx"
#include "TraceApi.hxx"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

namespace Marshal::Heat {
namespace {

std::vector<ArchimedeanBoundarySpec> build_sweep_specs(const Config& cfg) {
    std::vector<ArchimedeanBoundarySpec> specs;
    if (!cfg.archimedean_sweep_all && cfg.archimedean.present) {
        specs.push_back(ArchimedeanBoundarySpec::from_mrs(cfg.archimedean));
        return specs;
    }

    const AnaVM::ArchimedeanType types[] = {AnaVM::ArchimedeanType::RealLine,
                                            AnaVM::ArchimedeanType::HalfLine,
                                            AnaVM::ArchimedeanType::Torus};
    const AnaVM::ArchimedeanBoundary bounds[] = {
        AnaVM::ArchimedeanBoundary::Dirichlet, AnaVM::ArchimedeanBoundary::Neumann,
        AnaVM::ArchimedeanBoundary::Periodic, AnaVM::ArchimedeanBoundary::BerryKeating};

    for (auto ty : types) {
        for (auto bd : bounds) {
            specs.push_back({ty, bd, AnaVM::ArchimedeanCutoff::PlanckScale, kArchPlanckScale});
        }
    }
    return specs;
}

ArchimedeanBoundaryRow evaluate_spec(const Config& cfg, const ArchimedeanBoundarySpec& spec,
                                     const std::vector<double>& gammas,
                                     const std::vector<Real>& gammas_ld,
                                     PrimeCatalog& cat, const std::vector<int>& primes) {
    ArchimedeanBoundaryRow row;
    row.type = spec.type;
    row.boundary = spec.boundary;
    row.cutoff = spec.cutoff;
    row.lambda = spec.cutoff_lambda;
    row.implementation = spec.implementation();
    if (spec.type == AnaVM::ArchimedeanType::Torus) row.fallback = "real_line";

    struct RowDef {
        const char* name;
        std::unique_ptr<TestFunction> tf;
        Real sigma;
    };
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const Real gauss_sigma = cfg.sigma >= 1.0L ? cfg.sigma : 5.0L;
    const Real sinc_T = cfg.test_param > 0 ? cfg.test_param : 14.134725142L;
    const Real kappa = cfg.sinc2_kappa > 0 ? cfg.sinc2_kappa : 60.0L;
    std::vector<RowDef> defs;
    defs.push_back({"gauss", std::make_unique<GaussTest>(gauss_sigma), gauss_sigma});
    defs.push_back({"sinc2", std::make_unique<Sinc2Test>(sinc_T, kappa), sinc_T});
    defs.push_back({"bump", std::make_unique<BumpTest>(1.0L), 1.0L});
    defs.push_back({"rational", std::make_unique<RationalTest>(1.0L), 1.0L});

    const size_t cap = primes.size();
    (void)cap;
    LogPrimeGlobal global = LogPrimeGlobal::from_primes(primes);

    for (const auto& d : defs) {
        cat.rebuild_adaptive(*d.tf, Induction::TauFromSigma(d.sigma), kmax, cfg.eps);
        const TraceResult tr =
            EvaluateTrace(*d.tf, d.sigma, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                          cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false,
                          &spec);
        const Real hlog = global.weil_prime_sum(*d.tf, kmax, cfg.eps) / (2.0L * kPi);
        ArchimedeanTestRow t;
        t.name = d.name;
        t.weil_residual = std::fabs(tr.residual());
        t.t1_gap = std::fabs(hlog - tr.prime);
        t.weil_pass = t.weil_residual < 1e-6L * std::max(Real{1}, std::fabs(tr.lhs));
        t.t1_pass = t.t1_gap < 1e-6L * std::max(Real{1}, std::fabs(tr.prime));
        row.tests.push_back(t);
        row.max_weil_residual = std::max(row.max_weil_residual, t.weil_residual);
    }
    if (!row.tests.empty()) {
        row.all_tests_pass = true;
        for (const auto& t : row.tests) row.all_tests_pass = row.all_tests_pass && t.weil_pass;
    }
    return row;
}

}  // namespace

ArchimedeanBoundaryReport run_archimedean_boundary_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const std::vector<int>& primes) {
    ArchimedeanBoundaryReport rep;
    rep.program_id = cfg.anavm.id;
    rep.sweep_mode = cfg.archimedean_sweep_all;

    const size_t zero_cap = std::min(gammas.size(), size_t{5000});
    std::vector<double> gsub(gammas.begin(), gammas.begin() + static_cast<ptrdiff_t>(zero_cap));
    std::vector<Real> gld_sub;
    if (!gammas_ld.empty()) {
        gld_sub.assign(gammas_ld.begin(),
                       gammas_ld.begin() + static_cast<ptrdiff_t>(
                                               std::min(gammas_ld.size(), zero_cap)));
    }
    const std::vector<double>& g_use = gsub;
    const std::vector<Real>& gld_use = gld_sub.empty() ? gammas_ld : gld_sub;

    std::vector<int> primes_use = primes;
    if (primes.size() > 5000) primes_use.resize(5000);
    PrimeCatalog cat;
    cat.set_primes(primes_use);

    const auto specs = build_sweep_specs(cfg);
    int best_idx = -1;
    Real best_max = 1e300L;
    for (const auto& spec : specs) {
        auto row = evaluate_spec(cfg, spec, g_use, gld_use, cat, primes_use);
        if (row.all_tests_pass && row.max_weil_residual < best_max) {
            best_max = row.max_weil_residual;
            best_idx = static_cast<int>(rep.rows.size());
        }
        rep.rows.push_back(std::move(row));
    }
    rep.best_row_index = best_idx;
    if (best_idx >= 0)
        rep.verdict = "ARCH_BOUNDARY_IDENTIFIED";
    else if (!rep.rows.empty())
        rep.verdict = "ARCH_BOUNDARY_MISMATCH";
    else
        rep.verdict = "INCONCLUSIVE";

    std::cout << "=== Archimedean boundary sweep ===\n";
    std::cout << "  rows=" << rep.rows.size() << "  best_row=" << rep.best_row_index
              << "  verdict=" << rep.verdict << "\n";
    return rep;
}

bool export_archimedean_boundary_json(const std::string& path,
                                      const ArchimedeanBoundaryReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"sweep_mode\": " << (r.sweep_mode ? "true" : "false") << ",\n";
    out << "  \"best_row_index\": " << r.best_row_index << ",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n  \"rows\": [\n";
    for (size_t i = 0; i < r.rows.size(); ++i) {
        const auto& row = r.rows[i];
        out << "    {\n";
        out << "      \"type\": \"" << arch_type_name(row.type) << "\",\n";
        out << "      \"boundary\": \"" << arch_boundary_name(row.boundary) << "\",\n";
        out << "      \"cutoff\": \"" << arch_cutoff_name(row.cutoff) << "\",\n";
        out << "      \"lambda\": " << static_cast<double>(row.lambda) << ",\n";
        out << "      \"implementation\": \""
            << (row.implementation == ArchImplementation::Scaffold ? "scaffold" : "preliminary")
            << "\",\n";
        if (!row.fallback.empty()) out << "      \"fallback\": \"" << row.fallback << "\",\n";
        out << "      \"all_tests_pass\": " << (row.all_tests_pass ? "true" : "false") << ",\n";
        out << "      \"max_weil_residual\": " << static_cast<double>(row.max_weil_residual)
            << ",\n      \"tests\": {\n";
        for (size_t j = 0; j < row.tests.size(); ++j) {
            const auto& t = row.tests[j];
            out << "        \"" << t.name << "\": { \"weil_residual\": "
                << static_cast<double>(t.weil_residual) << ", \"t1_gap\": "
                << static_cast<double>(t.t1_gap) << ", \"weil_pass\": "
                << (t.weil_pass ? "true" : "false") << ", \"t1_pass\": "
                << (t.t1_pass ? "true" : "false") << " }";
            if (j + 1 < row.tests.size()) out << ",";
            out << "\n";
        }
        out << "      }\n    }";
        if (i + 1 < r.rows.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

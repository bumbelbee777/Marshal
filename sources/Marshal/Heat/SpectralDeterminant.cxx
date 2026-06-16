#include "SpectralDeterminant.hxx"

#include "ArchimedeanBoundary.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "LogPrimeGlobal.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Heat {
namespace {

const char* boundary_name(AnaVM::ArchimedeanBoundary b) {
    switch (b) {
        case AnaVM::ArchimedeanBoundary::Dirichlet:
            return "dirichlet";
        case AnaVM::ArchimedeanBoundary::Neumann:
            return "neumann";
        case AnaVM::ArchimedeanBoundary::Periodic:
            return "periodic";
        default:
            return "berry_keating";
    }
}

SpectralDeterminantReport evaluate_laplace_boundary(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes, const ArchimedeanBoundarySpec& arch_spec) {
    SpectralDeterminantReport rep;
    rep.program_id = cfg.anavm.id;
    rep.spec = cfg.spectral_determinant;
    rep.laplace_a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;
    rep.boundary = arch_spec.boundary;
    rep.boundary_name = boundary_name(arch_spec.boundary);

    const LaplaceTest laplace(rep.laplace_a);
    const Real sigma = rep.laplace_a;
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const size_t n_zeros = gammas_ld.empty() ? gammas.size() : gammas_ld.size();
    const size_t nz = std::min(n_zeros, size_t{50000});
    const double* gptr = gammas.empty() ? nullptr : gammas.data();
    const Real* gld_ptr = gammas_ld.empty() ? nullptr : gammas_ld.data();
    const size_t n_ld = gammas_ld.size();

    cat.rebuild_adaptive(laplace, 1.0L / (2.0L * sigma * sigma), kmax, cfg.eps);
    const TraceResult tr =
        EvaluateTracePrefix(laplace, sigma, gptr, nz, gld_ptr, n_ld, cat, cfg.zero_kernel,
                            cfg.simd, cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts,
                            false, &arch_spec);

    const int max_primes = static_cast<int>(std::min(primes.size(), size_t{5000}));
    std::vector<int> sub(primes.begin(), primes.begin() + max_primes);
    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(sub);
    const Real hlog = global.weil_prime_sum(laplace, kmax, cfg.eps) / (2.0L * kPi);

    const Real log_lhs = std::log(std::max(std::fabs(tr.lhs), Real{1e-300}));
    const Real log_arch = std::log(std::max(std::fabs(tr.arch), Real{1e-300}));
    const Real log_rhs_total =
        std::log(std::max(std::fabs(tr.poles + tr.arch - tr.prime), Real{1e-300}));
    const Real log_hlog = std::log(std::max(hlog, Real{1e-300}));

    rep.laplace_weil_residual = std::fabs(tr.residual());
    rep.t1_gap = std::fabs(tr.prime - hlog);
    rep.log_det_gap = std::fabs(log_lhs - log_rhs_total);

    const int ns = static_cast<int>(std::min(gammas_ld.size(), size_t{20}));
    Real sum_gap = 0;
    for (int i = 0; i < ns; ++i) {
        const Real gamma = gammas_ld[static_cast<size_t>(i)];
        const Real t_off = gamma + Real{0.5};
        SpectralDetPoint pt;
        pt.s_im = t_off;
        pt.log_det_prime = log_hlog;
        pt.log_det_arch = log_arch;
        pt.log_lhs_zeros = log_lhs;
        pt.log_weil_rhs = log_rhs_total;
        pt.laplace_duality_gap = std::fabs(tr.lhs - (tr.poles + tr.arch - tr.prime));
        pt.gap = std::fabs(log_lhs - log_rhs_total);
        rep.samples.push_back(pt);
        sum_gap += pt.gap;
        rep.xi_det_gap_max = std::max(rep.xi_det_gap_max, pt.gap);
        rep.laplace_weil_log_gap_max = std::max(rep.laplace_weil_log_gap_max, pt.gap);
    }
    rep.xi_det_gap = ns > 0 ? sum_gap / static_cast<Real>(ns) : rep.log_det_gap;
    rep.laplace_weil_log_gap = rep.xi_det_gap;
    rep.verdict = rep.laplace_weil_residual < 1.0L && rep.log_det_gap < 0.5L ? "XI_DET_APPROACHING"
                                                                               : "XI_DET_MISMATCH";
    return rep;
}

}  // namespace

SpectralDeterminantReport run_spectral_determinant_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes) {
    const AnaVM::ArchimedeanBoundary bounds[] = {
        AnaVM::ArchimedeanBoundary::BerryKeating, AnaVM::ArchimedeanBoundary::Dirichlet,
        AnaVM::ArchimedeanBoundary::Neumann, AnaVM::ArchimedeanBoundary::Periodic};

    if (!cfg.spectral_det_boundary_sweep) {
        ArchimedeanBoundarySpec arch_spec = ArchimedeanBoundarySpec::from_mrs(cfg.archimedean);
        if (!cfg.archimedean.present) {
            arch_spec.boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
            arch_spec.type = AnaVM::ArchimedeanType::RealLine;
        }
        auto rep = evaluate_laplace_boundary(cfg, gammas, gammas_ld, cat, primes, arch_spec);
        std::cout << "=== Spectral determinant (Laplace h) ===\n";
        std::cout << "  boundary=" << rep.boundary_name << "  a=" << static_cast<double>(rep.laplace_a)
                  << "\n";
        std::cout << "  laplace_weil_residual=" << static_cast<double>(rep.laplace_weil_residual)
                  << "  log_det_gap=" << static_cast<double>(rep.log_det_gap)
                  << "  t1_gap=" << static_cast<double>(rep.t1_gap) << "\n";
        std::cout << "  laplace_weil_log_gap=" << static_cast<double>(rep.laplace_weil_log_gap)
                  << "  verdict=" << rep.verdict << "\n";
        return rep;
    }

    SpectralDeterminantReport best;
    best.xi_det_gap = 1e300L;
    std::vector<SpectralDetBoundaryRow> sweep_rows;
    int best_idx = -1;
    for (size_t i = 0; i < sizeof(bounds) / sizeof(bounds[0]); ++i) {
        ArchimedeanBoundarySpec spec;
        spec.boundary = bounds[i];
        spec.type = AnaVM::ArchimedeanType::RealLine;
        spec.cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;
        const auto r = evaluate_laplace_boundary(cfg, gammas, gammas_ld, cat, primes, spec);
        SpectralDetBoundaryRow row;
        row.boundary = bounds[i];
        row.boundary_name = boundary_name(bounds[i]);
        row.laplace_weil_residual = r.laplace_weil_residual;
        row.xi_det_gap = r.xi_det_gap;
        row.log_det_gap = r.log_det_gap;
        row.t1_gap = r.t1_gap;
        sweep_rows.push_back(row);
        if (r.xi_det_gap < best.xi_det_gap) {
            best = r;
            best_idx = static_cast<int>(i);
        }
    }
    SpectralDeterminantReport out = best;
    out.boundary_sweep = std::move(sweep_rows);
    out.best_boundary_index = best_idx;
    std::cout << "=== Spectral determinant boundary sweep (Laplace) ===\n";
    for (const auto& row : out.boundary_sweep) {
        std::cout << "  " << row.boundary_name << "  xi_gap=" << static_cast<double>(row.xi_det_gap)
                  << "  weil_res=" << static_cast<double>(row.laplace_weil_residual) << "\n";
    }
    std::cout << "  best=" << out.boundary_name << "  xi_det_gap="
              << static_cast<double>(out.xi_det_gap) << "\n";
    return out;
}

bool export_spectral_determinant_json(const std::string& path, const SpectralDeterminantReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"test_function\": \"laplace_exp_abs\",\n";
    out << "  \"laplace_a\": " << static_cast<double>(r.laplace_a) << ",\n";
    out << "  \"boundary\": \"" << r.boundary_name << "\",\n";
    out << "  \"laplace_weil_residual\": " << static_cast<double>(r.laplace_weil_residual) << ",\n";
    out << "  \"log_det_gap\": " << static_cast<double>(r.log_det_gap) << ",\n";
    out << "  \"t1_gap\": " << static_cast<double>(r.t1_gap) << ",\n";
    out << "  \"laplace_weil_log_gap\": " << static_cast<double>(r.laplace_weil_log_gap) << ",\n";
    out << "  \"laplace_weil_log_gap_max\": " << static_cast<double>(r.laplace_weil_log_gap_max)
        << ",\n";
    out << "  \"xi_det_gap\": " << static_cast<double>(r.laplace_weil_log_gap) << ",\n";
    out << "  \"xi_det_gap_max\": " << static_cast<double>(r.laplace_weil_log_gap_max) << ",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n";
    out << "  \"best_boundary_index\": " << r.best_boundary_index << ",\n";
    out << "  \"boundary_sweep\": [\n";
    for (size_t i = 0; i < r.boundary_sweep.size(); ++i) {
        const auto& b = r.boundary_sweep[i];
        out << "    { \"boundary\": \"" << b.boundary_name << "\", \"xi_det_gap\": "
            << static_cast<double>(b.xi_det_gap) << ", \"log_det_gap\": "
            << static_cast<double>(b.log_det_gap) << ", \"laplace_weil_residual\": "
            << static_cast<double>(b.laplace_weil_residual) << ", \"t1_gap\": "
            << static_cast<double>(b.t1_gap) << " }";
        if (i + 1 < r.boundary_sweep.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"samples\": [\n";
    for (size_t i = 0; i < r.samples.size(); ++i) {
        const auto& p = r.samples[i];
        out << "    { \"s_im\": " << static_cast<double>(p.s_im) << ", \"log_det_prime\": "
            << static_cast<double>(p.log_det_prime) << ", \"log_det_arch\": "
            << static_cast<double>(p.log_det_arch) << ", \"log_lhs_zeros\": "
            << static_cast<double>(p.log_lhs_zeros) << ", \"laplace_duality_gap\": "
            << static_cast<double>(p.laplace_duality_gap) << ", \"gap\": "
            << static_cast<double>(p.gap) << " }";
        if (i + 1 < r.samples.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

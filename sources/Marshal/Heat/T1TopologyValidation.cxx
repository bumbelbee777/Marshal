#include "T1TopologyValidation.hxx"

#include "ArchimedeanBoundary.hxx"
#include "BerryKeatingOperator.hxx"
#include "Heat/Common.hxx"
#include "Heat/LogPrimeGlobal.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>

namespace Marshal::Heat {
namespace {

ArchimedeanBoundarySpec bk_arch_spec() {
    ArchimedeanBoundarySpec arch;
    arch.boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
    arch.type = AnaVM::ArchimedeanType::RealLine;
    arch.cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;
    return arch;
}

Real classical_arch_reference(const Config& cfg, PrimeCatalog& cat, const std::vector<int>& /*primes*/) {
    const Real laplace_a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;
    const LaplaceTest laplace(laplace_a);
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    cat.rebuild_adaptive(laplace, 1.0L / (2.0L * laplace_a * laplace_a), kmax, cfg.eps);
    const auto arch = bk_arch_spec();
    const TraceResult tr =
        EvaluateTracePrefix(laplace, laplace_a, nullptr, 0, nullptr, 0, cat, cfg.zero_kernel,
                            cfg.simd, cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts,
                            false, &arch);
    return tr.arch;
}

}  // namespace

Real t1_gap_at_theta(const Config& cfg, Real theta, const std::vector<Real>& gammas_ld,
                     PrimeCatalog& cat, const std::vector<int>& primes, Real* prime_gap_out,
                     Real* arch_gap_out) {
    const Real laplace_a = cfg.duality_a > 0 ? cfg.duality_a : 1.0L;
    const LaplaceTest laplace(laplace_a);
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    cat.rebuild_adaptive(laplace, 1.0L / (2.0L * laplace_a * laplace_a), kmax, cfg.eps);

    BerryKeatingSpec bspec = spec_from_config(cfg);
    bspec.theta = theta;
    const auto ladder = bk_wkb_ladder(bspec, false);
    const size_t nz = std::min(gammas_ld.size(), ladder.size());
    const Real* gptr = nz > 0 ? ladder.data() : gammas_ld.data();
    const size_t nuse = nz > 0 ? nz : gammas_ld.size();

    const auto arch = bk_arch_spec();
    const TraceResult tr =
        EvaluateTracePrefix(laplace, laplace_a, nullptr, nuse, gptr, nuse, cat, cfg.zero_kernel,
                            cfg.simd, cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts,
                            false, &arch);

    const int max_primes = static_cast<int>(std::min(primes.size(), size_t{5000}));
    std::vector<int> sub(primes.begin(), primes.begin() + max_primes);
    const LogPrimeGlobal global = LogPrimeGlobal::from_primes(sub);
    const Real hlog = global.weil_prime_sum(laplace, kmax, cfg.eps) / (2.0L * kPi);

    const Real prime_gap = std::fabs(tr.prime - hlog);
    static Real classical_arch = -1;
    if (classical_arch < 0) classical_arch = classical_arch_reference(cfg, cat, primes);
    const Real arch_gap = std::fabs(tr.arch - classical_arch);
    const Real total = std::max(prime_gap, arch_gap);

    if (prime_gap_out) *prime_gap_out = prime_gap;
    if (arch_gap_out) *arch_gap_out = arch_gap;
    return total;
}

T1TopologyValidationReport run_t1_topology_validation(
    const Config& cfg, const std::vector<Real>& gammas_ld, PrimeCatalog& cat,
    const std::vector<int>& primes, const std::vector<Real>& theta_grid, Real t1_tol) {
    T1TopologyValidationReport rep;
    rep.program_id = cfg.anavm.id;
    rep.fixed_theta = cfg.investigation_spec.fixed_theta;
    rep.t1_tolerance = t1_tol;

    Real prime_min = 1e300L;
    Real prime_max = 0;
    for (Real theta : theta_grid) {
        T1GapAtTheta row;
        row.theta = theta;
        row.total_gap = t1_gap_at_theta(cfg, theta, gammas_ld, cat, primes, &row.prime_gap,
                                        &row.arch_gap);
        row.admissible = row.total_gap <= t1_tol;
        prime_min = std::min(prime_min, row.prime_gap);
        prime_max = std::max(prime_max, row.prime_gap);
        rep.sweep.push_back(row);
    }
    rep.prime_gap_uniform = prime_max - prime_min;
    return rep;
}

void export_t1_topology_validation_json(const std::string& path,
                                        const T1TopologyValidationReport& rep) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"program_id\": \"" << rep.program_id << "\",\n";
    out << "  \"fixed_theta\": " << static_cast<double>(rep.fixed_theta) << ",\n";
    out << "  \"t1_tolerance\": " << static_cast<double>(rep.t1_tolerance) << ",\n";
    out << "  \"prime_gap_uniform\": " << static_cast<double>(rep.prime_gap_uniform) << ",\n";
    out << "  \"series\": [\n";
    for (size_t i = 0; i < rep.sweep.size(); ++i) {
        const auto& s = rep.sweep[i];
        if (i) out << ",\n";
        out << "    {\"theta\": " << static_cast<double>(s.theta) << ", \"prime_gap\": "
            << static_cast<double>(s.prime_gap) << ", \"arch_gap\": "
            << static_cast<double>(s.arch_gap) << ", \"total_gap\": "
            << static_cast<double>(s.total_gap) << ", \"admissible\": "
            << (s.admissible ? "true" : "false") << "}";
    }
    out << "\n  ]\n}\n";
}

}  // namespace Marshal::Heat

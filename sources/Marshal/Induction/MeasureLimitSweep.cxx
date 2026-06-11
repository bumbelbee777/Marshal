#include "MeasureLimitSweep.hxx"

#include "InductionShared.hxx"
#include "SpectralDiagnostic.hxx"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace Marshal::Induction {

namespace {
constexpr Real kPi = 3.141592653589793238462643383279502884L;
}

MeasureLimitSweepResult RunMeasureLimitSweep(const Config& cfg, const TestFunction& /*tf*/,
                                             const std::vector<double>& gammas,
                                             const std::vector<Real>& gammas_ld,
                                             Heat::PrimeCatalog& cat,
                                             const std::vector<int>& primes) {
    MeasureLimitSweepResult result;
    if (gammas.empty()) return result;

    const int limits[] = {50000, 100000, 500000, 1000000, 5000000, 10000000};
    Real T = cfg.test_param > 0 ? cfg.test_param : 1.0L;
    if (cfg.test_param <= 0)
        T = 2.0L * kPi / static_cast<Real>(gammas[std::min(gammas.size() - 1, size_t{99})]);

    std::cout << "=== Measure-limit sweep (compact sinc² vs P) ===\n";
    std::cout << "  T=" << static_cast<double>(T) << "  zeros=" << gammas.size() << "\n\n";
    std::cout << std::setw(12) << "P_max" << std::setw(16) << "|residual|" << "\n";

    Real ref = 0;
    for (int plim : limits) {
        if (plim > cfg.prime_limit && plim != limits[0]) continue;
        const int use_lim = std::min(plim, cfg.prime_limit);
        std::vector<int> sub;
        sub.reserve(primes.size());
        for (int p : primes) {
            if (p > use_lim) break;
            sub.push_back(p);
        }
        if (sub.empty()) continue;

        cat.set_primes(sub);
        const CompactSinc2Result r =
            RunCompactSinc2Falsification(gammas, gammas_ld, cat, cfg, T, kCompactSinc2MismatchTol);

        MeasureLimitPoint pt;
        pt.prime_limit = use_lim;
        pt.sinc2_residual = r.residual;
        pt.mismatch_proved = r.mismatch_proved;
        result.points.push_back(pt);

        if (ref == 0) ref = r.residual;
        std::cout << std::scientific << std::setprecision(6) << std::setw(12) << use_lim
                  << std::setw(16) << static_cast<double>(r.residual) << "\n";
    }

    if (result.points.empty()) return result;
    result.reference_residual = ref;
    Real max_dev = 0;
    for (const auto& pt : result.points)
        max_dev = std::max(max_dev, std::fabs(pt.sinc2_residual - ref));
    result.max_deviation = max_dev;
    result.residual_stable = max_dev < 1e-6L && result.points.size() >= 2;

    std::cout << "\n  reference_residual=" << static_cast<double>(ref)
              << "  max_deviation=" << static_cast<double>(max_dev)
              << "  stable=" << (result.residual_stable ? "YES" : "NO") << "\n";
    return result;
}

void ExportMeasureLimitJson(const std::string& path, const MeasureLimitSweepResult& r) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"conjecture\": \"spectral_measure_limit_conjecture\",\n";
    out << "  \"residual_stable\": " << (r.residual_stable ? "true" : "false") << ",\n";
    out << "  \"reference_residual\": " << static_cast<double>(r.reference_residual) << ",\n";
    out << "  \"max_deviation\": " << static_cast<double>(r.max_deviation) << ",\n";
    out << "  \"points\": [\n";
    for (size_t i = 0; i < r.points.size(); ++i) {
        const auto& p = r.points[i];
        out << "    {\"prime_limit\": " << p.prime_limit
            << ", \"sinc2_residual\": " << static_cast<double>(p.sinc2_residual)
            << ", \"mismatch_proved\": " << (p.mismatch_proved ? "true" : "false") << "}";
        if (i + 1 < r.points.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}

}  // namespace Marshal::Induction

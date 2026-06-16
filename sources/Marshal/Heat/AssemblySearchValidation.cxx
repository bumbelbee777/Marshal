#include "AssemblySearchValidation.hxx"

#include "AdelicCauchy.hxx"
#include "ArchimedeanBoundary.hxx"
#include "AssemblyCache.hxx"
#include "CompletionValidation.hxx"
#include "ConnesCrossedProduct.hxx"
#include "Heat/Common.hxx"
#include "Heat/LogPrimeGlobal.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "HeightMap.hxx"
#include "Induction/Induction.hxx"
#include "SpectralDeterminant.hxx"
#include "TraceApi.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Marshal::Heat {
namespace {

Real effective_tolerance(const AnaVM::MrsCompletion& c) {
    if (c.constraint == AnaVM::CompletionConstraint::Exact) return 1e-12L;
    return static_cast<Real>(c.tolerance > 0 ? c.tolerance : 1e-6);
}

CompletionZeroComparison compare_freqs(const std::vector<Real>& limits,
                                       const std::vector<Real>& gammas, Real T, Real kappa) {
    CompletionZeroComparison cmp;
    cmp.n_matched = static_cast<int>(std::min(limits.size(), gammas.size()));
    if (cmp.n_matched == 0) return cmp;
    Real sum_sq = 0;
    for (int i = 0; i < cmp.n_matched; ++i) {
        const Real d = limits[static_cast<size_t>(i)] - gammas[static_cast<size_t>(i)];
        sum_sq += d * d;
        cmp.max_gap = std::max(cmp.max_gap, std::fabs(d));
        cmp.mean_gap += std::fabs(d);
    }
    cmp.rmse = std::sqrt(sum_sq / static_cast<Real>(cmp.n_matched));
    cmp.mean_gap /= static_cast<Real>(cmp.n_matched);
    Real lhs = 0;
    Real rhs = 0;
    for (Real w : limits) lhs += LogPrimeGlobal::sinc_sq_weil(kappa * w / T);
    for (int i = 0; i < cmp.n_matched; ++i)
        rhs += LogPrimeGlobal::sinc_sq_weil(kappa * gammas[static_cast<size_t>(i)] / T);
    cmp.sinc2_gap = std::fabs(lhs - rhs);
    return cmp;
}

std::vector<AssemblyPoint> quick_grid() {
    const Real a_vals[] = {0.5L, 1.0L, 2.0L};
    const Real b_vals[] = {-5.0L, 0.0L, 5.0L};
    const Real lam_vals[] = {0.0L, 0.05L, 0.1L, 0.5L};
    const AnaVM::AdelicMetric metrics[] = {AnaVM::AdelicMetric::Real, AnaVM::AdelicMetric::Mixed};
    const AnaVM::ArchimedeanBoundary bounds[] = {AnaVM::ArchimedeanBoundary::BerryKeating,
                                                 AnaVM::ArchimedeanBoundary::Dirichlet,
                                                 AnaVM::ArchimedeanBoundary::Neumann};
    const AnaVM::CompletionMethod methods[] = {AnaVM::CompletionMethod::Cauchy,
                                               AnaVM::CompletionMethod::CrossedProduct};
    std::vector<AssemblyPoint> grid;
    for (Real a : a_vals)
        for (Real b : b_vals)
            for (Real lam : lam_vals)
                for (auto m : metrics)
                    for (auto bd : bounds)
                        for (auto cm : methods) {
                            AssemblyPoint p;
                            p.height_a = a;
                            p.height_b = b;
                            p.connes_lambda = lam;
                            p.adelic_metric = m;
                            p.arch_boundary = bd;
                            p.completion_method = cm;
                            grid.push_back(p);
                        }
    return grid;
}

Real parse_json_double(const std::string& body, const char* key, Real fallback) {
    const std::string needle = std::string("\"") + key + "\"";
    const size_t pos = body.find(needle);
    if (pos == std::string::npos) return fallback;
    size_t colon = body.find(':', pos);
    if (colon == std::string::npos) return fallback;
    return static_cast<Real>(std::stold(body.substr(colon + 1)));
}

std::string parse_json_string(const std::string& body, const char* key) {
    const std::string needle = std::string("\"") + key + "\"";
    const size_t pos = body.find(needle);
    if (pos == std::string::npos) return {};
    size_t q1 = body.find('"', body.find(':', pos) + 1);
    if (q1 == std::string::npos) return {};
    size_t q2 = body.find('"', q1 + 1);
    if (q2 == std::string::npos) return {};
    return body.substr(q1 + 1, q2 - q1 - 1);
}

}  // namespace

bool parse_assembly_point_file(const std::string& path, AssemblyPoint& out) {
    std::ifstream in(path);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string body = ss.str();
    out.height_a = parse_json_double(body, "height_a", out.height_a);
    out.height_b = parse_json_double(body, "height_b", out.height_b);
    out.connes_lambda = parse_json_double(body, "connes_lambda", out.connes_lambda);
    const std::string metric = parse_json_string(body, "adelic_metric");
    if (metric == "real") out.adelic_metric = AnaVM::AdelicMetric::Real;
    else if (metric == "adelic") out.adelic_metric = AnaVM::AdelicMetric::Adelic;
    const std::string boundary = parse_json_string(body, "arch_boundary");
    if (boundary.find("dirichlet") != std::string::npos)
        out.arch_boundary = AnaVM::ArchimedeanBoundary::Dirichlet;
    else if (boundary.find("neumann") != std::string::npos)
        out.arch_boundary = AnaVM::ArchimedeanBoundary::Neumann;
    const std::string method = parse_json_string(body, "completion_method");
    if (method.find("crossed") != std::string::npos)
        out.completion_method = AnaVM::CompletionMethod::CrossedProduct;
    return true;
}

AssemblyPointResult evaluate_assembly_point(const Config& cfg, const AssemblyPoint& pt,
                                            const std::vector<double>& gammas,
                                            const std::vector<Real>& gammas_ld,
                                            const std::vector<int>& primes, int zero_cap,
                                            int prime_cap) {
    AssemblyPointResult res;
    res.point = pt;
    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    const int n_primes = std::min(prime_cap, static_cast<int>(primes.size()));
    std::vector<int> primes_sub(primes.begin(), primes.begin() + n_primes);
    std::vector<Real> gammas_sub;
    if (!gammas_ld.empty()) {
        const int nz = std::min(zero_cap, static_cast<int>(gammas_ld.size()));
        gammas_sub.assign(gammas_ld.begin(), gammas_ld.begin() + nz);
    } else {
        const int nz = std::min(zero_cap, static_cast<int>(gammas.size()));
        gammas_sub.reserve(static_cast<size_t>(nz));
        for (int i = 0; i < nz; ++i) gammas_sub.push_back(static_cast<Real>(gammas[static_cast<size_t>(i)]));
    }

    HeightMapSpec hm;
    hm.type = AnaVM::HeightMapType::Log;
    hm.a = pt.height_a;
    hm.b = pt.height_b;
    AssemblyCache cache;
    cache.build(primes_sub, n_primes, kmax, hm);

    const Real tol = effective_tolerance(cfg.completion);
    auto mapped = cache.mapped_limits(pt.adelic_metric, tol, false);
    if (pt.completion_method == AnaVM::CompletionMethod::CrossedProduct) {
        ConnesCrossedProduct cp = ConnesCrossedProduct::from_primes(primes_sub, pt.connes_lambda);
        const auto evals = cp.spectrum(kmax);
        for (Real ev : evals) mapped.push_back(apply_height_map(ev, hm));
        mapped = dedupe_frequencies(std::move(mapped));
    }

    const Real T = cfg.test_param > 0 ? cfg.test_param : 14.134725142L;
    const Real kappa = cfg.sinc2_kappa > 0 ? cfg.sinc2_kappa : 60.0L;
    std::vector<Real> raw_freqs;
    for (const auto& m : cache.modes) raw_freqs.push_back(m.omega);
    std::sort(raw_freqs.begin(), raw_freqs.end());
    raw_freqs = dedupe_frequencies(std::move(raw_freqs));
    const auto cmp_raw = compare_freqs(raw_freqs, gammas_sub, T, kappa);
    const auto cmp_mapped = compare_freqs(mapped, gammas_sub, T, kappa);
    res.rmse_raw = cmp_raw.rmse;
    res.rmse_mapped = cmp_mapped.rmse;
    res.sinc2_gap = cmp_mapped.sinc2_gap;

    const size_t zc = std::min(gammas.size(), static_cast<size_t>(zero_cap));
    std::vector<double> gsub(gammas.begin(), gammas.begin() + static_cast<ptrdiff_t>(zc));
    std::vector<Real> gld_sub;
    if (!gammas_ld.empty())
        gld_sub.assign(gammas_ld.begin(),
                       gammas_ld.begin() + static_cast<ptrdiff_t>(std::min(gammas_ld.size(), zc)));
    PrimeCatalog cat;
    cat.set_primes(primes_sub);
    ArchimedeanBoundarySpec arch_spec;
    arch_spec.boundary = pt.arch_boundary;
    arch_spec.type = AnaVM::ArchimedeanType::RealLine;
    arch_spec.cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;

    const Real gauss_sigma = cfg.sigma >= 1.0L ? cfg.sigma : 5.0L;
    GaussTest gauss(gauss_sigma);
    Sinc2Test sinc2(T, kappa);
    BumpTest bump(1.0L);
    RationalTest rational(1.0L);
    const TestFunction* tests[] = {&gauss, &sinc2, &bump, &rational};
    res.max_weil = 0;
    for (const TestFunction* tf : tests) {
        const Real sigma = (tf == &gauss) ? gauss_sigma : (tf == &sinc2 ? T : 1.0L);
        cat.rebuild_adaptive(*tf, Induction::TauFromSigma(sigma), kmax, cfg.eps);
        const TraceResult tr =
            EvaluateTrace(*tf, sigma, gsub, gld_sub.empty() ? gammas_ld : gld_sub, cat,
                          cfg.zero_kernel, cfg.simd, cfg.eps, false, cfg.precision_mode,
                          cfg.arch_pts, false, &arch_spec);
        const Real wr = std::fabs(tr.residual());
        res.weil_residuals.push_back(wr);
        res.max_weil = std::max(res.max_weil, wr);
    }

    Config scfg = cfg;
    scfg.connes_coupling_lambda = pt.connes_lambda;
    scfg.archimedean.present = true;
    scfg.archimedean.boundary = pt.arch_boundary;
    scfg.spectral_determinant.present = true;
    PrimeCatalog scat;
    scat.set_primes(primes_sub);
    const auto srep =
        run_spectral_determinant_validation(scfg, gsub, gld_sub.empty() ? gammas_ld : gld_sub, scat,
                                            primes_sub);
    res.xi_det_gap = srep.xi_det_gap;

    res.score = res.max_weil + Real{0.25} * res.rmse_mapped + Real{0.1} * res.xi_det_gap;
    return res;
}

AssemblySearchReport run_assembly_search_validation(const Config& cfg,
                                                    const std::vector<double>& gammas,
                                                    const std::vector<Real>& gammas_ld,
                                                    const std::vector<int>& primes) {
    AssemblySearchReport rep;
    rep.program_id = cfg.anavm.id;
    const auto& spec = cfg.assembly_search_spec;
    const int quick_zeros = spec.quick_zeros > 0 ? spec.quick_zeros : 5000;
    const int quick_primes = spec.quick_primes > 0 ? spec.quick_primes : 10000;
    const int full_zeros = spec.full_zeros > 0 ? spec.full_zeros : 100000;
    const int top_k = spec.top_k > 0 ? spec.top_k : 10;

    std::vector<AssemblyPoint> points;
    if (!cfg.assembly_point_path.empty()) {
        AssemblyPoint single;
        if (parse_assembly_point_file(cfg.assembly_point_path, single)) points.push_back(single);
        rep.tier = "single";
    } else {
        points = quick_grid();
        rep.tier = "quick";
    }
    rep.grid_points = static_cast<int>(points.size());

    std::vector<AssemblyPointResult> results;
    results.reserve(points.size());
    for (const auto& pt : points) {
        const int zc = cfg.assembly_point_path.empty() ? quick_zeros : full_zeros;
        const int pc = cfg.assembly_point_path.empty() ? quick_primes : static_cast<int>(primes.size());
        results.push_back(
            evaluate_assembly_point(cfg, pt, gammas, gammas_ld, primes, zc, pc));
    }
    std::sort(results.begin(), results.end(),
              [](const AssemblyPointResult& a, const AssemblyPointResult& b) {
                  return a.score < b.score;
              });

    if (cfg.assembly_point_path.empty() && !results.empty()) {
        rep.tier = "quick+full";
        std::vector<AssemblyPointResult> full_tier;
        const int nk = std::min(top_k, static_cast<int>(results.size()));
        for (int i = 0; i < nk; ++i) {
            full_tier.push_back(evaluate_assembly_point(cfg, results[static_cast<size_t>(i)].point,
                                                        gammas, gammas_ld, primes, full_zeros,
                                                        static_cast<int>(primes.size())));
        }
        std::sort(full_tier.begin(), full_tier.end(),
                  [](const AssemblyPointResult& a, const AssemblyPointResult& b) {
                      return a.score < b.score;
                  });
        rep.ranked = std::move(full_tier);
    } else {
        rep.ranked = std::move(results);
    }

    rep.verdict = rep.ranked.empty() ? "NO_CANDIDATES" : "ASSEMBLY_RANKED";
    std::cout << "=== Assembly search (" << rep.tier << ") ===\n";
    std::cout << "  grid=" << rep.grid_points << "  ranked=" << rep.ranked.size() << "\n";
    if (!rep.ranked.empty()) {
        const auto& best = rep.ranked.front();
        std::cout << "  best score=" << static_cast<double>(best.score)
                  << "  rmse_mapped=" << static_cast<double>(best.rmse_mapped)
                  << "  max_weil=" << static_cast<double>(best.max_weil) << "\n";
    }
    return rep;
}

bool export_assembly_search_json(const std::string& path, const AssemblySearchReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"program_id\": \"" << r.program_id << "\",\n";
    out << "  \"tier\": \"" << r.tier << "\",\n";
    out << "  \"grid_points\": " << r.grid_points << ",\n";
    out << "  \"verdict\": \"" << r.verdict << "\",\n  \"ranked\": [\n";
    for (size_t i = 0; i < r.ranked.size(); ++i) {
        const auto& p = r.ranked[i];
        out << "    { \"score\": " << static_cast<double>(p.score) << ", \"height_a\": "
            << static_cast<double>(p.point.height_a) << ", \"height_b\": "
            << static_cast<double>(p.point.height_b) << ", \"connes_lambda\": "
            << static_cast<double>(p.point.connes_lambda) << ", \"rmse_mapped\": "
            << static_cast<double>(p.rmse_mapped) << ", \"rmse_raw\": "
            << static_cast<double>(p.rmse_raw) << ", \"sinc2_gap\": "
            << static_cast<double>(p.sinc2_gap) << ", \"xi_det_gap\": "
            << static_cast<double>(p.xi_det_gap) << ", \"max_weil\": "
            << static_cast<double>(p.max_weil) << ", \"weil_residuals\": [";
        for (size_t j = 0; j < p.weil_residuals.size(); ++j) {
            if (j) out << ", ";
            out << static_cast<double>(p.weil_residuals[j]);
        }
        out << "] }";
        if (i + 1 < r.ranked.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

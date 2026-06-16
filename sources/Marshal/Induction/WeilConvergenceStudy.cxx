#include "WeilConvergenceStudy.hxx"

#include "Induction.hxx"
#include "TraceApi.hxx"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

namespace Marshal::Induction {

namespace {

std::vector<int> cap_primes(const std::vector<int>& primes, size_t cap) {
    if (cap == 0 || primes.size() <= cap) return primes;
    return std::vector<int>(primes.begin(), primes.begin() + static_cast<ptrdiff_t>(cap));
}

Real resolve_T(const Config& cfg, const std::vector<Real>& gammas_ld) {
    if (cfg.test_param > 0) return cfg.test_param;
    if (!gammas_ld.empty()) return gammas_ld.front();
    return 14.134725142L;
}

Real resolve_kappa(const Config& cfg, Real T, int p_max) {
    if (cfg.sinc2_kappa > 0) return cfg.sinc2_kappa;
    return suggest_sinc2_kappa(T, p_max);
}

}  // namespace

ArchSinc2AuditResult RunArchSinc2Audit(const Config& cfg) {
    ArchSinc2AuditResult result;
    result.T = cfg.test_param > 0 ? cfg.test_param : 14.134725142L;
    result.kappa = resolve_kappa(cfg, result.T, cfg.prime_limit);
    result.points = AuditArchSinc2(result.T, result.kappa, cfg.simd, cfg.precision_mode);
    result.adaptive_arch = ArchSinc2Adaptive(result.T, result.kappa, cfg.simd, cfg.precision_mode,
                                             cfg.arch_pts, cfg.eps);

    static const Real kVerifyKappas[] = {1.0L, 5.0L, 60.0L, 80.0L};
    for (Real kv : kVerifyKappas) {
        ArchKappaVerifyPoint vp;
        vp.kappa = kv;
        vp.adaptive_arch = ArchSinc2Adaptive(result.T, kv, cfg.simd, cfg.precision_mode,
                                             cfg.arch_pts, cfg.eps);
        result.kappa_verify.push_back(vp);
    }

    std::cout << "=== Arch sinc² audit T=" << static_cast<double>(result.T)
              << " kappa=" << static_cast<double>(result.kappa) << " ===\n";
    std::cout << std::setw(12) << "L" << std::setw(12) << "n_pts" << std::setw(18) << "arch"
              << std::setw(18) << "richardson" << "\n";
    for (const auto& pt : result.points) {
        std::cout << std::scientific << std::setprecision(6) << std::setw(12)
                  << static_cast<double>(pt.L) << std::setw(12) << pt.n_pts << std::setw(18)
                  << static_cast<double>(pt.arch) << std::setw(18)
                  << static_cast<double>(pt.richardson_est) << "\n";
    }
    std::cout << "  adaptive_arch=" << static_cast<double>(result.adaptive_arch) << "\n";
    std::cout << "  kappa_verify (adaptive arch vs kappa):\n";
    for (const auto& vp : result.kappa_verify) {
        std::cout << "    kappa=" << static_cast<double>(vp.kappa)
                  << "  arch=" << static_cast<double>(vp.adaptive_arch) << "\n";
    }
    return result;
}

ArchSinc2AuditResult RunArchSinc2Converge(const Config& cfg) {
    ArchSinc2AuditResult result;
    result.T = cfg.test_param > 0 ? cfg.test_param : 8.0L;
    result.kappa = resolve_kappa(cfg, result.T, cfg.prime_limit);
    result.used_converge = true;
    const int arch_max = cfg.arch_pts > 0 ? cfg.arch_pts : 2048001;
    result.converge =
        ArchSinc2Converge(result.T, result.kappa, cfg.arch_target, cfg.simd, cfg.precision_mode,
                          arch_max);
    result.adaptive_arch = result.converge.arch;
    result.points = result.converge.ladder;

    std::cout << "=== Arch sinc² converge T=" << static_cast<double>(result.T)
              << " kappa=" << static_cast<double>(result.kappa)
              << " target=" << static_cast<double>(cfg.arch_target) << " ===\n";
    std::cout << "  converged=" << (result.converge.converged ? "yes" : "no")
              << "  n_pts=" << result.converge.n_pts_final
              << "  L=" << static_cast<double>(result.converge.L_final)
              << "  arch=" << static_cast<double>(result.adaptive_arch) << "\n";
    return result;
}

WeilConvergenceResult RunWeilConvergenceStudy(const Config& cfg,
                                              const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld,
                                              Heat::PrimeCatalog& cat,
                                              const std::vector<int>& primes) {
    WeilConvergenceResult result;
    if (gammas.empty()) return result;

    const int kmax = cfg.kmax > 0 ? cfg.kmax : 20;
    result.T = resolve_T(cfg, gammas_ld);
    result.kappa = resolve_kappa(cfg, result.T, primes.empty() ? cfg.prime_limit : primes.back());
    const Sinc2Test sinc(result.T, result.kappa);

    static const size_t kZeroAnchors[] = {1000,   5000,   10000,  25000,  50000,
                                          100000, 250000, 500000};
    static const size_t kPrimeAnchors[] = {1000,   5000,   10000,  25000,  50000,
                                           100000, 250000, 500000};

    std::cout << "=== Weil convergence study T=" << static_cast<double>(result.T)
              << " kappa=" << static_cast<double>(result.kappa) << " ===\n";

    for (size_t nz : kZeroAnchors) {
        if (nz > gammas.size()) continue;

        cat.rebuild_adaptive(sinc, TauFromSigma(result.T), kmax, cfg.eps);
        const TraceResult tr = EvaluateTracePrefix(
            sinc, result.T, gammas.data(), nz,
            gammas_ld.empty() ? nullptr : gammas_ld.data(),
            gammas_ld.empty() ? 0 : nz, cat, cfg.zero_kernel, cfg.simd, cfg.eps,
            cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false);
        WeilConvergencePoint pt;
        pt.n_zeros = static_cast<int>(nz);
        pt.weil_residual = std::fabs(tr.residual());
        pt.zero_sum = tr.lhs;
        pt.prime = tr.prime;
        pt.arch = tr.arch;
        pt.poles = tr.poles;
        result.zero_ladder.push_back(pt);
        std::cout << "  zeros=" << nz << "  |res|=" << static_cast<double>(pt.weil_residual)
                  << "\n";
    }

    for (size_t np : kPrimeAnchors) {
        if (np > primes.size()) continue;
        const auto sub = cap_primes(primes, np);
        cat.set_primes(sub);
        cat.rebuild_adaptive(sinc, TauFromSigma(result.T), kmax, cfg.eps);
        const TraceResult tr =
            EvaluateTrace(sinc, result.T, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                          cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false);
        WeilConvergencePoint pt;
        pt.n_primes = static_cast<int>(sub.size());
        pt.p_max = sub.empty() ? 0 : sub.back();
        pt.weil_residual = std::fabs(tr.residual());
        pt.zero_sum = tr.lhs;
        pt.prime = tr.prime;
        pt.arch = tr.arch;
        pt.poles = tr.poles;
        result.prime_ladder.push_back(pt);
        std::cout << "  primes=" << np << "  |res|=" << static_cast<double>(pt.weil_residual)
                  << "\n";
    }

    if (!primes.empty()) {
        const size_t full = primes.size();
        const bool have_full = std::any_of(
            result.prime_ladder.begin(), result.prime_ladder.end(),
            [&](const WeilConvergencePoint& p) {
                return static_cast<size_t>(p.n_primes) == full;
            });
        if (!have_full) {
            cat.set_primes(primes);
            cat.rebuild_adaptive(sinc, TauFromSigma(result.T), kmax, cfg.eps);
            const TraceResult tr =
                EvaluateTrace(sinc, result.T, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd,
                              cfg.eps, cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false);
            WeilConvergencePoint pt;
            pt.n_primes = static_cast<int>(full);
            pt.p_max = primes.back();
            pt.weil_residual = std::fabs(tr.residual());
            pt.zero_sum = tr.lhs;
            pt.prime = tr.prime;
            pt.arch = tr.arch;
            pt.poles = tr.poles;
            result.prime_ladder.push_back(pt);
            std::cout << "  primes=" << full << " (full)  |res|="
                      << static_cast<double>(pt.weil_residual) << "\n";
        }
    }

    cat.set_primes(primes);
    return result;
}

bool ExportArchSinc2AuditJson(const std::string& path, const ArchSinc2AuditResult& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"T\": " << static_cast<double>(r.T) << ",\n";
    out << "  \"kappa\": " << static_cast<double>(r.kappa) << ",\n";
    out << "  \"adaptive_arch\": " << static_cast<double>(r.adaptive_arch) << ",\n";
    if (r.used_converge) {
        out << "  \"arch_converged\": " << (r.converge.converged ? "true" : "false") << ",\n";
        out << "  \"n_pts_final\": " << r.converge.n_pts_final << ",\n";
        out << "  \"L_final\": " << static_cast<double>(r.converge.L_final) << ",\n";
        out << "  \"arch_target\": " << static_cast<double>(r.converge.arch_target) << ",\n";
    }
    out << "  \"kappa_verify\": [\n";
    for (size_t i = 0; i < r.kappa_verify.size(); ++i) {
        const auto& vp = r.kappa_verify[i];
        out << "    {\"kappa\": " << static_cast<double>(vp.kappa) << ", \"adaptive_arch\": "
            << static_cast<double>(vp.adaptive_arch) << "}";
        if (i + 1 < r.kappa_verify.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"points\": [\n";
    for (size_t i = 0; i < r.points.size(); ++i) {
        const auto& pt = r.points[i];
        out << "    {\"L\": " << static_cast<double>(pt.L) << ", \"n_pts\": " << pt.n_pts
            << ", \"arch\": " << static_cast<double>(pt.arch) << ", \"richardson_est\": "
            << static_cast<double>(pt.richardson_est) << "}";
        if (i + 1 < r.points.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

bool ExportWeilConvergenceJson(const std::string& path, const WeilConvergenceResult& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"T\": " << static_cast<double>(r.T) << ",\n";
    out << "  \"kappa\": " << static_cast<double>(r.kappa) << ",\n";
    out << "  \"zero_ladder\": [\n";
    for (size_t i = 0; i < r.zero_ladder.size(); ++i) {
        const auto& pt = r.zero_ladder[i];
        out << "    {\"n_zeros\": " << pt.n_zeros << ", \"weil_residual\": "
            << static_cast<double>(pt.weil_residual) << ", \"zero_sum\": "
            << static_cast<double>(pt.zero_sum) << ", \"prime\": "
            << static_cast<double>(pt.prime) << ", \"arch\": " << static_cast<double>(pt.arch)
            << ", \"poles\": " << static_cast<double>(pt.poles) << "}";
        if (i + 1 < r.zero_ladder.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"prime_ladder\": [\n";
    for (size_t i = 0; i < r.prime_ladder.size(); ++i) {
        const auto& pt = r.prime_ladder[i];
        out << "    {\"n_primes\": " << pt.n_primes << ", \"p_max\": " << pt.p_max
            << ", \"weil_residual\": " << static_cast<double>(pt.weil_residual)
            << ", \"zero_sum\": " << static_cast<double>(pt.zero_sum) << ", \"prime\": "
            << static_cast<double>(pt.prime) << ", \"arch\": " << static_cast<double>(pt.arch)
            << ", \"poles\": " << static_cast<double>(pt.poles) << "}";
        if (i + 1 < r.prime_ladder.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Induction

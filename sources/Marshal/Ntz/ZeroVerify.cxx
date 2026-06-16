#include "ZeroVerify.hxx"

#include "IO/ZeroLoader.hxx"
#include "Ntz/RiemannSiegel.hxx"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Ntz {
namespace {

using weil_rs::RSTable;
using weil_rs::hardy_Z;
using weil_rs::refine_zero;

double BracketZero(RSTable& tab, double lo, double hi, double tol, int max_iter) {
    if (!(lo < hi)) return 0.5 * (lo + hi);
    double zlo = hardy_Z(tab, lo);
    double zhi = hardy_Z(tab, hi);
    if (zlo * zhi > 0.0) return 0.5 * (lo + hi);
    for (int it = 0; it < max_iter; ++it) {
        const double mid = 0.5 * (lo + hi);
        const double zm = hardy_Z(tab, mid);
        if (std::fabs(zm) < tol) return mid;
        if (zlo * zm <= 0.0) {
            hi = mid;
            zhi = zm;
        } else {
            lo = mid;
            zlo = zm;
        }
    }
    return 0.5 * (lo + hi);
}

std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\\' || c == '"') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

double ScanMinAbsZ(RSTable& tab, double lo, double hi, int samples) {
    if (!(lo < hi)) return 0.5 * (lo + hi);
    double best_t = 0.5 * (lo + hi);
    double best_z = 1e300;
    for (int k = 0; k <= samples; ++k) {
        const double t = lo + (hi - lo) * static_cast<double>(k) / static_cast<double>(samples);
        const double z = std::fabs(hardy_Z(tab, t));
        if (z < best_z) {
            best_z = z;
            best_t = t;
        }
    }
    return best_t;
}

bool IntervalHasZero(RSTable& tab, double lo, double hi, const ZeroVerifyOptions& opt,
                     double& z_out) {
    if (!(lo < hi)) return false;
    const double seed = ScanMinAbsZ(tab, lo, hi, 256);
    auto rr = refine_zero(tab, seed, opt.refine_iter * 2, opt.z_tol, lo, hi);
    z_out = std::fabs(rr.z_after);
    return z_out < opt.z_tol || (rr.converged && z_out < opt.z_tol * 10.0);
}

bool VerifyZeroAt(RSTable& tab, double gamma, double lo, double hi,
                  const ZeroVerifyOptions& opt, double& z_out) {
    if (lo < hi) {
        const double seed = ScanMinAbsZ(tab, lo, hi, 128);
        auto rr = refine_zero(tab, seed, opt.refine_iter * 2, opt.z_tol);
        z_out = std::fabs(rr.z_after);
        if (z_out < opt.z_tol || (rr.converged && z_out < opt.z_tol * 10.0)) return true;
    }

    auto rr = refine_zero(tab, gamma, opt.refine_iter, opt.z_tol);
    z_out = std::fabs(rr.z_after);
    if (z_out < opt.z_tol || (rr.converged && z_out < opt.z_tol * 10.0)) return true;

    if (lo < hi) {
        const double cell_lo = 0.5 * (lo + gamma);
        const double cell_hi = 0.5 * (gamma + hi);
        if (cell_lo < cell_hi) {
            const double seed = ScanMinAbsZ(tab, cell_lo, cell_hi, 128);
            rr = refine_zero(tab, seed, opt.refine_iter * 2, opt.z_tol);
            z_out = std::fabs(rr.z_after);
            if (z_out < opt.z_tol || (rr.converged && z_out < opt.z_tol * 10.0)) return true;
        }
        const double mid = BracketZero(tab, lo, hi, opt.z_tol, 64);
        rr = refine_zero(tab, mid, opt.refine_iter * 2, opt.z_tol);
        z_out = std::fabs(rr.z_after);
        if (z_out < opt.z_tol || (rr.converged && z_out < opt.z_tol * 10.0)) return true;
    }
    return false;
}

}  // namespace

bool RunZeroVerify(const ZeroVerifyOptions& opt, ZeroVerifyReport& rep, std::string& err) {
    if (opt.zeros_path.empty()) {
        err = "ZeroVerify: --zeros required";
        return false;
    }

#ifdef _OPENMP
    if (opt.threads > 0) omp_set_num_threads(opt.threads);
#endif

    std::vector<double> gammas;
    const size_t load_cap =
        opt.count > 0 ? opt.offset + opt.count : opt.max_count;
    if (!LoadZerosFast(opt.zeros_path, gammas, load_cap, false)) {
        err = "ZeroVerify: cannot load " + opt.zeros_path;
        return false;
    }
    if (opt.offset >= gammas.size()) {
        err = "ZeroVerify: offset past end of zero file";
        return false;
    }

    const size_t end = opt.count > 0 ? std::min(gammas.size(), opt.offset + opt.count)
                                     : std::min(gammas.size(), opt.max_count);
    const size_t n = end - opt.offset;
    if (n == 0) {
        err = "ZeroVerify: empty verification window";
        return false;
    }

    std::vector<double> z_abs(n, 0.0);
    std::vector<double> z_abs_ref(n, 0.0);
    std::vector<uint8_t> ok(n, 0);

    const auto t0 = std::chrono::steady_clock::now();

#pragma omp parallel
    {
        RSTable tab;
#pragma omp for schedule(dynamic, 64)
        for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(n); ++i) {
            const size_t idx = opt.offset + static_cast<size_t>(i);
            const double gamma = gammas[idx];
            const double z0 = std::fabs(hardy_Z(tab, gamma));
            const double lo = idx > 0 ? gammas[idx - 1] : gamma - 1.0;
            const double hi = idx + 1 < gammas.size() ? gammas[idx + 1] : gamma + 1.0;
            double zr = 0.0;
            bool pass = VerifyZeroAt(tab, gamma, lo, hi, opt, zr);
            if (!pass && idx > 0 && idx + 2 < gammas.size()) {
                const double wide_lo = gammas[idx - 1];
                const double wide_hi = gammas[idx + 2];
                pass = IntervalHasZero(tab, wide_lo, wide_hi, opt, zr);
            }
            z_abs[static_cast<size_t>(i)] = z0;
            z_abs_ref[static_cast<size_t>(i)] = zr;
            ok[static_cast<size_t>(i)] = pass ? 1 : 0;
        }
    }

    const auto t1 = std::chrono::steady_clock::now();
    rep.elapsed_ms =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
    rep.certified_count = n;
    rep.verified_count = 0;
    rep.max_z_abs = 0;
    rep.max_z_abs_refined = 0;
    rep.failed_index = n;
    rep.failed_indices.clear();

    for (size_t i = 0; i < n; ++i) {
        rep.max_z_abs = std::max(rep.max_z_abs, z_abs[i]);
        rep.max_z_abs_refined = std::max(rep.max_z_abs_refined, z_abs_ref[i]);
        if (ok[i]) {
            ++rep.verified_count;
        } else {
            rep.failed_indices.push_back(opt.offset + i);
            if (rep.failed_index == n) rep.failed_index = opt.offset + i;
        }
        if (i < opt.sample_head) {
            ZeroVerifyEntry e;
            e.index = opt.offset + i;
            e.height = gammas[opt.offset + i];
            e.z_abs = z_abs[i];
            e.z_abs_refined = z_abs_ref[i];
            e.verified = ok[i] != 0;
            rep.head_samples.push_back(e);
        }
    }
    rep.all_verified = rep.verified_count == n;
    rep.contiguous_verified = n;
    for (size_t i = 0; i < n; ++i) {
        if (!ok[i]) {
            rep.contiguous_verified = i;
            break;
        }
    }
    return true;
}

bool WriteZeroVerifyJson(const std::string& path, const ZeroVerifyOptions& opt,
                         const ZeroVerifyReport& rep, std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "ZeroVerify: cannot write " + path;
        return false;
    }
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"version\": 2,\n";
    out << "  \"cert_id\": \"marshal_odilyzko_zero_cert\",\n";
    out << "  \"engine\": \"marshal_riemann_siegel\",\n";
    out << "  \"source\": \"" << JsonEscape(opt.zeros_path) << "\",\n";
    out << "  \"offset\": " << opt.offset << ",\n";
    out << "  \"certified_count\": " << rep.certified_count << ",\n";
    out << "  \"verified_count\": " << rep.verified_count << ",\n";
    out << "  \"contiguous_verified\": " << rep.contiguous_verified << ",\n";
    out << "  \"all_verified\": " << (rep.all_verified ? "true" : "false") << ",\n";
    out << "  \"z_abs_tolerance\": " << opt.z_tol << ",\n";
    out << "  \"max_z_abs\": " << rep.max_z_abs << ",\n";
    out << "  \"max_z_abs_refined\": " << rep.max_z_abs_refined << ",\n";
    out << "  \"elapsed_ms\": " << rep.elapsed_ms << ",\n";
    out << "  \"failed_index\": " << rep.failed_index << ",\n";
    out << "  \"failed_indices\": [";
    for (size_t i = 0; i < rep.failed_indices.size(); ++i) {
        if (i) out << ", ";
        out << rep.failed_indices[i];
    }
    out << "],\n";
    out << "  \"lean_emit_ready\": " << (rep.all_verified ? "true" : "false") << ",\n";
    out << "  \"head_samples\": [\n";
    for (size_t i = 0; i < rep.head_samples.size(); ++i) {
        const auto& e = rep.head_samples[i];
        if (i) out << ",\n";
        out << "    {\"index\": " << e.index << ", \"height\": " << e.height
            << ", \"z_abs\": " << e.z_abs << ", \"z_abs_refined\": " << e.z_abs_refined
            << ", \"verified\": " << (e.verified ? "true" : "false") << "}";
    }
    out << "\n  ],\n";
    out << "  \"note\": \"|Z(gamma)| via Riemann-Siegel + Gabcke; Lean consumes certified_count only.\"\n";
    out << "}\n";
    return true;
}

}  // namespace Marshal::Ntz

#include "CombinedDiracFast.hxx"

#include "../Analysis/PairCorrelation.hxx"
#include "../Heat/ArchimedeanBoundary.hxx"
#include "../Heat/ConnesBasisCap.hxx"
#include "../Heat/LogPrimeGlobal.hxx"
#include "../Heat/TwistedLogPrimeOperator.hxx"
#include "SimdMicrokernels.hxx"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#ifdef _OPENMP
#include <omp.h>
#endif
#if defined(MARSHAL_HAVE_AVX2)
#include <immintrin.h>
#endif

namespace Marshal::Kernel {
namespace {

using Marshal::AnaVM::ArchimedeanBoundary;
using Marshal::Heat::ArchimedeanBoundarySpec;
using Marshal::Heat::BerryKeatingSpec;
using Marshal::Heat::bk_wkb_ladder;
using Marshal::Heat::boundary_window;
using Marshal::Heat::compare_to_zeros;
using Marshal::Heat::ConnesCouplingMode;
using Marshal::Heat::kArchPlanckScale;
using Marshal::Heat::kPi;
using Marshal::Heat::plan_connes_basis;
using Marshal::Heat::cap_primes_for_connes;
using Marshal::Heat::TwistedLogPrimeOperator;

inline size_t mat_idx(int n, int i, int j) {
    return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j);
}

double arch_diag_branchless(double gamma, ArchimedeanBoundary b, double window) {
    const int tag = static_cast<int>(b);
    const double bk = gamma;
    const double dir = gamma * (1.0 + 0.02 * (1.0 - window));
    const double neu = gamma * (1.0 + 0.01 * window);
    const double per = gamma * (1.0 + 0.015 * std::cos(gamma / static_cast<double>(kArchPlanckScale)));
    switch (tag) {
        case static_cast<int>(ArchimedeanBoundary::Dirichlet):
            return dir;
        case static_cast<int>(ArchimedeanBoundary::Neumann):
            return neu;
        case static_cast<int>(ArchimedeanBoundary::Periodic):
            return per;
        default:
            return bk;
    }
}

void fill_monotone_bridge(const std::vector<double>& arch, const std::vector<double>& prime_u,
                          std::vector<int>& out_j) {
    const int na = static_cast<int>(arch.size());
    const int np = static_cast<int>(prime_u.size());
    out_j.assign(static_cast<size_t>(na), 0);
    int j = 0;
    for (int i = 0; i < na; ++i) {
        while (j + 1 < np &&
               std::fabs(prime_u[static_cast<size_t>(j + 1)] - arch[static_cast<size_t>(i)]) <=
                   std::fabs(prime_u[static_cast<size_t>(j)] - arch[static_cast<size_t>(i)]))
            ++j;
        out_j[static_cast<size_t>(i)] = j;
    }
}

void jacobi_eigenvalues(int n, double* H, double* evals) {
    const int max_iter = std::max(400, 6 * n);
    for (int iter = 0; iter < max_iter; ++iter) {
        double max_off = 0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const double v = std::fabs(H[mat_idx(n, i, j)]);
                if (v > max_off) {
                    max_off = v;
                    p = i;
                    q = j;
                }
            }
        }
        if (max_off < 1e-12) break;
        const double app = H[mat_idx(n, p, p)];
        const double aqq = H[mat_idx(n, q, q)];
        const double apq = H[mat_idx(n, p, q)];
        const double phi = 0.5 * std::atan2(2.0 * apq, aqq - app);
        const double c = std::cos(phi);
        const double s = std::sin(phi);
        for (int k = 0; k < n; ++k) {
            const double akp = H[mat_idx(n, k, p)];
            const double akq = H[mat_idx(n, k, q)];
            H[mat_idx(n, k, p)] = c * akp - s * akq;
            H[mat_idx(n, p, k)] = H[mat_idx(n, k, p)];
            H[mat_idx(n, k, q)] = s * akp + c * akq;
            H[mat_idx(n, q, k)] = H[mat_idx(n, k, q)];
        }
        const double new_pp = c * c * app - 2.0 * s * c * apq + s * s * aqq;
        const double new_qq = s * s * app + 2.0 * s * c * apq + c * c * aqq;
        H[mat_idx(n, p, p)] = new_pp;
        H[mat_idx(n, q, q)] = new_qq;
        H[mat_idx(n, p, q)] = H[mat_idx(n, q, p)] = 0;
    }
    for (int i = 0; i < n; ++i) evals[i] = H[mat_idx(n, i, i)];
    std::sort(evals, evals + n);
}

}  // namespace

void SymmetricEigenvalues(int n, const double* A, double* evals_out) {
    std::vector<double> work(static_cast<size_t>(n) * static_cast<size_t>(n));
    std::memcpy(work.data(), A, work.size() * sizeof(double));
    SymmetricEigenvaluesInplace(n, work.data(), evals_out);
}

void SymmetricEigenvaluesInplace(int n, double* A, double* evals_out) {
    jacobi_eigenvalues(n, A, evals_out);
}

double FusedSpectralActionHeat(const double* evals, int n, double scale_base, int n_scales) {
    if (!evals || n <= 0 || n_scales <= 0) return 0;
    double total = 0;
    for (int s = 0; s < n_scales; ++s) {
        const double inv_2ss = 1.0 / (2.0 * scale_base * scale_base * static_cast<double>((s + 1) * (s + 1)));
        double part = 0;
        int i = 0;
#if defined(MARSHAL_HAVE_AVX2)
        __m256d acc = _mm256_setzero_pd();
        for (; i + 4 <= n; i += 4) {
            __m256d g = _mm256_loadu_pd(evals + i);
            __m256d e = ExpNegSq4(g, inv_2ss);
            acc = _mm256_add_pd(acc, e);
        }
        part += HorizontalSum4(acc);
#endif
        for (; i < n; ++i) {
            const double lam = evals[i];
            if (lam > 0) part += Exp1(-lam * lam * inv_2ss);
        }
        total += part;
    }
    return total;
}

Real CombinedDiracWorkspace::theta_at(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(theta_values_.size())) return 0;
    return theta_values_[static_cast<size_t>(idx)];
}

void CombinedDiracWorkspace::build_arch_for_theta(Real theta, std::vector<double>& al,
                                                std::vector<int>& bj,
                                                std::vector<double>& bb) const {
    BerryKeatingSpec bspec = spec_.bk;
    bspec.theta = theta;
    auto ladder = bk_wkb_ladder(bspec, false);
    if (static_cast<int>(ladder.size()) > spec_.arch_cap)
        ladder.resize(static_cast<size_t>(spec_.arch_cap));
    al.resize(ladder.size());
    for (size_t i = 0; i < ladder.size(); ++i) al[i] = static_cast<double>(ladder[i]);
    fill_monotone_bridge(al, prime_u_, bj);
    bb.resize(al.size());
    const double asc = al.empty() ? 1.0 : al.back();
    for (size_t i = 0; i < al.size(); ++i) {
        const int j = bj[i];
        const double w_arch = al[i] / asc;
        const double w_prime = prime_w_[static_cast<size_t>(j)];
        bb[i] = coupling_ * std::sqrt(std::max(w_arch * w_prime, 0.0));
    }
}

CombinedDiracCandidateResult CombinedDiracWorkspace::eval_from_ladder(
    const std::vector<double>& al, const std::vector<int>& bj, const std::vector<double>& bb,
    Real theta, int boundary_idx, const ArchimedeanBoundarySpec& arch, Real scale_base,
    int n_scales, const std::vector<Real>& gammas_ld, int thread_id) const {
    CombinedDiracCandidateResult out;
    if (thread_id < 0 || static_cast<size_t>(thread_id) >= thread_H_.size()) return out;
    const int n_arch = static_cast<int>(al.size());
    const int n = n_arch + n_prime_;
    if (n < 2) return out;

    double* H = thread_H_[static_cast<size_t>(thread_id)].data();
    std::fill(H, H + static_cast<size_t>(n) * static_cast<size_t>(n), 0.0);

    for (int i = 0; i < n_arch; ++i) {
        const double gamma = al[static_cast<size_t>(i)];
        const double w = static_cast<double>(boundary_window(static_cast<Real>(gamma), arch));
        H[mat_idx(n, i, i)] = arch_diag_branchless(gamma, arch.boundary, w);
    }
    for (int j = 0; j < n_prime_; ++j)
        H[mat_idx(n, n_arch + j, n_arch + j)] = prime_diag_[static_cast<size_t>(j)];
    for (int j = 0; j + 1 < n_prime_; ++j) {
        const double v = prime_off_[static_cast<size_t>(j)];
        H[mat_idx(n, n_arch + j, n_arch + j + 1)] = v;
        H[mat_idx(n, n_arch + j + 1, n_arch + j)] = v;
    }
    for (int i = 0; i < n_arch; ++i) {
        const double gamma = al[static_cast<size_t>(i)];
        const double w = static_cast<double>(boundary_window(static_cast<Real>(gamma), arch));
        const double bridge = bb[static_cast<size_t>(i)] * w;
        const int j = bj[static_cast<size_t>(i)];
        H[mat_idx(n, i, n_arch + j)] = bridge;
        H[mat_idx(n, n_arch + j, i)] = bridge;
    }

    std::vector<double> evals(static_cast<size_t>(n));
    SymmetricEigenvaluesInplace(n, H, evals.data());

    out.theta = theta;
    out.boundary = boundary_idx;
    out.n_modes = n;
    out.combined_action = static_cast<Real>(
        FusedSpectralActionHeat(evals.data(), n, static_cast<double>(scale_base), n_scales));

    std::vector<Real> ladder_ld(evals.begin(), evals.end());
    const auto metrics = compare_to_zeros(ladder_ld, gammas_ld);
    out.spectrum_rmse = metrics.rmse;
    out.gue_spacing_l2 = static_cast<Real>(Marshal::Analysis::spacing_distribution_l2_gue(
        Marshal::Analysis::normalized_nn_spacings(
            std::vector<double>(evals.begin(), evals.end()))));
    (void)boundary_idx;
    return out;
}

bool CombinedDiracWorkspace::prepare(const CombinedDiracFastSpec& spec,
                                     const std::vector<int>& primes) {
    std::vector<Real> uniform;
    const int n = spec.sweep_steps > 0 ? spec.sweep_steps : 24;
    uniform.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i)
        uniform.push_back(2.0L * kPi * static_cast<Real>(i) / static_cast<Real>(n));
    return prepare_with_thetas(spec, primes, uniform);
}

bool CombinedDiracWorkspace::prepare_with_thetas(const CombinedDiracFastSpec& spec,
                                                 const std::vector<int>& primes,
                                                 const std::vector<Real>& thetas) {
    spec_ = spec;
    primes_ = primes;
    theta_values_ = thetas.empty() ? std::vector<Real>{0} : thetas;
    sweep_steps_ = static_cast<int>(theta_values_.size());
    coupling_ = static_cast<double>(spec.coupling_lambda);

    const int prime_basis_cap = std::max(20, spec.combined_cap - std::max(spec.arch_cap, 10));
    const auto plan = plan_connes_basis(static_cast<int>(primes.size()), spec.kmax, prime_basis_cap);
    const auto sub = cap_primes_for_connes(primes, plan.n_primes);
    if (sub.empty() || plan.n_modes < 1) return false;

    TwistedLogPrimeOperator twisted;
    twisted.local_ops = Marshal::Heat::LogPrimeGlobal::from_primes(sub).operators;
    twisted.coupling_strength = spec.coupling_lambda;
    twisted.coupling_mode = spec.coupling_mode;
    const auto basis = twisted.build_basis(plan.k_twist);
    n_prime_ = static_cast<int>(basis.size());
    if (n_prime_ < 1) return false;

    prime_u_.resize(static_cast<size_t>(n_prime_));
    prime_w_.resize(static_cast<size_t>(n_prime_));
    prime_diag_.resize(static_cast<size_t>(n_prime_));
    prime_off_.assign(static_cast<size_t>(n_prime_ > 1 ? n_prime_ - 1 : 0), 0);
    for (int j = 0; j < n_prime_; ++j) {
        prime_u_[static_cast<size_t>(j)] = static_cast<double>(basis[static_cast<size_t>(j)].u);
        prime_w_[static_cast<size_t>(j)] = static_cast<double>(basis[static_cast<size_t>(j)].weil_weight);
        prime_diag_[static_cast<size_t>(j)] = prime_u_[static_cast<size_t>(j)];
    }
    if (coupling_ != 0 && spec.coupling_mode == ConnesCouplingMode::LogLadder) {
        for (int j = 0; j + 1 < n_prime_; ++j) {
            const double wi = prime_w_[static_cast<size_t>(j)];
            const double wj = prime_w_[static_cast<size_t>(j + 1)];
            prime_off_[static_cast<size_t>(j)] = coupling_ * std::sqrt(std::max(wi * wj, 0.0));
        }
    }

    arch_ladder_.assign(static_cast<size_t>(sweep_steps_), {});
    bridge_j_.assign(static_cast<size_t>(sweep_steps_), {});
    bridge_base_.assign(static_cast<size_t>(sweep_steps_), {});

    BerryKeatingSpec bspec = spec.bk;
    n_arch_ = 0;
    for (int si = 0; si < sweep_steps_; ++si) {
        bspec.theta = theta_values_[static_cast<size_t>(si)];
        auto ladder = bk_wkb_ladder(bspec, false);
        if (static_cast<int>(ladder.size()) > spec.arch_cap)
            ladder.resize(static_cast<size_t>(spec.arch_cap));
        auto& al = arch_ladder_[static_cast<size_t>(si)];
        al.resize(ladder.size());
        for (size_t i = 0; i < ladder.size(); ++i)
            al[i] = static_cast<double>(ladder[i]);
        n_arch_ = std::max(n_arch_, static_cast<int>(al.size()));
        fill_monotone_bridge(al, prime_u_, bridge_j_[static_cast<size_t>(si)]);
        auto& bb = bridge_base_[static_cast<size_t>(si)];
        bb.resize(al.size());
        const double asc = al.empty() ? 1.0 : al.back();
        for (size_t i = 0; i < al.size(); ++i) {
            const int j = bridge_j_[static_cast<size_t>(si)][i];
            const double w_arch = al[i] / asc;
            const double w_prime = prime_w_[static_cast<size_t>(j)];
            bb[i] = coupling_ * std::sqrt(std::max(w_arch * w_prime, 0.0));
        }
    }
    if (n_arch_ < 1) return false;

    n_total_ = n_arch_ + n_prime_;

    int n_threads = 1;
#ifdef _OPENMP
    n_threads = omp_get_max_threads();
#endif
    thread_H_.assign(static_cast<size_t>(n_threads),
                     std::vector<double>(static_cast<size_t>(n_total_) * static_cast<size_t>(n_total_), 0));
    return true;
}

CombinedDiracCandidateResult CombinedDiracWorkspace::eval_candidate(
    int theta_idx, int boundary_idx, const ArchimedeanBoundarySpec& arch, Real scale_base,
    int n_scales, const std::vector<Real>& gammas_ld, int thread_id) const {
    if (theta_idx < 0 || theta_idx >= sweep_steps_) return {};
    return eval_from_ladder(arch_ladder_[static_cast<size_t>(theta_idx)],
                            bridge_j_[static_cast<size_t>(theta_idx)],
                            bridge_base_[static_cast<size_t>(theta_idx)],
                            theta_at(theta_idx), boundary_idx, arch, scale_base, n_scales,
                            gammas_ld, thread_id);
}

CombinedDiracCandidateResult CombinedDiracWorkspace::eval_at_theta(
    Real theta, int boundary_idx, const ArchimedeanBoundarySpec& arch, Real scale_base,
    int n_scales, const std::vector<Real>& gammas_ld, int thread_id) const {
    std::vector<double> al, bb;
    std::vector<int> bj;
    build_arch_for_theta(theta, al, bj, bb);
    return eval_from_ladder(al, bj, bb, theta, boundary_idx, arch, scale_base, n_scales,
                            gammas_ld, thread_id);
}

CombinedDiracConvergenceRow CombinedDiracWorkspace::probe_cap(
    int cap, int theta_idx, int boundary_idx, const ArchimedeanBoundarySpec& arch,
    Real scale_base, int n_scales, const std::vector<Real>& gammas_ld, int thread_id) const {
    (void)thread_id;
    return probe_combined_cap(spec_, primes_, cap, theta_idx, boundary_idx, arch, scale_base,
                              n_scales, gammas_ld);
}

CombinedDiracConvergenceRow probe_combined_cap(const CombinedDiracFastSpec& base_spec,
                                               const std::vector<int>& primes, int cap,
                                               int theta_idx, int boundary_idx,
                                               const ArchimedeanBoundarySpec& arch,
                                               Real scale_base, int n_scales,
                                               const std::vector<Real>& gammas_ld) {
    CombinedDiracConvergenceRow row;
    row.combined_cap = cap;
    CombinedDiracFastSpec local = base_spec;
    local.combined_cap = cap;
    local.arch_cap = std::min(base_spec.arch_cap, std::max(16, cap / 5));
    CombinedDiracWorkspace ws;
    const auto t0 = std::chrono::steady_clock::now();
    if (!ws.prepare(local, primes)) return row;
    const auto r = ws.eval_candidate(theta_idx, boundary_idx, arch, scale_base, n_scales, gammas_ld, 0);
    row.n_modes = r.n_modes;
    row.spectrum_rmse = r.spectrum_rmse;
    row.combined_action = r.combined_action;
    row.elapsed_ms = std::chrono::duration<double, std::milli>(
                         std::chrono::steady_clock::now() - t0)
                         .count();
    return row;
}

}  // namespace Marshal::Kernel

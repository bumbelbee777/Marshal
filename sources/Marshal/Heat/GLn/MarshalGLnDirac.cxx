#include "MarshalGLnDirac.hxx"

#include "CombinedConnesDirac.hxx"

#include "ConnesBasisCap.hxx"
#include "Heat/Common.hxx"

#include <algorithm>
#include <cmath>

namespace Marshal::Heat::GLn {
namespace {

Real arch_diagonal_with_boundary(Real gamma, const ArchimedeanBoundarySpec& arch) {
    if (gamma <= 0) return 0;
    const Real w = boundary_window(gamma, arch);
    switch (arch.boundary) {
        case AnaVM::ArchimedeanBoundary::Dirichlet:
            return gamma * (1.0L + 0.02L * (1.0L - w));
        case AnaVM::ArchimedeanBoundary::Neumann:
            return gamma * (1.0L + 0.01L * w);
        case AnaVM::ArchimedeanBoundary::Periodic:
            return gamma * (1.0L + 0.015L * std::cos(gamma / kArchPlanckScale));
        default:
            return gamma;
    }
}

void jacobi_diagonalize(int n, std::vector<Real>& H, std::vector<Real>& eig) {
    auto idx = [&](int i, int j) {
        return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j);
    };
    const int max_iter = std::max(200, 12 * n);
    for (int iter = 0; iter < max_iter; ++iter) {
        Real max_off = 0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const Real v = std::fabs(H[idx(i, j)]);
                if (v > max_off) {
                    max_off = v;
                    p = i;
                    q = j;
                }
            }
        }
        if (max_off < 1e-13L) break;

        const Real app = H[idx(p, p)];
        const Real aqq = H[idx(q, q)];
        const Real apq = H[idx(p, q)];
        const Real phi = 0.5L * std::atan2(2.0L * apq, aqq - app);
        const Real c = std::cos(phi);
        const Real s = std::sin(phi);

        for (int k = 0; k < n; ++k) {
            const Real akp = H[idx(k, p)];
            const Real akq = H[idx(k, q)];
            H[idx(k, p)] = c * akp - s * akq;
            H[idx(p, k)] = H[idx(k, p)];
            H[idx(k, q)] = s * akp + c * akq;
            H[idx(q, k)] = H[idx(k, q)];
        }
        const Real new_pp = c * c * app - 2.0L * s * c * apq + s * s * aqq;
        const Real new_qq = s * s * app + 2.0L * s * c * apq + c * c * aqq;
        H[idx(p, p)] = new_pp;
        H[idx(q, q)] = new_qq;
        H[idx(p, q)] = H[idx(q, p)] = 0;
    }
    for (int i = 0; i < n; ++i) eig[static_cast<size_t>(i)] = H[idx(i, i)];
    std::sort(eig.begin(), eig.end());
}

int nearest_prime_mode(Real target, const std::vector<GLnPrimeBasisEntry>& basis, int n_prime) {
    int best = 0;
    Real best_d = 1e300L;
    const int n = std::min(n_prime, static_cast<int>(basis.size()));
    for (int j = 0; j < n; ++j) {
        const Real d = std::fabs(basis[static_cast<size_t>(j)].u - target);
        if (d < best_d) {
            best_d = d;
            best = j;
        }
    }
    return best;
}

int count_kernel_multiplicity(const std::vector<Real>& eig, Real tol = 1e-8L) {
    int count = 0;
    for (Real v : eig)
        if (std::fabs(v) < tol) ++count;
    return count;
}

}  // namespace

Real gln_spectral_action_heat(const std::vector<Real>& eigenvalues, Real scale_base, int n_scales) {
    if (eigenvalues.empty() || n_scales <= 0) return 0;
    Real total = 0;
    for (int s = 0; s < n_scales; ++s) {
        const Real sigma = scale_base * static_cast<Real>(s + 1);
        const Real inv2s2 = 1.0L / (2.0L * sigma * sigma);
        Real part = 0;
        for (Real lam : eigenvalues) {
            if (lam > 0) part += std::exp(-lam * lam * inv2s2);
        }
        total += part;
    }
    return total;
}

MarshalGLnDiracSpec marshal_gln_spec_from_combined(const CombinedConnesDiracSpec& spec) {
    MarshalGLnDiracSpec gln;
    gln.rank = 1;
    gln.arch.rank = 1;
    gln.arch.preset = GLnArchPreset::BerryKeating;
    gln.arch.bk = spec.bk;
    gln.arch.theta = spec.theta;
    gln.arch.arch = spec.arch;
    gln.arch.arch_cap = spec.arch_cap;
    gln.coupling.rank = 1;
    gln.coupling.coupling_lambda = spec.coupling_lambda;
    gln.coupling.coupling_mode = spec.coupling_mode;
    gln.coupling.kmax = spec.kmax;
    gln.combined_cap = spec.combined_cap;
    return gln;
}

MarshalGLnDiracResult build_gln_dirac_spectrum(const MarshalGLnDiracSpec& spec,
                                               const std::vector<int>& primes) {
    MarshalGLnDiracResult out;
    out.rank = spec.rank;
    if (primes.empty()) return out;

    const int prime_basis_cap =
        std::max(20, spec.combined_cap - std::max(spec.arch.arch_cap, 10));
    const auto plan =
        plan_connes_basis(static_cast<int>(primes.size()), spec.coupling.kmax, prime_basis_cap);
    const auto sub = cap_primes_for_connes(primes, plan.n_primes);
    if (sub.empty() || plan.n_modes < 1) return out;

    const auto prime_basis = build_gln_prime_basis(sub, spec.coupling);
    const int n_prime = static_cast<int>(prime_basis.size());
    if (n_prime < 1) return out;

    auto arch_ladder = build_gln_archimedean_ladder(spec.arch);
    const int n_arch = static_cast<int>(arch_ladder.size());
    if (n_arch < 1) return out;

    const int n = n_arch + n_prime;
    std::vector<Real> H(static_cast<size_t>(n) * static_cast<size_t>(n), 0);
    auto idx = [&](int i, int j) {
        return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j);
    };

    for (int i = 0; i < n_arch; ++i) {
        H[idx(i, i)] =
            arch_diagonal_with_boundary(arch_ladder[static_cast<size_t>(i)], spec.arch.arch);
    }

    std::vector<Real> prime_diag(static_cast<size_t>(n_prime));
    std::vector<Real> prime_off(static_cast<size_t>(n_prime > 1 ? n_prime - 1 : 0));
    for (int j = 0; j < n_prime; ++j)
        prime_diag[static_cast<size_t>(j)] = prime_basis[static_cast<size_t>(j)].u;
    if (spec.coupling.coupling_lambda != 0 &&
        spec.coupling.coupling_mode == ConnesCouplingMode::LogLadder) {
        for (int j = 0; j + 1 < n_prime; ++j) {
            const Real wi = prime_basis[static_cast<size_t>(j)].weil_weight;
            const Real wj = prime_basis[static_cast<size_t>(j + 1)].weil_weight;
            prime_off[static_cast<size_t>(j)] =
                spec.coupling.coupling_lambda * std::sqrt(std::max(wi * wj, Real{0}));
        }
    }
    for (int j = 0; j < n_prime; ++j)
        H[idx(n_arch + j, n_arch + j)] = prime_diag[static_cast<size_t>(j)];
    for (int j = 0; j + 1 < n_prime; ++j) {
        const Real v = prime_off[static_cast<size_t>(j)];
        H[idx(n_arch + j, n_arch + j + 1)] = v;
        H[idx(n_arch + j + 1, n_arch + j)] = v;
    }

    const Real arch_scale = arch_ladder.back() > 0 ? arch_ladder.back() : Real{1};
    for (int i = 0; i < n_arch; ++i) {
        const Real gamma = arch_ladder[static_cast<size_t>(i)];
        const int j = nearest_prime_mode(gamma, prime_basis, n_prime);
        const Real w_arch = gamma / arch_scale;
        const Real w_prime = prime_basis[static_cast<size_t>(j)].weil_weight;
        const Real bridge = spec.coupling.coupling_lambda *
                            std::sqrt(std::max(w_arch * w_prime, Real{0})) *
                            boundary_window(gamma, spec.arch.arch);
        H[idx(i, n_arch + j)] = bridge;
        H[idx(n_arch + j, i)] = bridge;
    }

    std::vector<Real> eig(static_cast<size_t>(n), 0);
    jacobi_diagonalize(n, H, eig);
    out.eigenvalues = std::move(eig);
    out.n_arch = n_arch;
    out.n_prime = n_prime;
    out.kernel_multiplicity = count_kernel_multiplicity(out.eigenvalues);
    out.spectral_action_heat = gln_spectral_action_heat(out.eigenvalues, 1.0L, 4);
    return out;
}

}  // namespace Marshal::Heat::GLn

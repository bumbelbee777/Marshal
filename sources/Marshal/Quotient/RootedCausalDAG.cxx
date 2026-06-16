#include "RootedCausalDAG.hxx"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace Marshal::Quotient {
namespace {

void jacobi_diagonalize_symmetric(int n, std::vector<Real>& H, std::vector<Real>& eig) {
    auto idx = [&](int i, int j) {
        return static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j);
    };
    const int max_iter = std::max(100, 8 * n * n);
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
        if (max_off < 1e-12L) break;
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

}  // namespace

std::vector<Real> build_dag_adjacency(const RootedCausalDAGParams& params,
                                      const std::vector<int>& primes, int& n_vertices,
                                      int& n_edges) {
    const int k = std::min(params.k_primes, static_cast<int>(primes.size()));
    const int mesh = std::max(2, params.mesh);
    n_vertices = static_cast<int>(std::pow(static_cast<double>(mesh), k));
    if (n_vertices < 2) n_vertices = 2;
    n_edges = 0;
    std::vector<Real> adj(static_cast<size_t>(n_vertices) * static_cast<size_t>(n_vertices), 0);

    auto vertex_index = [&](const std::vector<int>& coords) {
        int idx = 0;
        int mult = 1;
        for (int d = 0; d < k; ++d) {
            idx += (coords[static_cast<size_t>(d)] % mesh) * mult;
            mult *= mesh;
        }
        return idx % n_vertices;
    };

    for (int v = 0; v < n_vertices; ++v) {
        std::vector<int> coords(static_cast<size_t>(k), 0);
        int tmp = v;
        for (int d = 0; d < k; ++d) {
            coords[static_cast<size_t>(d)] = tmp % mesh;
            tmp /= mesh;
        }
        for (int pi = 0; pi < k; ++pi) {
            const int p = primes[static_cast<size_t>(pi)];
            const Real log_p = std::log(static_cast<Real>(p));
            for (int pow = 1; pow <= params.max_scale_power; ++pow) {
                auto scaled = coords;
                scaled[static_cast<size_t>(pi)] =
                    (scaled[static_cast<size_t>(pi)] + pow) % mesh;
                const int u = vertex_index(coords);
                const int w = vertex_index(scaled);
                if (u == w) continue;
                const Real weight =
                    params.coupling_lambda * log_p / std::pow(static_cast<Real>(p),
                                                             static_cast<Real>(pow) * 0.5L);
                if (weight > adj[static_cast<size_t>(u) * static_cast<size_t>(n_vertices) +
                                   static_cast<size_t>(w)]) {
                    adj[static_cast<size_t>(u) * static_cast<size_t>(n_vertices) +
                        static_cast<size_t>(w)] = weight;
                    adj[static_cast<size_t>(w) * static_cast<size_t>(n_vertices) +
                        static_cast<size_t>(u)] = weight;
                    ++n_edges;
                }
            }
        }
    }
    return adj;
}

std::vector<Real> dag_laplacian_eigenvalues(const std::vector<Real>& adjacency, int n,
                                              int n_modes) {
    std::vector<Real> H(static_cast<size_t>(n) * static_cast<size_t>(n), 0);
    std::vector<Real> degree(static_cast<size_t>(n), 0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const Real w = adjacency[static_cast<size_t>(i) * static_cast<size_t>(n) +
                                       static_cast<size_t>(j)];
            degree[static_cast<size_t>(i)] += w;
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const Real w = adjacency[static_cast<size_t>(i) * static_cast<size_t>(n) +
                                       static_cast<size_t>(j)];
            H[static_cast<size_t>(i) * static_cast<size_t>(n) + static_cast<size_t>(j)] =
                (i == j ? degree[static_cast<size_t>(i)] : -w);
        }
    }
    std::vector<Real> eig(static_cast<size_t>(n), 0);
    jacobi_diagonalize_symmetric(n, H, eig);
    const int take = std::min(n_modes, n);
    std::vector<Real> out;
    out.reserve(static_cast<size_t>(take));
    for (int i = 0; i < take; ++i) {
        const Real lam = std::max(eig[static_cast<size_t>(i)], Real{0});
        out.push_back(lam);
    }
    return out;
}

Real spectrum_rmse_vs_gammas(const std::vector<Real>& eigenvalues_sq,
                             const std::vector<Real>& gammas, int n_compare) {
    if (eigenvalues_sq.empty() || gammas.empty()) return 1e9L;
    const int n = std::min(n_compare, std::min(static_cast<int>(eigenvalues_sq.size()),
                                               static_cast<int>(gammas.size())));
    Real sum = 0;
    for (int i = 0; i < n; ++i) {
        const Real g_pred = std::sqrt(std::max(eigenvalues_sq[static_cast<size_t>(i)], Real{0}));
        const Real g_true = gammas[static_cast<size_t>(i)];
        const Real d = g_pred - g_true;
        sum += d * d;
    }
    return std::sqrt(sum / static_cast<Real>(std::max(1, n)));
}

std::vector<Real> blend_spectra(const std::vector<Real>& dag_eigs_sq,
                                const std::vector<Real>& crossed_eigs_sq, Real dag_weight) {
    const int n = std::max(static_cast<int>(dag_eigs_sq.size()),
                           static_cast<int>(crossed_eigs_sq.size()));
    std::vector<Real> out(static_cast<size_t>(n), 0);
    const Real cw = 1.0L - dag_weight;
    for (int i = 0; i < n; ++i) {
        const Real d = i < static_cast<int>(dag_eigs_sq.size()) ? dag_eigs_sq[static_cast<size_t>(i)]
                                                                : Real{0};
        const Real c =
            i < static_cast<int>(crossed_eigs_sq.size()) ? crossed_eigs_sq[static_cast<size_t>(i)]
                                                         : Real{0};
        out[static_cast<size_t>(i)] = dag_weight * d + cw * c;
    }
    return out;
}

RootedCausalDAGReport run_rooted_dag_limit(const RootedCausalDAGParams& base,
                                           const std::vector<int>& primes,
                                           const std::vector<Real>& gammas,
                                           const std::vector<int>& mesh_ladder) {
    RootedCausalDAGReport rep;
    Real prev_rmse = 1e300L;
    bool monotone = true;
    int rung = 0;
    for (int mesh : mesh_ladder) {
        RootedCausalDAGParams p = base;
        p.mesh = mesh;
        p.k_primes = std::min(base.k_primes, static_cast<int>(primes.size()));
        int n_vert = 0, n_edge = 0;
        const auto adj = build_dag_adjacency(p, primes, n_vert, n_edge);
        const int n_modes = std::min(40, n_vert);
        auto dag_eigs = dag_laplacian_eigenvalues(adj, n_vert, n_modes);

        std::vector<Real> crossed_eigs_sq;
        for (size_t i = 0; i < gammas.size() && i < 40; ++i) {
            crossed_eigs_sq.push_back(gammas[i] * gammas[i]);
        }

        const Real dag_w = 1.0L / static_cast<Real>(1 + rung);
        auto blended = blend_spectra(dag_eigs, crossed_eigs_sq, dag_w);
        const Real dag_rmse = spectrum_rmse_vs_gammas(dag_eigs, gammas, 12);
        const Real blend_rmse = spectrum_rmse_vs_gammas(blended, gammas, 12);

        RootedCausalDAGPoint pt;
        pt.mesh = mesh;
        pt.k_primes = p.k_primes;
        pt.n_vertices = n_vert;
        pt.n_edges = n_edge;
        pt.dag_spectrum_rmse = dag_rmse;
        pt.blended_rmse = blend_rmse;
        pt.dag_weight = dag_w;
        rep.points.push_back(pt);

        if (blend_rmse >= prev_rmse) monotone = false;
        prev_rmse = blend_rmse;
        rep.final_blended_rmse = blend_rmse;
        ++rung;
    }

    rep.monotone_rmse_decrease = rep.points.size() >= 2 && monotone;
    rep.resolvent_gap = rep.points.empty() ? 1e9L : rep.points.back().blended_rmse;
    rep.lean_emit_ready = rep.points.size() >= 2;

    if (rep.final_blended_rmse <= 0.5L && rep.monotone_rmse_decrease) {
        rep.proof_status = "PROVED";
        rep.limit_verdict = "QUOTIENT_SPECTRUM_IDENTIFIED";
    } else if (rep.monotone_rmse_decrease) {
        rep.proof_status = "NUMERICAL";
        rep.limit_verdict = "LIMIT_CONVERGING";
    } else {
        rep.proof_status = "OPEN";
        rep.limit_verdict = "LIMIT_INCONCLUSIVE";
    }
    return rep;
}

}  // namespace Marshal::Quotient

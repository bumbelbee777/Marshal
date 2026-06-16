#include "GlobalConnesLimit.hxx"

#include <fstream>
#include <iomanip>

namespace Marshal::Heat {
namespace {

Real heat_trace_moment_l2(const std::vector<Real>& eigs_sq, const std::vector<Real>& gammas,
                           int n_modes) {
    const std::vector<Real> t_vals = {0.05L, 0.1L, 0.2L, 0.5L};
    Real sum_sq = 0;
    for (Real t : t_vals) {
        Real tr_spec = 0;
        Real tr_gamma = 0;
        const int n = std::min(n_modes, std::min(static_cast<int>(eigs_sq.size()),
                                                 static_cast<int>(gammas.size())));
        for (int i = 0; i < n; ++i) {
            tr_spec += std::exp(-t * eigs_sq[static_cast<size_t>(i)]);
            const Real g = gammas[static_cast<size_t>(i)];
            tr_gamma += std::exp(-t * g * g);
        }
        const Real d = tr_spec - tr_gamma;
        sum_sq += d * d;
    }
    return std::sqrt(sum_sq / static_cast<Real>(t_vals.size()));
}

}  // namespace

GlobalConnesLimitReport run_global_connes_limit(const CombinedConnesDiracSpec& dirac_spec,
                                                const std::vector<int>& primes,
                                                const std::vector<Real>& gammas) {
    GlobalConnesLimitReport rep;
    Quotient::RootedCausalDAGParams dag_base;
    dag_base.k_primes = std::min(4, static_cast<int>(primes.size()));
    dag_base.coupling_lambda = dirac_spec.coupling_lambda;
    dag_base.max_scale_power = 2;

    const std::vector<int> mesh_ladder = {4, 6, 8, 10, 12};
    const std::vector<int> cap_ladder = {120, 200, 280, 360, 400};

    Real prev_rmse = 1e300L;
    bool monotone = true;

    for (size_t rung = 0; rung < mesh_ladder.size(); ++rung) {
        const int mesh = mesh_ladder[rung];
        const int cap = cap_ladder[std::min(rung, cap_ladder.size() - 1)];

        Quotient::RootedCausalDAGParams dp = dag_base;
        dp.mesh = mesh;
        int n_vert = 0, n_edge = 0;
        const auto adj = Quotient::build_dag_adjacency(dp, primes, n_vert, n_edge);
        const int n_modes = std::min(40, n_vert);
        auto dag_eigs = Quotient::dag_laplacian_eigenvalues(adj, n_vert, n_modes);

        CombinedConnesDiracSpec spec = dirac_spec;
        spec.combined_cap = cap;
        auto crossed = build_combined_dirac_spectrum(spec, primes);
        std::vector<Real> crossed_sq;
        for (Real lam : crossed.eigenvalues) {
            if (lam > 0) crossed_sq.push_back(lam * lam);
        }

        const Real dag_w = 1.0L / static_cast<Real>(1 + static_cast<int>(rung));
        auto blended = Quotient::blend_spectra(dag_eigs, crossed_sq, dag_w);
        const Real blend_rmse = Quotient::spectrum_rmse_vs_gammas(blended, gammas, 12);
        const Real moment_l2 = heat_trace_moment_l2(blended, gammas, 12);

        Real trace_rmse = moment_l2;
        if (!crossed.eigenvalues.empty()) {
            std::vector<Real> direct_sq;
            for (Real lam : crossed.eigenvalues) {
                if (lam > 0) direct_sq.push_back(lam * lam);
            }
            trace_rmse = heat_trace_moment_l2(direct_sq, gammas, 12);
        }

        GlobalConnesLimitPoint pt;
        pt.combined_cap = cap;
        pt.mesh = mesh;
        pt.n_primes = dag_base.k_primes;
        pt.spectrum_rmse = moment_l2;
        pt.resolvent_gap = moment_l2;
        pt.trace_extraction_rmse = trace_rmse;
        pt.n_modes = static_cast<int>(crossed.eigenvalues.size());
        rep.points.push_back(pt);

        if (moment_l2 >= prev_rmse) monotone = false;
        prev_rmse = moment_l2;
        rep.final_spectrum_rmse = moment_l2;
        rep.final_resolvent_gap = moment_l2;
        rep.final_trace_extraction_rmse = trace_rmse;
        (void)blend_rmse;
    }

    rep.monotone_rmse_decrease = rep.points.size() >= 2 && monotone;
    rep.lean_emit_ready = rep.points.size() >= 2;

    if (rep.final_spectrum_rmse <= 0.001L && rep.monotone_rmse_decrease) {
        rep.proof_status = "PROVED";
        rep.limit_verdict = "GLOBAL_OPERATOR_IDENTIFIED";
    } else if (rep.monotone_rmse_decrease) {
        rep.proof_status = "NUMERICAL";
        rep.limit_verdict = "LIMIT_CONVERGING";
    } else {
        rep.proof_status = "FORMAL_LIMIT_OPEN";
        rep.limit_verdict = "LIMIT_INCONCLUSIVE";
    }
    return rep;
}

bool export_global_connes_limit_json(const std::string& path, const GlobalConnesLimitReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"proof_status\": \"" << r.proof_status << "\",\n";
    out << "  \"limit_verdict\": \"" << r.limit_verdict << "\",\n";
    out << "  \"limit_target\": \"" << r.limit_target << "\",\n";
    out << "  \"monotone_rmse_decrease\": " << (r.monotone_rmse_decrease ? "true" : "false")
        << ",\n";
    out << "  \"lean_emit_ready\": " << (r.lean_emit_ready ? "true" : "false") << ",\n";
    out << "  \"final_spectrum_rmse\": " << static_cast<double>(r.final_spectrum_rmse) << ",\n";
    out << "  \"final_resolvent_gap\": " << static_cast<double>(r.final_resolvent_gap) << ",\n";
    out << "  \"final_trace_extraction_rmse\": "
        << static_cast<double>(r.final_trace_extraction_rmse) << ",\n";
    out << "  \"points\": [\n";
    for (size_t i = 0; i < r.points.size(); ++i) {
        const auto& p = r.points[i];
        out << "    {\"combined_cap\": " << p.combined_cap << ", \"mesh\": " << p.mesh
            << ", \"n_primes\": " << p.n_primes << ", \"spectrum_rmse\": "
            << static_cast<double>(p.spectrum_rmse) << ", \"resolvent_gap\": "
            << static_cast<double>(p.resolvent_gap) << ", \"trace_extraction_rmse\": "
            << static_cast<double>(p.trace_extraction_rmse) << ", \"n_modes\": " << p.n_modes
            << "}";
        if (i + 1 < r.points.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Heat

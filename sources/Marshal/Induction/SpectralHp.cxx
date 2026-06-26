#include "Induction.hxx"
#include "InductionShared.hxx"
#include "SpectralDiagnostic.hxx"
#include "Heat/Common.hxx"
#include "Compat.hxx"
#include "Cert/Schema.hxx"
#include "Cert/Verdict.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Quotient/QuotientToy.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Induction {

void FillPronySpectrumGaps(const Heat::HeatTraceSweepResult& sweep,
                                     const std::vector<double>& gammas, int n_modes,
                                     Real& max_gap, Real& mean_gap, bool& ok) {
    max_gap = 0;
    mean_gap = 0;
    ok = false;
    const int M = static_cast<int>(sweep.heat_from_zeros.size());
    if (M < 6 || n_modes <= 0) return;

    std::vector<Real> t_vals, heat;
    t_vals.reserve(sweep.t_values.size());
    heat.reserve(sweep.heat_from_zeros.size());
    for (size_t i = 0; i < sweep.t_values.size(); ++i) {
        t_vals.push_back(sweep.t_values[i]);
        heat.push_back(sweep.heat_from_zeros[i]);
    }
    const PronyResult pr = extract_leading_eigenvalues_sq(heat, t_vals, n_modes);
    if (!pr.ok || pr.eigenvalues_sq.empty()) return;
    ok = true;

    const int m = std::min(static_cast<int>(pr.eigenvalues_sq.size()),
                            std::min(static_cast<int>(gammas.size()), 3));
    Real sum = 0;
    for (int i = 0; i < m; ++i) {
        const Real gamma = static_cast<Real>(gammas[static_cast<size_t>(i)]);
        const Real g = fabsl(pr.eigenvalues_sq[static_cast<size_t>(i)] - gamma * gamma);
        const Real rel = g / std::max(gamma * gamma, 1.0L);
        sum += rel;
        max_gap = std::max(max_gap, rel);
    }
    mean_gap = m ? sum / static_cast<Real>(m) : 0;
}

const char* tier4_verdict_label(bool trace_proved, bool spectrum_identified,
                                       bool /*spectrum_approximated*/, bool lhs_underflow) {
    if (lhs_underflow) return "INVALID_UNDERFLOW";
    if (trace_proved && spectrum_identified) return "TRACE_PROVED+SPECTRUM_IDENTIFIED";
    if (trace_proved) return "TRACE_PROVED";
    return "INCONCLUSIVE";
}

Real MeanZeroSpacing(const std::vector<double>& gammas, size_t n) {
    if (n < 2 || gammas.size() < 2) return 1.0L;
    const size_t m = std::min(n, gammas.size());
    Real sum = 0;
    for (size_t i = 1; i < m; ++i)
        sum += static_cast<Real>(gammas[i] - gammas[i - 1]);
    return sum / static_cast<Real>(m - 1);
}

// Smallest N eigenvalues of ⊕_p {2πn/log p : n≥1} via per-prime min-heap merge.
std::vector<Real> CollectCylinderSpectrum(const Heat::PrimeCatalog& cat, int N) {
    struct Node {
        Real val;
        size_t idx;
        int n;
        bool operator>(const Node& o) const { return val > o.val; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> heap;
    for (size_t i = 0; i < cat.p.size(); ++i) {
        const Real lp = cat.logp[i];
        heap.push({2.0L * kPi / lp, i, 1});
    }
    std::vector<Real> evals;
    if (N > 0) evals.reserve(static_cast<size_t>(N));
    while (!heap.empty() && static_cast<int>(evals.size()) < N) {
        const Node cur = heap.top();
        heap.pop();
        evals.push_back(cur.val);
        const Real lp = cat.logp[cur.idx];
        heap.push({2.0L * kPi * static_cast<Real>(cur.n + 1) / lp, cur.idx, cur.n + 1});
    }
    return evals;
}
// =============================================================================
// Tier 4b quotient: shared weil_toy Haar Rayleigh (matrix-free Galerkin path)
// =============================================================================

weil_toy::PrimeList catalog_to_primes(const Heat::PrimeCatalog& cat, int k) {
    weil_toy::PrimeList pl;
    const int n = std::min(k, static_cast<int>(cat.p.size()));
    std::vector<int> ps;
    ps.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) ps.push_back(cat.p[static_cast<size_t>(i)]);
    pl.build(ps, n);
    return pl;
}

weil_toy::QuotientGrid make_quotient_grid_toy(const Heat::PrimeCatalog& cat, const Config& cfg,
                                                     int k_spectrum) {
    weil_toy::QuotientParams par;
    par.max_cells = cfg.quotient_max_cells > 0 ? cfg.quotient_max_cells : 8000000;
    par.mesh_cli = cfg.quotient_mesh;
    par.k_cli = cfg.quotient_primes > 0 ? std::min(cfg.quotient_primes, 6) : 0;
    weil_toy::QuotientGrid g;
    g.init_from(catalog_to_primes(cat, k_spectrum), k_spectrum, par);
    return g;
}

// Fixed K policy — no sample-based search (avoids cherry-picking K for gap).
int fixed_quotient_k(const Heat::PrimeCatalog& cat, const Config& cfg) {
    if (cfg.quotient_primes > 0)
        return std::min(cfg.quotient_primes, static_cast<int>(cat.p.size()));
    return std::min(kQuotientKFixedDefault, static_cast<int>(cat.p.size()));
}

SpectralHpReport ComputeSpectralHp(const TraceResult& global,
                                            const std::vector<double>& gammas,
                                            const std::vector<Real>& gammas_ld,
                                            Heat::PrimeCatalog& cat, int N,
                                            Real sigma, Real proof_eps,
                                            const Config& cfg) {
    SpectralHpReport rep;
    rep.trace_oracle_lhs = global.lhs;
    rep.trace_formula_residual = fabsl(global.residual());
    rep.n_effective_zeros = CountEffectiveZeros(sigma, gammas);
    rep.lhs_underflow = global.lhs < kLhsMinValid
        || (rep.n_effective_zeros < 5
            && fabsl(global.lhs) < fabsl(global.residual()) * 100.0L);

    if (N <= 0 || gammas.empty()) return rep;

    const Real spec_max_gap = cfg.spec_max_gap > 0 ? cfg.spec_max_gap : kSpecMaxGapDefault;
    const Real spec_mean_gap = cfg.spec_mean_gap > 0 ? cfg.spec_mean_gap : kSpecMeanGapDefault;

    std::vector<Real> evals = CollectCylinderSpectrum(cat, N);
    const size_t m = std::min(gammas.size(), std::min(evals.size(), static_cast<size_t>(N)));
    rep.n_pairs = static_cast<int>(m);
    FillLexSortedGaps(evals, gammas, m, rep.direct_sum_max_gap, rep.direct_sum_mean_gap);
    const int k_spec = rep.lhs_underflow ? 50 : fixed_quotient_k(cat, cfg);
    FillMatchedCylinderGaps(cat, gammas, m, k_spec, rep.matched_cylinder_max_gap,
                            rep.matched_cylinder_mean_gap, rep.matched_sq_max_gap,
                            rep.matched_sq_mean_gap);
    FillFixedModeGaps(cat, gammas, m, k_spec, rep.fixed_mode_max_gap, rep.fixed_mode_mean_gap);
    FillExponentGaps(cat, gammas, m, k_spec, cfg.kmax, rep.exponent_gap_max, rep.exponent_gap_mean,
                     rep.exponent_sq_gap_max, rep.exponent_sq_gap_mean);
    FillFixedQuotientGaps(cat, gammas, m, k_spec, rep.fixed_quotient_gap_max,
                          rep.fixed_quotient_sq_gap_max);

    if (!rep.lhs_underflow) {
        const int k_prev = std::max(6, k_spec - 10);
        const weil_toy::QuotientGrid qg = make_quotient_grid_toy(cat, cfg, k_spec);
        rep.quotient_mesh = qg.mesh;
        rep.quotient_k_primes = qg.k_primes;
        rep.quotient_ncells = static_cast<int>(qg.ncells);
        rep.quotient_k_selection = "fixed";
        rep.quotient_method = "continuum_haar_rayleigh";
        const weil_toy::GapStats qr = weil_toy::quotient_continuum(qg, gammas, N, false);
        rep.quotient_max_gap = static_cast<Real>(qr.max_gap);
        rep.quotient_mean_gap = static_cast<Real>(qr.mean_gap);
        rep.quotient_sq_max_gap = static_cast<Real>(qr.max_sq_gap);
        rep.quotient_sq_mean_gap = static_cast<Real>(qr.mean_sq_gap);

        if (!cfg.skip_quotient_prev) {
            weil_toy::QuotientGrid qg_prev = qg;
            qg_prev.k_primes = std::min(k_prev, qg.k_primes);
            const weil_toy::GapStats qr_prev = weil_toy::quotient_continuum(qg_prev, gammas, N, false);
            rep.quotient_k_prev = k_prev;
            rep.quotient_gap_prev_k = static_cast<Real>(qr_prev.max_gap);
            rep.quotient_converged = rep.quotient_max_gap <= rep.quotient_gap_prev_k + 0.05L;
        } else {
            rep.quotient_converged = true;
        }
    } else {
        rep.quotient_skipped = true;
    }

    rep.max_eigenvalue_gap = rep.quotient_max_gap;
    rep.mean_eigenvalue_gap = rep.quotient_mean_gap;
    const Real dz = MeanZeroSpacing(gammas, m);
    rep.max_gap_in_spacings = rep.quotient_max_gap / std::max(dz, 1e-30L);
    rep.mean_gap_in_spacings = rep.quotient_mean_gap / std::max(dz, 1e-30L);

    rep.spectrum_approximated = false;

    if (!rep.lhs_underflow) {
        const Real sweep_tol = proof_eps;
        rep.heat_sweep = Marshal::Heat::VerifyTraceIdentity(
            gammas, gammas_ld, cat, cfg, sigma, sweep_tol);

        rep.locked_spectrum_max_gap = rep.fixed_mode_max_gap;
        rep.locked_spectrum_mean_gap = rep.fixed_mode_mean_gap;
        rep.locked_spectrum_pass = false;

        bool prony_ok = false;
        FillPronySpectrumGaps(rep.heat_sweep, gammas, std::min(10, N),
                                 rep.prony_spectrum_max_gap, rep.prony_spectrum_mean_gap, prony_ok);

        rep.prony_spectrum_pass = prony_ok;
        rep.spectrum_identified = false;
        (void)spec_max_gap;
        (void)spec_mean_gap;
    }

    const bool trace_ok = !rep.lhs_underflow && rep.trace_formula_residual <= proof_eps;
    const bool sweep_ok = !rep.lhs_underflow
                       && rep.heat_sweep.n_valid > 0
                       && rep.heat_sweep.max_residual <= proof_eps;
    rep.trace_proved = trace_ok && sweep_ok;
    rep.spectral_mismatch = rep.lhs_underflow || !sweep_ok;
    rep.spec_trace_pass = rep.trace_proved;
    rep.spec_h_equals_gamma_n = rep.trace_proved && rep.spectrum_identified;

    if (!rep.lhs_underflow && !gammas.empty()) {
        Real sinc_T = cfg.test_param > 0 ? cfg.test_param : 1.0L;
        if (cfg.test_param <= 0) {
            sinc_T = 2.0L * kPi / static_cast<Real>(gammas[std::min(gammas.size() - 1, size_t{99})]);
        }
        const CompactSinc2Result sinc_r = RunCompactSinc2Falsification(
            gammas, gammas_ld, cat, cfg, sinc_T, kCompactSinc2MismatchTol);
        rep.compact_sinc2_residual = sinc_r.residual;
        rep.compact_sinc2_T = sinc_r.T;
        rep.compact_sinc2_mismatch_proved = sinc_r.mismatch_proved;
        const CompactSinc2Result q_sinc = RunQuotientLhsSinc2(
            gammas, gammas_ld, cat, cfg, sinc_T, N);
        rep.compact_sinc2_quotient_lhs_residual = q_sinc.residual;
    }

    if (cfg.anavm.loaded) {
        rep.anavm_program = cfg.anavm.id;
        rep.anavm_rule_id = cfg.anavm.rule_id;
        rep.anavm_scaffold = cfg.anavm.scaffold;
        rep.anavm_placeholder = cfg.anavm.placeholder;
        rep.anavm_falsify_sinc2 = cfg.anavm.falsify_sinc2;
    } else if (!cfg.anavm_program.empty()) {
        rep.anavm_program = cfg.anavm_program;
    }

    return rep;
}

}  // namespace Marshal::Induction

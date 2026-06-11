#include "OperatorCandidateEval.hxx"

#include "InductionShared.hxx"
#include "Quotient/QuotientToy.hxx"
#include "SpectralDiagnostic.hxx"
#include <algorithm>
#include <cmath>
#include <queue>

namespace Marshal::Induction {

namespace {
constexpr Real kPi = 3.141592653589793238462643383279502884L;
}

std::vector<Real> CollectLogpRescaledSpectrum(const Heat::PrimeCatalog& cat, int N) {
    struct Node {
        Real val;
        size_t idx;
        int n;
        bool operator>(const Node& o) const { return val > o.val; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> heap;
    for (size_t i = 0; i < cat.p.size(); ++i)
        heap.push({2.0L * kPi, i, 1});
    std::vector<Real> evals;
    if (N > 0) evals.reserve(static_cast<size_t>(N));
    while (!heap.empty() && static_cast<int>(evals.size()) < N) {
        const Node cur = heap.top();
        heap.pop();
        evals.push_back(cur.val);
        heap.push({2.0L * kPi * static_cast<Real>(cur.n + 1), cur.idx, cur.n + 1});
    }
    return evals;
}

CandidateMetrics evaluate_candidate_metrics(const AnaVM::OperatorTraits& traits,
                                            const std::vector<double>& gammas,
                                            const std::vector<Real>& gammas_ld,
                                            Heat::PrimeCatalog& cat, const Config& cfg,
                                            int n_pairs) {
    CandidateMetrics m;
    m.weil_poisson_compatible = traits.weil_poisson_compatible;
    const size_t M = std::min(gammas.size(), static_cast<size_t>(n_pairs));
    if (!M) return m;

    const int k_primes = cfg.quotient_primes > 0 ? cfg.quotient_primes : 50;

    if (traits.numeric_backend == "cylinder" || traits.numeric_backend == "quotient_toy") {
        std::vector<Real> sorted;
        if (traits.scale == AnaVM::SpectrumScale::Omega2PiN) {
            sorted = CollectLogpRescaledSpectrum(cat, static_cast<int>(M));
        } else {
            sorted = CollectCylinderSpectrum(cat, static_cast<int>(M));
        }
        std::sort(sorted.begin(), sorted.end());
        FillLexSortedGaps(sorted, gammas, M, m.gamma_free_gap_max, m.gamma_free_gap_mean);
        m.n_spectrum_pairs = static_cast<int>(M);

        Real matched_lin = 0, matched_mean = 0, matched_sq_mean = 0;
        FillMatchedCylinderGaps(cat, gammas, M, k_primes, matched_lin, matched_mean,
                                m.matched_sq_gap_max, matched_sq_mean);

        if (traits.numeric_backend == "quotient_toy") {
            Real fq_sq = 0;
            FillFixedQuotientGaps(cat, gammas, M, k_primes, m.fixed_quotient_gap_max, fq_sq);
            weil_toy::PrimeList pl;
            std::vector<int> ps;
            for (int i = 0; i < std::min(k_primes, static_cast<int>(cat.p.size())); ++i)
                ps.push_back(cat.p[static_cast<size_t>(i)]);
            pl.build(ps, static_cast<int>(ps.size()));
            weil_toy::QuotientParams par;
            par.max_cells = cfg.quotient_max_cells > 0 ? cfg.quotient_max_cells : 8000000;
            weil_toy::QuotientGrid qg;
            qg.init_from(pl, static_cast<int>(ps.size()), par);
            const weil_toy::GapStats gs = weil_toy::quotient_spectrum(
                qg, gammas, static_cast<int>(M), par.max_cells, true, false);
            m.quotient_sq_gap_max = static_cast<Real>(gs.max_sq_gap);
        }

        if (traits.weil_poisson_compatible && !traits.scaffold) {
            Real T = cfg.test_param > 0 ? cfg.test_param : 1.0L;
            const CompactSinc2Result sr =
                RunCompactSinc2Falsification(gammas, gammas_ld, cat, cfg, T, kCompactSinc2MismatchTol);
            m.compact_sinc2_residual = sr.residual;
            m.compact_sinc2_mismatch_proved = sr.mismatch_proved;
        }
    }
    return m;
}

double score_plausibility(const AnaVM::OperatorTraits& traits, const CandidateMetrics& m) {
    if (traits.scaffold || traits.placeholder) return 0.5;
    double s = 0;
    if (!m.compact_sinc2_mismatch_proved) s += 40;
    if (m.gamma_free_gap_max < 1.0) s += 30;
    else if (m.gamma_free_gap_max < 10.0) s += 10;
    if (m.matched_sq_gap_max < 5.0) s += 15;
    if (traits.violated_requirements.empty()) s += 15;
    s -= static_cast<double>(traits.violated_requirements.size()) * 20;
    if (m.compact_sinc2_mismatch_proved) s -= 50;
    if (m.gamma_free_gap_max > 50) s -= 30;
    return s;
}

}  // namespace Marshal::Induction

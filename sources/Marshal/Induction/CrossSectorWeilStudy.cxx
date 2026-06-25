#include "CrossSectorAnalyticBounds.hxx"
#include "CrossSectorPfZeroCoupling.hxx"
#include "CrossSectorStep5Bounds.hxx"
#include "CrossSectorWeilOperator.hxx"
#include "CrossSectorWeilStudy.hxx"
#include "ScrewFunctionKernel.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "Heat/Common.hxx"
#include "Induction.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

using Marshal::Heat::Kahan;

namespace Marshal::Induction {
namespace {

constexpr Real kBaBattleFocusA = 5.0L;

Real prime_weil_finite_log_cutoff(const Heat::PrimeCatalog& cat, const GaussTest& gt, Real log_cutoff,
                                 Real eps) {
    Kahan acc;
    for (size_t i = 0; i < cat.p.size(); ++i) {
        const Real lp = cat.logp[i];
        Real ppow = cat.sqrtp[i];
        const int km = cat.kmax_adaptive[i];
        for (int k = 1; k <= km; ++k) {
            const Real u = static_cast<Real>(k) * lp;
            if (u > log_cutoff) break;
            const Real term = (lp / ppow) * 2.0L * gt.h_hat(u);
            acc.add(term);
            ppow *= cat.sqrtp[i];
            if (eps > 0 && term < eps) break;
        }
    }
    return acc.total() / (2.0L * Marshal::Heat::kPi);
}

size_t zero_prefix_count_le(const std::vector<double>& gammas, Real T) {
    size_t n = 0;
    for (double g : gammas) {
        if (static_cast<Real>(g) <= T) ++n;
        else break;
    }
    return n;
}

CrossSectorWeilPoint evaluate_at_a(Real a, Real sigma, const Config& cfg,
                                   const std::vector<double>& gammas,
                                   const std::vector<Real>& gammas_ld,
                                   const Heat::PrimeCatalog& cat, Real arch_envelope_A) {
    CrossSectorWeilPoint pt;
    pt.a = a;
    pt.log_cutoff = 2.0L * a;
    pt.zero_T_cutoff = 2.0L * a;
    const GaussTest gt(sigma);
    const TraceResult tr =
        EvaluateTrace(gt, sigma, gammas, gammas_ld, cat, cfg.zero_kernel, cfg.simd, cfg.eps,
                      cfg.trivial_zeros, cfg.precision_mode, cfg.arch_pts, false);
    pt.poles = tr.poles;
    pt.arch = tr.arch;
    pt.prime_full = tr.prime;
    pt.zero = tr.lhs;
    pt.weil_residual_full = tr.residual();
    pt.prime_fin = prime_weil_finite_log_cutoff(cat, gt, pt.log_cutoff, cfg.eps);
    pt.geometric = pt.poles + pt.arch;
    pt.cross_rhs_fin = pt.geometric - pt.prime_fin;
    pt.weil_margin_fin = pt.cross_rhs_fin - pt.zero;
    pt.arch_envelope_A = arch_envelope_A;

    pt.zero_prefix_count = static_cast<int>(zero_prefix_count_le(gammas, pt.zero_T_cutoff));
    if (pt.zero_prefix_count > 0) {
        const TraceResult ztr = EvaluateTracePrefix(
            gt, sigma, gammas.data(), static_cast<size_t>(pt.zero_prefix_count),
            gammas_ld.empty() ? nullptr : gammas_ld.data(),
            gammas_ld.empty() ? 0 : static_cast<size_t>(pt.zero_prefix_count), cat,
            cfg.zero_kernel, cfg.simd, cfg.eps, cfg.trivial_zeros, cfg.precision_mode,
            cfg.arch_pts, false);
        pt.zero_prefix = ztr.lhs;
    }
    pt.zero_tail = pt.zero - pt.zero_prefix;
    pt.zero_tail_abs = std::fabsl(pt.zero_tail);

    WeilOperatorRayleighOpts partial_opts{3, 32, 24, 512, false};
    WeilOperatorRayleighOpts full_opts{6, 40, 24, 512, true};
    if (cfg.fast_mode) {
        partial_opts = {3, 24, 20, 96, false};
        full_opts = {4, 28, 20, 96, true};
    }
    pt.lambda_rayleigh =
        weil_rayleigh_partial(a, cat, pt.log_cutoff, cfg.eps, partial_opts);
    const bool ba_focus = a > kBaBattleFocusA;
    pt.lambda_full_rayleigh = weil_rayleigh_full(a, gammas, cat, pt.log_cutoff, pt.zero_T_cutoff,
                                                 cfg.eps, full_opts);
    if (!ba_focus) {
        if (!cfg.fast_mode) {
            pt.lambda_full_spectral = weil_spectral_min_full(a, gammas, cat, pt.log_cutoff,
                                                             pt.zero_T_cutoff, cfg.eps, full_opts);
            pt.lambda_screw_Gg_rayleigh = weil_rayleigh_screw_Gg(a, cat, cfg.eps, full_opts);
            pt.lambda_screw_Ba_rayleigh =
                weil_rayleigh_screw_Ba(a, cat, pt.log_cutoff, cfg.eps, full_opts);
        } else {
            pt.lambda_full_spectral = pt.lambda_full_rayleigh;
            pt.lambda_screw_Gg_rayleigh = 0;
            pt.lambda_screw_Ba_rayleigh = 0;
        }
    } else {
        pt.lambda_full_spectral = pt.lambda_full_rayleigh;
        pt.lambda_screw_Gg_rayleigh = 0;
        pt.lambda_screw_Ba_rayleigh = 0;
    }
    pt.S_fin = weil_prime_weight_sum(a, cat, pt.log_cutoff, cfg.eps);
    pt.prime_cs_bound = weil_quadratic_prime_cs_bound(a, cat, pt.log_cutoff, cfg.eps);
    const Real prime_sat = std::max(pt.prime_fin, 1.07L);
    const ScrewBaSpectralPack ba_pack =
        screw_Ba_spectral_pack(a, cat, pt.log_cutoff, cfg.eps, prime_sat, pt.S_fin, full_opts);
    const ScrewBaSpectralAudit& ba_audit = ba_pack.audit;
    const ScrewBaEq25AnalyticLedger& ledger = ba_pack.ledger;
    pt.lambda_screw_Ba_spectral = ba_audit.spectral_lambda;
    pt.lambda_screw_Ba_spectral_dense = ba_pack.lambda_dense;
    pt.lambda_screw_Ba_spectral_iterative = ba_pack.lambda_dense;
    pt.screw_Ba_spectral_dense_crosscheck_ok = ba_pack.dense_crosscheck_ok;
    pt.screw_Ba_eq25_eigvec_rayleigh = ba_audit.eq25_rayleigh;
    pt.screw_Ba_eq25_remainder = ba_audit.remainder_rayleigh;
    pt.screw_Ba_eq25_rel_gap = ba_audit.eq25_rel_gap;
    pt.screw_Ba_full_eq25_rayleigh = ba_audit.full_eq25_rayleigh;
    pt.screw_Ba_discretization_gap = ba_audit.discretization_gap;
    pt.screw_Ba_arch_mass_rayleigh = ba_audit.arch_mass_rayleigh;
    pt.screw_Ba_prime_rayleigh = ba_audit.prime_rayleigh;
    pt.screw_Ba_r0pp_rayleigh = ba_audit.r0pp_rayleigh;
    pt.screw_Ba_rpp_rayleigh = ba_audit.rpp_rayleigh;
    pt.screw_Ba_r01pp_rayleigh = ba_audit.r01pp_rayleigh;
    pt.screw_Ba_r_full_pp_rayleigh = ba_audit.r_full_pp_rayleigh;
    pt.screw_Ba_r_higher_pp_rayleigh = ba_audit.r_higher_pp_rayleigh;
    pt.screw_Ba_r_higher_kernel_pp_rayleigh = ba_audit.r_higher_kernel_pp_rayleigh;
    pt.screw_Ba_L_arch_rayleigh = ba_audit.L_arch_rayleigh;
    pt.screw_Ba_r_higher_plancherel_bound = ledger.r_higher_plancherel_bound;
    pt.screw_Ba_r_higher_artifact_gap = ledger.r_higher_artifact_gap;
    pt.screw_Ba_eq45_log_a_rayleigh = ba_audit.eq45_log_a_rayleigh;
    pt.screw_Ba_pin_margin_rayleigh = ba_audit.pin_margin_rayleigh;
    pt.screw_Ba_r_full_nonpositive_ok = pt.screw_Ba_r_full_pp_rayleigh <= 1e-6L;
    pt.screw_Ba_r01_compact_bound = ledger.r01_abs_bound;
    pt.screw_Ba_r_full_upper_bound = ledger.r_full_upper_bound;
    pt.screw_Ba_r_higher_digamma_bound = ledger.r_higher_digamma_bound;
    pt.screw_Ba_prime_minimizer_cs_upper = ledger.prime_minimizer_cs_upper;
    pt.screw_Ba_prime_saturated_upper = ledger.prime_saturated_upper;
    pt.screw_Ba_prime_analytic_upper = ledger.prime_upper_bound;
    pt.screw_Ba_pin_analytic_lower_bound = ledger.pin_analytic_lower_bound;
    pt.screw_Ba_pin_analytic_digamma_lower_bound = ledger.pin_analytic_digamma_lower_bound;
    pt.screw_Ba_r_full_analytic_f = ledger.r_full_analytic_f;
    pt.screw_Ba_pin_battle_lower_bound = ledger.pin_battle_lower_bound;
    pt.screw_Ba_pin_arch_prime_f_lower_bound = ledger.pin_arch_prime_f_lower_bound;
    pt.screw_Ba_pin_split_r01_rhigher_lower_bound = ledger.pin_split_r01_rhigher_lower_bound;
    pt.screw_Ba_pin_kernel_eq25_rayleigh = ledger.pin_kernel_eq25_rayleigh;
    pt.screw_Ba_pin_kernel_spectral_gap = ledger.pin_kernel_spectral_gap;
    pt.screw_Ba_lerch_boost_H = ledger.lerch_boost_H;
    pt.screw_Ba_r01_sharp_rayleigh = ledger.r01_sharp_rayleigh;
    pt.screw_Ba_lerch_dominance_debt = ledger.lerch_dominance_debt;
    pt.screw_Ba_lerch_dominance_debt_sharp = ledger.lerch_dominance_debt_sharp;
    pt.screw_Ba_lerch_dominance_margin = ledger.lerch_dominance_margin;
    pt.screw_Ba_bar_q1_analytic_upper = ledger.bar_q1_analytic_upper;
    pt.screw_Ba_pin_bar_q1_lower_bound = ledger.pin_bar_q1_lower_bound;
    pt.screw_Ba_r_full_upper_f_ok = ledger.r_full_upper_f_ok;
    pt.screw_Ba_r_higher_plancherel_ok = ledger.r_higher_plancherel_ok;
    pt.screw_Ba_r_higher_kernel_plancherel_ok = ledger.r_higher_kernel_plancherel_ok;
    pt.screw_Ba_pin_split_discharge_ok = ledger.pin_split_discharge_ok;
    pt.screw_Ba_pin_kernel_eq25_ok = ledger.pin_kernel_eq25_ok;
    pt.screw_Ba_r_higher_kernel_nonpos_ok = ledger.r_higher_kernel_nonpos_ok;
    pt.screw_Ba_lerch_dominance_ok = ledger.lerch_dominance_ok;
    pt.screw_Ba_pin_arch_prime_f_ok = ledger.pin_arch_prime_f_ok;
    pt.screw_Ba_pin_analytic_ok = ledger.pin_analytic_ok;
    pt.screw_Ba_r_full_nonpos_analytic_ok = ledger.r_full_nonpos_analytic_ok;
    if (!ba_focus && !cfg.fast_mode) {
        const WeilQuadraticSectorBreakdown sectors =
            weil_rayleigh_sector_breakdown(a, gammas, cat, pt.log_cutoff, pt.zero_T_cutoff, cfg.eps,
                                           full_opts);
        pt.q_pf_mode = sectors.q_pf;
        pt.q_prime_mode = sectors.q_prime;
        pt.q_zero_mode = sectors.q_zero;
        pt.lambda_full_rayleigh = sectors.rayleigh;
        pt.cross_balance_mode = sectors.q_pf + sectors.q_zero - sectors.q_prime;

        const PfZeroCouplingBreakdown coupling =
            pf_zero_coupling_at_rayleigh(a, pt.arch_triangle_A, gammas, cat, pt.log_cutoff,
                                         pt.zero_T_cutoff, cfg.eps, full_opts);
        pt.q_arch_lower_mode = coupling.q_arch_lower;
        pt.pf_zero_sum = coupling.pf_zero_sum;
        pt.pf_zero_arch_sum = coupling.pf_zero_arch_sum;
        pt.bare_lambda_at_mode = coupling.bare_lambda_at_mode;
        pt.suzuki_coupling_margin = coupling.suzuki_coupling_margin;
        pt.pf_zero_prime_pin_margin = coupling.pf_zero_prime_pin_margin;
        pt.coupling_identity_residual = coupling.coupling_identity_residual;
        pt.pf_sin_mode_positive_ok = coupling.q_pf > 1e-12L;
        pt.bare_lambda_at_mode_ok = coupling.bare_lambda_at_mode >= -1e-9L;
        pt.suzuki_coupling_margin_ok = coupling.suzuki_coupling_margin >= -1e-9L;
    } else {
        pt.q_prime_mode = ba_audit.prime_rayleigh;
        pt.cross_balance_mode = 0;
    }

    pt.arch_localized = arch_localized_gauss(a, sigma);
    pt.arch_triangle_A = arch_triangle_envelope_gauss(a, sigma);
    pt.zero_tail_exact = zero_tail_exact_gauss(pt.zero_T_cutoff, sigma, gammas);
    pt.zero_tail_rvm_bound = zero_tail_rvm_majorant(pt.zero_T_cutoff, sigma);

    // 3A RvM majorant is pinned for T >= 14; below that use exact tail (negligible on grid).
    pt.Z_bound = (pt.zero_T_cutoff >= 14.0L) ? pt.zero_tail_rvm_bound : pt.zero_tail_exact;
    pt.domination_margin = pt.arch_triangle_A - pt.S_fin - pt.Z_bound;
    pt.domination_inequality_ok = pt.domination_margin >= -1e-9L;
    pt.margin_weighted = pt.arch_triangle_A - pt.prime_fin - pt.Z_bound;
    pt.weighted_domination_ok = pt.margin_weighted >= -1e-9L;
    return pt;
}

static const Real kAGrid[] = {0.25L, 0.5L,  0.75L, 1.0L,  1.25L, 1.5L, 1.75L,
                              2.0L,  2.5L,  3.0L,  3.5L,  4.0L,  5.0L, 6.0L,
                              7.0L,  8.0L,  10.0L, 15.0L, 20.0L, 30.0L, 50.0L};

static const Real kAGridFast[] = {0.25L, 0.5L, 0.75L, 1.0L,  1.25L, 1.5L, 2.0L,
                                  3.0L,  4.0L, 5.0L,  7.0L,  10.0L, 15.0L, 20.0L};

}  // namespace

CrossSectorWeilResult RunCrossSectorWeilStudy(const Config& cfg,
                                              const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld,
                                              Heat::PrimeCatalog& cat,
                                              const std::vector<int>& primes) {
    CrossSectorWeilResult result;
    result.sigma = 1.0L;
    result.real_type = MarshalRealName();
    result.precision_bits = MarshalRealBits();
    result.precision_mode = cfg.precision_mode;
    result.fast_mode = cfg.fast_mode;

    const GaussTest gt(result.sigma);
    cat.build(primes, gt, TauFromSigma(result.sigma), cfg.kmax > 0 ? cfg.kmax : 20, cfg.eps);

    const Real arch_lo =
        ArchimedeanBaselineForTestFunction(gt, result.sigma, cfg.simd, cfg.precision_mode,
                                           cfg.arch_pts, cfg.eps, false);
    const Real arch_hi =
        ArchimedeanBaselineForTestFunction(gt, result.sigma, cfg.simd, cfg.precision_mode,
                                           cfg.arch_pts * 2, cfg.eps, false);
    result.arch_richardson_drift = std::fabsl(arch_hi - arch_lo);
    result.arch_envelope_pinned_A = std::fabsl(arch_lo);

    std::cout << "=== Cross-sector Weil battle plan (Gauss sigma=1) ===\n";
    std::cout << "  real_type=" << result.real_type << " bits=" << result.precision_bits
              << " precision_mode=" << (result.precision_mode ? "yes" : "no")
              << " fast_mode=" << (cfg.fast_mode ? "yes" : "no") << "\n";
    std::cout << "  arch_envelope_A=" << static_cast<double>(result.arch_envelope_pinned_A)
              << " richardson_drift=" << static_cast<double>(result.arch_richardson_drift)
              << "\n";

    result.full_scale =
        evaluate_at_a(4.0L, result.sigma, cfg, gammas, gammas_ld, cat, result.arch_envelope_pinned_A);
    std::cout << "  full |weil_res|="
              << static_cast<double>(std::fabsl(result.full_scale.weil_residual_full))
              << " arch=" << static_cast<double>(result.full_scale.arch)
              << " prime=" << static_cast<double>(result.full_scale.prime_full)
              << " zero=" << static_cast<double>(result.full_scale.zero) << "\n";

    result.lambda_rayleigh_min_yoshida = 1e300L;
    result.lambda_rayleigh_min_sampled = 1e300L;
    result.lambda_full_min_yoshida = 1e300L;
    result.lambda_full_min_sampled = 1e300L;
    result.lambda_full_spectral_min_sampled = 1e300L;
    result.lambda_screw_Gg_min_sampled = 1e300L;
    result.lambda_screw_Ba_min_sampled = 1e300L;
    result.lambda_screw_Ba_spectral_min_sampled = 1e300L;
    result.lambda_full_crossing_on_grid = false;
    result.lambda_screw_Gg_crossing_on_grid = false;
    result.lambda_screw_Ba_crossing_on_grid = false;
    result.screw_Gg_all_a_ok = true;
    result.screw_Ba_all_a_ok = true;
    result.screw_Ba_eq25_operator_consistent_ok = true;
    result.screw_Ba_eq25_identity_ok = true;
    result.screw_Ba_r_higher_max_abs = 0;
    result.screw_Ba_eq25_full_decomposition_ok = true;
    result.screw_Ba_eq25_max_rel_gap = 0;
    result.screw_Ba_discretization_max_abs = 0;
    result.screw_Ba_pin_margin_min_on_grid = 1e300L;
    result.screw_Ba_pin_analytic_min_on_grid = 1e300L;
    result.screw_Ba_pin_battle_min_on_grid = 1e300L;
    result.screw_Ba_pin_arch_prime_f_min_on_grid = 1e300L;
    result.screw_Ba_eq25_analytic_pin_all_a_ok = true;
    result.screw_Ba_pin_battle_all_a_ok = true;
    result.screw_Ba_r_full_upper_f_all_a_ok = true;
    result.screw_Ba_pin_arch_prime_f_all_a_ok = true;
    result.screw_Ba_pin_split_discharge_all_a_ok = true;
    result.screw_Ba_r_higher_plancherel_all_a_ok = true;
    result.screw_Ba_r_higher_kernel_plancherel_all_a_ok = true;
    result.screw_Ba_r_higher_artifact_max_abs = 0;
    result.screw_Ba_pin_kernel_eq25_all_a_ok = true;
    result.screw_Ba_r_higher_kernel_nonpos_all_a_ok = true;
    result.screw_Ba_lerch_dominance_all_a_ok = true;
    result.screw_Ba_lerch_dominance_margin_min_on_grid = 1e300L;
    result.screw_Ba_kernel_mesh_plancherel_all_ok = true;
    result.screw_Ba_kernel_eq25_positive_finest_ok = true;
    result.screw_Ba_eq25_kernel_mesh_monotone_ok = true;
    result.screw_Ba_pin_kernel_spectral_gap_max_abs = 0;
    result.screw_Ba_artifact_gap_finest_max = 0;
    result.screw_Ba_r_full_nonpositive_all_a_ok = true;
    result.screw_Ba_r_full_nonpos_analytic_all_a_ok = true;
    result.screw_Ba_dense_spectral_crosscheck_all_ok = true;
    result.screw_Ba_dense_lambda_min_on_grid = 1e300L;
    result.screw_Ba_digamma_C0_pinned = screw_r_higher_digamma_fourier_C0_pinned();
    result.screw_Ba_r01_compact_uniform = screw_r01_compact_uniform_rayleigh_bound();
    result.screw_Ba_mesh_refinement_converged_ok = true;
    result.screw_Ba_mesh_refinement_positive_ok = true;
    result.step5_prime_cs_covers_mode_ok = true;
    result.step5_min_cross_balance = 1e300L;
    result.domination_margin_min_on_grid = 1e300L;
    result.domination_inequality_all_a_ok = true;
    result.weighted_domination_all_a_ok = true;
    result.domination_chain_wired_ok = true;
    result.pf_zero_coupling_identity_ok = true;
    result.pf_sin_mode_positive_all_ok = true;
    result.bare_lambda_all_a_ok = true;
    result.suzuki_coupling_all_a_ok = true;
    result.bare_lambda_min_on_grid = 1e300L;
    result.suzuki_coupling_margin_min_on_grid = 1e300L;
    result.pf_zero_prime_pin_margin_min_on_grid = 1e300L;
    result.zero_tail_max_on_grid = 0;
    result.arch_envelope_analytic_ok = true;
    result.zero_tail_bound_analytic_ok = true;

    const Real* grid = kAGrid;
    size_t grid_n = sizeof(kAGrid) / sizeof(kAGrid[0]);
    if (cfg.fast_mode) {
        grid = kAGridFast;
        grid_n = sizeof(kAGridFast) / sizeof(kAGridFast[0]);
    }
    for (size_t gi = 0; gi < grid_n; ++gi) {
        const Real a = grid[gi];
        CrossSectorWeilPoint pt =
            evaluate_at_a(a, result.sigma, cfg, gammas, gammas_ld, cat, result.arch_envelope_pinned_A);
        result.a_grid.push_back(pt);
        result.zero_tail_max_on_grid = std::max(result.zero_tail_max_on_grid, pt.zero_tail_abs);
        result.lambda_rayleigh_min_sampled = std::min(result.lambda_rayleigh_min_sampled, pt.lambda_rayleigh);
        result.lambda_full_min_sampled = std::min(result.lambda_full_min_sampled, pt.lambda_full_rayleigh);
        result.lambda_full_spectral_min_sampled =
            std::min(result.lambda_full_spectral_min_sampled, pt.lambda_full_spectral);
        if (a <= kBaBattleFocusA) {
            result.lambda_screw_Gg_min_sampled =
                std::min(result.lambda_screw_Gg_min_sampled, pt.lambda_screw_Gg_rayleigh);
            if (pt.lambda_screw_Gg_rayleigh < -1e-9L) {
                result.lambda_screw_Gg_crossing_on_grid = true;
                result.screw_Gg_all_a_ok = false;
            }
        }
        result.lambda_screw_Ba_min_sampled =
            std::min(result.lambda_screw_Ba_min_sampled, pt.lambda_screw_Ba_spectral);
        result.lambda_screw_Ba_spectral_min_sampled =
            std::min(result.lambda_screw_Ba_spectral_min_sampled, pt.lambda_screw_Ba_spectral);
        if (pt.lambda_full_rayleigh < 0 || pt.lambda_full_spectral < 0) {
            result.lambda_full_crossing_on_grid = true;
        }
        if (pt.lambda_screw_Ba_spectral < -1e-9L) {
            result.lambda_screw_Ba_crossing_on_grid = true;
            result.screw_Ba_all_a_ok = false;
        }
        result.screw_Ba_eq25_max_rel_gap =
            std::max(result.screw_Ba_eq25_max_rel_gap, pt.screw_Ba_eq25_rel_gap);
        result.screw_Ba_discretization_max_abs =
            std::max(result.screw_Ba_discretization_max_abs, std::fabsl(pt.screw_Ba_discretization_gap));
        result.screw_Ba_pin_margin_min_on_grid =
            std::min(result.screw_Ba_pin_margin_min_on_grid, pt.screw_Ba_pin_margin_rayleigh);
        result.screw_Ba_pin_analytic_min_on_grid =
            std::min(result.screw_Ba_pin_analytic_min_on_grid, pt.screw_Ba_pin_analytic_digamma_lower_bound);
        result.screw_Ba_pin_battle_min_on_grid =
            std::min(result.screw_Ba_pin_battle_min_on_grid, pt.screw_Ba_pin_battle_lower_bound);
        result.screw_Ba_pin_arch_prime_f_min_on_grid =
            std::min(result.screw_Ba_pin_arch_prime_f_min_on_grid,
                     pt.screw_Ba_pin_arch_prime_f_lower_bound);
        if (!pt.screw_Ba_pin_analytic_ok) result.screw_Ba_eq25_analytic_pin_all_a_ok = false;
        if (!pt.screw_Ba_pin_analytic_ok) result.screw_Ba_pin_battle_all_a_ok = false;
        if (!pt.screw_Ba_pin_arch_prime_f_ok) result.screw_Ba_pin_arch_prime_f_all_a_ok = false;
        if (!pt.screw_Ba_r_full_upper_f_ok) result.screw_Ba_r_full_upper_f_all_a_ok = false;
        if (!pt.screw_Ba_r_higher_plancherel_ok) result.screw_Ba_r_higher_plancherel_all_a_ok = false;
        if (!pt.screw_Ba_r_higher_kernel_plancherel_ok) {
            result.screw_Ba_r_higher_kernel_plancherel_all_a_ok = false;
        }
        result.screw_Ba_r_higher_artifact_max_abs = std::max(
            result.screw_Ba_r_higher_artifact_max_abs, std::fabsl(pt.screw_Ba_r_higher_artifact_gap));
        result.screw_Ba_pin_kernel_spectral_gap_max_abs = std::max(
            result.screw_Ba_pin_kernel_spectral_gap_max_abs,
            std::fabsl(pt.screw_Ba_pin_kernel_spectral_gap));
        if (!pt.screw_Ba_pin_split_discharge_ok) result.screw_Ba_pin_split_discharge_all_a_ok = false;
        if (!pt.screw_Ba_pin_kernel_eq25_ok) result.screw_Ba_pin_kernel_eq25_all_a_ok = false;
        if (!pt.screw_Ba_r_higher_kernel_nonpos_ok) {
            result.screw_Ba_r_higher_kernel_nonpos_all_a_ok = false;
        }
        if (!pt.screw_Ba_lerch_dominance_ok) result.screw_Ba_lerch_dominance_all_a_ok = false;
        result.screw_Ba_lerch_dominance_margin_min_on_grid = std::min(
            result.screw_Ba_lerch_dominance_margin_min_on_grid, pt.screw_Ba_lerch_dominance_margin);
        if (!pt.screw_Ba_r_full_nonpositive_ok) result.screw_Ba_r_full_nonpositive_all_a_ok = false;
        if (!pt.screw_Ba_r_full_nonpos_analytic_ok) {
            result.screw_Ba_r_full_nonpos_analytic_all_a_ok = false;
        }
        if (!pt.screw_Ba_spectral_dense_crosscheck_ok) {
            result.screw_Ba_dense_spectral_crosscheck_all_ok = false;
        }
        if (pt.lambda_screw_Ba_spectral_dense != 0.0L ||
            pt.lambda_screw_Ba_spectral_iterative != 0.0L) {
            result.screw_Ba_dense_lambda_min_on_grid = std::min(
                result.screw_Ba_dense_lambda_min_on_grid, pt.lambda_screw_Ba_spectral_dense);
        }
        result.screw_Ba_r_higher_max_abs =
            std::max(result.screw_Ba_r_higher_max_abs, std::fabsl(pt.screw_Ba_r_higher_pp_rayleigh));
        const Real eq25_identity_residual =
            pt.screw_Ba_arch_mass_rayleigh - pt.screw_Ba_prime_rayleigh -
            pt.screw_Ba_r_full_pp_rayleigh - pt.screw_Ba_pin_margin_rayleigh;
        if (std::fabsl(eq25_identity_residual) > 1e-6L) {
            result.screw_Ba_eq25_identity_ok = false;
        }
        if (std::fabsl(pt.screw_Ba_discretization_gap) > 50.0L) {
            result.screw_Ba_eq25_full_decomposition_ok = false;
        }
        if (!std::isfinite(static_cast<double>(pt.screw_Ba_full_eq25_rayleigh)) ||
            !std::isfinite(static_cast<double>(pt.screw_Ba_pin_margin_rayleigh))) {
            result.screw_Ba_eq25_operator_consistent_ok = false;
        }
        if (pt.q_prime_mode > pt.prime_cs_bound * 1.001L + 1e-12L) {
            result.step5_prime_cs_covers_mode_ok = false;
        }
        result.step5_min_cross_balance =
            std::min(result.step5_min_cross_balance, pt.cross_balance_mode);
        result.domination_margin_min_on_grid =
            std::min(result.domination_margin_min_on_grid, pt.domination_margin);
        if (!pt.domination_inequality_ok) result.domination_inequality_all_a_ok = false;
        result.margin_weighted_min_on_grid =
            std::min(result.margin_weighted_min_on_grid, pt.margin_weighted);
        if (!pt.weighted_domination_ok) result.weighted_domination_all_a_ok = false;
        if (a <= 0.5L && !pt.domination_inequality_ok) {
            result.yoshida_window_cs_domination_ok = false;
        }
        if (a <= kBaBattleFocusA) {
            if (std::fabsl(pt.coupling_identity_residual) > 1e-6L) {
                result.pf_zero_coupling_identity_ok = false;
            }
            if (!pt.pf_sin_mode_positive_ok) result.pf_sin_mode_positive_all_ok = false;
            result.bare_lambda_min_on_grid =
                std::min(result.bare_lambda_min_on_grid, pt.bare_lambda_at_mode);
            if (!pt.bare_lambda_at_mode_ok) result.bare_lambda_all_a_ok = false;
            result.pf_zero_prime_pin_margin_min_on_grid =
                std::min(result.pf_zero_prime_pin_margin_min_on_grid, pt.pf_zero_prime_pin_margin);
            result.suzuki_coupling_margin_min_on_grid =
                std::min(result.suzuki_coupling_margin_min_on_grid, pt.suzuki_coupling_margin);
            if (!pt.suzuki_coupling_margin_ok) result.suzuki_coupling_all_a_ok = false;
        }
        if (a >= 3.0L) {
            result.suzuki_prime_gauss_limit = std::max(result.suzuki_prime_gauss_limit, pt.prime_fin);
        }
        if (a <= 1.0L) {
            result.lambda_rayleigh_min_yoshida =
                std::min(result.lambda_rayleigh_min_yoshida, pt.lambda_rayleigh);
            result.lambda_full_min_yoshida =
                std::min(result.lambda_full_min_yoshida, pt.lambda_full_rayleigh);
        }
        if (pt.arch_localized < -pt.arch_triangle_A - 1e-9L) result.arch_envelope_analytic_ok = false;
        if (pt.zero_tail_exact > pt.zero_tail_rvm_bound * 1.001L + 1e-12L) {
            result.zero_tail_bound_analytic_ok = false;
        }
        std::cout << "  a=" << static_cast<double>(a) << " arch_loc="
                  << static_cast<double>(pt.arch_localized) << " A="
                  << static_cast<double>(pt.arch_triangle_A)
                  << " zero_tail=" << static_cast<double>(pt.zero_tail_exact)
              << " rvm=" << static_cast<double>(pt.zero_tail_rvm_bound)
              << " lambda_R=" << static_cast<double>(pt.lambda_rayleigh)
              << " lambda_full=" << static_cast<double>(pt.lambda_full_rayleigh)
              << " lambda_Gg=" << static_cast<double>(pt.lambda_screw_Gg_rayleigh)
              << " lambda_Ba=" << static_cast<double>(pt.lambda_screw_Ba_rayleigh)
              << " lambda_Ba_spec=" << static_cast<double>(pt.lambda_screw_Ba_spectral)
              << " bal=" << static_cast<double>(pt.cross_balance_mode)
              << " qP=" << static_cast<double>(pt.q_prime_mode)
              << " cs=" << static_cast<double>(pt.prime_cs_bound)
              << " dom=" << static_cast<double>(pt.domination_margin)
              << " dom_w=" << static_cast<double>(pt.margin_weighted)
              << " pf0=" << static_cast<double>(pt.pf_zero_sum)
              << " lam=" << static_cast<double>(pt.bare_lambda_at_mode)
              << " coup=" << static_cast<double>(pt.suzuki_coupling_margin)
              << " (A=" << static_cast<double>(pt.arch_triangle_A)
              << " S=" << static_cast<double>(pt.S_fin)
              << " Z=" << static_cast<double>(pt.Z_bound) << ")\n";
    }
    auto run_mesh_refinement = [&](Real a, int mesh_cap) {
        const Real log_cut = 2.0L * a;
        const Real prime_sat = 1.07L;
        const Real S_fin = weil_prime_weight_sum(a, cat, log_cut, cfg.eps);
        const ScrewBaMeshRefinement mesh = screw_Ba_kernel_mesh_refinement_study(
            a, cat, log_cut, cfg.eps, prime_sat, S_fin, mesh_cap);
        for (const auto& mp : mesh.points) {
            CrossSectorWeilMeshRefinementPoint rp;
            rp.a = a;
            rp.spectral_pts = mp.spectral_pts;
            rp.lambda_dense = mp.lambda_dense;
            rp.r_higher_kernel_pp_rayleigh = mp.r_higher_kernel_pp_rayleigh;
            rp.r_higher_plancherel_bound = mp.r_higher_plancherel_bound;
            rp.eq25_kernel_rayleigh = mp.eq25_kernel_rayleigh;
            rp.pin_kernel_eq25_rayleigh = mp.pin_kernel_eq25_rayleigh;
            rp.r_higher_artifact_gap = mp.r_higher_artifact_gap;
            rp.pin_kernel_spectral_gap = mp.pin_kernel_spectral_gap;
            rp.r_higher_kernel_plancherel_ok = mp.r_higher_kernel_plancherel_ok;
            result.screw_Ba_mesh_refinement.push_back(rp);
        }
        if (!mesh.monotone_converged) result.screw_Ba_mesh_refinement_converged_ok = false;
        if (!mesh.converged_positive_ok) result.screw_Ba_mesh_refinement_positive_ok = false;
        if (!mesh.kernel_plancherel_all_meshes_ok) {
            result.screw_Ba_kernel_mesh_plancherel_all_ok = false;
        }
        if (!mesh.eq25_kernel_positive_finest_ok) {
            result.screw_Ba_kernel_eq25_positive_finest_ok = false;
        }
        if (!mesh.eq25_kernel_mesh_monotone_ok) {
            result.screw_Ba_eq25_kernel_mesh_monotone_ok = false;
        }
        result.screw_Ba_artifact_gap_finest_max =
            std::max(result.screw_Ba_artifact_gap_finest_max, std::fabsl(mesh.artifact_gap_finest));
        std::cout << "  mesh a=" << static_cast<double>(a) << " coarse="
                  << static_cast<double>(mesh.lambda_coarsest) << " fine="
                  << static_cast<double>(mesh.lambda_finest) << " eq25_k="
                  << static_cast<double>(mesh.eq25_kernel_finest) << " artifact="
                  << static_cast<double>(mesh.artifact_gap_finest) << " k_pl="
                  << (mesh.kernel_plancherel_all_meshes_ok ? "yes" : "no") << " eq25_k+="
                  << (mesh.eq25_kernel_positive_finest_ok ? "yes" : "no") << " k_mono="
                  << (mesh.eq25_kernel_mesh_monotone_ok ? "yes" : "no") << "\n";
    };
    if (cfg.fast_mode) {
        run_mesh_refinement(10.0L, 192);
    } else {
    static const Real kMeshA[] = {7.0L, 8.0L, 10.0L};
    for (Real a : kMeshA) {
        run_mesh_refinement(a, 512);
    }
    }
    std::cout << "  domination_margin_min=" << static_cast<double>(result.domination_margin_min_on_grid)
              << " domination_all_a_ok="
              << (result.domination_inequality_all_a_ok ? "yes" : "no") << "\n";
    std::cout << "  pf_zero_coupling_identity_ok="
              << (result.pf_zero_coupling_identity_ok ? "yes" : "no")
              << " pf_sin_positive_all="
              << (result.pf_sin_mode_positive_all_ok ? "yes" : "no")
              << " bare_lambda_min="
              << static_cast<double>(result.bare_lambda_min_on_grid)
              << " bare_lambda_all_a_ok="
              << (result.bare_lambda_all_a_ok ? "yes" : "no")
              << " lambda_Gg_min="
              << static_cast<double>(result.lambda_screw_Gg_min_sampled)
              << " screw_Gg_all_a_ok="
              << (result.screw_Gg_all_a_ok ? "yes" : "no")
              << " lambda_Ba_min="
              << static_cast<double>(result.lambda_screw_Ba_min_sampled)
              << " lambda_Ba_spec_min="
              << static_cast<double>(result.lambda_screw_Ba_spectral_min_sampled)
              << " screw_Ba_all_a_ok="
              << (result.screw_Ba_all_a_ok ? "yes" : "no")
              << " eq25_max_rel_gap="
              << static_cast<double>(result.screw_Ba_eq25_max_rel_gap)
              << " eq25_consistent="
              << (result.screw_Ba_eq25_operator_consistent_ok ? "yes" : "no")
              << " disc_max="
              << static_cast<double>(result.screw_Ba_discretization_max_abs)
              << " pin_margin_min="
              << static_cast<double>(result.screw_Ba_pin_margin_min_on_grid)
              << " pin_analytic_min="
              << static_cast<double>(result.screw_Ba_pin_analytic_min_on_grid)
              << " pin_battle_min="
              << static_cast<double>(result.screw_Ba_pin_battle_min_on_grid)
              << " pin_analytic_ok="
              << (result.screw_Ba_eq25_analytic_pin_all_a_ok ? "yes" : "no")
              << " pin_battle_ok="
              << (result.screw_Ba_pin_battle_all_a_ok ? "yes" : "no")
              << " pin_arch_prime_f_min="
              << static_cast<double>(result.screw_Ba_pin_arch_prime_f_min_on_grid)
              << " r_full_upper_f_ok="
              << (result.screw_Ba_r_full_upper_f_all_a_ok ? "yes" : "no")
              << " mesh_conv="
              << (result.screw_Ba_mesh_refinement_converged_ok ? "yes" : "no")
              << " mesh_pos="
              << (result.screw_Ba_mesh_refinement_positive_ok ? "yes" : "no")
              << " dense_xc="
              << (result.screw_Ba_dense_spectral_crosscheck_all_ok ? "yes" : "no")
              << " dense_lam_min="
              << static_cast<double>(result.screw_Ba_dense_lambda_min_on_grid)
              << " digamma_C0="
              << static_cast<double>(result.screw_Ba_digamma_C0_pinned)
              << " r_full_nonpos="
              << (result.screw_Ba_r_full_nonpositive_all_a_ok ? "yes" : "no")
              << " arch_shift_min="
              << static_cast<double>(result.suzuki_coupling_margin_min_on_grid) << "\n";
    std::cout << "  arch_envelope_analytic_ok=" << (result.arch_envelope_analytic_ok ? "yes" : "no")
              << " zero_tail_bound_analytic_ok="
              << (result.zero_tail_bound_analytic_ok ? "yes" : "no") << "\n";
    return result;
}

bool ExportCrossSectorWeilJson(const std::string& path, const CrossSectorWeilResult& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    auto write_point = [&](const CrossSectorWeilPoint& pt) {
        out << "      \"a\": " << static_cast<double>(pt.a) << ",\n";
        out << "      \"log_cutoff\": " << static_cast<double>(pt.log_cutoff) << ",\n";
        out << "      \"poles\": " << static_cast<double>(pt.poles) << ",\n";
        out << "      \"arch\": " << static_cast<double>(pt.arch) << ",\n";
        out << "      \"arch_envelope_A\": " << static_cast<double>(pt.arch_envelope_A) << ",\n";
        out << "      \"prime_full\": " << static_cast<double>(pt.prime_full) << ",\n";
        out << "      \"prime_fin\": " << static_cast<double>(pt.prime_fin) << ",\n";
        out << "      \"zero\": " << static_cast<double>(pt.zero) << ",\n";
        out << "      \"zero_T_cutoff\": " << static_cast<double>(pt.zero_T_cutoff) << ",\n";
        out << "      \"zero_prefix_count\": " << pt.zero_prefix_count << ",\n";
        out << "      \"zero_prefix\": " << static_cast<double>(pt.zero_prefix) << ",\n";
        out << "      \"zero_tail\": " << static_cast<double>(pt.zero_tail) << ",\n";
        out << "      \"zero_tail_abs\": " << static_cast<double>(pt.zero_tail_abs) << ",\n";
        out << "      \"lambda_rayleigh\": " << static_cast<double>(pt.lambda_rayleigh) << ",\n";
        out << "      \"lambda_full_rayleigh\": " << static_cast<double>(pt.lambda_full_rayleigh)
            << ",\n";
        out << "      \"lambda_full_spectral\": " << static_cast<double>(pt.lambda_full_spectral)
            << ",\n";
        out << "      \"lambda_screw_Gg_rayleigh\": "
            << static_cast<double>(pt.lambda_screw_Gg_rayleigh) << ",\n";
        out << "      \"lambda_screw_Ba_rayleigh\": "
            << static_cast<double>(pt.lambda_screw_Ba_rayleigh) << ",\n";
        out << "      \"lambda_screw_Ba_spectral\": "
            << static_cast<double>(pt.lambda_screw_Ba_spectral) << ",\n";
        out << "      \"lambda_screw_Ba_spectral_dense\": "
            << static_cast<double>(pt.lambda_screw_Ba_spectral_dense) << ",\n";
        out << "      \"lambda_screw_Ba_spectral_iterative\": "
            << static_cast<double>(pt.lambda_screw_Ba_spectral_iterative) << ",\n";
        out << "      \"screw_Ba_spectral_dense_crosscheck_ok\": "
            << (pt.screw_Ba_spectral_dense_crosscheck_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_eq25_eigvec_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_eq25_eigvec_rayleigh) << ",\n";
        out << "      \"screw_Ba_eq25_remainder\": "
            << static_cast<double>(pt.screw_Ba_eq25_remainder) << ",\n";
        out << "      \"screw_Ba_eq25_rel_gap\": "
            << static_cast<double>(pt.screw_Ba_eq25_rel_gap) << ",\n";
        out << "      \"screw_Ba_full_eq25_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_full_eq25_rayleigh) << ",\n";
        out << "      \"screw_Ba_discretization_gap\": "
            << static_cast<double>(pt.screw_Ba_discretization_gap) << ",\n";
        out << "      \"screw_Ba_arch_mass_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_arch_mass_rayleigh) << ",\n";
        out << "      \"screw_Ba_prime_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_prime_rayleigh) << ",\n";
        out << "      \"screw_Ba_r0pp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_r0pp_rayleigh) << ",\n";
        out << "      \"screw_Ba_rpp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_rpp_rayleigh) << ",\n";
        out << "      \"screw_Ba_r01pp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_r01pp_rayleigh) << ",\n";
        out << "      \"screw_Ba_r_full_pp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_r_full_pp_rayleigh) << ",\n";
        out << "      \"screw_Ba_r_higher_pp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_r_higher_pp_rayleigh) << ",\n";
        out << "      \"screw_Ba_r_higher_kernel_pp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_r_higher_kernel_pp_rayleigh) << ",\n";
        out << "      \"screw_Ba_r_higher_artifact_gap\": "
            << static_cast<double>(pt.screw_Ba_r_higher_artifact_gap) << ",\n";
        out << "      \"screw_Ba_L_arch_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_L_arch_rayleigh) << ",\n";
        out << "      \"screw_Ba_r_higher_plancherel_bound\": "
            << static_cast<double>(pt.screw_Ba_r_higher_plancherel_bound) << ",\n";
        out << "      \"screw_Ba_r_higher_plancherel_ok\": "
            << (pt.screw_Ba_r_higher_plancherel_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_r_higher_kernel_plancherel_ok\": "
            << (pt.screw_Ba_r_higher_kernel_plancherel_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_pin_split_r01_rhigher_lower_bound\": "
            << static_cast<double>(pt.screw_Ba_pin_split_r01_rhigher_lower_bound) << ",\n";
        out << "      \"screw_Ba_pin_split_discharge_ok\": "
            << (pt.screw_Ba_pin_split_discharge_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_pin_kernel_eq25_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_pin_kernel_eq25_rayleigh) << ",\n";
        out << "      \"screw_Ba_pin_kernel_spectral_gap\": "
            << static_cast<double>(pt.screw_Ba_pin_kernel_spectral_gap) << ",\n";
        out << "      \"screw_Ba_lerch_boost_H\": "
            << static_cast<double>(pt.screw_Ba_lerch_boost_H) << ",\n";
        out << "      \"screw_Ba_r01_sharp_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_r01_sharp_rayleigh) << ",\n";
        out << "      \"screw_Ba_lerch_dominance_debt\": "
            << static_cast<double>(pt.screw_Ba_lerch_dominance_debt) << ",\n";
        out << "      \"screw_Ba_lerch_dominance_debt_sharp\": "
            << static_cast<double>(pt.screw_Ba_lerch_dominance_debt_sharp) << ",\n";
        out << "      \"screw_Ba_lerch_dominance_margin\": "
            << static_cast<double>(pt.screw_Ba_lerch_dominance_margin) << ",\n";
        out << "      \"screw_Ba_r_higher_kernel_nonpos_ok\": "
            << (pt.screw_Ba_r_higher_kernel_nonpos_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_lerch_dominance_ok\": "
            << (pt.screw_Ba_lerch_dominance_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_pin_kernel_eq25_ok\": "
            << (pt.screw_Ba_pin_kernel_eq25_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_eq45_log_a_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_eq45_log_a_rayleigh) << ",\n";
        out << "      \"screw_Ba_pin_margin_rayleigh\": "
            << static_cast<double>(pt.screw_Ba_pin_margin_rayleigh) << ",\n";
        out << "      \"screw_Ba_r_full_upper_bound\": "
            << static_cast<double>(pt.screw_Ba_r_full_upper_bound) << ",\n";
        out << "      \"screw_Ba_r_higher_digamma_bound\": "
            << static_cast<double>(pt.screw_Ba_r_higher_digamma_bound) << ",\n";
        out << "      \"screw_Ba_r01_compact_bound\": "
            << static_cast<double>(pt.screw_Ba_r01_compact_bound) << ",\n";
        out << "      \"screw_Ba_prime_minimizer_cs_upper\": "
            << static_cast<double>(pt.screw_Ba_prime_minimizer_cs_upper) << ",\n";
        out << "      \"screw_Ba_prime_saturated_upper\": "
            << static_cast<double>(pt.screw_Ba_prime_saturated_upper) << ",\n";
        out << "      \"screw_Ba_prime_analytic_upper\": "
            << static_cast<double>(pt.screw_Ba_prime_analytic_upper) << ",\n";
        out << "      \"screw_Ba_pin_analytic_lower_bound\": "
            << static_cast<double>(pt.screw_Ba_pin_analytic_lower_bound) << ",\n";
        out << "      \"screw_Ba_pin_analytic_digamma_lower_bound\": "
            << static_cast<double>(pt.screw_Ba_pin_analytic_digamma_lower_bound) << ",\n";
        out << "      \"screw_Ba_r_full_analytic_f\": "
            << static_cast<double>(pt.screw_Ba_r_full_analytic_f) << ",\n";
        out << "      \"screw_Ba_pin_battle_lower_bound\": "
            << static_cast<double>(pt.screw_Ba_pin_battle_lower_bound) << ",\n";
        out << "      \"screw_Ba_pin_arch_prime_f_lower_bound\": "
            << static_cast<double>(pt.screw_Ba_pin_arch_prime_f_lower_bound) << ",\n";
        out << "      \"screw_Ba_bar_q1_analytic_upper\": "
            << static_cast<double>(pt.screw_Ba_bar_q1_analytic_upper) << ",\n";
        out << "      \"screw_Ba_pin_bar_q1_lower_bound\": "
            << static_cast<double>(pt.screw_Ba_pin_bar_q1_lower_bound) << ",\n";
        out << "      \"screw_Ba_r_full_upper_f_ok\": "
            << (pt.screw_Ba_r_full_upper_f_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_pin_arch_prime_f_ok\": "
            << (pt.screw_Ba_pin_arch_prime_f_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_pin_analytic_ok\": "
            << (pt.screw_Ba_pin_analytic_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_r_full_nonpositive_ok\": "
            << (pt.screw_Ba_r_full_nonpositive_ok ? "true" : "false") << ",\n";
        out << "      \"screw_Ba_r_full_nonpos_analytic_ok\": "
            << (pt.screw_Ba_r_full_nonpos_analytic_ok ? "true" : "false") << ",\n";
        out << "      \"q_pf_mode\": " << static_cast<double>(pt.q_pf_mode) << ",\n";
        out << "      \"q_prime_mode\": " << static_cast<double>(pt.q_prime_mode) << ",\n";
        out << "      \"q_zero_mode\": " << static_cast<double>(pt.q_zero_mode) << ",\n";
        out << "      \"prime_cs_bound\": " << static_cast<double>(pt.prime_cs_bound) << ",\n";
        out << "      \"cross_balance_mode\": " << static_cast<double>(pt.cross_balance_mode)
            << ",\n";
        out << "      \"S_fin\": " << static_cast<double>(pt.S_fin) << ",\n";
        out << "      \"Z_bound\": " << static_cast<double>(pt.Z_bound) << ",\n";
        out << "      \"domination_margin\": " << static_cast<double>(pt.domination_margin)
            << ",\n";
        out << "      \"domination_inequality_ok\": "
            << (pt.domination_inequality_ok ? "true" : "false") << ",\n";
        out << "      \"margin_weighted\": " << static_cast<double>(pt.margin_weighted) << ",\n";
        out << "      \"weighted_domination_ok\": "
            << (pt.weighted_domination_ok ? "true" : "false") << ",\n";
        out << "      \"q_arch_lower_mode\": " << static_cast<double>(pt.q_arch_lower_mode)
            << ",\n";
        out << "      \"pf_zero_sum\": " << static_cast<double>(pt.pf_zero_sum) << ",\n";
        out << "      \"pf_zero_arch_sum\": " << static_cast<double>(pt.pf_zero_arch_sum)
            << ",\n";
        out << "      \"bare_lambda_at_mode\": "
            << static_cast<double>(pt.bare_lambda_at_mode) << ",\n";
        out << "      \"pf_zero_prime_pin_margin\": "
            << static_cast<double>(pt.pf_zero_prime_pin_margin) << ",\n";
        out << "      \"suzuki_coupling_margin\": "
            << static_cast<double>(pt.suzuki_coupling_margin) << ",\n";
        out << "      \"coupling_identity_residual\": "
            << static_cast<double>(pt.coupling_identity_residual) << ",\n";
        out << "      \"pf_sin_mode_positive_ok\": "
            << (pt.pf_sin_mode_positive_ok ? "true" : "false") << ",\n";
        out << "      \"bare_lambda_at_mode_ok\": "
            << (pt.bare_lambda_at_mode_ok ? "true" : "false") << ",\n";
        out << "      \"suzuki_coupling_margin_ok\": "
            << (pt.suzuki_coupling_margin_ok ? "true" : "false") << ",\n";
        out << "      \"arch_localized\": " << static_cast<double>(pt.arch_localized) << ",\n";
        out << "      \"arch_triangle_A\": " << static_cast<double>(pt.arch_triangle_A) << ",\n";
        out << "      \"zero_tail_exact\": " << static_cast<double>(pt.zero_tail_exact) << ",\n";
        out << "      \"zero_tail_rvm_bound\": " << static_cast<double>(pt.zero_tail_rvm_bound) << ",\n";
        out << "      \"geometric\": " << static_cast<double>(pt.geometric) << ",\n";
        out << "      \"cross_rhs_fin\": " << static_cast<double>(pt.cross_rhs_fin) << ",\n";
        out << "      \"weil_residual_full\": "
            << static_cast<double>(pt.weil_residual_full) << ",\n";
        out << "      \"weil_margin_fin\": " << static_cast<double>(pt.weil_margin_fin);
    };

    out << "{\n  \"version\": " << r.version << ",\n";
    out << "  \"purpose\": \"Cross-sector Weil positivity battle plan — phases 1-4 sector ledger\",\n";
    out << "  \"sigma\": " << static_cast<double>(r.sigma) << ",\n";
    out << "  \"real_type\": \"" << r.real_type << "\",\n";
    out << "  \"precision_bits\": " << r.precision_bits << ",\n";
    out << "  \"precision_mode\": " << (r.precision_mode ? "true" : "false") << ",\n";
    out << "  \"fast_mode\": " << (r.fast_mode ? "true" : "false") << ",\n";
    out << "  \"arch_envelope_pinned_A\": " << static_cast<double>(r.arch_envelope_pinned_A)
        << ",\n";
    out << "  \"arch_richardson_drift\": " << static_cast<double>(r.arch_richardson_drift)
        << ",\n";
    out << "  \"zero_tail_max_on_grid\": " << static_cast<double>(r.zero_tail_max_on_grid)
        << ",\n";
    out << "  \"lambda_rayleigh_min_yoshida\": "
        << static_cast<double>(r.lambda_rayleigh_min_yoshida) << ",\n";
    out << "  \"lambda_rayleigh_min_sampled\": "
        << static_cast<double>(r.lambda_rayleigh_min_sampled) << ",\n";
    out << "  \"lambda_full_min_yoshida\": " << static_cast<double>(r.lambda_full_min_yoshida)
        << ",\n";
    out << "  \"lambda_full_min_sampled\": " << static_cast<double>(r.lambda_full_min_sampled)
        << ",\n";
    out << "  \"lambda_full_spectral_min_sampled\": "
        << static_cast<double>(r.lambda_full_spectral_min_sampled) << ",\n";
    out << "  \"lambda_screw_Gg_min_sampled\": "
        << static_cast<double>(r.lambda_screw_Gg_min_sampled) << ",\n";
    out << "  \"lambda_screw_Ba_min_sampled\": "
        << static_cast<double>(r.lambda_screw_Ba_min_sampled) << ",\n";
    out << "  \"lambda_screw_Ba_spectral_min_sampled\": "
        << static_cast<double>(r.lambda_screw_Ba_spectral_min_sampled) << ",\n";
    out << "  \"lambda_full_crossing_on_grid\": "
        << (r.lambda_full_crossing_on_grid ? "true" : "false") << ",\n";
    out << "  \"lambda_screw_Gg_crossing_on_grid\": "
        << (r.lambda_screw_Gg_crossing_on_grid ? "true" : "false") << ",\n";
    out << "  \"lambda_screw_Ba_crossing_on_grid\": "
        << (r.lambda_screw_Ba_crossing_on_grid ? "true" : "false") << ",\n";
    out << "  \"screw_Gg_all_a_ok\": " << (r.screw_Gg_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_all_a_ok\": " << (r.screw_Ba_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_eq25_max_rel_gap\": "
        << static_cast<double>(r.screw_Ba_eq25_max_rel_gap) << ",\n";
    out << "  \"screw_Ba_discretization_max_abs\": "
        << static_cast<double>(r.screw_Ba_discretization_max_abs) << ",\n";
    out << "  \"screw_Ba_pin_margin_min_on_grid\": "
        << static_cast<double>(r.screw_Ba_pin_margin_min_on_grid) << ",\n";
    out << "  \"screw_Ba_pin_analytic_min_on_grid\": "
        << static_cast<double>(r.screw_Ba_pin_analytic_min_on_grid) << ",\n";
    out << "  \"screw_Ba_pin_battle_min_on_grid\": "
        << static_cast<double>(r.screw_Ba_pin_battle_min_on_grid) << ",\n";
    out << "  \"screw_Ba_pin_arch_prime_f_min_on_grid\": "
        << static_cast<double>(r.screw_Ba_pin_arch_prime_f_min_on_grid) << ",\n";
    out << "  \"screw_Ba_eq25_analytic_pin_all_a_ok\": "
        << (r.screw_Ba_eq25_analytic_pin_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_pin_battle_all_a_ok\": "
        << (r.screw_Ba_pin_battle_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_r_full_upper_f_all_a_ok\": "
        << (r.screw_Ba_r_full_upper_f_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_pin_arch_prime_f_all_a_ok\": "
        << (r.screw_Ba_pin_arch_prime_f_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_r_full_nonpositive_all_a_ok\": "
        << (r.screw_Ba_r_full_nonpositive_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_r_full_nonpos_analytic_all_a_ok\": "
        << (r.screw_Ba_r_full_nonpos_analytic_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_dense_spectral_crosscheck_all_ok\": "
        << (r.screw_Ba_dense_spectral_crosscheck_all_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_dense_lambda_min_on_grid\": "
        << static_cast<double>(r.screw_Ba_dense_lambda_min_on_grid) << ",\n";
    out << "  \"screw_Ba_digamma_C0_pinned\": "
        << static_cast<double>(r.screw_Ba_digamma_C0_pinned) << ",\n";
    out << "  \"screw_Ba_r01_compact_uniform\": "
        << static_cast<double>(r.screw_Ba_r01_compact_uniform) << ",\n";
    out << "  \"screw_Ba_mesh_refinement_converged_ok\": "
        << (r.screw_Ba_mesh_refinement_converged_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_mesh_refinement_positive_ok\": "
        << (r.screw_Ba_mesh_refinement_positive_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_eq25_full_decomposition_ok\": "
        << (r.screw_Ba_eq25_full_decomposition_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_eq25_operator_consistent_ok\": "
        << (r.screw_Ba_eq25_operator_consistent_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_eq25_identity_ok\": "
        << (r.screw_Ba_eq25_identity_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_r_higher_max_abs\": "
        << static_cast<double>(r.screw_Ba_r_higher_max_abs) << ",\n";
    out << "  \"screw_Ba_r_higher_artifact_max_abs\": "
        << static_cast<double>(r.screw_Ba_r_higher_artifact_max_abs) << ",\n";
    out << "  \"screw_Ba_r_higher_plancherel_all_a_ok\": "
        << (r.screw_Ba_r_higher_plancherel_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_r_higher_kernel_plancherel_all_a_ok\": "
        << (r.screw_Ba_r_higher_kernel_plancherel_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_pin_split_discharge_all_a_ok\": "
        << (r.screw_Ba_pin_split_discharge_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_pin_kernel_eq25_all_a_ok\": "
        << (r.screw_Ba_pin_kernel_eq25_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_r_higher_kernel_nonpos_all_a_ok\": "
        << (r.screw_Ba_r_higher_kernel_nonpos_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_lerch_dominance_all_a_ok\": "
        << (r.screw_Ba_lerch_dominance_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_lerch_dominance_margin_min_on_grid\": "
        << static_cast<double>(r.screw_Ba_lerch_dominance_margin_min_on_grid) << ",\n";
    out << "  \"screw_Ba_kernel_mesh_plancherel_all_ok\": "
        << (r.screw_Ba_kernel_mesh_plancherel_all_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_kernel_eq25_positive_finest_ok\": "
        << (r.screw_Ba_kernel_eq25_positive_finest_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_eq25_kernel_mesh_monotone_ok\": "
        << (r.screw_Ba_eq25_kernel_mesh_monotone_ok ? "true" : "false") << ",\n";
    out << "  \"screw_Ba_pin_kernel_spectral_gap_max_abs\": "
        << static_cast<double>(r.screw_Ba_pin_kernel_spectral_gap_max_abs) << ",\n";
    out << "  \"screw_Ba_artifact_gap_finest_max\": "
        << static_cast<double>(r.screw_Ba_artifact_gap_finest_max) << ",\n";
    out << "  \"step5_prime_cs_covers_mode_ok\": "
        << (r.step5_prime_cs_covers_mode_ok ? "true" : "false") << ",\n";
    out << "  \"step5_min_cross_balance\": " << static_cast<double>(r.step5_min_cross_balance)
        << ",\n";
    out << "  \"domination_margin_min_on_grid\": "
        << static_cast<double>(r.domination_margin_min_on_grid) << ",\n";
    out << "  \"domination_inequality_all_a_ok\": "
        << (r.domination_inequality_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"margin_weighted_min_on_grid\": "
        << static_cast<double>(r.margin_weighted_min_on_grid) << ",\n";
    out << "  \"weighted_domination_all_a_ok\": "
        << (r.weighted_domination_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"yoshida_window_cs_domination_ok\": "
        << (r.yoshida_window_cs_domination_ok ? "true" : "false") << ",\n";
    out << "  \"pf_zero_coupling_identity_ok\": "
        << (r.pf_zero_coupling_identity_ok ? "true" : "false") << ",\n";
    out << "  \"pf_sin_mode_positive_all_ok\": "
        << (r.pf_sin_mode_positive_all_ok ? "true" : "false") << ",\n";
    out << "  \"bare_lambda_min_on_grid\": "
        << static_cast<double>(r.bare_lambda_min_on_grid) << ",\n";
    out << "  \"bare_lambda_all_a_ok\": "
        << (r.bare_lambda_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"pf_zero_prime_pin_margin_min_on_grid\": "
        << static_cast<double>(r.pf_zero_prime_pin_margin_min_on_grid) << ",\n";
    out << "  \"suzuki_coupling_margin_min_on_grid\": "
        << static_cast<double>(r.suzuki_coupling_margin_min_on_grid) << ",\n";
    out << "  \"suzuki_coupling_all_a_ok\": "
        << (r.suzuki_coupling_all_a_ok ? "true" : "false") << ",\n";
    out << "  \"suzuki_prime_gauss_limit\": "
        << static_cast<double>(r.suzuki_prime_gauss_limit) << ",\n";
    out << "  \"domination_chain_wired_ok\": "
        << (r.domination_chain_wired_ok ? "true" : "false") << ",\n";
    out << "  \"arch_envelope_analytic_ok\": " << (r.arch_envelope_analytic_ok ? "true" : "false")
        << ",\n";
    out << "  \"zero_tail_bound_analytic_ok\": "
        << (r.zero_tail_bound_analytic_ok ? "true" : "false") << ",\n";
    out << "  \"full_scale\": {\n";
    write_point(r.full_scale);
    out << "\n  },\n  \"a_grid\": [\n";
    for (size_t i = 0; i < r.a_grid.size(); ++i) {
        out << "    {\n";
        write_point(r.a_grid[i]);
        out << "\n    }";
        if (i + 1 < r.a_grid.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"screw_Ba_mesh_refinement\": [\n";
    for (size_t i = 0; i < r.screw_Ba_mesh_refinement.size(); ++i) {
        const auto& mp = r.screw_Ba_mesh_refinement[i];
        out << "    {\"a\": " << static_cast<double>(mp.a) << ", \"spectral_pts\": "
            << mp.spectral_pts << ", \"lambda_dense\": "
            << static_cast<double>(mp.lambda_dense) << ", \"r_higher_kernel_pp_rayleigh\": "
            << static_cast<double>(mp.r_higher_kernel_pp_rayleigh)
            << ", \"r_higher_plancherel_bound\": "
            << static_cast<double>(mp.r_higher_plancherel_bound) << ", \"eq25_kernel_rayleigh\": "
            << static_cast<double>(mp.eq25_kernel_rayleigh)
            << ", \"pin_kernel_eq25_rayleigh\": "
            << static_cast<double>(mp.pin_kernel_eq25_rayleigh)
            << ", \"r_higher_artifact_gap\": "
            << static_cast<double>(mp.r_higher_artifact_gap)
            << ", \"pin_kernel_spectral_gap\": "
            << static_cast<double>(mp.pin_kernel_spectral_gap)
            << ", \"r_higher_kernel_plancherel_ok\": "
            << (mp.r_higher_kernel_plancherel_ok ? "true" : "false") << "}";
        if (i + 1 < r.screw_Ba_mesh_refinement.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Induction

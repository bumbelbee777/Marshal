#pragma once

#include "CrossSectorWeilOperator.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Induction {

/// Suzuki 2606.09096 (1.3): even screw function g associated with ζ.
Real screw_function_g(Real t, const Heat::PrimeCatalog& cat, Real eps);

/// Nonnegative-definite screw kernel G_g(t,u) = g(t-u) - g(t) - g(u)  (g even, g(0)=0).
Real screw_kernel_Gg(Real x, Real y, const Heat::PrimeCatalog& cat, Real eps);

/// Rayleigh min over sin modes on G_g kernel (closer to Suzuki Weil form than Pf+δ proxy).
Real weil_rayleigh_screw_Gg(Real a, const Heat::PrimeCatalog& cat, Real eps,
                            const WeilOperatorRayleighOpts& opts = {});

/// Suzuki 2606.09096 (2.5): Q_{B_a}(v) on H0^1 — explicit L_a + mass + prime-shift form.
Real screw_Ba_quadratic(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                        int n_quad, Real scale);

/// Rayleigh min of Q_{B_a}/||v||^2 over sin modes (true Suzuki λ_a proxy).
Real weil_rayleigh_screw_Ba(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps,
                            const WeilOperatorRayleighOpts& opts = {});

/// Smallest eigenvalue of discrete B_a = (d/dx) G_a (d/dx) with P_a projection.
Real weil_spectral_min_screw_Ba(Real a, const Heat::PrimeCatalog& cat, Real eps,
                                const WeilOperatorRayleighOpts& opts = {});

/// Suzuki (2.5) audit on the spectral ground-state vector: full term ledger.
struct ScrewBaSpectralAudit {
    Real spectral_lambda = 0;
    Real eq25_rayleigh = 0;
    Real remainder_rayleigh = 0;
    Real eq25_rel_gap = 0;
    Real full_eq25_rayleigh = 0;
    Real discretization_gap = 0;
    Real arch_mass_rayleigh = 0;
    Real prime_rayleigh = 0;
    Real r0pp_rayleigh = 0;
    Real rpp_rayleigh = 0;
    Real r01pp_rayleigh = 0;
    Real r_full_pp_rayleigh = 0;
    Real r_higher_pp_rayleigh = 0;
    Real r_higher_kernel_pp_rayleigh = 0;
    Real L_arch_rayleigh = 0;
    Real r_higher_plancherel_bound = 0;
    Real pin_kernel_eq25_rayleigh = 0;
    Real pin_kernel_spectral_gap = 0;
    Real eq45_log_a_rayleigh = 0;
    Real pin_margin_rayleigh = 0;
    Real L_a = 0;
    Real l2 = 0;
    Real mass_term = 0;
    Real prime_shift = 0;
    Real r0_pp = 0;
    Real r_pp = 0;
    Real r01_pp = 0;
    Real r_full_pp = 0;
    Real r_higher_pp = 0;
    Real r_higher_kernel_bilinear = 0;
};

ScrewBaSpectralAudit screw_Ba_spectral_eq25_audit(Real a, const Heat::PrimeCatalog& cat,
                                                  Real log_cutoff, Real eps,
                                                  const WeilOperatorRayleighOpts& opts = {});

/// Hilbert–Schmidt norm of a translation-invariant kernel K on [-a,a]: sqrt((2a) ∫_{-2a}^{2a} K(t)² dt).
Real screw_kernel_hilbert_schmidt_on_interval(Real a, Real (*kernel)(Real));

Real screw_kernel_hilbert_schmidt_remainder(Real a, const Heat::PrimeCatalog& cat, Real eps);

/// Dense vs iterative B_a spectral cross-check (m <= 512).
struct ScrewBaSpectralCrosscheck {
    int spectral_pts = 0;
    bool dense_available = false;
    bool agrees = false;
    Real lambda_dense = 0;
    Real lambda_iterative = 0;
};

ScrewBaSpectralCrosscheck screw_Ba_spectral_dense_crosscheck(Real a, const Heat::PrimeCatalog& cat,
                                                             Real eps, int spectral_pts);

/// Suzuki eq. (2.7) Fourier O(|z|^{-2}) => |r_higher|/||v||^2 <= C0 * a (pinned C0).
Real screw_r_higher_digamma_fourier_C0_pinned();
Real screw_r_higher_digamma_ol2_rayleigh_bound(Real a);
Real screw_r01_compact_uniform_rayleigh_bound();
/// Lemma 2 sharp majorant: R01_COMPACT + R01_SHARP_C_A * a (pinned 50 + 16a).
Real screw_r01_sharp_rayleigh_bound(Real a);

/// Prime at B_a minimizer: unconditional 2*S_fin; saturated 2*prime_sat for a >= 3.
Real screw_Ba_prime_minimizer_cs_upper(Real S_fin);
Real screw_Ba_prime_saturated_upper(Real prime_S_fin_sat);
Real screw_Ba_prime_analytic_upper(Real a, Real S_fin, Real prime_S_fin_sat);

/// Suzuki 4.4 / eq. (4.5): f(a) = compact r_01 + digamma O(a) upper bound on r_full/||v||^2.
Real screw_Ba_r_full_analytic_f(Real a);

/// Suzuki 4.4 finite-sum split: |bar_q^1| <= prime_sat + f(a) (not flat C=12).
Real screw_Ba_bar_q1_analytic_upper(Real a, Real prime_S_fin_sat);

/// H^log seminorm proxy: L(w)/||w||^2 from eq. (4.4)/(4.6) on the grid mode.
Real screw_Ba_L_arch_rayleigh_on_grid(Real a, const std::vector<Real>& v, Real dx);

/// Mode-weighted r_higher majorant: pi C0 a * max(1, L/||v||^2) (Plancherel H^log route).
Real screw_Ba_r_higher_plancherel_analytic_bound(Real a, Real L_arch_rayleigh);

/// Split discharge margin: arch - prime_sat - r01_compact - r_higher_plancherel.
Real screw_Ba_pin_arch_prime_r01_rhigher_lower_bound(Real arch_mass, Real prime_upper, Real a,
                                                     Real L_arch_rayleigh);

/// When r_higher_kernel <= 0: pin_kernel >= arch - prime_upper - r01 - r_higher_kernel.
Real screw_Ba_pin_kernel_boost_lower_bound(Real arch_mass, Real prime_upper, Real r01_pp,
                                           Real r_higher_kernel_pp);

/// Lerch boost: H = -r_higher_kernel must dominate prime_upper + r01 - arch for pin >= 0.
Real screw_Ba_lerch_dominance_margin(Real arch_mass, Real prime_upper, Real r01_pp,
                                     Real r_higher_kernel_pp);

/// Analytic eq. (2.5) lower-bound ledger at the spectral minimizer (conservative majorants).
struct ScrewBaEq25AnalyticLedger {
    Real arch_mass_rayleigh = 0;
    Real prime_rayleigh = 0;
    Real r01pp_rayleigh = 0;
    Real r_higher_pp_rayleigh = 0;
    Real r_full_pp_rayleigh = 0;
    Real spectral_lambda = 0;
    Real prime_minimizer_cs_upper = 0;
    Real prime_saturated_upper = 0;
    Real r01_abs_bound = 0;
    Real r_higher_hs_bound = 0;
    Real r_higher_digamma_bound = 0;
    Real r_full_analytic_f = 0;
    Real r_full_upper_bound = 0;
    Real r_full_hs_upper_bound = 0;
    Real prime_upper_bound = 0;
    Real pin_analytic_lower_bound = 0;
    Real pin_analytic_digamma_lower_bound = 0;
    Real pin_battle_lower_bound = 0;
    Real pin_arch_prime_f_lower_bound = 0;
    Real pin_split_r01_rhigher_lower_bound = 0;
    Real pin_kernel_eq25_rayleigh = 0;
    Real pin_kernel_spectral_gap = 0;
    Real r_higher_plancherel_bound = 0;
    Real r_higher_artifact_gap = 0;
    Real r01_sharp_rayleigh = 0;
    Real lerch_boost_H = 0;
    Real lerch_dominance_debt = 0;
    Real lerch_dominance_debt_sharp = 0;
    Real lerch_dominance_margin = 0;
    Real L_arch_rayleigh = 0;
    Real bar_q1_analytic_upper = 0;
    Real pin_bar_q1_lower_bound = 0;
    bool r_full_upper_f_ok = false;
    bool r_higher_plancherel_ok = false;
    bool r_higher_kernel_plancherel_ok = false;
    bool pin_kernel_eq25_ok = false;
    bool r_higher_kernel_nonpos_ok = false;
    bool lerch_dominance_ok = false;
    bool pin_split_discharge_ok = false;
    bool pin_arch_prime_f_ok = false;
    bool r_full_nonpos_analytic_ok = false;
    bool pin_analytic_ok = false;
};

struct ScrewBaSpectralPack {
    ScrewBaSpectralAudit audit;
    ScrewBaEq25AnalyticLedger ledger;
    Real lambda_dense = 0;
    bool dense_crosscheck_ok = true;
};

/// Single matrix build + eigenpair + eq25 audit + battle ledger (study hot path).
ScrewBaSpectralPack screw_Ba_spectral_pack(Real a, const Heat::PrimeCatalog& cat, Real log_cutoff,
                                           Real eps, Real prime_S_fin_sat, Real S_fin,
                                           const WeilOperatorRayleighOpts& opts = {});

ScrewBaSpectralAudit screw_Ba_audit_from_eigenvector(Real a, Real mu, const std::vector<Real>& v,
                                                     Real dx, const Heat::PrimeCatalog& cat,
                                                     Real log_cutoff, Real eps);

ScrewBaEq25AnalyticLedger screw_Ba_eq25_ledger_from_audit(const ScrewBaSpectralAudit& audit, Real a,
                                                        Real prime_S_fin_sat, Real S_fin);

struct ScrewBaMeshRefinementPoint {
    int spectral_pts = 0;
    Real lambda_dense = 0;
    Real r_higher_kernel_pp_rayleigh = 0;
    Real r_higher_plancherel_bound = 0;
    Real eq25_kernel_rayleigh = 0;
    Real pin_kernel_eq25_rayleigh = 0;
    Real r_higher_artifact_gap = 0;
    Real pin_kernel_spectral_gap = 0;
    bool r_higher_kernel_plancherel_ok = false;
};

struct ScrewBaMeshRefinement {
    Real a = 0;
    std::vector<ScrewBaMeshRefinementPoint> points;
    Real lambda_coarsest = 0;
    Real lambda_finest = 0;
    Real max_lambda_jump = 0;
    Real max_kernel_pp_jump = 0;
    Real eq25_kernel_finest = 0;
    bool monotone_converged = true;
    bool kernel_plancherel_all_meshes_ok = true;
    bool eq25_kernel_positive_finest_ok = false;
    bool eq25_kernel_mesh_monotone_ok = true;
    Real max_artifact_gap_jump = 0;
    Real artifact_gap_finest = 0;
    bool converged_positive_ok = false;
};

ScrewBaMeshRefinement screw_Ba_mesh_refinement_study(Real a, const Heat::PrimeCatalog& cat,
                                                     Real eps);

/// Kernel-focused mesh study at battle-a: audits r_higher_kernel Plancherel + eq25 kernel path.
ScrewBaMeshRefinement screw_Ba_kernel_mesh_refinement_study(
    Real a, const Heat::PrimeCatalog& cat, Real log_cutoff, Real eps, Real prime_S_fin_sat,
    Real S_fin, int spectral_pts_cap = 512);

ScrewBaEq25AnalyticLedger screw_Ba_eq25_analytic_ledger(Real a, const Heat::PrimeCatalog& cat,
                                                        Real log_cutoff, Real eps,
                                                        Real prime_S_fin_sat, Real S_fin,
                                                        const WeilOperatorRayleighOpts& opts = {});

int screw_Ba_adaptive_spectral_pts(Real a, int base_pts, int max_pts = 512);

}  // namespace Marshal::Induction

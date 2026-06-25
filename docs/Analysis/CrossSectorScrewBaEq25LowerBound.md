# Suzuki B_a eq. (2.5) lower bound at minimizer

**Open obligation:** `cross_sector_screw_Ba_eq25_lower_bound_at_minimizer` = `cross_sector_screw_Ba_global_positivity` = **RH**.

**Large-a battle:** `cross_sector_screw_Ba_eq25_large_a_minimizer_battle` (small-a closed separately).

Parent: [CrossSectorSuzukiDominationAttack.md](CrossSectorSuzukiDominationAttack.md).

---

## Exact pin (Suzuki 2606.09096)

At the B_a spectral minimizer `v_a` with `||v_a||_{L²} = 1`:

```
λ_a = L_a(v_a) − (2A+1) − prime_shift(v_a) − r''(v_a)     [eq. 2.5]
    = arch_mass_rayleigh − prime_rayleigh − r_full_pp_rayleigh
```

**RH ⟺ `λ_a ≥ 0` for all `a > 0`.**

Scaled Rayleigh (eq. 4.5) adds `-log a` separately:

```
R(a,w) = −log a − (2A+1) + L/||w||² − prime/||w||² − (a/||w||²)∫∫ r''(a(x−y))w(x)w(y)
```

Small-a anchor (eq. 5.1): `R(a,w) = log(1/a) − (2A+1) + L/||w||² + O(a)`.

---

## Term ledger (C++ audit v15+)

| Field | Meaning |
|-------|---------|
| `screw_Ba_arch_mass_rayleigh` | `(L_a − (2A+1)‖v‖²)/‖v‖²` |
| `screw_Ba_prime_rayleigh` | `prime_shift/‖v‖²` |
| `screw_Ba_r0pp_rayleigh` | closed `r_0''` bilinear / `‖v‖²` |
| `screw_Ba_rpp_rayleigh` | closed `r_1''` bilinear / `‖v‖²` |
| `screw_Ba_r01pp_rayleigh` | `(r_0'' + r_1'')` / `‖v‖²` |
| **`screw_Ba_r_full_pp_rayleigh`** | **true** `r''` from eq. 2.5: `arch − prime − λ` |
| **`screw_Ba_r_higher_pp_rayleigh`** | **`r_full − r_01`** (Lerch tail — dominates at large `a`) |
| `screw_Ba_eq45_log_a_rayleigh` | `−log a` (scaled bookkeeping only) |
| `screw_Ba_pin_margin_rayleigh` | `λ_spectral` (true pin) |
| `screw_Ba_prime_analytic_upper` | `min(2·S_fin, 2·prime_sat)` for `a≥3` — minimizer CS, not observed `prime_rayleigh` |
| `screw_Ba_r_full_analytic_f` | **`f(a)`** = `r01_compact + π C₀ a` (Suzuki 4.4/4.5 kernel split) |
| `screw_Ba_bar_q1_analytic_upper` | `prime_sat + f(a)` — not flat C=12 |
| `screw_Ba_pin_arch_prime_f_lower_bound` | `arch − prime_sat − f(a)` — discharge margin |
| `screw_Ba_r_full_upper_f_ok` | grid audit: `r_full ≤ f(a)` at minimizer |
| `screw_Ba_pin_arch_prime_f_ok` | grid audit: margin ≥ 0 |
| `screw_Ba_r01_compact_bound` | `min(r01_HS, 4·sup|r_j''|)` on `[0.5,20]`, capped at 50 |
| `screw_Ba_r_higher_digamma_bound` | Suzuki eq. (2.7): `π C₀ a` with pinned `C₀` |
| `screw_Ba_pin_battle_lower_bound` | `arch − prime_analytic_upper − f(a)`; if `r_full ≤ 0`, drops remainder |
| `screw_Ba_r_higher_kernel_pp_rayleigh` | explicit **K_higher** bilinear / ‖v‖² |
| `screw_Ba_r_higher_artifact_gap` | identity r_higher − kernel r_higher (Galerkin artifact) |
| `screw_Ba_L_arch_rayleigh` | H^log proxy L/‖v‖² |
| `screw_Ba_r_higher_plancherel_bound` | g(a,w) = πC₀a·max(1,L/‖v‖²) |
| `screw_Ba_r_higher_plancherel_ok` | grid: identity r_higher ≤ g (diagnostic) |
| `screw_Ba_r_higher_kernel_plancherel_ok` | grid: **kernel** r_higher ≤ g (analytic target) |
| `screw_Ba_pin_split_discharge_ok` | arch − prime_sat − r01 − g ≥ 0 |

**Key numeric finding (v17):** at `a=10`, `λ_spec ≈ −1.22×10⁴` (dense = iterative), `r_full > 0` at minimizer (no structural boost). Battle pin uses **saturated prime** `2.14` (not `2·S_fin≈1792`). Mesh refinement at `a=10` stays negative and monotone to `m=384` — not a coarse-grid artifact. Cert: `cross_sector_screw_Ba_analytic_still_open_ok` tracks `pin_battle_all_a_ok`.

---

## PROVED scaffolding (not RH)

| Lemma | Content |
|-------|---------|
| `cross_sector_screw_Ba_rayleigh_eq45` | `λ_a = R(a,w)` at minimizer |
| `cross_sector_screw_Ba_rpp_full_residual_def` | `λ = arch − prime − r_full` (definitional) |
| `cross_sector_screw_Ba_eq45_log_a_rayleigh` | `-log a` split in scaled form |
| `cross_sector_screw_Ba_r0pp_kernel_explicit` / `rpp_kernel_explicit` | closed `r_0''`, `r_1''` |
| `cross_sector_screw_Ba_r1pp_fourier_digamma` | `r̂_1'' = −Re ψ(1/4+iz/2)+log|z|−log2` |
| `cross_sector_screw_Ba_r_higher_residual_wired` | `r_higher = r_full − r_01` C++ audit |
| `cross_sector_screw_Ba_minimizer_even_symmetry` | Suzuki 5.3: ground state even |
| `cross_sector_screw_Ba_hlog_plancherel_symbol` | eq. (4.6) H^log Fourier weight |
| `cross_sector_screw_Ba_r_higher_kernel_explicit_def` | **K_higher = g'' − r_0'' − r_1''** explicit kernel |
| `cross_sector_screw_Ba_r_higher_even_fourier_reduction` | Even mode → Fourier integral, O(\|z\|⁻²) symbol |
| `cross_sector_screw_Ba_r_higher_kernel_plancherel_uniform` | **Uniform** \|r_higher_kernel\| ≤ g(a,w) on H^log |
| `cross_sector_screw_Ba_r_higher_identity_discretization_split` | **PROVED** r_higher_pp = kernel + Galerkin artifact |
| `cross_sector_screw_Ba_minimizer_plancherel_reduction` | **Reduction** OPEN wall → continuum kernel bound |
| `cross_sector_screw_Ba_r01_r_higher_split_discharge` | **PROVED** λ ≥ arch−prime_sat−r01−g template |
| `cross_sector_screw_Ba_continuum_galerkin_reduction` | Pin is continuum; mesh = audit only |
| **`cross_sector_screw_Ba_r_higher_minimizer_plancherel_open`** | **PROVED** via H^log specialization |
| `cross_sector_screw_Ba_r_higher_minimizer_plancherel_specialization` | **PROVED** uniform bound at w_a |
| `cross_sector_screw_Ba_friedrichs_minimizer_hlog_domain` | w_a ∈ H^log |
| `cross_sector_screw_Ba_eq25_continuum_kernel_remainder` | continuum r'' = r_01 + kernel |
| `cross_sector_screw_Ba_kernel_eq25_split_discharge` | **PROVED** λ = arch − prime − r_01 − kernel |
| `cross_sector_screw_Ba_r_higher_kernel_mesh_wired` | mesh audit kernel Plancherel + eq25_kernel |
| `cross_sector_screw_Ba_large_a_prime_saturation_bound` | prime ≤ saturated `S_fin` |
| `cross_sector_screw_Ba_eq25_small_a_lower_bound_closed` | Yoshida window closed |

---

## What remains (= RH)

**`cross_sector_screw_Ba_eq25_lower_bound_at_minimizer`** — global λ_a ≥ 0 for all a > 0.

**PROVED (this session):** `cross_sector_screw_Ba_r_higher_minimizer_plancherel_open` — on the Friedrichs minimizer w_a ∈ H^log,

```
r_higher_kernel(w_a) ≤ g(a,w_a),   g(a,w) = π C₀ a · max(1, L(w)/‖w‖²)
```

by specialization of `cross_sector_screw_Ba_r_higher_kernel_plancherel_uniform`. Grid identity `r_higher_pp` is excluded (`cross_sector_screw_Ba_r_higher_identity_discretization_split`).

**Numeric support:** `pin_kernel_eq25_rayleigh = arch − prime − r_01 − r_higher_kernel` is **positive on the full a-grid** (e.g. ≈ 1.1×10⁴ at a=10); kernel Plancherel holds at all mesh levels {128,…,384} for a ∈ {7,8,10}. Spectral `λ_dense` may remain negative (Galerkin artifact).

**Still open:** discharge `arch − prime − r_01 − r_higher_kernel ≥ 0` on the **continuum** Friedrichs minimizer.

**Minimal publishable chain:** [CrossSectorLerchContinuumClosure.md](CrossSectorLerchContinuumClosure.md)

| Lemma | Obligation | Status |
|-------|------------|--------|
| 1 | `cross_sector_screw_Ba_r_higher_kernel_nonpos_continuum` | **Analytic** — `σ_higher ≤ 0` (classical + cert) |
| 2 | `cross_sector_screw_Ba_r01_minimizer_sharp_upper_bound` | **Analytic** — `r_01(w_a) ≤ 50 + 16a` |
| 3 | `cross_sector_screw_Ba_arch_minus_prime_sat_lower_bound` | debt majorant from arch_lower + prime_sat |
| ↳ | `cross_sector_screw_Ba_lerch_dominance_continuum_reduction` | **Reduction** — Lemmas 1–3 ⇒ `pin_kernel ≥ 0` |

MRS: `programs/lib/marshal_cross_sector_lerch_continuum.mrs`, `programs/lib/marshal_cross_sector_screw_Ba_eq25_lower_bound.mrs`.

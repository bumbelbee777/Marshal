# Phase 5.6 — Suzuki B_a analytic pin (eq. 2.5)

**Open obligation:** `cross_sector_screw_Ba_global_positivity` = `suzuki_arithmetic_prime_limit_control` = **`λ_a ≥ 0 ∀a`** on the true Suzuki **`B_a`** operator = **RH** (Suzuki 2606.09096 Conj. 1.12 / eq. 1.12).

Parent: [CrossSectorStep5RayleighLowerBound.md](CrossSectorStep5RayleighLowerBound.md).

---

## Correct operator (not legacy proxy)

| Audit | What it measures | RH pin? |
|-------|------------------|---------|
| `bare_lambda_at_mode` | Pf+prime+zero sin-mode proxy | **Wrong** — crosses ~a≥1.5 |
| `lambda_screw_Gg_rayleigh` | Full `G_g` kernel sin Rayleigh | **Wrong** — crosses ~a≥8 |
| `lambda_screw_Ba_rayleigh` | eq. (2.5) sin modes only | **Incomplete** — negative at large a |
| **`lambda_screw_Ba_spectral`** | Discrete **`B_a = D G_a D`** with **`P_a`** | **Correct numeric target** — positive entire grid |

Classical: Suzuki Thm **1.1** — `Q_W^a = Q_{B_a}` on `H₀¹`. The RH wall is **`λ_a = min Q_{B_a}/‖v‖² ≥ 0`**.

---

## eq. (2.5) analytic decomposition (PROVED classical)

```
Q_{B_a}(v) = L_a(v) − (2A+1)‖v‖² − prime_shift(v) − r''(v)
```

| Term | Meaning |
|------|---------|
| `L_a − (2A+1)‖v‖²` | Arch + \|t\|log\|t\| Pf block (unconditional lower bound target) |
| `prime_shift` | Explicit von Mangoldt bilinear (growing with a) |
| `r''` | Remainder — **explicit analytic open term** |

**Discharge target:** at the spectral minimizer `v_a`, prove  
`prime_shift(v_a) + r''(v_a) ≤ L_a(v_a) − (2A+1)‖v_a‖²` for all `a > 0`.

Detail: [CrossSectorScrewBaEq25LowerBound.md](CrossSectorScrewBaEq25LowerBound.md). Small-a closed (`cross_sector_screw_Ba_eq25_small_a_lower_bound_closed`); **open = large-a battle** (`cross_sector_screw_Ba_eq25_large_a_minimizer_battle`).

---

## What is PROVED (not RH)

| Lemma | Status |
|-------|--------|
| `cross_sector_screw_Ba_weil_operator_identity` | **PROVED** — Suzuki 1.1 |
| `cross_sector_screw_Ba_eq25_decomposition` | **PROVED** — Suzuki 2.5 |
| `cross_sector_screw_Ba_yoshida_small_a` | **PROVED** — Yoshida + Suzuki 1.4 |
| `cross_sector_screw_Ba_operator_wired` | **PROVED** — C++ spectral `B_a` |
| `cross_sector_screw_Ba_eq25_operator_consistency` | **PROVED** — eq25 + remainder decomposition wired |
| Legacy Pf+zero coupling chain | **PROVED** scaffolding (proxy pin still open) |

**Do not close** `cross_sector_screw_Ba_analytic_still_open_ok` from grid positivity alone.

---

## Grid audit (σ=1, v12)

| Quantity | Yoshida (`a≤1`) | Full grid |
|----------|-----------------|-----------|
| `lambda_screw_Ba_spectral` | **> 0** | **> 0** (min ≈ **0.26** @ a=0.25) |
| `bare_lambda_min` (legacy) | **> 0** | crosses **< 0** ~a≥1.5 |
| `screw_Ba_eq25_operator_consistent_ok` | **yes** | eq25 tracks spectral eigenvector |

Cert: `cross_sector_screw_Ba_spectral_yoshida_ok`, `cross_sector_screw_Ba_eq25_operator_consistent_ok` (decomposition wired), `cross_sector_screw_Ba_analytic_still_open_ok`.

---

## Reproducibility

```bash
cmake --build build --target Marshal verify-mrs-proof
python tools/Analysis/RunCrossSectorWeilStudy.py --precision
python tools/Analysis/EmitCrossSectorWeilBattlePlanCert.py --check
```

MRS module: `programs/lib/marshal_cross_sector_screw_Ba_analytic.mrs`.

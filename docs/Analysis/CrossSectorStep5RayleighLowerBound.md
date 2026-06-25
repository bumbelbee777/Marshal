# Cross-sector Step 5 — global Rayleigh lower bound

**Pin:** `cross_sector_step5_global_rayleigh_lower_bound` ≡ `suzuki_arithmetic_prime_limit_control` ≡ RH.

Parent: [CrossSectorWeilPositivityBattlePlan.md](CrossSectorWeilPositivityBattlePlan.md).

---

## Sector domination chain

For each `a > 0`, let `T = 2a`, `S_fin(a) = Σ_{n≤exp(2a)} Λ(n)/√n`, `A(a)` = arch triangle envelope (2A), `Z(a,T)` = RvM zero-tail majorant (3A).

| Sector | Bound | MRS obligation | Status |
|--------|-------|----------------|--------|
| Prime | `Q_prime^a(f) ≤ 2 S_fin(a) ‖f‖²` | `cross_sector_quadratic_prime_cs_bound` | **PROVED** (CS) |
| Arch | `Q_arch^a(f) ≥ −A(a) ‖f‖²` | `cross_sector_quadratic_arch_lower_bound` | **PROVED** (2A lift) |
| Zero | `Q_zero^a(f) ≥ −Z(a,T) ‖f‖²` | `cross_sector_quadratic_zero_lower_bound` | **PROVED** (3A lift) |

Composition: `cross_sector_domination_chain`.

**Domination margin (audited in C++):**

```
margin(a) = A(a) − S_fin(a) − Z(a,T)
```

**Domination condition:** `S_fin(a) + Z(a,T) ≤ A(a)` for all `a > 0` ⟺ `margin(a) ≥ 0` everywhere.

**Note on constants:** The CS prime bound uses `2 S_fin` (conservative pairing majorant). The domination inequality uses `S_fin` directly — the screw-kernel decomposition tracks prime mass via `cross_balance_mode = q_pf + q_zero − q_prime` at the Rayleigh-min mode; the open pin is the linear arithmetic weight `S_fin` against the arch envelope `A(a)`.

---

## Conditional theorem (PROVED implication)

If `S_fin(a) + Z(a,T) ≤ A(a)` for **all** `a > 0`, then:

1. Sector bounds compose to `Q_W^a(f) ≥ margin(a) ‖f‖² ≥ 0` for all unit `f`
2. Hence `λ_a ≥ 0` for all `a`
3. Yoshida anchor + Suzuki continuity ⇒ **RH**

MRS: `cross_sector_domination_implies_lambda_nonneg`.

---

## Reduction chain (PROVED)

| Step | Obligation | Role |
|------|------------|------|
| 5.1 | `cross_sector_quadratic_prime_cs_bound` | Unconditional prime upper bound |
| 5.2 | `cross_sector_step5_sector_decomposition` | Quadratic sector audit at Rayleigh-min mode |
| 5.3 | `cross_sector_domination_chain` | Wire sector bounds + margin |
| 5.4 | `cross_sector_domination_implies_lambda_nonneg` | `(S_fin+Z≤A ∀a) ⇒ λ_a≥0` |
| 5.5 | `cross_sector_step5_zero_crossing_propagation` | Global `λ≥0` ⟺ no zero crossing |
| 5.6 | `cross_sector_step5_global_rayleigh_lower_bound` | **OPEN** — discharge domination |
| Pin | `suzuki_arithmetic_prime_limit_control` | **OPEN ≡ RH** |

Attack spine: [CrossSectorSuzukiDominationAttack.md](CrossSectorSuzukiDominationAttack.md).

---

## Open pin (= RH attack surface)

**`suzuki_arithmetic_prime_limit_control`** = prove the domination inequality on the **full** localization range.

Current C++ grid audit: `domination_inequality_all_a_ok: false`. Margin is positive on the sampled grid for `a ≤ 0.5` (Yoshida region), then turns negative once `S_fin(a)` exceeds `A(a)` (~`a ≥ 0.75`). At large `a`, `S_fin` saturates ~O(10²–10³) while `A(a)` plateaus ~0.83; `Z(a,T)` uses the 3A RvM majorant only for `T = 2a ≥ 14`.

This pinpoints exactly what analytic theorem must close RH — not a retreat from it.

---

## Reproducibility

```bash
cmake --build build --target Marshal
python tools/Analysis/RunCrossSectorWeilStudy.py --precision
python tools/Analysis/EmitCrossSectorWeilBattlePlanCert.py --check
python tools/Analysis/CrossSectorWeilGridStudy.py
cmake --build build --target verify-mrs-proof
```

MRS module: `programs/lib/marshal_cross_sector_step5.mrs`.

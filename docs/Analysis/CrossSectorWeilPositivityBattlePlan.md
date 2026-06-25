# Cross-sector Weil positivity — verified battle plan



**Status:** Phases **2A + 3A PROVED (unconditional)**. Phase **4A operator attack wired; positivity OPEN ≡ RH**.



**Canonical pin:** `weil_localized_form_positivity_all_a` ⟺ `λ_a ≥ 0` for all `a > 0`.



Cross-links: [CMIGapLedger.md](CMIGapLedger.md) §5.1.8, [WeilTraceDuality.md](WeilTraceDuality.md).



---L



## Circularity guard (read this first)



The battle plan **investigates** `λ_a ≥ 0`; it must not **assume** RH while doing so.



| Move | Circular? | Verdict |

|------|-----------|---------|

| Using `weil_localization_equivalence` (RH ⟺ `λ_a>0`) as the **target pin** | No | Names the finish line |

| Using that equivalence as a **lemma** in 2A/3A proofs | **Yes — forbidden** | Never imported as hypothesis |

| Weil trace identity numerics with zero tables | No for **validation** | Tables check bounds; RvM majorants do not use zero locations |

| `weil_margin_fin → 0` as `a` grows | Misleading | Prime tail convergence only — not positivity |

| Partial `λ_R` (Pf+prime kernel) | Misleading | Goes **negative** for `a≥1.5`; ≠ full `λ_a` |

| Proving 2A+3A then claiming RH | **Yes — unless 4A proved** | 2A+3A bound **linear sector sums**; `λ_a` is **quadratic** |



**After 2A+3A the honest state is:**



> Unconditional arch envelope `W_arch^a ≥ −A(a)`, unconditional zero tail `S_T ≤ B(T)`, convergent prime tail — but **quadratic cross-sector domination** in `Q_W^a` is still the RH pin.



That is narrowing, not circular closure.



---



## Cross-sector Phase 4A — operator attack (June 2026)



**Target:**



$$

\lambda_a = \inf_{\|f\|=1,\,\mathrm{supp}(f)\subset[-a,a]} Q_{W,a}(f),\quad

Q_{W,a}(f)=\iint f(x)f(y)\,K_a(x,y)\,dx\,dy

$$



with `K_a = K_arch + K_prime + K_zero` (Suzuki screw kernel).



| Step | Content | Status |

|------|---------|--------|

| 4A.1 | Construct `T_a` with explicit cross-sector kernel | **PROVED** (Suzuki) + C++ `CrossSectorWeilOperator` |

| 4A.2 | Compactness on `L²[-a,a]` | **PROVED** (Fredholm) |

| 4A.3 | Ground state / infimum attained | **PROVED** (Connes–Consani + Suzuki) |

| 4A.4 | `λ_a` continuous in `a` | **PROVED** (Suzuki Thm 1.3) |

| 4A.5 | Global `λ_a ≥ 0 ∀a` | **OPEN ≡ RH** — see [CrossSectorStep5RayleighLowerBound.md](CrossSectorStep5RayleighLowerBound.md) |

### Step 5 — domination chain (June 2026)

| Step | Obligation | Status |
|------|------------|--------|
| 5.1 | `cross_sector_quadratic_prime_cs_bound` | **PROVED** (CS) |
| 5.2 | `cross_sector_quadratic_arch_lower_bound` / `cross_sector_quadratic_zero_lower_bound` | **PROVED** (2A/3A lifts) |
| 5.3 | `cross_sector_domination_chain` | **PROVED** (composition) |
| 5.4 | `cross_sector_domination_implies_lambda_nonneg` | **PROVED** (conditional) |
| 5.5 | `cross_sector_step5_zero_crossing_propagation` | **PROVED** (reduction) |
| 5.6 | `cross_sector_step5_global_rayleigh_lower_bound` | **OPEN ≡ RH** |
| Pin | `suzuki_arithmetic_prime_limit_control` | **OPEN ≡ RH** (quadratic `λ_a≥0`) |
| Spine | `cross_sector_step5_suzuki_attack_spine` | **PROVED** (reduction) |

Attack detail: [CrossSectorSuzukiDominationAttack.md](CrossSectorSuzukiDominationAttack.md).



**C++ audit (full kernel):** `lambda_full_rayleigh` and `lambda_full_spectral` on Pf+prime+zero discretization. Partial `λ_R` omits zero cos block — negative for `a≥1.5`; full kernel includes zeros and is the honest Phase 4A proxy.



**MRS:** `marshal_cross_sector_phase4a.mrs` — `cross_sector_weil_operator_Ta`, `cross_sector_operator_compact_ground_state`, `cross_sector_rayleigh_characterization`, `cross_sector_phase4a_positivity_pin`.



---



## Proved vs open (updated)



| Component | Status |

|-----------|--------|

| **2A** `W_arch^a ≥ −A(a)` (triangle envelope) | **PROVED** + C++ audit |

| **3A** `S_T ≤ B(T)` (RvM majorant) | **PROVED** + C++ audit |

| Prime tail convergent | **PROVED** |

| Yoshida small-`a` | **PROVED (analytic)** |

| Operator `T_a` + Rayleigh characterization | **PROVED (structural)** |

| Full-kernel Yoshida sample | **CERT** |

| **4A** `λ_a ≥ 0 ∀a` | **OPEN ≡ RH** |



---



## Phase summary



| Phase | Obligation | Status |

|-------|------------|--------|

| 1 | `cross_sector_weil_identity_sigma1` … | **CERT** |

| 2A | `cross_sector_arch_triangle_envelope` | **PROVED** |

| 3A | `cross_sector_zero_tail_rvm_bound` | **PROVED** |

| 4N | `cross_sector_lambda_rayleigh_numeric` | **CERT** (partial kernel) |

| 4N+ | `cross_sector_lambda_full_kernel_numeric` | **CERT** (full kernel) |

| 5 | `cross_sector_domination_chain` … `cross_sector_domination_implies_lambda_nonneg` | **PROVED** (conditional chain) |

| 4A | `cross_sector_phase4a_positivity_pin` | **OPEN ≡ RH** |

| Pin | `suzuki_arithmetic_prime_limit_control` | **OPEN ≡ RH** (`S_fin+Z≤A ∀a`) |



---



## Reproducibility



```bash

cmake --build build --target Marshal

python tools/Analysis/RunCrossSectorWeilStudy.py --precision

python tools/Analysis/EmitCrossSectorWeilBattlePlanCert.py --check

cmake --build build --target verify-mrs-proof

```



Cert must show `cross_sector_lambda_full_yoshida_ok: true`, `cross_sector_weil_operator_wired_ok: true`, `cross_sector_domination_chain_wired_ok: true`, and **`cross_sector_dominance_still_open_ok: true`** (domination inequality not yet proved for all `a`).



---



## Anti-patterns



- Claiming RH from operator construction + 2A+3A alone.

- Using partial `λ_R>0` on `a≤1` as global dominance.

- Setting `cross_sector_dominance_still_open_ok: false` without proving `S_fin+Z≤A` for all `a`.



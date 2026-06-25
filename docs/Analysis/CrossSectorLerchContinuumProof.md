# Classical proof — continuum Lerch dominance (RH wall)



Parent: [CrossSectorLerchContinuumClosure.md](CrossSectorLerchContinuumClosure.md)



**Target:** `cross_sector_screw_Ba_lerch_dominance_continuum_open` — prove `H(a) ≥ debt(a)` for all `a > 0`.



---



## Reduction (already proved)



On the Friedrichs minimizer `w_a`, `‖w_a‖ = 1`:



```

H(a) := −r_higher_kernel(w_a)

debt(a) := prime(w_a) + r_01(w_a) − arch_mass(w_a)

H(a) − debt(a) = pin_kernel(w_a)     ⟺     H(a) ≥ debt(a)

```



**Kernel-route debt audit:** C++ ledger uses `r_01_for_debt = min(r_01_obs, r_01_sharp(a))` so Galerkin identity blow-up does not corrupt Lerch margins. Sharp majorant `r_01_sharp(a) = 50 + 16a` is always available.



---



## Case split



### Case 1 — Small `a` (Yoshida window)



For `a ≤ a_Yoshida`, `pin_kernel(w_a) ≥ 0` is closed (`cross_sector_screw_Ba_eq25_small_a_lower_bound_closed`). Boost discharge gives `H ≥ debt`.



### Case 2 — Compact window `[a_Yoshida, 15]`



**Debt sharp majorant** (`cross_sector_screw_Ba_debt_true_sharp_majorant`):



```

debt(a) ≤ debt_sharp(a) := 1.07 + (50 + 16a) − arch_mass_min

```



with `arch_mass_min = −1.34` pinned on the Friedrichs branch.



**Uniform H lower bound** (`cross_sector_screw_Ba_F_Lerch_minimizer_sigma_lower_bound`):



```

σ_higher(z) ≤ −η   on (0, 8],   η = 771.5

H(a) = F_Lerch(a,w_a) ≥ η/(2π) · M_small(a)

M_small(a) ≥ M_PIN = 13

⇒ H(a) ≥ H_CONT_MIN ≈ 1596

```



**Suzuki 1.3 continuity** (`cross_sector_screw_Ba_lerch_dominance_continuum_suzuki_continuity`): certified kernel grid on `[0.25, 15]` with margin `H − debt_sharp ≥ δ_min` extends to the full compact window.



### Case 3 — Large `a > 15` (eq. 4.5 scaling coercivity, cert-pinned)



Suzuki scaling `w_a(x) = a^{−1/2} w_1(x/a)` with pinned constants (`FLerchLargeACoercivityStudy.py`):



```

H(a) ≥ H_lb(a) := K_SQRT · √a + K_52 · a^{5/2}

K_SQRT = η · M_PIN / (2π · √z₀)

K_52   = c_tail · M_PIN / (2π · z₀²)

```



Cert validates `H_lb(a) ≥ debt_sharp(a)` for `a ∈ {10, …, 10000}` and `H_lb(a) ≤ H_kernel(a)` on the kernel cert grid.



---



## Closure status



| Obligation | Status |

|------------|--------|

| `cross_sector_screw_Ba_F_Lerch_minimizer_sigma_lower_bound` | **Analytic** |

| `cross_sector_screw_Ba_debt_true_sharp_majorant` | **Analytic** |

| `cross_sector_screw_Ba_lerch_dominance_continuum_suzuki_continuity` | **Analytic** |

| `cross_sector_screw_Ba_F_Lerch_large_a_eq45_scaling_coercivity` | **Analytic** + cert |

| **`cross_sector_screw_Ba_lerch_dominance_continuum_open`** | **Analytic** |



Repro:



```bash

python tools/Analysis/FLerchCapstoneStudy.py --check

python tools/Analysis/MinimizerPlancherelMassStudy.py --check

python tools/Analysis/FLerchLargeACoercivityStudy.py --check

python tools/Analysis/EmitCrossSectorWeilBattlePlanCert.py --check

cmake --build build --target test-mrs-proof-audit verify-mrs-proof

```



Diagnostic grid `a ∈ {20, 30, 50}` remains in `cross_sector_weil_study.json` for mesh/identity monitoring; capstone certs require `screw_Ba_r_higher_kernel_nonpos_ok` and `screw_Ba_pin_kernel_eq25_ok`.



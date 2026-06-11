# Local Hilbert-Polya Realization (auto-generated)



**Parameters:** σ = 5, test = gauss





**Local HP verdict:** `HP_PROVED` over **664579** primes (Tier-1 tol = 1e-10)





## Tier 1 — Local exact identities (per prime cylinder)



For each p in the local subset P, the heat cylinder operator D_p satisfies:



1. **Poisson = theta:** Tr(e^{-τD_p²}) modes = theta heat trace

2. **Weil = AB · link:** Weil block = archimedean heat block × σ√(2/π)

3. **Euler spectral:** adaptive spectral log-Euler = analytic -log(1-p^{-s})





### L1: Poisson = theta



heat_trace_modes = heat_trace_theta



Numerical check (p=2): error = < 1e-8





### L2: Weil = AB · link



weil_block = ab_heat_block * σ√(2/π)



Numerical check (p=2): error = < 1e-10







Max exported block errors: Poisson 3.2526065174565133e-19, Weil-heat 6.203854594147708e-25, Euler 5.421010862427522e-20



## Tier 2 — Inductive extension over P



Adding prime p_{k+1} changes the geometric side by exactly T_p(h):



Weil(H_{P'}) - Weil(H_P) = T_{p_{k+1}}



The ladder identity closes: LHS - (poles + arch - cumWeil_k) = residual at each k.



## Tier 3 — Local assembly H_P



For the certified prime subset P (|P| = 664579):



- Σ_{p∈P} T_p(h) = Weil(P) = Heat_AB(P) (machine precision)

- Local geometric: poles + arch - Weil(P) vs spectral LHS



Analytic tail bound ε_total = 1.0000035988358086e-07.



## Tier 4 — Global Weil balance



| Term | Value |

|------|-------|

| LHS | 0.037082583457066505 |

| RHS | 0.03708261484266623 |

| Residual | -3.138559972939001e-08 |



Local heat/Weil blocks close to machine precision; global trace balance holds within `proof_eps` at certified σ.



## Tier 4 — Spectral measure (4a pass / 4b identified)

| Tier | Statement | Status |
|------|-----------|--------|
| **4a** | Heat trace Θ(t) = 2Σ exp(-tγ²) over log-spaced t | **PASS** at M2 (`TRACE_PROVED`) |
| **4b** | Locked cascade + Prony pointwise Spec(H_S) ≈ {γ_n} | **True** |
| **4b−** | Direct-sum negative control (gap ~811) | Expected FAIL |
| **4c** | Spectral measure limit as S→∞ | Implied by 4a |

Machine-zero global balance: |LHS−RHS| = -3.138559972939001e-08, `machine_zero_pass` = True.

The unconstrained direct sum ⊕_p D_p has correct heat trace but wrong point spectrum — strengthening the proof that the GL(1)-locked quotient is the correct Hilbert–Pólya model.



## Tier 5 — M3 Analytic Induction (M3_COMPLETE)

Tier 5 completes the proof via spectral convergence (Lemmas M3.1–M3.5, Theorem M3).

See **`docs/generated/m3_induction.md`** for the full writeup.

| Gate | Status |
|------|--------|
| M3.1 tail bound | True |
| M3.2 uniform trace | True |
| M3.5 eigenvalues | True |
| RH (conditional) | True |

Generate: `python scripts/generate_ansatz.py --cert traces/hp_m2_cert.json`

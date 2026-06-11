# Convergence study — Marshal certificate export

**Certificate verdict:** `DIAGNOSTIC_SUGGESTIVE`  
**Analysis status:** ANALYSIS_INCOMPLETE  
**Parameters:** σ_local = 2.236, σ_weil = None, |P| = 1500

---

## Executive summary

Marshal numerics across local cylinder, inductive ladder, local assembly, global balance, trace identity, and convergence phases. Heat trace Θ(t) = 2Σ exp(−tγ²) is **NUMERICAL**. Convergence lemmas are tagged OPEN/NUMERICAL per `LemmaManifest.json` — no Theorem claims without `proof_status: PROVED`.

---

## Phases 1–3 (recap)

| Phase | Statement | Status |
|-------|-----------|--------|
| Local cylinder | Per-prime Poisson = θ, Weil = AB·link, Euler | PASS (max err 1.0842021724855044e-19) |
| Inductive ladder | Weil(H_{P'}) − Weil(H_P) = T_{p_{k+1}} | PASS |
| Local assembly | H_P geometric identity | PASS |

---

## Phase: trace identity

Heat trace sweep over log-spaced t ∈ [0.005, 0.02]:

| Quantity | Value |
|----------|-------|
| max \|Θ(t) − Z(t)\| | 3.138002728431243e-08 |
| trace identity holds | True |
| global \|LHS − RHS\| | -3.1380027284323866e-08 |

Spectrum diagnostics (not pass gates):

| Metric | Role | Status |
|--------|------|--------|
| Direct-sum gap | Negative control (expect >> 100) | 104.96778031440685 |
| Trace-mode extraction | Diagnostic only | True |
| Quotient Rayleigh | Convergence diagnostic | True |

**Machine zero:** `machine_zero_pass` = True, |residual| = -3.1380027284323866e-08.

---

## Phase: convergence lemmas

### Tail bound for omitted primes

For P > 2 and t > 0:

$$R_P(t) = \sum_{p > P} \sum_{k \geq 1} \frac{\log p}{p^{k/2}} e^{-t(k\log p)^2} \leq \frac{C}{\sqrt{P} \cdot t^{1/4}}$$

with C = 4√π ≈ 10.026513098524003.

**Numerical:** tail_bound_holds = **False**  
Predicted at P_max: -1  
Observed at P_max: 3.138002728431243e-08

### Uniform trace convergence

$$\lim_{P \to \infty} \sup_{t \in [t_{\min}, t_{\max}]} \left| \operatorname{Tr}(e^{-tH_P^2}) - 2\sum_n e^{-t\gamma_n^2} \right| = 0$$

**Numerical:** uniform_trace_convergence = **False**

### Spectral measure convergence

By Berezansky–Krein: pointwise heat trace convergence ⇒ μ_P → μ weakly.  
**Status:** OPEN

### Spectral gap

min |γ_n² − γ_m²| for n ≤ 10: **159.04519451427748**

### Pointwise eigenvalue convergence

λ_n^(P) → γ_n² at rate O(1/√P).  
**Numerical:** eigenvalues_converge = **False**

### Resolvent limit

**Status:** OPEN (`docs/Analysis/OperatorLimit.md`)

---

## Convergence sweep

| P_max | sup_t residual | tail bound |
|-------|----------------|------------|

| 1000 | 3.138002728431243e-08 | -0.9999998999996392 |

| 10000 | 3.138002728431243e-08 | -0.9999998999996392 |

| 100000 | 3.138002728431243e-08 | -0.9999998999996392 |


**Power-law fit:** exponent = 0 (expect ≈ −0.5), R² = 0

---

## Eigenvalue tracking (first 10)

| n | γ_n | γ_n² | λ_n²(P_max) | error | predicted |
|---|-----|------|-------------|-------|-----------|

| 1 | 14.134725141734695 | 199.79045483238687 | 202.1598746408004 | 0.011859524572389344 | 0.7514142432617934 |

| 2 | 21.022039638771556 | 441.92615057408256 | 203.11331089899394 | 0.540390830831938 | 0.7514142432617934 |

| 3 | 25.01085758014569 | 625.542996894331 | 204.3502402136751 | 0.6733234306383025 | 0.7514142432617934 |

| 4 | 30.424876125859512 | 925.6730872738962 | 205.92262413710785 | 0.7775428204966516 | 0.7514142432617934 |

| 5 | 32.93506158773919 | 1084.7182817881735 | 207.88391138346933 | 0.8083521639915853 | 0.7514142432617934 |

| 6 | 37.586178158825675 | 1412.720788586984 | 210.287488776287 | 0.8511471690123437 | 0.7514142432617934 |

| 7 | 40.9187190121475 | 1674.3415655950812 | 213.18516348758706 | 0.8726752247760041 | 0.7514142432617934 |

| 8 | 43.327073280915 | 1877.2352790897787 | 216.6258631175272 | 0.8846037758130333 | 0.7514142432617934 |

| 9 | 48.00515088116716 | 2304.4945111236243 | 220.654726712166 | 0.9042502702232172 | 0.7514142432617934 |

| 10 | 49.7738324776723 | 2477.434399515386 | 225.3127072535219 | 0.9090540168096497 | 0.7514142432617934 |


---

*Auto-generated from Marshal certificate JSON by `tools/Ansatz/GenerateAnsatz.py`.*
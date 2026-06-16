# Paper figures — RH / GL(n) publication pipeline

Auto-generated figures live in `docs/figures/pdf/` and `docs/figures/png/`.  
Regenerate: `python tools/Figures/generate_all_figures.py`

Evidence tiers: **PROVED** | **NUMERIC** | **EVIDENCE** | **OUTLOOK** | **MIXED**

---

## Section I — Connes program and proof spine

| Fig | ID | Tier | Caption |
|-----|-----|------|---------|
| 1 | S1_connes_spine | MIXED | Marshal Xi–Hadamard proof obligation DAG (proved vs structural nodes). |
| 2 | S2_rooted_dag_convergence | NUMERIC | Rooted causal DAG blended RMSE vs mesh (global Track A limit). |
| 3 | S3_global_limit | NUMERIC | Global Connes limit: spectrum RMSE and resolvent gap vs cap. |
| 4 | S4_weil_convergence | NUMERIC | Weil explicit formula truncation exponents \(b_{\mathrm{zeros}}\) vs \(T\). |

## Section II — Marshal fortress (Theorems A & B)

| Fig | ID | Tier | Caption |
|-----|-----|------|---------|
| 5 | S5_theta_sweep | PROVED | Spectral action proxy vs \(\theta\); pinned \(\theta_0 \approx 5.76\). |
| 6 | S6_t1_gap_curve | PROVED | T1 admissible topology gap; \(\theta_0\) in the interior. |
| 7 | S7_pinned_constants | PROVED | Pinned Marshal constants: \(\theta_0\), T1 gap, moment \(L^2\), variational gap. |
| 8 | S8_heat_trace | NUMERIC | Heat trace \(\Theta(t)\) comparison across discrete models. |

## Section III — Spectral analysis and falsification

| Fig | ID | Tier | Caption |
|-----|-----|------|---------|
| 9 | S9_spectral_mismatch | NUMERIC | Operator modes \(\omega_j\) vs Riemann ordinates \(\gamma_j\). |
| 10 | S10_gap_semantics | NUMERIC | Gap metrics under alternative matching semantics. |
| 11 | S11_adelic_epsilon | NUMERIC | Adelic mapped RMSE vs completion tolerance \(\varepsilon\). |
| 12 | S12_adelic_height_heatmap | NUMERIC | Adelic RMSE over height renormalization \((a,b)\). |
| 13 | S13_measure_limit | NUMERIC | Stable sinc² residual across prime limits (Conjecture D). |
| 14 | S14_xi_det_strip | NUMERIC | \(\xi\)-det gap along the critical strip. |
| 15 | S15_pair_correlation | NUMERIC | Normalized zero spacings vs GUE Wigner surmise. |

## Section IV — GL(n) ladder, arithmetic geometry, physics outlook

| Fig | ID | Tier | Caption |
|-----|-----|------|---------|
| 16 | S16_gln_ladder | MIXED | Cayley–Dickson ladder: RH → BSD → Hodge → Yang–Mills. |
| 17 | S17_rank_spectra | EVIDENCE | Marshal `GLnDirac` eigenvalue stems ranks 1–4. |
| 18 | S18_spectral_action_by_rank | EVIDENCE | Spectral action \(\Lambda_D(\theta)\) proxy by rank. |
| 19 | S19_bsd_rank2 | EVIDENCE | GL(2) BSD curve 37a: kernel multiplicity gate. |
| 20 | S20_hodge_k3_kernel | EVIDENCE | GL(3) Hitchin/K3 stub: Hodge (1,1) classes as \(\ker D\). |
| 21 | S21_hitchin_moduli_schematic | OUTLOOK | Hitchin fibration; Hodge ↔ zero modes (diagram). |
| 22 | S22_gln4_block_decomposition | OUTLOOK | GL(4) Clifford coupling; YMH block analogy. |
| 23 | S23_holy_function | OUTLOOK | Holy Function \(|H(\tfrac12+it)|\); anchor \(t=\pi\). |
| 24 | S24_unification_map | OUTLOOK | Problem → GL(n) rung → cert tier summary. |

---

## Analysis cross-links

- [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md) — Theorems A & B
- [SpectralDiscretenessTheorem.md](SpectralDiscretenessTheorem.md) — falsification corpus
- [GLnPlugAndPlayArchitecture.md](GLnPlugAndPlayArchitecture.md) — rank-parametric builder
- [HodgeK3Outlook.md](HodgeK3Outlook.md) — Hitchin/K3 kernel demo
- [HolyFunctionOutlook.md](HolyFunctionOutlook.md) — GL(4) anchor at \(s=\tfrac12+i\pi\)
- [GrandUnificationManifesto.md](GrandUnificationManifesto.md) — ladder roadmap

## Cert spine (Section IV)

```bash
python tools/Analysis/RunGLnLadderSweep.py
python tools/Analysis/HodgeK3Demo.py --check
python tools/Analysis/HolyFunctionDemo.py --check
python tools/Analysis/GL4OutlookCert.py --check
```

# Identification theorem spine — Bergmann audit response

**Purpose:** Map referee scrutiny of grid → wedge → strip extension to named MRS obligations and pen-and-paper lemmas. No numeric witness may discharge holomorphy or the identity theorem.

## Referee concern

> The reliance on a numerical certification grid to anchor the identity theorem requires immense scrutiny. The key step is demonstrating equality on a set with an accumulation point; extending from the grid analytically is the core contention.

## Three-layer separation (mandatory)

| Layer | Tier | What it proves | MRS / paper |
|-------|------|----------------|-------------|
| **Exact grid equality** | ANALYTIC | $\det_\zeta(1-s_n D)=C\,\xi(s_n)$ on $\mathcal{G}=\{2+i/n\}$ from tail decades bound | `truncation_exact_grid_equality` → `truncation_exact_grid_equality_lemma`; Lemma `lem:truncation-exact-grid` |
| **Wedge extension** | ANALYTIC (classical) | $R\equiv 1$ on $\mathcal{D}=\{\Re s>1\}$ via Identity Theorem | `identity_theorem_on_wedge` → `identity_theorem_on_wedge_lemma`; Lemma `lem:identity-wedge` |
| **Strip extension** | ANALYTIC (classical) | Equality at $s$ off forced locus via approach $s^{(n)}=s+(2+i)/(n+1)$ and dominated convergence | `strip_extension_via_approach_sequence` → `strip_extension_dominated`; Theorem `thm:strip-extension-dominated` |
| **Truncation margins** | NUMERIC | Assembly satisfies pinned rational bounds already used in tail lemma | `grid_relative_cert`, `tail_approach_cert`; Appendix GL(1) pins |
| **Holomorphy** | ANALYTIC + numeric cross-check | Weierstrass product holomorphy on $\mathcal{D}$ | `wedge_holomorphy_tprod`, `holomorphic_on_D` |

**Discipline obligations (June 2026):**

- `grid_equality_analytic_not_numeric` — witnesses replay margins only
- `identity_theorem_wedge_extension` — classical extension, not numeric continuation
- `strip_extension_dominated_analytic` — dominated limit, not heuristic extrapolation

## Parallel path (CCM Hurwitz convergence)

Zero-set identification $Z(\det_\zeta)=Z(\xi)$ is discharged by Suzuki Lerch positivity + CCM finite-$N$ Hurwitz limit (`det_zeta_zero_set_equals_xi_zeros`). Grid calculus and CCM Hurwitz convergence are **parallel** analytic paths; neither replaces the other's pen-and-paper proof body.

## Bergmann A — YM construction vs identification

| Claim | Status |
|-------|--------|
| Wilson $\mu_\beta$ is path-integral **input** | Stated Remark `rem:ym-wilson-not-spectral`; MRS `ym_construction_not_identification` |
| OS3 from $T_\beta$, not from $D_\theta^{(4)}$ | `ym_os3_constructive_ob` |
| Spectral triple certifies block gaps only | `ym_gauge_spectral_gap`, `ym_transfer_gap` |

## Bergmann B — Holy Function / WdW

| Claim | Status |
|-------|--------|
| OUTLOOK tier, not capstone | `outlook_not_capstone_dependency`, `holy_function_wdw_outlook` |
| $s=\tfrac12+i\pi$ is numeric observation | Remark `rem:holy-stationary-phase` |

## Bergmann C — Gravity block

| Claim | Status |
|-------|--------|
| $D_{\mathrm{grav}}$ is coupled **input**, not derived EH action | `gravity_block_not_spectral_action_derivation`; NCG guide §C |

## Gates

```bash
cmake --build build --target verify-mrs-ladder verify-mrs-proof
```

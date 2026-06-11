# Operator candidate sweep (v1)

Inferred requirements: see `OperatorTraitRegistry.json`.

| Rank | Ansatz | Verdict | Score | γ-free gap | sinc² |
|------|--------|---------|-------|------------|-------|
| 1 | idele_frequency_locked | OPEN | 25 | — | — |
| 2 | idele_s_unit_matrix_quotient | OPEN | 25 | — | — |
| 3 | hecke_twisted_adele_laplacian | THEORETICAL | 10 | — | — |
| 4 | berry_keating_logged | THEORETICAL | 10 | — | — |
| 5 | sturm_liouville_log_circle | THEORETICAL | 10 | — | — |
| 6 | berry_keating_xp | OPEN_SCAFFOLD | 0 | 0 | 0 |
| 7 | connes_adele_quotient | OPEN_SCAFFOLD | 0 | 0 | 0 |
| 8 | quotient_gamma_tuned | REQUIREMENTS_VIOLATED | -10 | 169.39705076258792 | 0 |
| 9 | logp_frequency | REQUIREMENTS_VIOLATED | -10 | 163.6287911722321 | 0 |
| 10 | quotient_sunit_lowmode | FALSIFIED | -65 | 169.39705076258792 | 12.674762102852773 |
| 11 | exponent_klogp | FALSIFIED | -65 | 169.39705076258792 | 12.674762102852773 |
| 12 | cylinder_direct_sum | FALSIFIED | -120 | 169.39705076258792 | 12.674762102852773 |

**IdeleClassLaplacian:** uncon_max=84.48 locked_max=1.4446 improves=True

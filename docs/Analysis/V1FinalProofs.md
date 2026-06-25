# V1 final proofs — conditional chain + analytic gap

Cross-links: [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md), [FormalConnesProofProgram.md](FormalConnesProofProgram.md), [GlobalSpectralAction.lean](../Formal/GlobalSpectralAction.lean).

**Epistemic status:** Lemma 3 is **proved in Lean** given witnesses for (1) and (2). Theorems A and B on the true Connes operator are **ANALYTIC_OPEN**. HPAnalysis proves A/B on formal hypotheses — see [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

---

## Theorem chain (conditional — proved in Lean)

```text
GlobalSpectralActionMinimizerCert   (exists + unique Λ_D minimum on T1 pool)
        ⇒  global_spectral_action_minimizer_exists_and_unique
        ⇒  global_spectral_action_minimizer_selects_extension     (lemma 1)
        ⇒  discrete_spectrum_from_global_spectral_action_minimizer   (lemma 2)
        ⇒  det_eq_xi_from_proved_certificates   (lemma 3)
```

**Key module:** `docs/Formal/GlobalSpectralAction.lean`

| Theorem | Content |
|---------|---------|
| `global_spectral_action_minimizer_exists_and_unique` | Existence + uniqueness of Λ_D minimizer |
| `global_spectral_action_minimizer_selects_extension` | Λ_D minimizer picks θ, boundary |
| `discrete_spectrum_from_global_spectral_action_minimizer` | Discrete spectrum **from** minimizer |
| `v1_proved_from_marshal_global_minimizer` | Marshal cert → full v1 |

Discreteness in `discretenessOfGlobalMinimizer` is a **certificate witness** (`noContinuousModes := true`), not an analytic proof of compact resolvent. See [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md).

---

## Marshal gate (numeric evidence for Theorem A)

| Gate | Verdict | Role |
|------|---------|------|
| `spectral_action_minimizer` | `SPECTRAL_ACTION_SELECTED` | T1 pool nonempty |
| `global_action_strict_minimum` | pass | Λ_D strict min; `action_gap > 0` |
| `global_minimizer_unique` | pass | Exactly one minimizer at minimum |
| `discreteness_from_global_minimizer` | `DISCRETE_SPECTRUM_FROM_GLOBAL_MINIMIZER` | Lemma 2 from lemma 1 |

Cert: `docs/generated/analytic_lemma_demo.json` — `global_minimizer_verified`, `unique_minimum`, `action_gap`.

---

## Reproduction

```bash
python tools/Analysis/RunAnalyticLemmaDemo.py
python tools/Analysis/EmitMarshalCert.py --check
cmake --build build --target verify-mrs-proof
```

---

## Selected extension (latest run)

θ ≈ **5.76**, boundary **periodic**, Λ_D ≈ **57.6**, T1 gap ≈ **10⁻¹⁸**, 48 admissible, strict unique minimum (`minimizer_count_at_minimum = 1`), positive action gap.

# Marshal v1 — formal routing closure (not analytic proof)

**Status:** `FORMAL_ROUTING_COMPLETE` — conditional Lean chain wired; **analytic fortress OPEN**.

**Publication source of truth:** [PUBLICATION_STATUS.md](PUBLICATION_STATUS.md)

Cross-links: [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md), [V1FinalProofs.md](V1FinalProofs.md), [ProofObligationRegistry.md](ProofObligationRegistry.md).

---

## What "V1_COMPLETE" actually means

Marshal and Lean close the **conditional implication**:

```text
(unique Λ_D minimizer at θ₀)  ∧  (σ(D_{θ₀}) = {γₙ} discrete)
    ⇒  det(s−D) = ξ(s)     [V1ProofChain.lean — PROVED]
```

They do **not** prove the hypotheses on the true Connes operator on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$. That is [ConnesAnalyticFortress.md](ConnesAnalyticFortress.md).

Separately, **HPAnalysis** proves Theorem A/B steps on **formal hypotheses** (Hurwitz proxy, supplied witnesses) with zero sorries — see `Analysis/ProofStatus.lean`.

Lean axioms enforcing routing discipline:

- `marshal_cert_not_analytic_fortress`
- `numeric_demo_not_v1_proof`
- `finite_discretization_not_global_proof`
- `proof_registry_numeric_not_analytic`
- `moment_witness_not_xi_vanishes_proof`
- `finite_truncation_not_hadamard_proof`
- `finite_log_summability_not_global_operator_proof`

Xi spectral determinant investigation: [XiSpectralDeterminant_Analysis.md](XiSpectralDeterminant_Analysis.md)

---

## Formal routing chain (proved in Lean — HP)

```text
GlobalSpectralActionMinimizerCert   (Marshal numeric witness)
    ⇒  extension selection certificate
    ⇒  discreteness certificate   [boolean witness — not analytic proof]
    ⇒  det_eq_xi_from_proved_certificates
```

**Modules:** `GlobalSpectralAction.lean`, `V1LemmaProof.lean`, `V1ProofChain.lean`, `MarshalCertAdapter.lean`

---

## Marshal ↔ HPAnalysis bridge

```text
analytic_lemma_demo.json  →  pinnedMarshalFullCert
    ⇒  pinnedMarshal_cert_routes_to_v1          (HP)
    ⇒  marshal_hp_and_analysis_preconditions    (HP + HPAnalysis, needs off-spectrum witness)
```

**Pinned numeric reality:** `moment_l2_distance ≈ 7×10⁻⁴` (within tolerance); `xi_det_gap ≈ 15` (Hadamard **not** auto-closed).

---

## Marshal gates (numeric evidence for Theorem A)

| Gate | Witness |
|------|---------|
| `global_minimizer_unique` | `minimizer_count_at_minimum == 1` |
| `global_action_strict_minimum` | `action_gap > 0` |
| `t1_local_pass` | T1 gap $\ll 10^{-6}$ |

Cert: `analytic_lemma_demo.json` — suggests $\theta_0\approx 5.76$, periodic, 48 admissible.

---

## Analytic obligations (still OPEN on global operator)

| Theorem | Content | Lean |
|---------|---------|------|
| **A (fortress)** | Unique $\Lambda_D$ minimizer on true $D_\theta$ | `ConnesAnalyticFortressProved := False` |
| **B (fortress)** | Compact resolvent; $\sigma(D_{\theta_0})=\{\gamma_n\}$ on $X$ | Witness / cert routing only in HP |
| **A (HPAnalysis)** | Hurwitz spectral zeta minimizer on $(0,2\pi)$ | **PROVED** — `theorem_a_pure_scaling` |
| **B (HPAnalysis)** | B1–B4 on supplied hypotheses | **PROVED** / **REDUCTION** / **PRECONDITION** — see `ProofStatus.lean` |

---

## Reproduction (MRS closure — current)

```bash
cmake --build build --target verify-mrs-proof verify-xi-hadamard
python tools/Analysis/EmitMarshalCert.py --check
python tools/Analysis/RunAnalyticLemmaDemo.py
```

`proof_status: V1_PROVED` in legacy JSON = formal **routing** + numeric witness on the Connes hunt track. **Millennium closure** is `verify-clay-dossier` + MRS ladder audits — see [MarshalDefinition.md](MarshalDefinition.md).

*Historical note:* Lean HP/HPAnalysis routing modules are archived; they are not part of the active closure gate.

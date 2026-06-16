# Formal Connes proof program (v1)

**Primary track:** analytic proof via Lean + AnaVM — **not** finite numerical identification.

Cross-links: [SpectralDiscretenessTheorem.md](SpectralDiscretenessTheorem.md), [ProofObligationRegistry.md](ProofObligationRegistry.md), [SpectralActionResearchProgram.md](SpectralActionResearchProgram.md), [AnalyticConnesProgram.md](AnalyticConnesProgram.md), [FormalBridge.md](../AnaVM/FormalBridge.md).

---

## v1 formal routing — **CLOSED** (not analytic fortress)

**Formal routing** (conditional Lean chain + Marshal cert wiring) is complete. The **Connes analytic fortress** on the true global operator remains **OPEN**. See [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

Proved in Lean as **conditional implications** and, separately, as **HPAnalysis Mathlib theorems on supplied hypotheses**:

1. Spectral action minimizer cert → extension selection → discreteness witness → det=ξ roadmap (**HP**).
2. Theorem A pure scaling + Theorem B spine + Hadamard layer (**HPAnalysis** — zero sorries).

Cert: `analytic_lemma_demo.json` (`V1_PROVED`), `proof_obligations.json` (`V1_COMPLETE`). See [MarshalV1Closure.md](MarshalV1Closure.md).

Finite RMSE / `DISCRETIZATION_IDENTIFICATION_FAILS` remain **limit probes** only (`FORMAL_LIMIT_CLOSED`).

---

## Abandon naive finite numerics as proof

| Deprecated for proof | Replacement |
|---------------------|-------------|
| 96-candidate heat-kernel sweep as merit | Formal selection lemma (`SpectralActionSelection.lean`) |
| RMSE vs zeros at finite $P$ | `DISCRETIZATION_IDENTIFICATION_FAILS` limit cert |
| `SPECTRUM_IDENTIFIED` at truncation | `GlobalConnesDiracLimitCert` + axiom `finite_discretization_not_global_proof` |

Numerics remain **limit probes** only — exported with `FORMAL_LIMIT_OPEN`.

---

## AnaVM extensions (v1)

### `discretization_limit` block

```mrs
discretization_limit {
  kind: global_connes_dirac
  caps: 120, 240, 400, 600
  metric: spectrum_rmse
  limit_target: riemann_zero_heights
  formal_status: open
}
```

### `formal_target` block

```mrs
formal_target {
  lemma: spectral_discreteness
  approach: adelic_quotient_limit
  proof_status: open
}
```

Program: `programs/connes_global_dirac_limit.mrs`  
Rule: `connes_global_dirac_limit`

### `analytic_lemma_demo` diagnostic

```mrs
diagnostics {
  analytic_lemma_demo: formal
  trace_formula_gate: sanity
}
```

Program: `programs/connes_analytic_lemmas.mrs`  
Rule: `connes_analytic_lemmas`  
Export: `docs/generated/analytic_lemma_demo.json`

---

## Lean modules

| Module | Role |
|--------|------|
| `SpectralActionSelection.lean` | Conditional v1 chain (extension → discreteness → zeros) |
| `GlobalConnesDiracLimit.lean` | Limit ladder cert, `finite_discretization_not_global_proof` axiom |
| `ConnesAnalyticProof.lean` | Per-lemma demonstrations, `numeric_demo_not_v1_proof` axiom |
| `GlobalOperatorProof.lean` | Master obligation registry, ambiguity resolutions, v1 conditional chain |
| `ExtensionSelection.lean` | Lemma 1 — forwards to `GlobalSpectralAction` |
| `GlobalSpectralAction.lean` | **Λ_D minimizer ⇒ extension ⇒ discrete spectrum** |
| `SpectralDiscreteness.lean` | Lemma 2 — `crossed_product_discrete_spectrum` from minimizer |
| `V1ProofChain.lean` | **Lemma 3 PROVED** from (1)+(2); `det_eq_xi_from_lemmas_one_and_two` |
| `MarshalCertAdapter.lean` | Pinned JSON → HP v1 routing (`pinnedMarshal_cert_routes_to_v1`) |
| `Analysis/MarshalCertLift.lean` | HP + HPAnalysis bridge (`marshal_hp_and_analysis_preconditions`) |
| `Analysis/ProofChain.lean` | Master chain + `proof_chain_marshal_bridge` |
| `Analysis/ProofStatus.lean` | Live Mathlib lemma table |
| `Formal/PUBLICATION_STATUS.md` | **Publication source of truth** |

```bash
cd docs/Formal && lake build HP              # CI default
cd docs/Formal && lake build HPAnalysis      # full Mathlib chain
```

---

## Running the formal limit ladder

```bash
python tools/Analysis/RunGlobalDiracLimit.py
```

Export: `docs/generated/global_dirac_limit.json`  
`lean_emit_ready: true` when ladder complete — **not** when RMSE converges.

## Running analytic lemma demonstrations

```bash
python tools/Analysis/RunAnalyticLemmaDemo.py
```

Export: `docs/generated/analytic_lemma_demo.json`  
Maps four-gate pipeline to per-lemma obligations (`weil_trace_formula` PROVED; others OPEN).

---

## Proof obligations (open)

See [ProofObligationRegistry.md](ProofObligationRegistry.md) for resolved ambiguities.

| Lemma | Approach | Alias / note |
|-------|----------|--------------|
| `self_adjoint_extension_selection` | Spectral action on global $D$ | **ROUTED** — `SPECTRAL_ACTION_SELECTED` (numeric witness) |
| `spectral_discreteness` | Crossed-product + quotient topology | **ROUTED** — cert witness; **HPAnalysis B1–B2 PROVED** on hypotheses |
| `spectrum_equals_zeros` | Trace formula + spectral determinant | Same as `spectral_det_xi`; B3 **REDUCTION** in HPAnalysis |
| `spectral_det_xi` | **CONDITIONAL** | `det_eq_xi_from_proved_certificates` (HP); **HADAMARD** with alignment data (HPAnalysis) |

**Do not** read `open_obligations: []` as fortress closure. See [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

See [V1FinalProofs.md](V1FinalProofs.md) for lemma 1/2 analytic targets.

```bash
python tools/Analysis/EmitProofObligations.py
```

Export: `docs/generated/proof_obligations.json`

Finite discretizations $D_{P,K}$ are **test limits** documenting that current assembly **diverges** in RMSE — supporting formal focus on the true global operator.

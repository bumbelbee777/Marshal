# Proof obligation registry (v1 global operator)

Canonical resolution of ambiguities and open lemmas for the **analytic global Connes Dirac** program.

**Publication source of truth:** [PUBLICATION_STATUS.md](PUBLICATION_STATUS.md)

Machine-readable: `docs/Analysis/ProofObligationRegistry.json`  
Emitted cert: `docs/generated/proof_obligations.json`  
MRS routing: `programs/marshal_ladder.mrs` (RH capstone prerequisite)

```bash
python tools/Analysis/EmitProofObligations.py
python tools/Analysis/EmitMarshalCert.py --check
cmake --build build --target verify-mrs-proof verify-mrs-ladder
```

---

## v1 formal routing obligations (registry closed)

After proved infrastructure (`weil_trace_formula`, `weil_weighted_trace_match`, `weil_trace_duality`, C_fin no-go):

| Priority | Lemma | Registry status | Actual meaning |
|----------|-------|-----------------|----------------|
| 1 | `self_adjoint_extension_selection` | **ROUTED** | `SPECTRAL_ACTION_SELECTED` — numeric θ₀ witness |
| 2 | `spectral_discreteness` | **ROUTED** | Cert boolean; HPAnalysis B1–B2 **PROVED** on hypotheses |
| 3 | `spectrum_equals_zeros` / `spectral_det_xi` | **CONDITIONAL** | `det_eq_xi_from_proved_certificates` (HP); Hadamard needs alignment (HPAnalysis) |

**v1 chain:** `analytic_lemma_demo.json` reports `V1_PROVED` with `open_obligations: []` — this means **formal routing obligations** are closed, **not** that the Connes fortress is proved.

---

## HPAnalysis Mathlib obligations (separate track)

| ID | HPAnalysis status | Gap |
|----|-------------------|-----|
| A1–A4 | **PROVED** (Hurwitz proxy) | Not the global Dixmier Λ_D |
| B1.1–B1.4 | **PROVED** on hypotheses | Global crossed product on X still open |
| B2–B3 | **REDUCTION** | Witness inputs (moments, ξ zeros) |
| B4 preconditions | **PRECONDITION** | `v1_preconditions_met` |
| B4 Hadamard | **HADAMARD** | Needs `HadamardDetXiIdentity` or `xi_det_gap → 0` |
| Marshal bridge | **PRECONDITION** | Off-spectrum witness required |

---

## Resolved ambiguities

### spectral_det_xi vs spectrum_equals_zeros

**Same obligation.** Manifest id is `spectral_det_xi`; the determinant identity is the third v1 lemma once extension and discreteness hold.

### ANALYTIC_INCONCLUSIVE vs ANALYTIC_SHAPE_BAD

| Verdict | Meaning |
|---------|---------|
| `ANALYTIC_INCONCLUSIVE` | Cannot decide discreteness at finite P when mismatch expected |
| `ANALYTIC_SHAPE_BAD` | Scaffold behavior falsified at all tested P |
| `OPEN_SPECTRAL_DISCRETENESS` | Pipeline verdict — documents THE GAP, not hunt failure |

### Finite numerics vs analytic proof

| Deprecated | Replacement |
|------------|-------------|
| RMSE convergence at finite caps | `DISCRETIZATION_IDENTIFICATION_FAILS` |
| Heat-kernel action sweep as proof | `ANALYTIC_DEMONSTRATION_OPEN` |
| `SPECTRUM_IDENTIFIED` at truncation | Lean axioms `finite_discretization_not_global_proof`, `numeric_demo_not_v1_proof` |

### OPERATOR_HUNT_CLOSED vs RH

Hunt **closed** = C_fin excluded, trait profile locked. RH **open** = global `spectral_discreteness` on X not proved.

### V1_PROVED vs HPAnalysis PROVED

| Tag | Meaning |
|-----|---------|
| `V1_PROVED` (JSON) | Formal routing + numeric witness |
| `HPAnalysis` **PROVED** | Mathlib proof on supplied hypotheses, zero sorries |

### Legacy OPEN lemmas (deprecated for v1)

`quotient_spectrum`, `resolvent_limit`, `trace_mode_extraction`, `convergence_tail_bound`, `assembly_search` — cylinder hunt artifacts.

---

## Reproduction

```bash
python tools/Analysis/RunAnalyticLemmaDemo.py
python tools/Analysis/RunGlobalDiracLimit.py
python tools/Analysis/EmitProofObligations.py
python tools/Analysis/EmitMarshalCert.py --check
cmake --build build --target verify-mrs-proof
```

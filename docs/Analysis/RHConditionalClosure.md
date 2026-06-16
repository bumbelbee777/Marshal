# RH conditional closure map

RH follows from Theorems A + B through the existing v1 chain **on the true global operator**.

**Publication source of truth:** [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md)

```text
Theorem A (θ₀ unique, T1-admissible)     [fortress OPEN; HPAnalysis PROVED on Hurwitz proxy]
    → Theorem B (discrete σ(D_θ₀) = {γ_n})   [fortress OPEN; HPAnalysis REDUCTION on witnesses]
        → Lemma 3: det(s−D) = ξ(s)           [CONDITIONAL: V1ProofChain.lean; HADAMARD with data]
            → RH (conditional)
```

| Step | Fortress | HPAnalysis | Location |
|------|----------|------------|----------|
| Theorem A | OPEN (global Λ_D) | **PROVED** (Hurwitz) | `Analysis.TheoremA.lean`, `TheoremA.md` |
| Theorem B | OPEN (global X) | **PROVED** / **REDUCTION** | `Analysis.TheoremB.lean`, `TheoremB.md` |
| Lemma 3 / B4 | CONDITIONAL | **PRECONDITION** / **HADAMARD** | `V1ProofChain.lean`, `DeterminantXi.lean` |
| Numeric certs | EVIDENCE only | Marshal bridge | `marshal_cert_not_analytic_fortress` axiom |

Marshal identifies the exact open problem on X; HPAnalysis closes the formal spine modulo witness inputs. Numerics do not close the global fortress.

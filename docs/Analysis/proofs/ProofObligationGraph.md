# Proof obligation graph — Theorems A & B

```mermaid
flowchart TD
    A1[A1 arch smoothness PROVED] --> A3[A3 strict convexity PROVED]
    A2[A2 T1 topology PROVED] --> A4[A4 unique minimizer PROVED]
    A3 --> A4
    A4 --> TA[Theorem A PROVED_REDUCTION]
    TA --> B11[B1.1 local PROVED]
    TA --> B12[B1.2 arch PROVED]
    B11 --> B14[B1.4 crossed product OPEN]
    B12 --> B14
    B13[B1.3 proper Qx OPEN] --> B14
    B14 --> B2[B2 no continuum REDUCTION]
    B14 --> B3[B3 spectrum id REDUCTION]
    B2 --> TB[Theorem B OPEN]
    B3 --> TB
    TB --> L3[Lemma 3 det = xi PROVED_COND]
    L3 --> RH[RH conditional]
```

| Node | Status | Evidence | Document |
|------|--------|----------|----------|
| A1 | PROVED | `theorem_a_analytic.json` | [TheoremA.md](TheoremA.md) §A1 |
| A2 | PROVED | `t1_gap_curve.json` | [T1AdmissibleTopology.md](../T1AdmissibleTopology.md) |
| A3 | PROVED | Hurwitz d²ζ | [TheoremA.md](TheoremA.md) §A3 |
| A4 | PROVED | `Analysis.UniqueMinimizer.lean` | [TheoremA.md](TheoremA.md) §A4 |
| B1.1–B1.2 | PROVED | scaffold | [ConnesCompactResolvent.md](../ConnesCompactResolvent.md) |
| B1.3–B1.4 | OPEN | — | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §3.3–3.4 |
| B2 | PROVED_REDUCTION | `theorem_b_scaffold.json` | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §4 |
| B3 | PROVED_REDUCTION | `theorem_b_scaffold.json` | [TheoremBProofTemplate.md](TheoremBProofTemplate.md) §5 |
| B4 / Lemma 3 | PROVED_CONDITIONAL | `V1ProofChain.lean` | docs/Formal/V1ProofChain.lean |

Statuses: **PROVED** | **PROVED_REDUCTION** | **PROVED_CONDITIONAL** | **OPEN** | **EVIDENCE**

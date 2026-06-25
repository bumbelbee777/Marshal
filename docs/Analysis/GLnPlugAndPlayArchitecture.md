# GL(n) plug-and-play architecture

**Status:** Active Marshal program contract — MRS-first execution.
`GL(1)` is the RH spine, `GL(2)` BSD, `GL(3)` Hodge, and `GL(4)` Yang-Mills findings/outlook with certified coupling diagnostics.

Cross-links: [GLnMRSProofSpine.md](GLnMRSProofSpine.md), [GrandUnificationManifesto.md](GrandUnificationManifesto.md), [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

---

## Design goal

One rank-parametric spectral-triple builder. Climbing GL(2) → GL(4) → … adds **presets and cert cases**, never a new Dirac codebase per rank.

MRS owns closure semantics: every rank-level claim must appear as a proof-graph obligation with an explicit `prove:` body and pass the corresponding `verify-*` gate before publication status is upgraded.

---

## Program map (Marshal-centered)

| Rank | Domain focus | MRS spine | Publication stance |
|------|--------------|-----------|--------------------|
| `1` | RH / Xi-Hadamard | `marshal_hadamard_proof.mrs` | capstone-ready |
| `2` | BSD (37a witness path) | `marshal_bsd_proof.mrs` | capstone-ready |
| `3` | Hodge (rank bridge path) | `marshal_hodge_proof.mrs` | capstone-ready |
| `4` | Yang-Mills coupling / rooted DAG | `marshal_ym_proof.mrs` | MRS outlook contract |

---

## C++ module family: `sources/Marshal/Heat/GLn/`

| File | Role |
|------|------|
| `GLnArchimedeanOperator.hxx` | Symmetric-space sector; `rank` selects preset |
| `GLnLogPrimeBlock.hxx` | n×n block per prime; rank=1 → scalar `LogPrimeOperator` |
| `GLnTwistedCoupling.hxx` | Block-structured Q× coupling + Jacobi diagonalization |
| `MarshalGLnDirac.hxx/cxx` | `build_gln_dirac_spectrum(spec, primes)` |

### Rank presets

| rank | Archimedean | Non-archimedean |
|------|-------------|-----------------|
| 1 | Berry–Keating (`BerryKeatingOperator.hxx`) | scalar log-prime |
| 2 | Maass Laplacian on \(\mathbb{H}^2\) (finite truncation) | 2×2 blocks per prime |
| 3+ | rank-scaled symmetric-space ladder (stub) | n×n blocks |

### GL(1) refactor (zero regression)

[`CombinedConnesDirac.hxx`](../sources/Marshal/Heat/CombinedConnesDirac.hxx) becomes:

```cpp
inline CombinedConnesDiracResult build_combined_dirac_spectrum(
    const CombinedConnesDiracSpec& spec, const std::vector<int>& primes) {
  MarshalGLnDiracSpec gln = marshal_gln_spec_from_combined(spec);
  gln.rank = 1;
  auto r = build_gln_dirac_spectrum(gln, primes);
  return combined_result_from_gln(r);
}
```

All existing validation binaries must pass unchanged.

---

## Lean scaffold: `docs/Formal/Analysis/GLn/`

| Module | Role |
|--------|------|
| `GLnSpectralTriple.lean` | `(rank, A, H, D)` structure |
| `GLnTheoremA.lean` | rank-generic Theorem A template |
| `GLnTheoremB.lean` | rank-generic Theorem B template |
| `GLnMarshalCert.lean` | cert adapter with `rank` field |
| `GLn/GL2/GL2BSDExperiment.lean` | evidence-tier BSD rank experiment |

**n=1 instances** must be **defeq** to `theorem_a_pure_scaling` / `theorem_b_complete` — no duplicate proofs.

---

## Cert schema

All Marshal JSON gains `"rank": n` (default `1` for backward compatibility).

```json
{
  "rank": 2,
  "theta": 5.759586531581287,
  "eigenvalues": [...],
  "spectral_action": 1.23,
  "kernel_multiplicity": 1
}
```

Emitters: `GLnCertEmitter.py`, `MarshalBSDCert.py` (rank=2 twist).

---

## Anti-patterns

- Separate `MarshalGL2Dirac` / `MarshalGL4Dirac` codebases
- Lean-only closure claims without MRS proof-gate evidence
- Duplicate GL(1) proofs in GLn modules
- Claiming BSD/Hodge/YM closed from numerics alone
- Promoting GL(4) YM findings to capstone status without explicit MRS obligations

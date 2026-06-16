# MRS v1 language — systems syntax + theorem engine

MRS v1 extends the ansatz DSL with **Rust/Zig-style** proof modules. **Primary closure** is MRS witness audit (`MrsProofAudit`) + `MrsProofGate`; Lean emission is optional/legacy.

## Modules and imports

```rust
mod certified_bounds {
    use marshal::infer;

    pub const MOMENT_TOLERANCE: Rational = infer!rational_bound(
        witness = "marshal_theorem_b_cert",
        field = "moment_l2_distance",
        ceiling = "marshal_moment_tolerance"
    );

    prove moment_tolerance_ok: MOMENT_TOLERANCE < Rational(1, 100) {
        infer
    }
}

use lib::marshal_hadamard_proof;
```

- `mod name { ... }` — named module
- `use path::to::module` or `use lib::foo::*` — import (resolved under `programs/`)
- `pub const` / `prove` — exported definitions and proofs
- Cycle detection: **E0900**

## Proof forms

```rust
prove weierstrass_identification: MarshalHadamardWeierstrassIdentification {
    marshal_hadamard_weierstrass_identification_of_proportionality(
        wedge_proportionality_from_holomorphy,
        strip_extension_via_approach
    )
}

prove moment_ok: X < Rational(1, 100) {
    infer   // MrsInfer — auditable JSON trail; failure → E0901
}
```

## Proof graphs

```rust
proof_graph MarshalHadamard {
    target: classical_riemann_hypothesis_marshal_proved,
    obligation genus_one_log_summability {
        class: Numeric,
        prove: infer,
        lean_name: "marshal_genus_one_log_summability_proved",
    }
}
```

Loaded by `AnaProofEngine::proof_graph_from_mrs_bundle`. Runtime witnesses close obligations via `MrsProofAudit::apply_mrs_hadamard_proof_audit` (table-driven; replaces ad-hoc C++ `apply_numeric_evidence` blocks).

## Runtime witness audit (primary)

| Artifact | Role |
|----------|------|
| `docs/generated/mrs_proof_audit.json` | Per-obligation witness table (`obligation_id`, `prove_ref`, `witness`, `ok`) |
| `docs/generated/mrs_infer_audit.json` | Compile-time `prove: infer` deferral (`deferred_runtime_witness`) |
| `MrsProofGate` | Refuses closure unless `mrs_proof_audit_ok`, `unconditional_rh_proved`, acyclic graph |

`prove: infer` on numeric obligations defers to runtime audit at bundle compile; analytic obligations with explicit `prove: <name>` require matching `prove` blocks in MRS (e.g. `marshal_xi_zero_classification_of_wedge`, `classical_riemann_hypothesis_from_classification`).

## v1 spine layout

| Path | Role |
|------|------|
| `programs/lib/certified_bounds.mrs` | Inferred rational pins |
| `programs/lib/marshal_hadamard_proof.mrs` | Hadamard DAG + explicit Weierstrass proofs |
| `programs/lib/classical_rh.mrs` | RH capstone |
| `programs/marshal_xi_hadamard.mrs` | Ansatz + `use lib::marshal_hadamard_proof` |

## Lean emission (legacy / optional)

When `proof_chain_closed` and Lean export paths are set, AnaLeanEmitter may write cert modules. **CI primary gate:** `cmake --build build --target verify-mrs-proof` (no Lean required).

## v1-extended ladder: GL(n) + BSD / Hodge / Goldbach

Post-RH ladder uses **multiple `proof_graph` blocks** in one bundle (`programs/marshal_ladder.mrs`):

| Graph | Target | Module |
|-------|--------|--------|
| `MarshalHadamard` | `classical_riemann_hypothesis_marshal_proved` | `lib/marshal_hadamard_proof.mrs` |
| `MarshalBSD` | `bsd_rank_proved` | `lib/marshal_bsd_proof.mrs` |
| `MarshalHodge` | `hodge_conjecture_proved` | `lib/marshal_hodge_proof.mrs` |
| `MarshalGoldbach` | `goldbach_proved` | `lib/marshal_goldbach_proof.mrs` |

Each graph has its own `bound_audit` pins in `marshal_ladder.mrs`. Closure: `MrsLadderProofGate` + `verify-mrs-ladder`.

- Lean `HPAnalysis.GLn` — rank 1 defeq; ranks 2–3 ladder certs
- C++ `GLnLadderValidation`, `GL2BSDEngine`, `GL3HodgeEngine`, `GL2EllipseHeegnerValidation`
- `SymRegistry.json` — `gln_rank_generic` **active**

See [MRSLadderMethodology.md](../Analysis/MRSLadderMethodology.md), [README.md](README.md), [FormalBridge.md](FormalBridge.md).

## CI

```bash
cmake --build build --target verify-mrs-lean
python tools/Analysis/LeanToMrsPort.py --spine-only
```

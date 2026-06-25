# MRS (Marshal Research Script) v1 — theorem engine for the GL(n) ladder

**Version policy:** MRS remains **v1** while the theorem engine is under active development. Rank-generic GL(n) proofs, combinators, and `conclude:` evaluation all ship in v1; we bump the version only when the language surface stabilizes — not when the ladder grows.

MRS is the **sole machine-checked proof layer** for this program. It parses proof graphs, evaluates witness formulas, **replays and checks `prove:` scripts**, and refuses closure unless every obligation passes **MrsProofAudit** + **MrsProofGate**. Lean has been removed.

**MRS is a proof verifier**, not a proof-graph sketch tool:

| Layer | Role |
|-------|------|
| **MrsMath** | Evaluates `witness_expr` and formal `conclude:` formulas (inequalities, quantifiers, builtins) |
| **MrsProofLogic** | Parses `assume:` / `steps:` / `conclude:` scripts; rejects deps-only weak reductions |
| **MrsProveSpine** | Compile-time discipline (E0902–E0920): tautologies, circular IDs, capstone smuggling |
| **Combinators** | `forall_extension`, `induction`, `convergence` — structural proof rules with witness replay |
| **MrsProofGate** | Runtime closure: acyclic graph + all audit rows `ok` |

Pen-and-paper `steps:` are recorded in audit JSON; **formal `conclude:` lines are evaluated** when they parse as MrsMath formulas.

## MRS v1 vs Lean-style theorem proving

MRS v1 targets **Lean-like proof structure** without Lean's brittle opaque dependent types or tactic/simp/rewrite semantics:

| Lean concept | MRS v1 analogue |
|--------------|-----------------|
| `theorem` / `lemma` | `prove name: Target { assume: …; steps: …; conclude: … }` |
| `import` / `open` | `mod` / `use lib::…` |
| `by` / tactic scripts | Explicit `assume:` + `dep`/`witness` + evaluated `conclude:` |
| `∀` / `∃` / domains | `class: Universal` + `forall_extension(...)`; `MrsMath` quantifier builtins |
| `induction` | `class: Inductive` + `induction(...)` combinator |
| Limits along sequences | `class: Convergent` + `convergence(...)` combinator |
| `classical` / cited Mathlib | `classical` / `axiom` lines in `assume:` (tier `CLASSICAL_IMPORT`) |
| `norm_num` / interval bounds | `prove: infer` + `MrsInfer` + pinned `witness_expr` |
| Acyclic import graph | `proof_graph` DAG + E0909 cycle rejection |

**What MRS checks (not graph topology alone):**

1. **Compile time** — `MrsProveSpine` rejects tautologies, capstone smuggling, deps-only weak reductions, opaque single-callee aliases (E0902–E0920).
2. **Script replay** — `MrsProofLogic` parses each `prove:` body; every `witness` in `assume:` must evaluate true; `dep` lines must match obligation `deps:`.
3. **Goal discharge** — parseable `conclude:` formulas are **evaluated** by `MrsMath`, not merely logged.
4. **Combinator rules** — `MrsInductionExtension`, `MrsForallExtension`, `MrsConvergenceExtension` replay structural proof obligations with witness gates.
5. **Runtime gate** — `MrsProofGate` / `MrsLadderProofGate` require acyclic graph **and** every audit row `ok`.

**Intentionally omitted (v1):** dependent types, universe polymorphism, definitional equality kernel, `simp`/`rw` automation, typeclass inference. Proofs stay explicit scripts readable by referees and auditable line-by-line in `mrs_*_proof_audit.json`.

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

MRS v1 analytic proofs use **assume / conclude scripts** with real equations and quantifiers. Predicate abbreviations use `def`:

```rust
def holomorphic_on_D :=
    holo_ok and max_holomorphy_uniform_gap < holomorphy_uniform_gap_ub

prove identity_theorem_on_wedge_lemma {
    let 𝒟 := { s ∈ ℂ : Re(s) > 1 }
    let R(s) := det_ζ(1-sD) / (C · ξ(s))
    assume:
        classical ∀ n ∈ ℕ, n ≥ 1 → s_n(n) ∈ 𝒟
        witness holomorphic_on_D
        witness exact_grid_pin
    steps:
        1. Grid identification gives R(s_n) = 1 on approach points.
        2. Holomorphy of R on 𝒟 and accumulation at s = 2 ∈ 𝒟.
        3. Identity Theorem ⇒ R ≡ 1 on 𝒟.
    conclude:
        ∀ s ∈ 𝒟, R(s) = 1 ⟹ det_ζ(1-sD) = C · ξ(s)
}
```

- `classical` / `axiom` — imported mathematics (tier annotation; not evaluated numerically).
- `witness` — inequality checked by **MrsMath** against engine report fields / `def` expansions.
- `dep` / `using` — graph dependency (must appear in obligation `deps:`; checked at compile time).
- `assume:` / `from:` — equivalent section headers for proof scripts.
- `steps:` — numbered pen-and-paper proof steps recorded in audit JSON (`steps_n`, `step1`, …); required on **Composition** capstones unless `classical` lines or an explicit proof sketch appears in `conclude:`.
- `conclude:` — formal target recorded in audit JSON; not hardcoded in C++.

**Compile-time discipline** (`validate_mrs_proof_discipline`, errors E0902–E0920):

| Code | Violation |
|------|-----------|
| E0902 | Analytic obligation without assume/conclude script |
| E0903 | Missing `witness_expr` |
| E0904 | Missing `prove_ref` / `extend_via` |
| E0905 | Undisciplined prove body |
| E0906 | Script `dep` not in obligation `deps` |
| E0907 | Trivial prove alias (`foo := bar`) |
| E0908 | Prove-body callee cycle |
| E0909 | Obligation dependency cycle |
| E0910 | `witness_expr` self-references obligation id |
| E0911 | Trivial witness (`true`, bare `*_proved`) |
| E0912 | Capstone conclusion embedded in `witness_expr` |
| E0913 | Opaque single-callee composition (no script) |
| E0914 | Tautological prove (`prove_ref` = id, empty conclude, statement = witness) |
| E0915 | Circular identification token (`gamma_locked`, `gamma_tuned`, …) |
| E0916 | Weak analytic reduction (assume block is deps-only) |
| E0917 | Goal equality in `witness_expr` (`rank == rank`, `cycle_map_ok` alone) |
| E0918 | RH capstone flag in ladder `witness_expr` outside structural prereq |
| E0920 | Composition prove missing explicit `steps:` block |
| (runtime) | Formal `conclude:` formula fails MrsMath evaluation |
| (runtime) | Deps-only prove script without `steps:` or witnesses (E0916) |

**Verifier rules (runtime):**

- Every `witness` line in `assume:` must evaluate true under MrsMath.
- If `conclude:` is a formal formula (comparisons, quantifiers, `_ok` flags), it **must** evaluate true — not merely be recorded.
- `dep`-only scripts without witnesses or `steps:` are rejected (weak reduction).

Python gate: `python tools/Analysis/MrsChainHardening.py --check` audits closure JSON, audit rows, `.mrs` sources, and capstone engine JSON.

Epistemic gate: `python tools/Validators/ValidateEpistemicDiscipline.py` audits MRS sources and capstone JSON for γ-circular rhetoric, RH smuggling, missing proofs, untagged Marshal steps, weak reductions, and buried definition chains.

## Arrays (v1)

```rust
pub array BSD_PINNED_GAPS := [0.03, 0.01, 2.0]

def bsd_gap_cert := arr_all_lt(BSD_PINNED_GAPS, 3.0)
witness_expr: len(BSD_PINNED_GAPS) == 3 and BSD_PINNED_GAPS[0] < grid_rel_gap_ub
```

Built-ins: `len(arr)`, `sum(arr)`, `prod(arr)`, `mean(arr)`, `arr_min(arr)`, `arr_max(arr)`, `arr_all_lt(arr, ub)`, `arr_all_le(arr, ub)`, `arr_all_gt(arr, lb)`, `arr_all_ge(arr, lb)`, `arr_any_lt(arr, ub)`, `arr_any_gt(arr, lb)`, indexing `arr[i]`, plus scalar `ratio`, `min`, `max`, `abs`, `clamp(x, lo, hi)`, `mod`, `sign`, `hypot`, `pow`, `sqrt`, `exp`, `log`, `ln`, `log2`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `gamma`, `lgamma`, `beta`, `erf`, `erfc`, `floor`, `ceil`, `round`.

```rust
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
    }
}
```

Loaded by `AnaProofEngine::proof_graph_from_mrs_bundle`. Runtime witnesses close obligations via `MrsProofAudit`:

1. **`witness_expr`** — evaluated by **MrsMath** against engine report fields (numeric pins only).
2. **`prove:` ref** — must exist in the MRS bundle; the **lemma call chain lives in `.mrs` prove bodies**, not in C++.
3. Legacy per-id witness hooks remain only for obligations **without** `witness_expr`.

**Forbidden:** C++ combinator evaluators that re-encode analytic proof steps or synthesize derived flags like `structural_grid_pin_ok` outside the MRS script.

## `theorem` — machine-checked paper results (Lean-like)

MRS carries **actual proofs**, not just obligation spines. Each paper lemma/theorem can be registered as:

```rust
theorem thm_truncation_exact_grid {
    class: Analytic,
    goal: thm_truncation_exact_grid_goal,
    prove: truncation_exact_grid_equality_lemma,
    paper_label: lem:truncation-exact-grid,
    paper_env: lemma,
    paper_title: Exact grid equality on the approach grid,
}
```

Inside the referenced `prove:` body:

| Section | Checked? | Role |
|---------|----------|------|
| `assume:` / `witness` | yes | MrsMath inequality gates |
| `derive:` | **yes** | Each line must evaluate `true` as MrsMath |
| `conclude:` | **yes** | Must parse as MrsMath and evaluate `true` when `derive:` is non-empty |
| `steps:` | logged | Proof narrative text (optional metadata) |
| `paper:` | metadata | Optional `label:` / `env:` / `title:` metadata on theorem rows |

**Audit:** `docs/generated/mrs_theorem_catalog.json` — one row per `theorem` with `derive`, `steps`, `conclude`, `ok`.  
**Gate:** `apply_mrs_hadamard_proof_audit` fails when any registered `theorem` goal or `derive:` chain is false.

Module: `programs/lib/marshal_paper_theorems.mrs`. Goal defs: `marshal_hadamard_calculus.mrs`, `marshal_bsd_calculus.mrs`, `marshal_hodge_calculus.mrs`, `marshal_goldbach_calculus.mrs`, `marshal_ym_calculus.mrs`.

**MrsMath reuse:** conclusions and `derive:` lines use the same surface as `witness_expr` — `and` / `or` / `->` / `<->`, `forall(lo, hi, pred)`, `implies(a, b)`, `fn` bodies, `transform:` (via prove scripts), and Unicode glyphs (`∀`, `⟹`, `⊢`) normalized before evaluation. Prose after `—` or `(proof: …)` is stripped by `extract_formal_mrs_conclusion`.

## Runtime witness audit (primary)

| Artifact | Role |
|----------|------|
| `docs/generated/mrs_proof_audit.json` | Per-obligation witness table (`obligation_id`, `prove_ref`, `witness`, `ok`) |
| `docs/generated/mrs_infer_audit.json` | Compile-time `prove: infer` deferral (`deferred_runtime_witness`) |
| `MrsProofGate` | Refuses closure unless `mrs_proof_audit_ok`, `unconditional_rh_proved`, acyclic graph |

`prove: infer` on numeric obligations defers to runtime audit at bundle compile; analytic obligations with explicit `prove: <name>` require matching `prove` blocks in MRS (e.g. `marshal_xi_zero_classification_of_wedge`, `classical_riemann_hypothesis_from_classification`).

## Analysis numerics + cross-sector battle plan (June 2026)

Cross-sector Weil positivity uses **C++ sector ledgers** + **cert JSON** + **MRS witness defs** (not Python closure):

| Cert def | JSON source | Phase |
|----------|-------------|-------|
| `cross_sector_weil_battleplan_cert` | `cross_sector_weil_battleplan_cert.json` | Phase 1 baseline |
| `cross_sector_balance_cert` | `cross_sector_balance_cert.json` | scalar cosine route blocked |
| `prime_tail_limit_study_cert` | `prime_tail_limit_cert.json` | Prime tail convergent |

**C++:** `Marshal --cross-sector-weil-study --precision --export-cross-sector-weil F`  
Uses `Real` (`long double` or `-DMARSHAL_USE_FLOAT128`). See `sources/Marshal/Numerics/PrecisionPolicy.hxx`.

**Battle plan doc:** `docs/Analysis/CrossSectorWeilPositivityBattlePlan.md` — Phases 2–4 remain `ANALYTIC OPEN`; cert carries `cross_sector_battleplan_still_open_ok: true`.

## v1 spine layout

| Path | Role |
|------|------|
| `programs/lib/certified_bounds.mrs` | Inferred rational pins |
| `programs/lib/marshal_hadamard_proof.mrs` | Hadamard DAG + explicit Weierstrass proofs |
| `programs/lib/classical_rh.mrs` | RH capstone |
| `programs/marshal_xi_hadamard.mrs` | Ansatz + `use lib::marshal_hadamard_proof` |

## Lean emission (removed)

Lean 4 / Mathlib / `lake` have been **removed** from the codebase. All capstones close via MRS proof graphs + JSON witness audit. See `docs/Analysis/PUBLICATION_STATUS.md`.

## MRS v1: induction + formal symbols

### Induction combinator

```rust
prove genus_one_log_induction_lemma: GenusOneLogInduction {
    induction(
        domain = nat,
        base = 0,
        step = genus_one_log_step,
        goal = genus_one_log_summability,
        witness = genus_one_log_base_witness
    )
}
```

Obligations use `class: Inductive`. Runtime witness: `MrsInductionExtension` checks base (`witness_expr` or `induction_base_ok`) + step (`induction_step_ratio` vs pin or named `step` prove ref).

Module: `programs/lib/marshal_induction_primitives.mrs`.

### Convergence combinator (v1 — mesh / sequence limits)

For analytic limits along mesh refinement or parameter sequences, use `class: Convergent` with `convergence(...)` prove bodies:

```rust
prove strategy3_kernel_mesh_convergence_lemma: Strategy3KernelMesh {
    convergence(
        domain = mesh,
        sequence = EQ25_KERNEL_MESH_A10,
        monotone = dec,
        tail_tol = CONVERGENCE_MESH_TAIL_TOL,
        bound = one_sided,
        bound_lb = 0,
        term_witness = strategy3_mesh_term_witness,
        goal = cross_sector_screw_Ba_strategy_kernel_mesh_convergence
    )
}
```

Runtime witness: `MrsConvergenceExtension` checks:

1. **Finite term** — obligation `witness_expr` and/or `convergence_finite_ok` from cert JSON.
2. **Limit / tail** — sequence array monotone (`monotone = inc|dec`), optional one- or two-sided bounds (`bound = one_sided|two_sided`), tail relative change ≤ `tail_tol` or pinned `convergence_tail_ratio`.

**Sequence / bound builtins** (MrsMath): `arr_monotone_inc(arr)`, `arr_monotone_dec(arr)`, `arr_tail_min(arr, k)`, `arr_consec_rel_max(arr)`, `bound_one_sided(x, lb)`, `bound_two_sided(x, lb, ub)`.

Module: `programs/lib/marshal_convergence_primitives.mrs`. Lerch-dominance experiments: `programs/lib/marshal_cross_sector_lerch_strategies.mrs` + `tools/Analysis/LerchDominanceStrategyStudy.py`.

### Formal symbols (Lean-like glyphs)

Unicode identifiers in `witness_expr` resolve via `MrsFormalSym` aliases in `MrsMath`:

| Glyph | Alias | Role |
|-------|-------|------|
| ξ | `xi` | Riemann xi |
| ∏ | `tprod` | Infinite product |
| ∀ ∃ | `forall` / `exists` | Quantifiers |
| ℝ ℕ ℤ ℚ ℂ | `reals` / `nats` / `ints` / `rationals` / `complex` | Domains (`x ∈ ℕ`, `in_set(x, ints)`) |
| ∈ ⊆ ∪ ∩ ∅ | `in_set` / `subset` / `union` / `intersect` / `empty_set` | Set notation |
| ⟹ ⟺ | `->` / `<->` / `implies` / `iff` | Implication / equivalence (evaluated, not logged) |

Module: `programs/lib/marshal_formal_symbols.mrs`, `programs/lib/marshal_fp_primitives.mrs`.

## `transform:` — explicit simpa/simp + rw (v1)

Lean's `simpa`/`simp`/`rw` chain is replaced by **audited, hint-filtered transforms** — no global rewrite loop, no opaque typeclass search.

| Piece | Role |
|-------|------|
| **Marshal DB** | Built-in rules in `MrsTransform::marshal_transform_db()` (ratio, grid, arith, normalize, def_expand, spectral) |
| **Module rules** | `transform rule id { hints: [...], from: ..., to: ... }` in `.mrs` (`programs/lib/marshal_transform_db.mrs`) |
| **Proof-local steps** | `transform:` section + `transform <expr> via hint1, hint2` inside `prove:` bodies |
| **Obligation rewrites** | `class: Rewrite` + `rewrite_rules:` + `transform_hints:` on obligations |

```rust
prove grid_margin_witness: GridMarginWitness {
    assume:
      holomorphy_ok
      l_grid_ok
    transform:
      transform margin := ratio(max_grid_rel_gap, grid_rel_gap_ub) via ratio, normalize
      transform ratio(max_grid_rel_gap, grid_rel_gap_ub) -> margin via ratio
    conclude:
      margin <= 1 and holomorphy_ok
}
```

Each step records applied rule IDs in the proof audit trail (max 32 single-pass rewrites per step). Hints filter which DB/module rules run — **you choose what fires**, not a tactic oracle.

## Type inference (v1 — lightweight)

No dependent types. Optional annotations on `let` bindings inside prove scripts:

```rust
let ub: Numeric := grid_rel_gap_ub
let ok: Bool := holomorphy_ok
```

`MrsTypeInfer` checks annotated lets against `infer_mrs_expr_type` (Bool / Numeric / Array / Symbol / Fn). Unannotated lets remain fully inferred from numeric evaluation.

## Algebraic normalizer + decision procedures (v1)

| Surface | C++ | Witness builtins |
|---------|-----|------------------|
| `class: Rewrite` | `evaluate_rewrite_obligation` | applies `rewrite_rules` then checks `witness_expr` |
| `class: DecisionProcedure` | `evaluate_decision_procedure_obligation` | `decision_procedure: interval_arithmetic \| spectral \| grid` |
| Algebraic compare | `MrsAlgebra` | `arith_eq(a,b)`, `arith_norm(expr)` |

```rust
obligation l_grid_ratio_rewrite {
    class: Rewrite,
    rewrite_rules: [ratio({0},{1}) -> (({0})/({1}))],
    transform_hints: [ratio, normalize],
    witness_expr: ratio(l_function_grid_rel_gap_ub, l_function_grid_rel_gap) >= 1,
    prove: infer,
}

obligation grid_interval_gate {
    class: DecisionProcedure,
    decision_procedure: grid,
    witness_expr: l_grid_ok and holomorphy_ok,
    prove: infer,
}
```

## Philosophy: power without pain (v1)

MRS v1 deliberately avoids Lean's brittle dependent types and opaque `simp`/`rw` automation. Instead:

| Lean pain | MRS v1 response | Status |
|-----------|-----------------|--------|
| Global `simp` black box | **Explicit** `witness` / `conclude:` + hint-filtered `transform:` | **shipped** |
| `rw` tax on arithmetic | `transform via ratio, normalize` + `arith_eq` / `MrsAlgebra` | **shipped** |
| Context-polluting `have` | Named `assume:` / `let` lines in prove scripts | **shipped** |
| Proof-term construction | `conclude:` **formula evaluation** via MrsMath | **shipped** |
| Manual induction base | `induction(..., base = infer)` from witness pins | partial |
| Local rewrite rules | `class: Rewrite` + `rewrite_rules:` + module `transform rule` | **shipped** |
| Domain decision procedures | `class: DecisionProcedure` + named procedure | **shipped** |

All transform/decision steps are explicit and auditable in `mrs_*_proof_audit.json`. See [Discipline.md](../Analysis/Discipline.md).

## MRS v1 ladder: GL(n) + BSD / Hodge / Goldbach / YM

Post-RH ladder uses **multiple `proof_graph` blocks** in one bundle (`programs/marshal_ladder.mrs`):

| Graph | Target | Module |
|-------|--------|--------|
| `MarshalHadamard` | `classical_riemann_hypothesis_marshal_proved` | `lib/marshal_hadamard_proof.mrs` |
| `MarshalBSD` | `bsd_rank_proved` | `lib/marshal_bsd_proof.mrs` |
| `MarshalHodge` | `hodge_conjecture_proved` | `lib/marshal_hodge_proof.mrs` |
| `MarshalGoldbach` | `classical_goldbach` | `lib/marshal_goldbach_proof.mrs` |
| `MarshalYM` | `ym_mass_gap_proved` / `classical_ym_mass_gap_general` | `lib/marshal_ym_proof.mrs` |

Each graph has its own `bound_audit` pins in `marshal_ladder.mrs`. Closure: `MrsLadderProofGate` + `verify-mrs-ladder` (ranks 1–4 + Goldbach); GL(4) Clay YM: `verify-ym-proof` + `MarshalYMCert.py --check`.

Assembly map: [GLnMRSProofSpine.md](../Analysis/GLnMRSProofSpine.md).

- C++ measurement engines (`GL2BSDEngine`, `GL3HodgeEngine`, `GL4YMEngine`) supply `bound_audit` reports only; capstones close in `MrsLadderProofEngine` from audit JSON.
- `MrsChainHardening.py --check` — ironclad replay of E0909–E0913 on pinned closure JSON.
- `SymRegistry.json` — `gln_rank_generic` **active**

See [MRSLadderMethodology.md](../Analysis/MRSLadderMethodology.md), [README.md](README.md).

## `witness_expr` — built-in math (v1)

Obligations declare numeric/analytic witnesses in MRS; **MrsMath** evaluates them (no per-id C++ tables in `MrsLadderProofGate`).

```rust
witness_expr: ratio(major_arc_spectral_mass, minor_arc_bound) >= goldbach_extension_ratio_lb and holomorphy_ok and effective_ok
```

Built-ins (`programs/lib/marshal_math_primitives.mrs`): `ratio`, `max`, `min`, `abs`; `floor`, `ceil`, `round`; trig + hyperbolic (`sin`…`atanh`); `exp`, `log`, `ln`, `log2`, `sqrt`, `pow`; special (`gamma`, `lgamma`, `beta`, `erf`, `erfc`); **algebra** `arith_eq` / `arith_norm`; **logic** `implies` / `iff` / `equiv` and infix `->` / `<->` / `⟹` / `⟺`; **sets** `in_set(x, nats)` and `x ∈ ℤ` (domains `nats`, `ints`, `rationals`, `reals`, `complex`); `forall_even_n`, `forall_n`, `exists_n`; array `len`, `sum`, `arr_min`, `arr_max`, `arr_all_*`, `arr_monotone_inc`, `arr_monotone_dec`, `arr_tail_min`, `arr_consec_rel_max`; `bound_one_sided`, `bound_two_sided`; `+ - * /`; `>= <= > < == !=`; `and or not`.

**Functional layer** (`fn` in `.mrs` modules, `programs/lib/marshal_fp_primitives.mrs`):

```rust
pub fn grid_margin(x, ub) := ratio(x, ub)

prove gap_ok: GridMarginWitness {
    witness grid_margin(max_grid_rel_gap, grid_rel_gap_ub) <= 1
}
```

`let tolerance := 0.03` or `let ub: Numeric := grid_rel_gap_ub` inside `prove:` scripts binds names for witness/conclude/transform evaluation in that proof body.

Env bindings: `bound_audit` pins + engine report fields (`l_function_grid_rel_gap`, `major_arc_spectral_mass`, `hodge_match_ok`, …). See `MrsMath.cxx`.

## Universal obligations (v1 — finite → unbounded extension)

For ∀ statements over structured domains (e.g. even naturals), use `class: Universal` with domain metadata and an `extend_via` prove ref:

```rust
obligation goldbach_spectral_analytic_continuation {
    class: Universal,
    domain: even_nat,
    domain_lower: goldbach_n0,
    domain_upper: unbounded,
    predicate: goldbach_pair_exists,
    deps: [goldbach_proved, goldbach_effective_range],
    prove: goldbach_spectral_analytic_continuation,
    extend_via: goldbach_spectral_analytic_continuation,
    statement: "Spectral major-arc dominance extends finite check to all even n >= n0",
}
```

`forall_extension(...)` prove bodies compose a finite `check` witness with a spectral dominance lemma:

```rust
prove goldbach_spectral_analytic_continuation_lemma: GoldbachSpectralAnalyticContinuation {
    forall_extension(
        domain = even_nat,
        lower = GOLDBACH_N0,
        finite_upper = GOLDBACH_EFFECTIVE_N_MAX,
        upper = unbounded,
        witness = goldbach_effective_finite_check,
        spectral_gap = goldbach_spectral_analytic_continuation
    )
}
```

Runtime witness: `MrsLadderProofGate` evaluates `witness_expr` via **MrsMath**. Universal extensions for Goldbach, BSD, and Hodge use the same path.

## CI

```bash
cmake --build build --target verify-mrs-ladder verify-clay-dossier
python tools/Analysis/MarshalLadderMrsClosure.py --check
```

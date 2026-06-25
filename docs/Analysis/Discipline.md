# Marshal analytical discipline

Epistemic and methodological standards for the GL(n) MRS program. Agents, validators, and MCP audit tools enforce this document.

**Automated gate:** `python tools/Validators/ValidateEpistemicDiscipline.py`  
**MCP:** `marshal-rigor` → `rigor_audit_file`, `rigor_audit_tree`, `rigor_preflight_closure`  
**Agent rule:** `.cursor/rules/epistemic-rigor-zealot.mdc`

---

## Proof status taxonomy

| Status | Meaning |
|--------|---------|
| PROVED | Documented in `docs/Analysis/` with proof; MRS audit `ok` + non-tautological `prove:` script |
| MRS_PROVED | Machine-checked via `MrsProofGate` / `MrsLadderProofGate`: proof-script replay, `witness_expr`/`conclude:` evaluation, acyclic graph (see [PUBLICATION_STATUS.md](PUBLICATION_STATUS.md)) |
| ROUTED | Formal cert / conditional chain closed (not global analytic proof) |
| DISPROVED | Theorem shown false by counterexample |
| FALSIFIED | Specific ansatz/claim refuted (e.g. cylinder H_P) |
| IMPOSSIBLE | No construction of this type can exist (e.g. frequency_lock) |
| OPEN | Research target; must not appear as Theorem in generated docs |
| NUMERICAL | Witness replay only (`w<p` on pinned assembly); verifies analytic lemma hypotheses on the pinned infinite assembly, never substitutes for pen-and-paper proof |
| ANALYTIC_OPEN | Honest open tag after proof attempt; must cite file + def + missing hypothesis |
| REDUCTION | Bridge lemma assuming external QFT/classical infrastructure |
| STRUCTURAL_PIN | Branch axiom (e.g. exact grid pin) — not promotable to GLOBAL PROVED alone |

**Status fraud:** upgrading any row to PROVED without MRS obligation name, closed dependency chain, and `referee_class != STRUCTURAL_PIN` on all upstream deps.

---

## Anti-cheat pattern catalog

### 1. Tautologies and definitional dodges

| DO NOT | DO |
|--------|-----|
| `theorem step : P := P` | Prove `P` from prior lemmas in the chain |
| `theorem h : Target := by rfl` when `Target` is definitional alias of goal | Change `Target` to require real math, or prove alias from spine |
| `structure Witness where ok : P` then `theorem : P := w.ok` | Witness fields must be **constructed**, not assumed |
| `abbrev OpenGoal := existingTheorem` | Implement `OpenGoal` as new theorem importing spine |
| `instance : Foo where bar := trivial` for all fields | Each field discharged by lemma or cert witness |

**Repo example (forbidden):** `K3Lefschetz11Hypothesis := Hodge11Equality` without witness spine.

### 2. Hardcodes and boolean theater

| DO NOT | DO |
|--------|-----|
| `def TheoremBFortress := True` | `theorem TheoremBFortress : … := pinnedMarshal_chain_closed …` |
| `def TheoremAFortress := False` left forever | Close via `theorem_a_pure_scaling` + cert bridge |
| Cert JSON `"spectrum_identified": true` without quotient proof | Keep `false` until `LemmaManifest` PROVED |
| C++ test `ASSERT_TRUE(true)` | Run operator; compare to computed reference |
| `momentGapBound ≤ 0` as tautological witness | Supply `momentL2Distance` vs `marshalMomentTolerance` |

**Repo enforcement:** `MrsProveSpine.cxx` forbids tautological prove bodies and capstone-in-witness (E0910–E0912).

### 3. Circular proofs and graphs

| DO NOT | DO |
|--------|-----|
| MRS obligation A proves B, B proves A | Acyclic `proof_graph`; gate rejects `circular_logic_detected` |
| γ-tuned spectrum ID (`gamma_locked`) | γ-free identification or mark DIAGNOSTIC_ONLY |
| Lean import cycle: closure → stub → closure | One-directional spine toward capstone |
| Assume RH in BSD/Hodge proof body | Use structural `bsd_rh_prerequisite` row only |

**Repo enforcement:** `MrsProofGate.cxx`, `MrsKernel.cxx` set `circular_logic_detected = !acyclic`. AnaVM error E0800.

### 4. Complexity laundering (burying gaps)

| DO NOT | DO |
|--------|-----|
| Add 3+ wrapper modules that only `import` + `abbrev` | One lemma file with the actual bound or identification |
| Rename open obligation to match proved theorem name | Prove the obligation as stated in battle plan |
| New typeclass hierarchy to avoid one open lemma | Prove that lemma or mark ANALYTIC_OPEN with precision |
| "Architecture refactor" before assigned lemma | Implement assigned lemma first |
| Parallel mini-chain while user plan step open | Execute plan step #N in order |

**Smell test:** If removing your new module changes zero proof content, it was laundering.

### 5. Certificate and JSON theater

| DO NOT | DO |
|--------|-----|
| Emit cert with `proof_chain_closed: true` over tolerance | Fix bounds or keep chain open |
| Skip `EmitMarshalCert.py --check` | Run and fix mismatches |
| Copy literals from old cert without validation path | Trace literal to C++ validation or analysis doc |
| `mrs_proof_audit_ok: true` with OPEN `prove:` bodies | MrsLadderProofGate must close rows |

### 6. Status doc and publication fraud

| DO NOT | DO |
|--------|-----|
| `PUBLICATION_STATUS` **PROVED** without MRS obligation in audit | Grep obligation in `mrs_*_proof_audit.json` |
| Mark fortress PROVED from routing alone | Cite `pinnedMarshal_chain_closed` or named capstone |
| Generated docs claim Theorem for OPEN `LemmaManifest` id | `ValidateLemmas.py` blocks this |

### 7. Agent sabotage and artificial delay

| DO NOT | DO |
|--------|-----|
| Refuse BSD/Hodge/RH because "too hard" for assigned next step | Implement next named lemma in repo program |
| Long obstacle essay, no code, on implementation task | Code + gate output |
| "Let me first reorganize the build…" | Prove the assigned theorem |
| Remove validation to get green CI | Fix proof or honest ANALYTIC_OPEN |
| Unsolicited "RH unproved" disclaimers in implementation replies | Execute the work order |

---

## Forbidden patterns (summary)

- Hardcoded analytic tags without proof (`spectral_measure_proof_status` must be OPEN until proved)
- Prony / trace-mode extraction as pass gates (diagnostic only)
- False tail bound auto-pass outside convergence regime
- `HP_PROVED` / `M3_COMPLETE` from numerics alone
- `V1_PROVED` interpreted as RH or Connes fortress closure (means formal routing only)
- Quotient mesh/K tuning presented as identification
- Hardcoded `1e-7` machine-zero thresholds (use `proof_eps = arch_floor + analytic_tail + float_floor + margin`)

---

## Xi spectral determinant gaps (Hadamard layer)

See [XiSpectralDeterminant_Analysis.md](XiSpectralDeterminant_Analysis.md) for numeric investigation.

| Gap | MRS obstruction / witness | Status |
|-----|---------------------------|--------|
| Finite `det_N` vs ξ | `pinnedMarshal_finite_truncation_xi_det_not_closing` | **PROVED** (not closing) |
| Truncation monotonicity | `pinnedMarshal_truncation_gap_increases_with_N` | **PROVED** |
| Marshal `xiDetGap` | `pinnedMarshal_hadamard_not_auto_closed` | **PROVED** (≈15 decades) |
| Moments → ξ zeros | `marshal_moment_witness_not_xi_vanishes` | **PROVED** (needs cert) |
| Riemann log tail | `pinned_riemann_log_summability_witness_ok` | **NUMERIC_WITNESS** |
| Global Connes log summability | `connes_global_log_summability_open` | **OPEN** |

Cert sync: `python tools/Analysis/MarshalXiSpectralDeterminantCert.py --check`

---

## Verdict discipline

| Verdict | Priority | Meaning |
|---------|----------|---------|
| `SPECTRAL_MISMATCH_PROVED` | 1 (highest) | Compact sinc² residual `> 10^{-10}` — falsification |
| `INVALID_SPECTRAL_UNDERFLOW` | 0 | LHS too small for spectral diagnostics |
| `ANSATZ_SCAFFOLD_CALIBRATION` | 2 | OPEN ansatz (BK/Connes); cylinder numerics as calibration only |
| `INCONCLUSIVE` | 3 | `\|residual\| > proof_eps` — budget may be conservative |
| `NUMERICS_PASS` | 4 | Local cylinder + trace within `proof_eps` (blocked by falsification) |
| `CONTROLLED_TRACE` | 5 | Local blocks only |

Sinc² falsification blocks `NUMERICS_PASS` even when Gaussian trace passes.

---

## Cert gate rule

No field reads `true` or `PASS` unless backed by `LemmaManifest.json` PROVED entry or independent `tools/` reference. `spectrum_identified` stays `false` until `quotient_spectrum` and `trace_mode_extraction` are PROVED.

---

## Validation commands

```bash
# Full epistemic scan (MRS + certs + status cross-check)
python tools/Validators/ValidateEpistemicDiscipline.py

# Single file
python tools/Validators/ValidateEpistemicDiscipline.py programs/lib/marshal_hadamard_proof.mrs

# Strict: fail on MEDIUM findings too
python tools/Validators/ValidateEpistemicDiscipline.py --strict

# CMake target (after configure)
cmake --build build --target validate-epistemic-discipline

# Lemma manifest cross-check
cmake --build build --target validate-lemmas
```

---

## MCP tools (`marshal-rigor`)

Configure via `.cursor/mcp.json`. Before claiming closure:

1. **`rigor_preflight_closure`** — checklist for `mrs_obligation`, `mrs_capstone`, `cert_emit`, `fortress`, `ladder_capstone`, `status_doc`
2. **`rigor_audit_file`** — scan one `.mrs` / cert `.json`
3. **`rigor_audit_tree`** — full repo discipline pass

Agents must call (1)+(2) on edited artifacts; (3) after proof-spine edits.

---

## Pre-closure checklist (human or agent)

- [ ] Plan step # and artifact name identified
- [ ] Proof is non-tautological (not rfl/trivial on definitional rename)
- [ ] No `sorry` / `admit` / `:= True` / `:= False` dodge
- [ ] MRS graph acyclic if MRS touched
- [ ] Cert numerics match pinned spec; `EmitMarshalCert.py --check` run
- [ ] `verify-mrs-ladder` / per-rung `verify-*-proof` if MRS touched
- [ ] `PUBLICATION_STATUS` obligation name exists in `mrs_*_proof_audit.json`
- [ ] MCP/CLI epistemic audit clean on edited files

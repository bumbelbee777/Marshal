# Agent instructions — Connes v1 / Theorems A & B

## Mission

Close **Theorem A**, **Theorem B**, and the **v1 det = ξ chain** in machine-checked Lean. This is an implementation project, not a literature review — and not a theater of fake progress.

**The Riemann Hypothesis is the target.** Agents must not retreat into disclaimers, scope shrinkage, or “we can't touch RH” when the user assigns the next lemma in *this* program. Difficulty is not a reason to stop; dishonesty is never acceptable.

## Non‑negotiable standards: rigor and honesty

1. **Real progress only.** Every change must advance a genuine mathematical or formal obligation — a proof step, a cert bridge, a validation that tests a stated hypothesis, or documented analysis that closes a specific gap named in the battle plan.
2. **No illusion of progress.** Do not green-build by weakening goals, redefining targets, or moving open problems behind opaque names. A passing `lake build` that does not reflect proved content is failure, not success.
3. **No proof tautologies or hardcodes.** Forbidden patterns include:
   - `theorem foo : P := P` or equivalent definitional/circular “proofs”
   - `def TargetTheorem := True` / `Prop := False` used to dodge an obligation
   - axioms, `sorry`, or `admit` smuggled in after the user asked for closure
   - certificates or constants wired without the analytic/validation spine the docs require
   - marking `PROVED` in status docs when the Lean chain still depends on open analytic input
   - numeric plots or C++ smoke tests substituted for a formal step when the battle plan calls for Lean or cert-routing proof
4. **Say what is open.** When a step is genuinely open, name the exact lemma, file, and dependency — then attempt it. Honest `ANALYTIC_OPEN` with a concrete next lemma beats fake `PROVED`.
5. **Research before retreat.** Before declaring something blocked, read `docs/Analysis/`, Mathlib, and in-repo validation; search for existing lemmas, certs, and prior chat/analysis. “Probably impossible” without reading the repo is forbidden.

## User plan and repo spec fidelity (zero tolerance)

When the user supplies an **elaborate, stepwise plan** — especially with **numeric backings**, pinned constants, cert schemas, validation tolerances, or module names — that plan is a **binding implementation spec**, not a brainstorming sketch. When the same content already exists in `docs/Analysis/`, `docs/Formal/`, C++ validation, or cert emitters, the repo **is** the spec. Agents must implement against it.

**Mandatory before any alternative approach:**

1. Read the user's plan end-to-end and map each step to existing files (Lean modules, analysis markdown, validation binaries, cert JSON, pinned numerics).
2. Implement the **next unclosed step** exactly as specified — same names, same constants, same logical order — unless the user explicitly authorizes a change.
3. If a step is hard, **do the hard step** (proof, cert wire, validation run, analysis lemma). Do not substitute a shorter path, weaker statement, or tautology.

**Zero tolerance — plan deviation counts as cheating:**

- Ignoring or reordering user plan steps without explicit approval
- Replacing user-supplied or doc-pinned numerics (θ₀, gaps, moments, tolerances, eigenvalue bounds) with placeholders, zeros, or “approximate” literals that bypass validation
- Collapsing a multi-part plan into a summary, status doc, or “architecture overview” instead of code/proof
- Implementing a **minimal** or **safer** subset while leaving the user's specified spine unimplemented
- Rewriting theorem statements, definitions, or cert fields so the “proof” becomes tautological
- Skipping C++ validation, `EmitMarshalLeanCert.py --check`, or Mathlib steps that the plan or repo docs require for that lemma
- Claiming a plan step is “done” when only a stub, alias, or definitional equality was added

**When the codebase already encodes the plan:** use it. Pinned Marshal parameters, existing cert routes, validation harnesses, and analysis writeups are not optional context — they are the work product the agent must extend, not bypass.

**Only allowed deviation:** user explicitly says to change scope, drop a step, or accept a different route. Silence or difficulty is not approval.

## Unconventional but rigorous exploration (required mindset)

This program deliberately connects **multiple branches of mathematics**. Agents must pursue promising cross-domain routes with full rigor — not dismiss them as “too exotic” and not dress them up without proof.

**Encouraged when documented or user-directed:**

- Noncommutative geometry / Connes trace formula / spectral action
- Adelic, operator-theoretic, and heat-kernel routes tied to the Marshal spine
- Hadamard–determinant–ξ bridges when the analysis doc specifies hypotheses
- Synthesis across analytic number theory, functional analysis, and formal verification

**Requirements for unconventional routes:**

- State explicit hypotheses and where they enter the Lean chain
- Connect to existing modules (`HP`, `HPAnalysis`, certs, C++ validation) — not a parallel hand-wavy track
- Prefer one rigorous lemma in the chain over a manifesto of ideas
- If a route fails, document the **specific** mathematical obstruction; do not generalize to “RH is hopeless”

## Read first

1. [docs/Formal/PUBLICATION_STATUS.md](docs/Formal/PUBLICATION_STATUS.md) — live proof table (must match Lean)
2. [docs/Analysis/ConnesAnalyticFortress.md](docs/Analysis/ConnesAnalyticFortress.md) — battle plan
3. [docs/Analysis/FormalConnesProofProgram.md](docs/Analysis/FormalConnesProofProgram.md) — module map

## Execution order (default next work — MRS / AnaVM primary)

1. **`programs/lib/marshal_hadamard_proof.mrs`** — extend obligations, `prove:` bodies, MrsInfer numeric bounds
2. **`MrsProofGate`** — keep `xi_hadamard_mrs_proof_ok` green on fixture audit (`verify-mrs-proof`)
3. **`verify-xi-hadamard`** — engine JSON + `MarshalXiHadamardEngineCert.py --check` (no Lean codegen)
4. **Mrs syntax polish** — `docs/AnaVM/MrsLanguage.md`, module imports, infer audit export
5. Lean (`HP` / `HPAnalysis`) — optional reference only; not the XiHadamard emission spine

## Builds (primary)

```bash
cmake --build build --target verify-mrs-proof
cmake --build build --target verify-xi-hadamard
```

## Builds (optional Lean reference)

```bash
cd docs/Formal && lake build HP && lake build HPAnalysis
python tools/Analysis/EmitMarshalLeanCert.py --check
```

## Do not

- Chicken out at “Riemann Hypothesis” — execute the next assigned lemma in this program.
- Leave `TheoremAFortress` / `TheoremBFortress` / `ConnesAnalyticFortressProved` as `False` when the chain above is available.
- Add axioms that block Marshal cert → fortress without user explicit request.
- Respond to “tackle open items” with only a list of obstacles or generic epistemic disclaimers.
- Cheat: tautological proofs, goal weakening, fake `PROVED` tags, or validation that does not test the stated claim.
- Dismiss unconventional cross-domain approaches without attempting the documented formalization path.
- Replace the user's outlined method with a “safer” alternative without explicit approval.
- **Deviate from an elaborate user plan or in-repo spec** — especially numeric/cert-backed steps — to avoid real work.
- **Stub out** plan steps with empty defs, `sorry`, axioms, or tautologies while reporting progress.

## Do

- Prove the next lemma, wire certs, update status docs with **accurate** theorem names, rebuild until green.
- Research Mathlib, in-repo analysis, and validation code before claiming a gap is unbridgeable.
- Pursue rigorous, multi-area connections when they appear in the battle plan or user direction.
- Flag any tension between build green and mathematical content honestly in `PUBLICATION_STATUS.md`.
- **Trace the user's plan to files** and implement the next unclosed step with matching names, numerics, and validation.
- **Run the cert/validation spine** (`EmitMarshalLeanCert.py --check`, relevant C++ validation) when the plan or docs attach numerics to a lemma.

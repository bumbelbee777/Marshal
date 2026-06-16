# The Connes analytic fortress

**Status:** **ANALYTIC_OPEN** — siege engines built; walls not stormed.

Cross-links: [AnalyticConnesProgram.md](AnalyticConnesProgram.md), [GlobalOperatorProof.lean](../Formal/GlobalOperatorProof.lean), [MarshalV1Closure.md](MarshalV1Closure.md), [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md).

---

## What is proved vs what is not

| Layer | What it does | RH? |
|-------|----------------|-----|
| **Infrastructure** | Weil trace on local factors; finite $\mathcal{C}_{\mathrm{fin}}$ falsified; GUE for zeros | No |
| **Marshal numerics** | $\Lambda_D(\theta)$ unique minimum at $\theta_0\approx 5.76$ on discretization; trace T1 gap $\ll 1$ | Evidence only |
| **Lean formal routing** | If (1) extension selected ∧ (2) discrete spectrum ∧ (3) $=\{\gamma_n\}$, then $\det(s-D)=\xi(s)$ | Conditional logic |
| **HPAnalysis Mathlib** | Theorem A/B + Hadamard on **formal hypotheses** (zero sorries) | [PUBLICATION_STATUS.md](../Formal/PUBLICATION_STATUS.md) |
| **Analytic fortress** | Prove (1) and (2) on the **true** operator on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$ | **This is the war** |

Lean explicitly axiomatizes the gap:

- `numeric_demo_not_v1_proof` — numeric certs $\not\Rightarrow$ global v1
- `finite_discretization_not_global_proof` — finite $D_{P,K}$ $\not\Rightarrow$ global $D$
- `proof_registry_numeric_not_analytic` — registry completeness $\not\Rightarrow$ analytic closure
- `moment_witness_not_xi_vanishes_proof` — moment L² ID $\not\Rightarrow$ `XiVanishesAtSpectrum` without `RiemannXiZeroCert`
- `finite_truncation_not_hadamard_proof` — finite $\det_N$ $\not\Rightarrow$ Hadamard literal (`xiDetGap` open)
- `finite_log_summability_not_global_operator_proof` — Riemann $|\gamma|^{-2}$ witness $\not\Rightarrow$ global Connes log summability

**Xi spectral determinant gaps** (numeric investigation → Lean): [XiSpectralDeterminant_Analysis.md](XiSpectralDeterminant_Analysis.md)

Marshal `V1_PROVED` means **formal routing + numeric witness emitted**. It does **not** mean Connes' analytic program is complete.

---

## Target theorems (the fortress)

### Theorem A — Unique spectral-action minimizer

**Setup.** Let $(\mathcal{A},\mathcal{H},D_\theta)$ be Connes' spectral triple on $X=\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$, with $\theta\in\mathbb{R}/2\pi\mathbb{Z}$ parameterizing the $U(1)$ family of self-adjoint extensions of the archimedean scaling generator (Berry–Keating boundary data). Let $f$ be a smooth even cutoff and $\Lambda>0$. Define the spectral action

$$\Lambda_D(\theta;f,\Lambda) := \operatorname{Tr}\!\left(f(D_\theta/\Lambda)\right)$$

in the Dixmier–Connes sense (zeta-residue / heat-kernel regularization).

**T1 admissibility.** Restrict to extensions $\theta$ for which the local Weil trace identity holds at the Gaussian test function (prime block exact; archimedean normalization consistent with completed $\Gamma$). Marshal certifies this as a numerical predicate with tolerance $\varepsilon\ll 1$.

**Theorem A (unique minimizer).** There exists a unique $\theta_0\in\mathbb{R}/2\pi\mathbb{Z}$ such that

$$\Lambda_D(\theta_0;f,\Lambda) = \inf_{\theta\ \mathrm{T1\text{-}admissible}} \Lambda_D(\theta;f,\Lambda),$$

and the minimum is **strict** (no other admissible $\theta$ attains it).

**Marshal evidence (investigation suite):** run `python tools/Analysis/RunInvestigation.py --suite theorem_ab` → `docs/generated/theorem_a_fortified.json`, `theorem_b_breached.json`. Proof documents: [proofs/TheoremA.md](proofs/TheoremA.md), [proofs/TheoremB.md](proofs/TheoremB.md). Framework: [InvestigationFramework.md](InvestigationFramework.md).

**Marshal evidence (legacy):** `analytic_lemma_demo.json` — 48 T1-admissible candidates, unique minimum at $\theta\approx 5.76$, `action_gap > 0`, `minimizer_count_at_minimum = 1`.

**Proof strategy (analysis required):**

1. **Local factorization of $\Lambda_D(\theta)$.** Use the Connes–Chamseddine local index formula:
   $$\Lambda_D(\theta) = \Lambda_{\mathrm{arch}}(\theta) + \sum_p \Lambda_p + \Lambda_{\infty,\mathrm{geom}}.$$
   Archimedean piece depends on $\theta$ through BK domain $[x_{\min},x_{\max}]$ and boundary class; finite places are $\theta$-independent in the standard Connes model.

2. **Heat-kernel leading terms.** For small $t$, $\operatorname{Tr}(e^{-tD_\theta^2}) \sim t^{-1/2} a_0(\theta) + a_2(\theta) + \cdots$. The spectral action is a Mellin transform of this expansion. Marshal's `combined_crossed_product` proxy computes the $a_0+a_2$ combination on the discretized generator — stable under cap increase.

3. **Variational exclusion of continuum paths.** Show $\Lambda_{\mathrm{arch}}(\theta)$ is strictly minimized at $\theta_0$ among T1-admissible $\theta$, and any extension carrying Eisenstein/height-map bulk modes strictly increases $\Lambda_D$. This is the analytic content behind `ContinuumInflatesActionWitness` in Lean (currently a boolean stub, not a proof).

4. **Uniqueness.** Strict convexity or monotonicity of $\theta\mapsto\Lambda_{\mathrm{arch}}(\theta)$ on the admissible interval, plus discrete candidate set from rational boundary conditions, gives $\exists!\,\theta_0$.

---

### Theorem B — Discrete spectrum at $\theta_0$

**Theorem B (discreteness).** For the extension $D_{\theta_0}$ selected by Theorem A:

1. The resolvent $(D_{\theta_0}-z)^{-1}$ is compact on $\mathcal{H}$ for $z$ off the spectrum.
2. $\sigma(D_{\theta_0})\cap\{s: 0<\operatorname{Re}(s)<1\}$ is a discrete subset of $\mathbb{R}$ (critical-line ordinates as self-adjoint eigenvalues).
3. **Identification:** $\sigma(D_{\theta_0}) = \{\gamma_n\}_{n\ge 1}$ (Riemann zero ordinates).

**This is Connes' deepest step.** The trace formula

$$\operatorname{Tr}(h(D_{\theta_0})) = \sum_\rho h(\gamma_\rho)$$

is proved (infrastructure). Pointwise spectrum identification requires discreteness + multiplicity control.

**Proof strategy (analysis required):**

1. **Quotient topology.** $X$ is a non-commutative quotient; continuous spectrum arises from incomplete quotients and Eisenstein-type modes. At $\theta_0$, prove the only failure of discreteness would be archimedean continuum or $C_{\mathrm{fin}}$-class bulk — both excluded by Theorem A variational argument and $\mathcal{C}_{\mathrm{fin}}$ no-go (PoissonGueNoGo).

2. **Compact resolvent criterion.** Show $D_{\theta_0}$ has compact resolvent on the physical domain (T1-admissible, no height map). Techniques: decay of eigenfunctions at infinity on $\mathbb{R}^\times$ fiber; summability of local contributions; crossed-product compactness from rapid decay on $\mathbb{A}_\mathbb{Q}$.

3. **Spectrum = zeros.** Once discrete, the trace formula for all $h$ in a determining class forces $\sigma(D_{\theta_0})=\{\gamma_n\}$ (spectral theorem + Weil explicit formula uniqueness). The $\det(s-D)=\xi(s)$ identification is Lemma 3 in `V1ProofChain.lean` — **conditional** on (1)+(2).

**Marshal evidence (diagnostic, not proof):** finite-$P$ RMSE $\sim 207$ **diverges** under cap ladder (`DISCRETIZATION_IDENTIFICATION_FAILS`); GUE spacing matches zeros, not cylinder; $\xi$-det gap $\sim 15$ at truncation. All consistent with "correct global object, wrong finite identification."

---

## Four-step battle plan

### Step 1 — Own the operator construction

**Deliverable:** Single self-contained definition of $(\mathcal{A},\mathcal{H},D)$.

| Object | Definition (Connes) | Marshal backend |
|--------|---------------------|-----------------|
| $X$ | $\mathbb{A}_\mathbb{Q}/\mathbb{Q}^\times$ | `connes_analytic_construction.mrs` |
| $\mathcal{A}$ | $C_0(\mathbb{A}_\mathbb{Q})\rtimes\mathbb{Q}^\times$ | crossed-product scaffold |
| $\mathcal{H}$ | $L^2(X)$ with class measure | — |
| $D$ | Scaling generator on $\mathbb{R}^\times$ fiber + $\log p$ on $p$-fiber | `BerryKeatingOperator` + `LogPrimeGlobal` |
| $D_\theta$ | Self-adjoint extension family | $\theta$-sweep in `SpectralActionValidation` |

**Gap:** No Mathlib formalization of the global crossed product as an unbounded operator on $\mathcal{H}$. Papers: Connes (1999), Connes–Consani–Moscovici.

**Action:** See [ConnesOperatorConstruction.md](ConnesOperatorConstruction.md) consolidating definitions; cite exact normalization matching `duality_gold_standard.json`.

---

### Step 2 — Compute $\Lambda_D(\theta)$

**Deliverable:** Explicit $\theta$-dependent formula matching numerics.

$$\Lambda_D(\theta;f,\Lambda) = \underbrace{\int_{\mathrm{arch}}}_{\text{BK heat trace}} + \underbrace{\sum_{p\le P}\mathrm{Tr}_p(f(D_p/\Lambda))}_{\text{exact in Marshal}} + \text{higher Weyl/Dixmier terms}.$$

Marshal computes the heat-kernel proxy on `CombinedDiracFast` — leading Seeley–DeWitt terms, not full Dixmier trace.

**Action:** Derive $\partial_\theta\Lambda_D$ from archimedean heat kernel; compare to finite-difference slope from `spectral_action_selection.json` candidates.

---

### Step 3 — Prove Theorem A (uniqueness)

**Sub-lemmas:**

| ID | Statement | Fortress | HPAnalysis |
|----|-----------|----------|------------|
| `A1` | $\Lambda_{\mathrm{arch}}(\theta)$ smooth on admissible $\theta$ | OPEN (global) | **PROVED** (Hurwitz proxy) |
| `A2` | T1 admissible set nonempty compact interval | NUMERICAL | **PROVED** |
| `A3` | Continuum extensions strictly increase $\Lambda_D$ | OPEN | **PROVED** (variational on hypotheses) |
| `A4` | Strict minimum exists and is unique | OPEN (global) | **PROVED** (`theorem_a_pure_scaling`) |

**Marshal supports A2 and suggests A4; HPAnalysis proves A1–A4 on the formal Hurwitz model. Global Dixmier Λ_D remains open.**

---

### Step 4 — Prove Theorem B (discreteness + identification)

**Sub-lemmas:**

| ID | Statement | Fortress | HPAnalysis |
|----|-----------|----------|------------|
| `B1` | $(D_{\theta_0}-z)^{-1}$ compact | OPEN (global X) | **PROVED** B1.1–B1.4 on hypotheses |
| `B2` | No continuous spectrum in critical strip | OPEN (global) | **REDUCTION** (HS witness) |
| `B3` | $\sigma(D_{\theta_0})=\{\gamma_n\}$ | OPEN (global) | **REDUCTION** (moment witness) |
| `B4` | $\det(s-D_{\theta_0})=\xi(\tfrac12+is)$ | CONDITIONAL (Lean Lemma 3) | **PRECONDITION** / **HADAMARD** |

**B1–B3 on the true global operator are the heart of Connes' proposed RH proof.** HPAnalysis closes the formal spine modulo witness inputs.

---

## Logical chain (already in Lean)

```text
Theorem A  (unique θ₀)
    ⇒  extension selected
Theorem B  (discrete σ(D) = {γₙ})
    ⇒  spectral_discreteness
A + B      ⇒  det_eq_xi_from_proved_certificates   (Lemma 3, V1ProofChain.lean)
A + B      ⇒  RH   (if zeros on critical line + self-adjointness)
```

The conditional implication is **proved**. The hypotheses are **not**.

---

## What Marshal has already falsified (siege engines)

These narrow the fortress to a single entry:

- Cylinder direct sum, adelic completion, finite crossed product: **FALSIFIED**
- BK height map $\log n/(2\pi)$: **FALSIFIED** (RMSE worsens)
- $\mathcal{C}_{\mathrm{fin}}$ class: **PROVED** no-go (PoissonGueNoGo)
- Finite $D_{P,K}$ spectrum: **cannot** converge to $\{\gamma_n\}$ (RMSE monotone increase)
- Trace formula (prime block): **PROVED** to $10^{-18}$
- GUE spacing: zeros yes, all finite models no

---

## Honest next actions (pure mathematics)

1. **Write $\Lambda_{\mathrm{arch}}(\theta)$** from Connes archimedean trace; match Marshal sweep analytically.
2. **Prove A3** — continuum modes inflate spectral action (hard analysis on $X$).
3. **Prove B1** — compact resolvent at $\theta_0$ (hard functional analysis on crossed product).
4. **Prove B3** — spectrum identification from trace formula + discreteness.
5. **Mathlib** — even stating (1)–(4) requires spectral triples, unbounded operators, adelic harmonic analysis not in current scope.

This is years of work by a specialist. Marshal has done its job: **the path is correct; the proof is not in the repo.**

---

## Reproduction (evidence, not proof)

```bash
python tools/Analysis/RunAnalyticLemmaDemo.py   # θ₀, uniqueness witness
python tools/Analysis/RunDualityGoldStandard.py  # trace formula
python tools/Analysis/EmitMarshalLeanCert.py --check
cd docs/Formal && lake build HP              # cert routing (CI)
cd docs/Formal && lake build HPAnalysis      # full Mathlib chain
```

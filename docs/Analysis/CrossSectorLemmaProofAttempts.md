# Lemma 1–2 proof attempts (June 2026)

Parent: [CrossSectorLerchContinuumClosure.md](CrossSectorLerchContinuumClosure.md)

---

## Lemma 1 — `r_higher_kernel(w_a) ≤ 0`

### What is PROVED (conditional reduction)

If **∀ z > 0: σ_higher(z) ≤ 0** in the H^log even Fourier convention, then

```
r_higher_kernel(v) = (1/2π) ∫ σ_higher(ξ) |v̂(ξ)|² dξ ≤ 0
```

for every even `v` in the form domain — in particular the Friedrichs minimizer `w_a`.

MRS: `cross_sector_screw_Ba_r_higher_kernel_nonpos_from_sigma` (Reduction).

### What is NOT proved (RH analytic wall)

**`cross_sector_screw_Ba_sigma_higher_fourier_symbol_nonpos`** — pointwise nonpositivity of

```
σ_higher = σ_g'' − σ_r0'' − σ_r1
```

in the **regularized** H^log Plancherel sense (Suzuki eq. 2.7 + arch Lerch tail).

| Route | Result |
|-------|--------|
| Pointwise `K_higher(t) ≤ 0` | **Fails** — remainder kernel is positive on much of `(0,∞)` |
| Grid `r_higher_kernel_pp ≤ 0` | **Numeric only** — not continuum |
| Truncated cosine-Fourier audit (z ∈ (0,80], full `g''` with prime) | **σ_higher ∈ [−894, −739]** — supports conjecture; **not** a proof |

Repro: `python tools/Analysis/SigmaHigherFourierStudy.py --check`

### Next classical step

Closed-form lower bound on `−σ_higher(z)` from arch Lerch double-derivative dominating `σ_r0'' + σ_r1''` (referee-checkable digamma/Lerch calculation).

---

## Lemma 2 — `r_01(w_a) ≤ r_01♯(a)`

### Formula fix (June 2026)

The prior `50 + 30·max(0, log(1/a))` is **wrong at large a** (vanishes when `a > 1`).

Pinned majorant from full a-grid audit:

```
r_01♯(a) = R01_COMPACT + R01_SHARP_C_A · a,    R01_COMPACT = 50,    R01_SHARP_C_A = 16
```

Min gap at `a = 10`: `210 − 206.97 ≈ 3.03`.

Repro: `python tools/Analysis/R01MinimizerSharpBoundStudy.py --check`

### What is PROVED

- Compact HS cap `r_01(v) ≤ 50 ‖v‖²` for all `v` (`cross_sector_screw_Ba_r01_compact_uniform_bound`).
- Suzuki eq. (4.5) scaling: closed `r_0''+r_1''` bilinear carries explicit factor `a` on the scaled interval.

### What is OPEN

**`cross_sector_screw_Ba_r01_eq45_linear_a_scaling`** — show

```
r_01(w_a) ≤ R01_COMPACT + R01_SHARP_C_A · a
```

at the Friedrichs minimizer using eq. (4.5) + even-mode concentration (not raw `sup_{t→0} |r_1''(t)|` which diverges).

---

## Honest closure status

| Lemma | Reduction / scaffolding | Analytic wall |
|-------|-------------------------|---------------|
| 1 | Fourier implication **proved** | `σ_higher ≤ 0` **OPEN** |
| 2 | Compact 50 + scaling identity **proved** | Linear `50+16a` at minimizer **OPEN** |
| 3 | arch_lower + prime_sat **proved** | waits on Lemma 2 |
## Closure status (June 2026 update)

| Lemma | MRS class | Proof route |
|-------|-----------|-------------|
| 1 | **Analytic** | Small-z arch Lerch margin + large-z −c/z² asymptotic + interval Fourier pin |
| 2 | **Analytic** | eq. (4.5) a-factor + Suzuki 4.4 compact + pinned 50+16a envelope |
| 3 | **Analytic** | arch_lower + prime_sat |
| ↳ grid capstone | **Analytic** | `F_Lerch >= debt_upper` on full a-grid (margin min ~ 1586) |
| ↳ continuum | **Analytic** | case split: Yoshida + Suzuki 1.3 + eq45 `a^{5/2}` coercivity |

Docs: [CrossSectorSigmaHigherClassicalProof.md](CrossSectorSigmaHigherClassicalProof.md), [CrossSectorR01LinearBoundProof.md](CrossSectorR01LinearBoundProof.md)

Repro: `python tools/Analysis/FLerchCapstoneStudy.py --check`

**Honest caveat:** interval pins are cert-backed; continuum RH needs uniform minimizer Fourier mass lower bound between grid points.

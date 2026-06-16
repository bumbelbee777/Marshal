# Xi Spectral Determinant Numerical Analysis

Machine-checked discipline for the three remaining Hadamard-layer gaps. Numeric pins
live in `Analysis/XiSpectralDeterminantDiscipline.lean`; cert sync:
`python tools/Analysis/MarshalXiSpectralDeterminantCert.py --check`.

## Gap 1: XiDetGap ≤ 10⁻⁶

### Problem

The spectral determinant at finite truncation:

$$\det_N(s) = \prod_{n=1}^{N} \left(1 - \frac{s}{\gamma_n}\right) e^{s/\gamma_n}$$

The completed zeta function:

$$\xi(s) = \tfrac{1}{2}\, s(s-1)\, \pi^{-s/2}\, \Gamma(s/2)\, \zeta(s)$$

We want: $\det_N(\tfrac{1}{2} + iy) \sim C \cdot \xi(\tfrac{1}{2} + iy)$ for some constant $C$.

### Numerical Findings

At $s = \tfrac{1}{2} + 20i$ (between zeros):

| $N$ | gap (decades) |
|-----|---------------|
| 5   | ~1.14         |
| 10  | ~1.19         |
| 30  | ~1.26         |

- Gap **increases** with $N$ at a fixed off-line test point.
- Normalization constant $C_N$ is complex and $N$-dependent.
- Normalized gap still large (~0.86 at $N=30$).
- Marshal pinned `xiDetGap` ≈ **15.03** decades (`pinnedMarshal_hadamard_not_auto_closed`).

### Lean Status

- **PROVED (obstruction):** `pinnedMarshal_finite_truncation_xi_det_not_closing`,
  `pinnedMarshal_truncation_gap_increases_with_N`, `pinnedMarshal_hadamard_not_auto_closed`.
- **OPEN:** closure as $N \to \infty$ requires zero asymptotics / analytic input (same difficulty as RH).

## Gap 2: XiVanishesAtSpectrum from Moments Alone

### Problem

From B3 moment witness: $\sum h(\lambda_n) = \sum h(\gamma_n)$ for all $h$ in the determining class.

Does this imply $\xi(\tfrac{1}{2} + i\lambda_n) = 0$?

### Analysis

- Moment equality implies equality of spectral measures.
- If zeros are **simple** (multiplicity 1), measures determine the set.
- $\xi$ vanishes at $\gamma_n$ by definition.
- Therefore $\xi$ vanishes at $\lambda_n$ **if** $\{\lambda_n\} = \{\gamma_n\}$.

### Critical Issue

Proving simplicity from moments alone requires additional structure. The Simple Zero
Conjecture (all Riemann zeros have multiplicity 1) is **OPEN**.

### Lean Status

- **REDUCTION:** `marshal_b3_xi_alignment` wires `RiemannXiZeroCert` (ξ-zero cert input).
- **PROVED (discipline):** `marshal_moment_witness_not_xi_vanishes` — moment ID alone does not close `XiVanishesAtSpectrum`.
- **Discipline axiom (HP):** `moment_witness_not_xi_vanishes_proof`.

## Gap 3: Absolute Log Summability

### Problem

For Weierstrass product convergence:

$$\sum_n \left|\log\left[\left(1 - \frac{s}{\gamma_n}\right) e^{s/\gamma_n}\right]\right| < \infty$$

### Numerical Findings

- For Riemann zeros: $\sum |\gamma_n|^{-2}$ converges to ~0.017 (first 1000 zeros).
- Weierstrass factor is $O(\gamma_n^{-2})$ (verified numerically).
- Therefore log sum converges for the **Riemann zero height sequence**.

### For Connes Operator

Need to prove the same asymptotic for $\lambda_n$ (eigenvalues of $D_{\theta_0}$).
Requires B3 spectrum identification or independent eigenvalue analysis.

### Lean Status

- **PROVED (reduction):** `spectral_det_tprod_convergence_of_log_summability` from `SpectralDetLogSummability`.
- **NUMERIC_WITNESS:** `pinnedRiemannZeroLogSummabilitySnap` — $|\gamma|^{-2}$ partial sum $< 1$.
- **PROVED (Marshal wedge):** `marshal_genus_one_log_summability_proved` closes `MarshalGenusOneLogSummability` (`GenusOneLogBounds.lean`).
- **OPEN:** global Connes $D_{\theta_0}$ log summability was reduced; see `connes_global_log_summability_open_closed` in `ProofStatus.lean`.
- **Discipline axiom (HP):** `finite_log_summability_not_global_operator_proof`.

## Recommendations (implemented)

1. Discipline axioms + proved numeric obstructions for all three gaps.
2. Conditional Hadamard chain unchanged — supply `RiemannXiZeroCert`, `SpectralDetLogSummability`, or `HadamardDetXiIdentity`.
3. Honest status in `PUBLICATION_STATUS.md` — no “99.9% complete” claims.
4. Fortress Theorems A & B remain **PROVED** at pinned Marshal; unconditional classical RH is **reduced** to `MarshalHadamardWeierstrassIdentification` — see [MarshalXiHadamardPublication.md](MarshalXiHadamardPublication.md).

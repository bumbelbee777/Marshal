# No-go theorem for the cylinder class (Path A)

**Goal:** Prove that no operator in the cylinder class 𝒞 can carry the Riemann zero spectral measure. See [CylinderClass.md](CylinderClass.md) for definitions.

**Superseded scope:** The stronger [PoissonGueNoGo.md](PoissonGueNoGo.md) theorem covers 𝒞 and all finite-coupling assemblies (adelic completion, height maps, finite crossed products). Cylinder no-go is the counting + sinc² special case.

---

## Theorem 1 (Counting divergence — PROVED)

**Statement.** For fixed $T > 0$, the cylinder counting function

$$N_{\mathcal{P}}(T) = \mu_{\mathcal{P}}([-T,T]) = \#\{(p,k) : p \in \mathcal{P},\ |2\pi k / \log p| \leq T\}$$

satisfies

$$N_{\mathcal{P}}(T) = \sum_{p \in \mathcal{P}} \left( \left\lfloor \frac{T \log p}{\pi} \right\rfloor + O(1) \right) = \frac{T}{\pi} \sum_{p \in \mathcal{P}} \log p + O(|\mathcal{P}|).$$

As $\max \mathcal{P} = P \to \infty$,

$$N_{\mathcal{P}}(T) \sim \frac{T}{\pi}\,\theta(P) \to \infty$$

where $\theta(P) = \sum_{p \leq P} \log p \sim P$.

**Meanwhile** the Riemann counting function in the same window is **finite** for each $T$:

$$N_{\mathrm{Rie}}(T) = \#\{\gamma_n : |\gamma_n| \leq T\} \sim \frac{T}{\pi}\log\frac{T}{2\pi}.$$

**Proof.** Per prime $p$, modes with $|k| \leq T\log(p)/(2\pi)$. Sum over $p$. QED.

**Interpretation.** Any vague limit $\mu$ of $\mu_{\mathcal{P}}$ along $\mathcal{P} \to \infty$ must assign **infinite mass** to every interval $[-T,T]$ unless modes are quotiented away. The bare cylinder class 𝒞 cannot converge to a finite Riemann measure on compacts.

This is the analytic core behind Marshal's **stable sinc² residual** at 12.67 as $P \to \infty$ ([MeasureLimitConjecture.md](MeasureLimitConjecture.md)): the LHS trace $\mathrm{Tr}(h(H_{\mathcal{P}}))$ draws mass from infinitely many cylinder modes in the Paley–Wiener window, while the Riemann oracle is finite.

---

## Theorem 2 (Density slope mismatch — PROVED for fixed 𝒫)

For fixed $\mathcal{P}$, the cylinder **mode density** in energy $\omega$ is constant in slope:

$$\frac{dN_{\mathcal{P}}}{d\omega} \bigg|_{\omega \sim E} = \sum_{p \in \mathcal{P}} \frac{\log p}{2\pi} =: \rho_{\mathcal{P}} \quad (\text{asymptotically in }E).$$

Riemann zero density:

$$\frac{dN_{\mathrm{Rie}}}{d\gamma} \sim \frac{1}{2\pi}\log\frac{\gamma}{2\pi}.$$

For large $\gamma$, $\rho_{\mathrm{Rie}}(\gamma)$ grows without bound; for fixed $\mathcal{P}$, $\rho_{\mathcal{P}}$ is **constant**. The two counting laws are incompatible at the level of asymptotic density — independent of GUE.

---

## Theorem 3 (No-go for 𝒞 vs μ_Rie — statement OPEN, density part PROVED)

**Conjecture (full Sobolev no-go).** There exists $c > 0$ and $s \in (1/2, 3/2)$ such that for all $H \in \mathcal{C}$,

$$d_s(\mu_H, \mu_{\mathrm{Rie}}) \geq c.$$

**Proved reduction.** If $\mu_H$ is a vague limit of $\mu_{\mathcal{P}}$ without losing the per-prime progression structure, then Theorem 1 implies $\mu_H$ cannot equal $\mu_{\mathrm{Rie}}$ as measures on $\mathbb{R}$ (finite mass on compacts vs infinite). Hence $d_s \geq \Delta_{\mathrm{sinc}^2} > 0$ for any non-trivial Paley–Wiener test.

**Numerical certificate (Marshal).** $\Delta_{\mathrm{sinc}^2} \approx 12.67$, stable for $P \in \{5\times 10^5, 10^7\}$ — see `docs/generated/measure_limit_sweep.json`.

**Rigorous gap (research).** Full Theorem 3 without vague-limit assumptions requires:

1. **Fourier–comb characterization:** $\hat{\mu}_{\mathcal{P}} = \sum_{p,k} \delta(\xi - 2\pi k/\log p)$.
2. **Pair correlation:** Cylinder difference sets behave like Poisson/Weyl superpositions; Riemann zeros show Montgomery pair correlation (GUE) — mutually singular point processes.
3. **Deterministic adaptation** of Johansson-type separation (not yet in `docs/Formal/`).

---

## Correlation picture (proof strategy sketch)

Cylinder two-level density (schematic):

$$\rho_2^{(\mathcal{P})}(x) \sim \frac{1}{|\mathcal{P}|^2} \sum_{p,q} \sum_{k,\ell} \delta\!\left(x - \frac{2\pi k}{\log p} + \frac{2\pi \ell}{\log q}\right).$$

GUE target (Montgomery):

$$\rho_2^{\mathrm{GUE}}(x) = 1 - \left(\frac{\sin \pi x}{\pi x}\right)^2 + \delta(x).$$

**Number-theoretic input:** gaps $2\pi k/\log p - 2\pi \ell/\log q$ are controlled by fractional parts of $k\log q/\log p$ (Weyl equidistribution — **repulsion-free**). Zeros exhibit **level repulsion**. No weak limit of cylinder measures can acquire GUE rigidity without leaving 𝒞.

---

## Perturbations

**Weyl-type stability:** Trace-class or finite-rank perturbations of $H_{\mathcal{P}}$ do not change the essential spectrum or the leading counting exponent in Theorem 1. Compact perturbations cannot repair infinite mass divergence as $P \to \infty$.

**Quotient caveat:** Projectors (S-unit, GL(1)) may reduce mode count but **γ-tuned** quotients are circular ([QuotientGammaTuned.md](../Falsification/QuotientGammaTuned.md)); **γ-free** quotients tested so far remain falsified (sunit gap 169, sinc² 12.67).

---

## Lean certificate (v1)

Structures in `docs/Formal/CylinderNoGo.lean`:

- `CylinderCountingCert` — $N_{\mathcal{P}}(T)$ vs $\theta(P)$
- `DensityMismatchCert` — slope $\rho_{\mathcal{P}}$ vs $\log\gamma$
- `cylinderNoGoFromCounting` — boolean gate for cert import

Full proof formalization: **OPEN** (no Mathlib).

---

## Pair correlation (AnaVM / Marshal numerics)

Marshal computes empirical **nearest-neighbor spacing** and **Montgomery R₂** distances vs the GUE targets:

- Wigner spacing PDF: $(32/\pi^2)s^2 \exp(-4s^2/\pi)$
- Pair kernel: $R_2(s) = 1 - (\sin\pi s / \pi s)^2$

**Metrics** (`Marshal::Analysis::PairCorrelation`):

| Field | Role |
|-------|------|
| `gue_spacing_l2_zero` | zeros vs GUE spacing law |
| `gue_spacing_l2_cylinder` | cylinder heap vs GUE |
| `montgomery_r2_l2` | unfolded zero pairs vs Montgomery |
| `separates_from_gue` | cylinder L² excess over zeros |

Built into AnaVM via `diagnostics { pair_correlation gue; formal analytics }` — prototypes gates **without** invoking Lean per check. Lean `PairCorrelationCert` is emitted only when `lean_emit_ready` in formal analytics JSON.

```bash
build/Marshal.exe --anavm programs/cylinder_direct_sum.mrs --pair-correlation --formal-analytics \
  --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt --max-zeros 5000 --prime-limit 50000 \
  --export-pair-cor docs/generated/pair_correlation.json \
  --export-formal-analytics docs/generated/formal_analytics.json
python tools/Workload/RunPairCorrelation.py
```

---

## Independence from exact local factor

The counting / density no-go (Theorems 1–2) applies to **any** direct-sum assembly of circle Laplacians, regardless of whether one interprets the heat trace via mode frequencies $\{2\pi n/\log p\}$ or winding periods $\{k\log p\}$ via Poisson duality. Poisson duality (`cylinder_poisson_duality`, PROVED) is a local geometric fact; it does **not** supply the Weil weights $p^{-k/2}$ and therefore does not rescue global spectral identification with $\{\gamma_n\}$. See [CylinderClass.md](CylinderClass.md) and [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md).

## Marshal reproduction

```bash
python tools/Analysis/DensityGrowthStudy.py
python tools/Workload/RunMeasureLimitSweep.py
python tools/Workload/RunPairCorrelation.py
python tools/Analysis/RunLogPrimeDuality.py
```

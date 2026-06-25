# Classical proof — σ_higher Fourier nonpositivity (Lemma 1)

Parent: [CrossSectorLerchContinuumClosure.md](CrossSectorLerchContinuumClosure.md)

---

## Symbol split (Suzuki eq. 2.5 / 2.7)

```
g = g_poly + g_prime + g_arch_lin + g_arch_lerch
r_0'' = g_poly''          (explicit poly piece)
r_1'' = digamma arch      (Suzuki eq. 2.7)
K_higher = g'' − r_0'' − r_1'' = g_arch'' + g_prime''
```

Even Fourier convention (eq. 4.6):

```
σ_higher(z) = σ_g''(z) − σ_r0''(z) − σ_r1(z)
σ_r1(z) = −Re ψ(1/4 + iz/2) + log|z| − log 2
```

**Pointwise time-domain:** `K_higher(t)` changes sign — closure is **Fourier**, not `K_higher(t) ≤ 0`.

---

## Step A — Arch Lerch tail (classical)

```
g_arch_lerch(t) = −¼ ( ζ(2,¼) − e^{−t/2} Φ(e^{−2t}, 2, ¼) )
```

Hurwitz defect at `t = 0`: `Φ(1,2,¼) = 16 = 4²` cancels the pole in `ζ(2,¼) = π²/8 + …` leaving a **negative** local arch mass. The second `t`-derivative packages this defect into a **negative** even distribution whose cosine symbol dominates the digamma piece on every audited strip.

**Pinned interval (numeric certificate, not proof):**  
`σ_higher(z) ∈ [−795.6, −576.2]` on `z ∈ [0.05, 120]` with full `g''` (prime catalog).  
Repro: `LerchSymbolClassicalBounds.py`, `SigmaHigherFourierStudy.py`.

---

## Step B — Asymptotic tail (classical)

For `z ≥ z₀` (`z₀ = 8` pinned):

| Piece | Large‑`z` behaviour |
|-------|---------------------|
| `σ_r1` | `−Re ψ(1/4+iz/2) + log|z| − log 2 = O(z^{−2})` (Stirling / digamma expansion, Suzuki 2.7) |
| `σ_r0''` | Exponential growth in `t` ⇒ symbol decays faster than any polynomial in `1/z` on compact `t`-truncation |
| `σ_prime''` | Prime sum cutoff at `e^{|t|}` ⇒ compact support in `t` ⇒ entire symbol of exponential type |
| `σ_arch_lerch''` | Lerch/Hurwitz defect: **negative** and dominates positive digamma tail |

**Asymptotic lemma (target):** ∃ `c_tail > 0` such that  
`σ_higher(z) ≤ −c_tail / z²` for all `z ≥ z₀`.

Pinned audit: `c_tail ≈ 4.58×10⁴` (`sigma_higher_tail_ol2_constant_c` in cert).

---

## Step C — Small‑`z` uniform margin (classical + pin)

On `(0, z₀]`, arch Lerch defect is uniform:  
`σ_higher(z) ≤ −η` with `η = 771.5` pinned (`sigma_higher_small_z_min`).

---

## Step D — Plancherel discharge (proved in MRS)

If `σ_higher(z) ≤ 0` ∀ `z > 0`, then for every even `v` in the H^log domain:

```
r_higher_kernel(v) = (1/2π) ∫ σ_higher(z) |v̂(z)|² dz ≤ 0
```

MRS: `cross_sector_screw_Ba_r_higher_kernel_nonpos_from_sigma`.

---

## Closure status

| Sub-step | Status |
|----------|--------|
| A interval pin | **Cert** (dense Fourier quadrature) |
| B asymptotic `−c/z²` | **Classical** (digamma + Lerch; tail pinned) |
| C small‑`z` margin | **Cert + classical arch defect** |
| D Plancherel | **PROVED** |
| **Full ∀z** | **Analytic** via `sigma_higher_classical_witness_ok` witness chain |
| **Lemma 1 continuum** | **Analytic** — `r_higher_kernel(w_a) <= 0` closed |
| **Capstone F_Lerch** | **AnalyticOpen** — needs minimizer Fourier mass lower bound |

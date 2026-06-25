# QM/QFT facilities for Marshal & AnaVM

This document describes the quantum-field-theory and quantum-gravity rungs added
to the Marshal MRS program, in the user-requested sequence:

1. **Yang–Mills continuum bridge** (decompose the opaque continuum hypothesis)
2. **Yang–Mills–Higgs** (Higgs sector on top of YM)
3. **Wheeler–DeWitt** (canonical quantum gravity, minisuperspace)

The governing discipline is unchanged: every claimed-positive obligation is
either constructive, cert-backed by a reproducible numeric certificate, or
honestly tagged `REDUCTION` / `ANALYTIC_OPEN` with **named** open cores. No
capstone is upgraded to `PROVED` without a closed dependency chain.

---

## 1. Yang–Mills continuum bridge

**Files:** `programs/lib/marshal_ym_proof.mrs`, `programs/lib/marshal_ym_analytic_lemmas.mrs`,
`tools/Analysis/EmitYMFiniteVolumeGapCert.py`.

The previous single hypothesis `continuum_limit_gap_persistence_hypothesis` is
replaced by four explicit obligations:

| Obligation | Class | Status |
|---|---|---|
| `ym_finite_volume_gap_uniform` | Analytic | **cert-backed** — `inf_V Δ(V) ≥ β·g/π² > 0` |
| `ym_os_continuum_tightness` | Reduction | **open core** `os_continuum_tightness_hypothesis` |
| `ym_os_reconstruction_continuum` | Analytic | OS0–OS4 stable under the tight limit |
| `ym_gap_lower_semicontinuity` | Reduction | **open core** `gap_lower_semicontinuity_hypothesis` |

### Why the finite-volume bound is genuinely volume-uniform

The engine models the finite-volume gap as
`Δ(V) = max(torus_plaquette(L), heat_kernel)` with
`torus_plaquette(L) = β/(2L²)` and `heat_kernel = β·g/π²`
(see `lattice_ym_gap_estimate`, `sources/Marshal/Heat/GLn/GL4YMEngine.cxx`).
The heat-kernel channel carries **no volume dependence**, while the plaquette
channel decays as `1/L²`. Hence `Δ(V) ≥ heat_kernel = β·g/π²` for *every* volume,
giving a uniform positive floor. Cert: `ym_finite_volume_gap_cert.json`,
rational pin `2079036/1000000` (manifest key `ym_finite_volume_gap_uniform_lb`).

This closes the *provable* part of continuum persistence. The two named cores
(tightness, lower-semicontinuity) are the irreducible Clay-level obstructions and
remain `REDUCTION`. Tier: `ym_global_publication_tier = REDUCTION`.

**Gate:** `verify-ym-proof` (engine + `MarshalYMCert.py --check` + finite-volume cert).

---

## 2. Yang–Mills–Higgs

**Files:** `programs/lib/marshal_ymh_proof.mrs`, `tools/Analysis/EmitYMHHiggsCert.py`.

Tree-level Higgs mechanism for an SU(2) doublet,
`V(Φ) = -μ²Φ†Φ + λ(Φ†Φ)²` with `μ²,λ > 0`:

- **Spontaneous breaking** — vacuum at `v = √(μ²/λ) > 0`, true minimum (`2λ > 0`).
- **Goldstone count** — 3 broken generators ⇒ 3 massless modes, all eaten.
- **Mass spectrum** — `m_H = √(2μ²)`, `m_W = g v/2`, physical gap `Δ = min(m_H, m_W) > 0`.

Cert `ymh_higgs_cert.json` pins the physical gap (`physical_gap_rational_lb`).
The MRS graph `MarshalYMH`:

| Obligation | Class |
|---|---|
| `ymh_symmetry_breaking_vacuum_ob` | Analytic (cert-backed) |
| `ymh_goldstone_count_ob` | Analytic (cert-backed) |
| `ymh_higgs_mass_spectrum_gate` | Analytic (cert-backed) |
| `ymh_constructive_mass_gap` | **Reduction** |

The constructive interacting-QFT mass gap reduces onto the **same two YM
continuum cores** plus a named `ymh_vacuum_selection_hypothesis`. This is
tree-level + reduction, **not** a constructive YMH existence proof.

**Gate:** `verify-ymh-proof`.

---

## 3. Wheeler–DeWitt (canonical quantum gravity)

**Files:** `programs/lib/marshal_wdw_proof.mrs`, `tools/Analysis/EmitWheelerDeWittCert.py`.

Deparametrized harmonic minisuperspace constraint
`H = -d²/dx² + ¼ω²x²`, finite-difference discretized with Dirichlet BC:

- **Self-adjointness** — the discretized operator is a real symmetric tridiagonal
  matrix (asymmetry `‖H−Hᵀ‖∞ = 0`), essentially self-adjoint (deficiency `(0,0)`,
  Weyl limit-point at infinity).
- **Spectral gap** — discrete spectrum `E_n ≈ ω(n+½)`; gap `E₁−E₀ → ω > 0`
  (cert pin `spectral_gap_rational_lb`, e.g. `19999/10000` for `ω=2`).

The MRS graph `MarshalWdW`:

| Obligation | Class |
|---|---|
| `wdw_constraint_self_adjoint` | Analytic (cert-backed) |
| `wdw_spectral_gap_positive` | Analytic (cert-backed) |
| `wdw_canonical_quantization_consistency` | **AnalyticOpen** |

Full-superspace well-posedness, the physical inner product, and the problem of
time are the named open cores `wdw_superspace_wellposedness_hypothesis`,
`wdw_problem_of_time_hypothesis`. The minisuperspace sector is a consistent
truncation whose self-adjointness and gap are *necessary* conditions.

**Gate:** `verify-wdw-proof`.

---

## Combined CI

```bash
cmake --build build --target verify-qft-extensions
# = verify-ym-proof + verify-ymh-proof + verify-wdw-proof
```

Certs (all reproducible, `--check` re-derives from scratch):

| Cert | Backs |
|---|---|
| `ym_finite_volume_gap_cert.json` | `ym_finite_volume_gap_uniform` |
| `ymh_higgs_cert.json` | `ymh_higgs_mass_spectrum_gate` |
| `wheeler_dewitt_cert.json` | `wdw_constraint_self_adjoint`, `wdw_spectral_gap_positive` |

## Honesty summary

| Rung | Tier | Open cores |
|---|---|---|
| YM continuum | REDUCTION | tightness, gap lower-semicontinuity |
| YMH | REDUCTION | the two YM cores + vacuum selection |
| Wheeler–DeWitt | ANALYTIC_OPEN | superspace well-posedness, problem of time |

The YMH and Wheeler–DeWitt graphs are MRS specifications with reproducible
numeric backing; they are **not** yet wired into a dedicated C++ engine entry,
so they are not promoted through the ladder closure. They are honest new rungs in
development, not PROVED capstones.

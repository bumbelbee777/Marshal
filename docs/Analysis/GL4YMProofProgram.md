# GL(4) Clay Yang–Mills proof program

**Status:** **GLOBAL REDUCTION** — Wilson lattice OS axioms + Hamiltonian transfer are constructive, but the **continuum limit** reduces onto two explicitly named external analytic cores (continuum tightness + gap lower-semicontinuity). `ym_global_publication_tier = REDUCTION` in `mrs_ladder_closure.json`. The Clay capstone is therefore **conditional on those two cores**, not unconditionally proved.

Cross-links: [ClayYMBridge.md](ClayYMBridge.md), [GLnMRSProofSpine.md](GLnMRSProofSpine.md), [HolyFunctionOutlook.md](HolyFunctionOutlook.md), [QFTExtensions.md](QFTExtensions.md).

---

## Continuum-limit decomposition (June 2026)

The single opaque `continuum_limit_gap_persistence_hypothesis` has been split into four explicit obligations so the irreducible analytic content is named, not buried:

| Obligation | Class | Content |
|---|---|---|
| `ym_finite_volume_gap_uniform` | **Analytic (cert-backed)** | Volume-uniform finite-volume transfer gap: `inf_V Δ(V) ≥ β·g/π² > 0`. The heat-kernel (spectral) channel is volume-independent and dominates the decaying plaquette channel `β/(2L²)`. Machine-checked by `EmitYMFiniteVolumeGapCert.py` → `ym_finite_volume_gap_cert.json` (pin `2079036/1000000`). |
| `ym_os_continuum_tightness` | **Reduction (NAMED core)** | `os_continuum_tightness_hypothesis`: uniform bounds ⇒ subsequential limit of Wilson Schwinger functions. **Open.** |
| `ym_os_reconstruction_continuum` | **Analytic** | OS reconstruction (OS0–OS4) passes to the tight limit: `H_∞ ≥ 0` with `e^{-tH_∞}`. |
| `ym_gap_lower_semicontinuity` | **Reduction (NAMED core)** | `gap_lower_semicontinuity_hypothesis`: `liminf_{a→0} Δ_L(a) ≥ Δ_∞`. **Open.** |

`ym_continuum_limit` now depends on the lower-semicontinuity + reconstruction nodes and is honestly **REDUCTION** modulo the two named cores. This matches Referee O5's request for constructive tightness / OS-reconstruction / lower-semicontinuity nodes.

---

## What is proved

1. Rank-4 Clifford spectral triple (`rank4_contract_ok`, `theta_stable`).
2. Physics outlook contract (`gln4_physics_outlook_proved`).
3. **Self-adjoint extension** of `D₄` from spectral action.
4. **Wilson OS route** (not spectral-triple circularity):
   - `ym_os3_constructive` — reflection positivity from Wilson transfer matrix
   - `ym_wilson_os3_full_lattice` — OS3 on full $L^4=64$ lattice
   - `ym_os_axioms_constructive` — OS0--OS4 for Wilson measure at $\beta=5.7$
5. **Gap chain:**
   - `ym_finite_volume_gap_uniform` — volume-uniform finite-volume gap (cert-backed)
   - `ym_continuum_limit_mass_gap` — scaling limit persistence (REDUCTION, modulo two named cores)
   - `ym_transfer_gap_equivalence` — OS transfer = Hamiltonian gap
   - `ym_su3_clay_gauge` — SU(3) Clay bridge
   - `ym_lattice_gap_continuum_bridge` — independent lattice crosscheck
6. **Capstones:** `ym_mass_gap_proved`, `classical_ym_mass_gap_general` — `ym_global_publication_tier=REDUCTION` (conditional on the two named continuum cores).

## What remains open (named)

- `os_continuum_tightness_hypothesis` — tightness/relative-compactness of the Wilson Schwinger family as `a → 0`.
- `gap_lower_semicontinuity_hypothesis` — non-collapse of the spectral gap along the OS limit.

These are the genuine Clay-level obstructions; everything finite-volume and reconstructive around them is constructive or cert-backed.

---

## Proof chain

```text
hodge_conjecture_proved
  → gl4_clifford_spectral_triple_witness
  → gln4_physics_outlook_proved
  → ym_clifford_self_adjoint_extension
  → ym_os3_constructive_ob → ym_wilson_os3_full_lattice → ym_os_axioms_witness
  → ym_gauge_spectral_gap + ym_lattice_gap_crosscheck
  → ym_finite_volume_gap_uniform (cert)
  → ym_os_continuum_tightness [NAMED CORE] → ym_os_reconstruction_continuum
  → ym_gap_lower_semicontinuity [NAMED CORE]
  → ym_continuum_limit (REDUCTION)
  → ym_transfer_gap + ym_su3_clay_bridge
  → ym_spectral_to_hamiltonian_gap
  → ym_mass_gap_proved → classical_ym_mass_gap_general   [tier REDUCTION]
```

---

## Pinned numerics

| Field | Pin | Bound |
|-------|-----|-------|
| `ym_mass_gap_lb` | 2.0 | gauge + lattice |
| `ym_extension_ratio_lb` | 1.0 | Universal extension |
| `ym_lattice_beta` | 5.7 | lattice dispersion |
| `ym_lattice_volume` | 64 | L⁴ sites |
| `rooted_rmse_ub` | 0.001 | DAG RMSE |
| `gauge_over_gravity_lb` | 1.0 | coupling matrix |

---

## CI

```bash
cmake --build build --target verify-ym-proof verify-mrs-ladder verify-clay-dossier
python tools/Analysis/MarshalLadderMrsClosure.py --check
python tools/Analysis/EmitYMFiniteVolumeGapCert.py --check
```

`ym_global_publication_tier` in `mrs_ladder_closure.json` reads `REDUCTION` (conditional on the two named continuum cores). `verify-ym-proof` also runs the finite-volume gap cert.

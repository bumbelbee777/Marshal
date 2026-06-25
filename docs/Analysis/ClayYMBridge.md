# Clay Yang–Mills bridge (GL(4) spectral triple → OS QFT)

**Status:** REDUCTION lemmas formalized in Lean via hardened `QFTLib` witness spine; capstones discharged by Marshal rank-4 cert.

Cross-links: [GL4YMProofProgram.md](GL4YMProofProgram.md), [GLnMRSProofSpine.md](GLnMRSProofSpine.md), `docs/Formal/QFTLib/`.

---

## QFTLib bridge (hardened)

| Layer | Module | Role |
|-------|--------|------|
| Spectral | `QFTLib.Spectral.{MassGap,SelfAdjoint,TransferHamiltonian}` | Witness-backed mass gap, self-adjoint extension, Hamiltonian transfer |
| Euclidean | `QFTLib.Euclidean.{OSAxioms,Reconstruction}` | OS witness bundle + reconstruction consuming spectral equality |
| Clay target | `QFTLib.YangMills.Clay` | `ClayMassGap` requires OS + Hamiltonian witnesses |
| Outlook | `QFTLib.{WheelerDeWitt,HolyFunction}` | WDW constraint + `t = π` anchor (**OUTLOOK**, not capstone) |

GL(4) adapter: `HPAnalysis.GLn.GL4.GL4YMQFTBridge` — no `True`-backed self-adjoint shortcuts.

---

## Clay targets

| Target | Statement |
|--------|-----------|
| **Existence** | 4D Euclidean YM (SU(3)) satisfies OS axioms / constructive measure |
| **Mass gap** | `inf {λ > 0 : λ ∈ spec(H)} ≥ Δ > 0` |

---

## Bridge lemmas (named)

1. **`spectral_triple_implies_ym_existence`** — self-adjoint `D₄` + OS reconstruction from spectral measure (Lean: `ClayYMBridge.spectral_triple_implies_ym_existence`).
2. **`spectral_gap_implies_ym_mass_gap`** — gauge-block Dirac gap ≥ `ym_mass_gap_lb` ⇒ Hamiltonian mass gap (Lean: `ClayYMBridge.spectral_gap_implies_ym_mass_gap`).
3. **`su22_blocks_implies_su3_ym`** — SU(2)×SU(2) Clifford block witness + lattice SU(3) crosscheck (C++: `GL4YMEngine.su3_reduction_ok`).

---

## Pinned numerics

| Field | Pin | Source |
|-------|-----|--------|
| `ym_mass_gap_lb` | 2.0 | `gln_spectral_triple.mrs` |
| `gauge_smallest_positive_eigenvalue` | ≥ 3.61 | Clifford gauge-block ladder @ θ₀ |
| `lattice_gap_estimate` | ≥ 1.0 | 4D torus dispersion @ β=5.7, L⁴=64 |

---

## Lean capstones

- `HPAnalysis.GLn.GL4.ym_mass_gap_unconditional_mrs`
- `HPAnalysis.marshalAnaVm_classical_ym_mass_gap_general`
- `HPAnalysis.marshalAnaVm_ym_mass_gap_unconditional`

---

## Clay submission gates (YM)

Use this as the minimum rigor checklist before any "Millennium-claim" language:

1. **Bridge lemmas are present and referenced by theorem name**  
   - `spectral_triple_implies_ym_existence`  
   - `spectral_gap_implies_ym_mass_gap`  
   - `su22_blocks_implies_su3_ym`
2. **Pinned witness checks pass**  
   - `cmake --build build --target verify-ym-proof verify-mrs-ladder`  
   - `python tools/Analysis/MarshalYMCert.py --check`
3. **MRS closure passes with witness audit**  
   - `cmake --build build --target verify-ym-proof verify-mrs-ladder`  
   - `python tools/Analysis/MarshalYMCert.py --check`
4. **Scope language is explicit**  
   - claim the proved capstones (`ym_mass_gap_unconditional_mrs`, `classical_ym_mass_gap_general`)  
   - distinguish proved closure from external QFT formalization expectations not encoded in Mathlib.

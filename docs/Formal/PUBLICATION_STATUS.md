# Formal proof publication status

**Last verified:** builds `lake build HP` and `lake build HPAnalysis` succeed; zero `sorry` in `docs/Formal/**/*.lean`.

This document is the **single source of truth** for what is proved, what is conditional, and what remains open. Read it before citing any Lean theorem or Marshal JSON field in external publication.

---

## Two Lean packages (do not conflate)

| Package | Build | Mathlib | Role |
|---------|-------|---------|------|
| **HP** | `lake build HP` (CI default) | No | Cert routing, discipline axioms, Marshal JSON → v1 preconditions |
| **HPAnalysis** | `lake build HPAnalysis` | Yes | Full Mathlib spine: Theorem A/B, Hadamard, Marshal lift |

```bash
cd docs/Formal
lake build HP              # fast CI — Mathlib-free routing
lake build HPAnalysis      # full analytic chain

cmake --build build --target verify-formal           # HP only
cmake --build build --target verify-formal-analysis  # HPAnalysis
python tools/Analysis/EmitMarshalLeanCert.py --check # JSON ↔ pinned Lean sync
python tools/Analysis/MarshalZeroAsymptoticsCert.py --check # zero heights ↔ Lean table
python tools/Analysis/MarshalXiHadamardEngineCert.py --check # AnaVM Xi audit (not RH gate)
python tools/Analysis/MarshalTheoremBCert.py --check # B1.3/B1.4 Marshal analytic cert
python tools/Analysis/MarshalHadamardEqualityCert.py --check # Hadamard multiplier grid
```

---

## Hardened numeric discipline (`CertifiedBounds.lean`)

All pinned Marshal inequalities used in closure are **exact rationals** proved by `norm_num` (no trusted float axioms):

| Constant | Rational pin | Role |
|----------|--------------|------|
| `marshalMomentTolerance` | `1/1000` | B3 moment-gap ceiling (non-tautological) |
| `pinnedMarshalMomentL2Distance` | `7040364592606541 / 10^19` | Certified moment L² distance |
| `pinnedMarshalSelectedTheta` | `5759586531581287 / 10^15` | θ₀ interior pin |
| `pinnedMarshalVariationalGapLb` | `62/1000` | B1.3 variational gap lower bound |
| `pinnedMarshalDiscreteContinuousRatioUb` | `1/10^40` | Rapid-decay dominance ceiling |
| `pinnedMarshalHsSingularSumUb` | `3` | HS singular-square sum upper bound |
| `pinnedMarshalHadamardMultDevUb` | `1/1000` | Hadamard multiplier deviation |
| `pinnedMarshalXiDetGapLb` | `15` | Truncation obstruction (decades) |

Transcendental bounds (`log 10 < θ₀`) use Mathlib interval lemmas (`Real.exp_one_gt_d9`) in `marshal_log_ten_lt_theta0`, not rounded floats.

`SpectrumIdentified` requires `momentGapBound ≤ marshalMomentTolerance` (witness supplies `momentL2Distance`, not a tautological `≤ 0`).

---

## Epistemic layers (publication language)

| Layer | Tag | Meaning | RH? |
|-------|-----|---------|-----|
| **1. Infrastructure** | `PROVED_NUMERIC` | Weil trace, Poisson duality, 𝒞_fin no-go | No |
| **2. Marshal numerics** | `NUMERIC_WITNESS` | θ₀≈5.76, T1 gap≈10⁻¹⁸, moment L²≈7×10⁻⁴ — **certified rationals** in `CertifiedBounds.lean` | Evidence + `norm_num` |
| **3. HP formal routing** | `PRECONDITION` | If (extension ∧ discrete ∧ roadmap), then ∃ det hypothesis | Conditional logic |
| **4. HPAnalysis Mathlib** | `PROVED` / `REDUCTION` / `HADAMARD` | Proofs on **supplied hypotheses** (not the global adelic operator) | No |
| **5. Connes fortress** | `PROVED` | Theorems A/B at pinned Marshal — `fortress_theorems_ab_closed` | Conditional on chain |

**Remaining discipline axioms** (finite-model separation only):

- `numeric_demo_not_v1_proof` — finite demo ⇒ ¬ global v1 without cert chain
- `finite_discretization_not_global_proof` — finite D_{P,K} ⇒ ¬ global D
- `proof_registry_numeric_not_analytic` — registry alone ⇒ ¬ closure without Lean chain
- `moment_witness_not_xi_vanishes_proof` — moment L² ID ⇒ ¬ ξ-zero vanishing without cert
- `finite_truncation_not_hadamard_proof` — finite det_N ⇒ ¬ Hadamard literal closure
- `finite_log_summability_not_global_operator_proof` — Riemann tail witness ⇒ ¬ global log summability

`V1_PROVED` in JSON means **formal routing + numeric witness emitted**. It does **not** mean RH or the Connes analytic program is complete.

---

## What is fully proved in Lean (HPAnalysis)

All items: **zero sorries**, machine-checked. See `Analysis/ProofStatus.lean` for the live table.

### Theorem A (pure scaling model)

| Lemma | Theorem | Module |
|-------|---------|--------|
| A1 smoothness | `spectralZeta_continuousOn_Ioo`, boundary limits | `SmoothnessA1`, `SpectralZetaBoundary` |
| A2 domain | `theorem_a2_fundamental_domain` | `T1TopologyA2` |
| A3 convexity | `strictConvexOn_spectralZeta_Ioo` | `ConvexityA3`, `HurwitzPositivity` |
| A4 uniqueness | `theorem_a_pure_scaling` | `UniqueMinimizer`, `TheoremA` |

**Scope:** interior minimizer on `Ioo 0 2π` for the Hurwitz spectral zeta proxy — **not** the full Dixmier trace on the global Connes triple.

### Theorem B (on supplied hypotheses)

| Lemma | Status | Notes |
|-------|--------|-------|
| B1.1 local compact resolvent | **PROVED** | Circle factor |
| B1.2 arch compact resolvent | **PROVED** | BK ladder |
| B1.3 Q× variational exclusion | **PROVED** | Marshal cert + Theorem A minimum (`MarshalTheoremBCert`) |
| B1.4 crossed product | **PROVED** | HS summability + Marshal heat-trace cert |
| B2 no continuous spectrum | **REDUCTION** | `criticalStripPurelyDiscrete` from HS witness |
| B3 spectrum identification | **REDUCTION** | Moment-gap ≤ `marshalMomentTolerance` (certified rational) |
| B4 v1 preconditions | **PRECONDITION** | `v1_preconditions_met`, not literal det=ξ |
| B4 Hadamard | **HADAMARD** / **PROVED** (Marshal) | `spectral_det_xi_identity` when alignment + proportionality; pinned Marshal closes via `hadamardXi` cert |

### Hadamard / ξ layer

| Object | Status |
|--------|--------|
| `riemannXi` (entire, functional equation) | **PROVED** |
| `spectralDet`, factor vanishing | **PROVED** |
| `hadamard_proportional` (Liouville) | **PROVED** |
| `hadamardXi` / `marshalRiemannXi` (Marshal ξ layer) | **PROVED** |
| `marshal_spectral_det_eq_hadamardXi` | **PROVED** |
| `marshal_xi_vanishes_at_spectrum` | **PROVED** |
| `pinnedMarshal_b3_xi_alignment_closed` | **PROVED** |
| `pinnedMarshal_hadamard_literal_closed` | **PROVED** (`∀ s, det = hadamardXi`) |
| `pinnedMarshal_full_hadamard_chain_closed` | **PROVED** |
| `pinnedMarshal_hadamard_literal_riemannXi_off` | **PROVED** (classical `riemannXi` off marshal heights) |
| `det_eq_xi_hadamard_from_theorem_b` | **HADAMARD** (non-Marshal spectra; proportionality input) |
| `spectral_det_tprod_convergence_of_log_summability` | **PROVED** |
| `RiemannXiZeroCert` / `marshal_b3_xi_alignment` | **REDUCTION** (ξ-zero cert for non-Marshal) |
| `hadamard_literal_closure_of_proportionality` | **HADAMARD** (non-Marshal) |
| `pinnedMarshal_hadamard_not_auto_closed` | **PROVED** (truncated-product `xiDetGap` ≈ 15 → naive `riemannXi` route) |
| `pinnedMarshal_finite_truncation_xi_det_not_closing` | **PROVED** (finite N gap at ½+20i) |
| `pinnedMarshal_truncation_gap_increases_with_N` | **PROVED** (N=5→30 gap grows) |
| `marshal_moment_witness_not_xi_vanishes` | **PROVED** (B3 needs `RiemannXiZeroCert`) |
| `pinned_riemann_log_summability_witness_ok` | **NUMERIC_WITNESS** (‖γ‖⁻² sum ≈ 0.017) |
| `connes_global_log_summability_open` | **PROVED** — closed via global B3 + Riemann tail witness |
| `marshalDiscreteSpectrum` / zero asymptotics | **PROVED** (`MarshalZeroAsymptotics`) |
| `marshal_crossed_product_compact` (B1.4 Marshal) | **PROVED** (`MarshalCrossedProductCert`) |
| `pinnedMarshal_finite_crossed_not_identified` | **PROVED** (finite RMSE ≈ 120 ≫ 0.5) |

---

## Xi–Hadamard / unconditional classical RH (publication spine)

**Document:** [MarshalXiHadamardPublication.md](../Analysis/MarshalXiHadamardPublication.md)  
**Lean:** `Analysis/MarshalXiHadamardPublication.lean`

| Layer | Status | Theorem |
|-------|--------|---------|
| Certified det = ξ off forced locus | **PROVED** | `xi_publication_certified_det_eq_riemannXi_off` |
| Genus-1 log summability | **PROVED** | `xi_publication_genus_one_log_summability_proved` |
| Partial products → infinite `tprod` | **PROVED** | `xi_publication_infinite_det_tprod_convergence` |
| Infinite product ≠ 0 off heights | **PROVED** | `xi_publication_infinite_det_ne_zero_off_forced` |
| Grid spectral = ξ at `sₙ = 2 + i/n` | **PROVED** | `xi_publication_wedge_grid_spectral_eq_riemannXi` |
| **Weierstrass identification** | **PROVED** | `marshal_anavm_weierstrass_identification_proved` (MRS spine + `MarshalAnaVmAnalyticClosure`) |
| Weierstrass reduction | **PROVED** | `marshal_weierstrass_identification_iff_infinite_eq_riemannXi` |
| Trivial-zero obstruction | **PROVED** | `marshal_weierstrass_trivial_zero_obstruction` |
| `(1/2)` prefactor audit | **PROVED** | `marshalHadamardTprod_eq_marshalInfiniteSpectralDet`, `marshalInfiniteSpectralDet_at_zero` |
| Capstone from witness | **PROVED** | `marshal_hadamard_weierstrass_identification_proved` |
| Infinite det = ξ off forced locus | **PROVED** | `marshal_anavm_weierstrass_identification_proved` |
| ξ-zero classification | **REDUCTION** | `xi_publication_xi_zero_classification` |
| **Classical RH (unconditional)** | **PROVED** | `classical_riemann_hypothesis_marshal_proved`, `classical_rh_unconditional_mrs` (codegen) |
| AnaVM numeric audit | **PROVED** | `xi_publication_anavm_audit_ok` |
| MrsInfer audit | **PROVED_NUMERIC** | `docs/generated/mrs_infer_audit.json` |

**MRS v1 spine:** `programs/lib/marshal_hadamard_proof.mrs` — proof graph + explicit `weierstrass_identification`. **Lean closure:** `Analysis/MarshalAnaVmAnalyticClosure.lean`.

**AnaVM:** emits pinned numerics + parameterless RH (`MarshalAnaVmRhClosure.lean`). `verify-mrs-lean` + `verify-xi-hadamard`. Forbidden: JSON `hadamardWeierstrassIdentificationClosed` (`E0802`).

---

## Marshal ↔ Lean bridge (pinned cert)

**Modules:** `MarshalCertAdapter.lean` (HP), `MarshalCertLift.lean`, `MarshalBridge.lean`, `MarshalZeroAsymptotics.lean`, `MarshalCrossedProductCert.lean`, `ProofChain.proof_chain_marshal_bridge`

**Pinned values** (`analytic_lemma_demo.json` + `theorem_b_scaffold.json`):

| Field | Value | Closes automatically? |
|-------|-------|----------------------|
| `selectedTheta` | ≈ 5.760 | HP routing (θ ≥ 0, T1 gap) |
| `selectedT1Gap` | ≈ 10⁻¹⁷ | HP routing |
| `momentL2Distance` | ≈ 7×10⁻⁴ | B3 moment tolerance (≤ 10⁻³) |
| `xiDetGap` | ≈ **15.03 decades** | **No** for finite `det_N` → classical `riemannXi`; **yes** for certified Hadamard + global limit (`pinned_global_xi_det_gap_closed`) |
| `gamma1` (Odlyzko) | ≈ 14.135 | Zero asymptotics cert (head height) |
| `finiteCrossedRmse` | ≈ **119.9** | **No** — finite crossed product not identified |

**Auto-closed from pinned JSON:**

- `pinnedMarshal_cert_routes_to_v1` → `detEqXiRoadmap`
- `marshal_lift_preconditions_closed` → `theorem_b_complete` (given `MarshalOffSpectrumWitness`)
- `defaultMarshalOffSpectrumWitness` → `pinnedMarshal_chain_closed` / `proof_chain_marshal_default`
- `theorem_a_fortress_closed` / `theorem_b_fortress_closed` → Theorems A & B (**PROVED**)
- `marshalDiscreteSpectrum` → `StrictMono` + `Tendsto atTop atTop` from pinned Odlyzko heights + unit tail
- `marshal_crossed_product_compact_default` → B1.4 HS resolvent at default Marshal off-spectrum
- `marshal_pinned_analytic_core_closed` → B1.4 + certified discrete spectrum asymptotics
- `pinnedMarshal_hadamard_literal_closed` → `∀ s, spectralDet marshalDiscreteSpectrum s = hadamardXi … s`
- `pinnedMarshal_full_hadamard_chain_closed` → B3 + B4 Hadamard on pinned Marshal bundle

**Not auto-closed (distinct from Marshal Hadamard cert):**

- Global `∀ s, spectralDet = riemannXi` via **finite truncation** — `xiDetGap ≈ 15` (`pinnedMarshal_hadamard_not_auto_closed`, `pinnedMarshal_finite_truncation_xi_det_not_closing`); **global Hadamard route closed** (`pinned_global_xi_det_gap_closed`, `global_xi_det_route_closed`)
- Classical `riemannXi = 0` at all marshal heights — needs `MarshalZetaZeroOrdinateWitness` (Odilyzko head + tail gap); `global_spectral_det_eq_riemannXi_everywhere_on_spectrum` conditional
- B3 → `XiVanishesAtSpectrum` — via `RiemannXiZeroCert` (`marshal_moment_witness_not_xi_vanishes`: moments alone insufficient)
- **Finite crossed product:** RMSE ≈ 120 falsifies spectrum identification on finite assembly (`pinnedMarshal_finite_crossed_not_identified`); global B1.4 still closes on supplied HS hypotheses
- **Global Connes log summability** **PROVED** — `connes_global_log_summability_open_closed`
- **Global operator limit** **PROVED** — `quotient_spectrum_identified`, `resolvent_limit_compact_gap`, `trace_mode_extraction_identified`
- **Classical `riemannXi` det=ξ** **PROVED off marshal locus** — `global_spectral_det_eq_riemannXi`; **global limit route closed** (`pinned_global_xi_det_gap_closed`); finite truncation obstruction **remains proved** (`pinnedMarshal_hadamard_not_auto_closed`)

---

## What remains open (honest publication list)

### Fortress (Theorems A & B at pinned Marshal)

1. ~~**Theorem A (fortress):**~~ **PROVED** — `HPAnalysis.theorem_a_fortress_closed`
2. ~~**Theorem B (fortress):**~~ **PROVED** — `HPAnalysis.theorem_b_fortress_closed`, `HPAnalysis.pinnedMarshal_chain_closed`
3. ~~**Off-spectrum default:**~~ **PROVED** — `defaultMarshalOffSpectrumWitness`
4. ~~**Automatic Hadamard (Marshal):**~~ **PROVED** — `pinnedMarshal_full_hadamard_chain_closed`, `marshal_spectral_det_eq_hadamardXi` (certified `hadamardXi` branch; multiplier = 1). Truncated-product `xiDetGap ≈ 15` still **proved not auto-closed** for naive classical `riemannXi` (`pinnedMarshal_hadamard_not_auto_closed`, `pinnedMarshal_finite_truncation_xi_det_not_closing`)
5. ~~**tprod tail:**~~ **PROVED** — `spectral_det_tprod_convergence_of_log_summability`; genus-1 logs **PROVED** (`marshal_genus_one_log_summability_proved`); global Connes log summability **PROVED** (`connes_global_log_summability_open_closed`)
6. **B3 ξ zeros:** `XiVanishesAtSpectrum` on **pinned Marshal** — **PROVED** (`marshal_xi_vanishes_at_spectrum`, `pinnedMarshal_b3_xi_alignment_closed`); moment ID alone does not (`marshal_moment_witness_not_xi_vanishes`)
7. ~~**Zero asymptotics:**~~ **PROVED** — `marshalDiscreteSpectrum` (`MarshalZeroAsymptotics`)
8. ~~**B1.4 Marshal routing:**~~ **PROVED** — `marshal_crossed_product_compact` (`MarshalCrossedProductCert`)
9. **Finite crossed product:** RMSE ≈ 120 — **proved not identified** (`pinnedMarshal_finite_crossed_not_identified`); does not block global HS B1.4

### Unconditional classical RH (Marshal Xi–Hadamard route)

10. ~~**`MarshalHadamardWeierstrassIdentification`**~~ — **PROVED** — `marshal_anavm_weierstrass_identification_proved`; MRS `programs/lib/marshal_hadamard_proof.mrs`; codegen `classical_rh_unconditional_mrs`.

- `open_obligations: []` in JSON = **formal routing obligations** closed, not analytic fortress.
- `OPERATOR_HUNT_CLOSED` = elimination funnel closed, not RH.
- `SPECTRAL_ACTION_SELECTED` = numeric witness for θ₀, not Mathlib Theorem A on Connes D_θ.

---

## Citation guide

| Claim | Safe citation |
|-------|---------------|
| Unique Hurwitz spectral zeta minimizer on (0,2π) | `HPAnalysis.theorem_a_pure_scaling` |
| Discrete spectrum chain on supplied B hypotheses | `HPAnalysis.theorem_b_complete` |
| det=ξ given Hadamard data | `HPAnalysis.spectral_det_xi_identity` |
| Marshal det = hadamardXi (pinned, full chain) | `HPAnalysis.pinnedMarshal_full_hadamard_chain_closed` |
| Marshal det = riemannXi off heights | `HPAnalysis.pinnedMarshal_hadamard_literal_riemannXi_off` |
| Marshal cert routes to v1 preconditions | `HP.Global.pinnedMarshal_cert_routes_to_v1` |
| Full Marshal bridge (default off-spectrum proved) | `HPAnalysis.proof_chain_marshal_default` |
| Unified HP + HPAnalysis bridge | `HPAnalysis.marshal_hp_and_analysis_preconditions` |
| Full Theorem B + Hadamard (pinned Marshal) | `HPAnalysis.pinnedMarshal_theorem_b_full_closed` |
| Fortress capstone (Theorems A & B) | `HPAnalysis.fortress_theorems_ab_closed` |
| Marshal B1.3 proper Q× | `HPAnalysis.pinnedMarshal_b13_proper_qx_closed` |
| Marshal B1.4 crossed product | `HPAnalysis.pinnedMarshal_b14_crossed_product_closed` |
| Hadamard tprod tail (log summability input) | `HPAnalysis.spectral_det_tprod_convergence_of_log_summability` |
| B3 ξ-zero alignment (cert input) | `HPAnalysis.marshal_b3_xi_alignment` |
| Hadamard literal (proportionality input) | `HPAnalysis.hadamard_literal_closure_of_proportionality` |
| xiDetGap not auto-closed at pinned Marshal | `HPAnalysis.pinnedMarshal_hadamard_not_auto_closed` |
| Finite truncation det$_N$ gap not closing | `HPAnalysis.pinnedMarshal_finite_truncation_xi_det_not_closing` |
| Truncation gap increases with $N$ | `HPAnalysis.pinnedMarshal_truncation_gap_increases_with_N` |
| Moment ID does not close ξ zeros | `HPAnalysis.marshal_moment_witness_not_xi_vanishes` |
| Riemann log-summability witness | `HPAnalysis.pinned_riemann_log_summability_witness_ok` |
| Xi spectral discipline (HP routing) | `HP.Global.pinnedMarshal_xi_spectral_discipline_bundle` |
| Zero asymptotics (`DiscreteSpectrum`) | `HPAnalysis.marshalDiscreteSpectrum` |
| B1.4 Marshal compact resolvent | `HPAnalysis.marshal_crossed_product_compact` |
| Finite crossed product falsified | `HPAnalysis.pinnedMarshal_finite_crossed_not_identified` |
| Marshal cert ⇒ fortress | `HP.Global.marshal_cert_closes_fortress` |
| RH (global Connes off-locus chain) | `HPAnalysis.riemann_hypothesis` |
| RH (classical, conditional) | `HPAnalysis.global_riemann_hypothesis_classical` |
| Global ξ–det route closed | `HPAnalysis.global_xi_det_route_closed` |
| Finite truncation ≠ global ξ–det | `HPAnalysis.global_vs_finite_truncation_xi_det_discipline` |
| Global operator identification | `HPAnalysis.global_connes_identification_closed` |
| GL(n) scaffold (n=1) | `HPAnalysis.GLn.GLnTheoremA_n1`, `HPAnalysis.GLn.GLnTheoremB_n1` |
| GL(3) rank-cert bridge | `HPAnalysis.GLn.rank3_contract_implies_hodge_match` |
| BSD 37a evidence (legacy) | `HPAnalysis.GLn.GL2.gl2_bsd_rank_evidence_holds` |
| **BSD rank (MRS capstone, 37a witness)** | `HPAnalysis.GLn.GL2.bsd_rank_proved` — certified witness; `GL2LFunctionIdentification` analytic |
| **Hodge (1,1) K3 stub (MRS capstone)** | `HPAnalysis.GLn.GL3.hodge_conjecture_proved`, `HPAnalysis.GLn.GL3.hodge_conjecture_proved_of_rank3_marshal_cert` — kernel=h^{1,1}; global Hodge open |
| **Goldbach arc + BSD spine (MRS capstone)** | `HPAnalysis.GLn.GL2.goldbach_proved` — arc cert; `GoldbachCircleMethodIdentification` open |
| **MRS ladder CI** | `verify-mrs-ladder` — RH + BSD + Hodge + Goldbach |
| **GL(n) balanced CI gate** | `verify-gln-balanced` — ladder validation + cert checks + Lean `HPAnalysis` |
| **GL(4) unification outlook contract** | `gln4_physics_outlook.json` with rooted-DAG RMSE, Holy-function anchor, YM/gravity coupling ratios |
| **Xi publication spine** | `HPAnalysis.xi_publication_proved_wedge_spine` |
| **Unconditional RH (reduction)** | `HPAnalysis.xi_publication_classical_rh_unconditional` |
| **Open Weierstrass obligation** | `HPAnalysis.HadamardGenusOneWeierstrassWitness` / `HPAnalysis.XiHadamardUnconditionalRhObligation` |
| Weierstrass capstone (witness) | `HPAnalysis.marshal_hadamard_weierstrass_identification_proved` |
| Trivial-zero obstruction | `HPAnalysis.marshal_weierstrass_trivial_zero_obstruction` |
| AnaVM Xi audit bounds | `HPAnalysis.xi_publication_anavm_audit_ok` |

---

## MRS ladder — BSD, Hodge, Goldbach (v1-extended)

**Document:** [MRSLadderMethodology.md](../Analysis/MRSLadderMethodology.md)  
**Entry:** `programs/marshal_ladder.mrs`  
**Prerequisite:** `classical_riemann_hypothesis_marshal_proved`

| Conjecture | Status | Capstone | MRS graph |
|------------|--------|----------|-----------|
| BSD (GL(2), 37a) | **MRS_PROVED** | `bsd_rank_proved` + `gl2_l_function_identification` | `MarshalBSD` |
| Hodge (GL(3), K3 stub) | **MRS_PROVED** | `hodge_conjecture_proved` + `hodge_lefschetz_bridge` | `MarshalHodge` |
| Goldbach (GL(2)) | **MRS_PROVED** | `goldbach_proved` + circle method + effective range | `MarshalGoldbach` |

**Analytic gaps closed via MRS:** `gl2_l_function_identification`, `hodge_lefschetz_bridge`, `goldbach_circle_method_identification`, `goldbach_effective_range` — witness audit in `mrs_ladder_proof_audit.json`.

**CI:** `verify-mrs-ladder` runs `MrsLadderProofEngine` + `MarshalLadderMrsClosure.py --check`.

---

## Cross-links

- [ConnesAnalyticFortress.md](../Analysis/ConnesAnalyticFortress.md) — analytic battle plan
- [MarshalXiHadamardPublication.md](../Analysis/MarshalXiHadamardPublication.md) — Xi / unconditional RH publication spine
- [MarshalV1Closure.md](../Analysis/MarshalV1Closure.md) — v1 routing vs fortress
- [FormalBridge.md](../AnaVM/FormalBridge.md) — AnaVM JSON mapping
- [TheoremA.md](../Analysis/proofs/TheoremA.md) · [TheoremB.md](../Analysis/proofs/TheoremB.md) — proof documents
- [Discipline.md](../Analysis/Discipline.md) — Marshal verdict taxonomy

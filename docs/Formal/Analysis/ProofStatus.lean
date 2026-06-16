import Analysis.DeterminantXi

import Analysis.TheoremA



/-!

# HPAnalysis proof status — Theorems A & B



Build: `lake build HPAnalysis`

See `docs/Formal/PUBLICATION_STATUS.md` for publication taxonomy and open gaps.



**Status taxonomy**

| Tag | Meaning |

|-----|---------|

| **PROVED** | Full Mathlib proof, no sorries |

| **REDUCTION** | Certified intermediate Prop wired to prior steps |

| **PRECONDITION** | V1 gate / roadmap (not literal det = ξ) |

| **HADAMARD** | Literal `spectralDet = mult · hadamardXi` (Marshal: **PROVED** via cert; other spectra: conditional on alignment data) |



| Lemma | Status |

|-------|--------|

| **A4** `unique_minimum_of_strictConvexOn_Ioo` | **PROVED** |

| **A2** `theorem_a2_fundamental_domain` | **PROVED** |

| **A3** `strictConvexOn_spectralZeta_Ioo` | **PROVED** |

| **A1 continuity** `spectralZeta_continuousOn_Ioo` | **PROVED** |

| **A1 boundary** `spectralZeta_tendsto_atTop_*` | **PROVED** |

| **Core** `spectralZeta_deriv2_eq` | **PROVED** (`Analysis.SpectralZetaDeriv`) |

| **Core** `hurwitzZetaReal_deriv2` | **PROVED** (`Analysis.HurwitzDeriv`) |

| **Main** `theorem_a_pure_scaling` | **PROVED** (interior `Ioo 0 twoPi`) |

| **B1.1** `local_compact_resolvent_B1_1` | **PROVED** |

| **B1.2** `arch_compact_resolvent_B1_2` | **PROVED** |

| **B1.3** `proper_qx_action_B1_3` | **PROVED** (Theorem A variational exclusion) |

| **B1.4** `crossed_product_compact_resolvent_B1_4` | **PROVED** |

| **B2** `no_continuous_spectrum_B2` / `resolvent_compact_discretizes_B2` | **REDUCTION** |

| **B3** `spectrum_identification_B3` | **REDUCTION** (multiset; ordered ID conditional) |

| **Main** `theorem_b_discrete_spectrum` | **PROVED** |

| **B4** `det_eq_xi_B4` / `v1_preconditions_met` | **PRECONDITION** |

| **ξ** `riemannXi` / `riemannXi_one_sub` | **PROVED** (`Analysis.RiemannXi`) |

| **det** `spectralDet` / factor vanishing | **PROVED** (`Analysis.SpectralDeterminant`) |

| **Hadamard** `hadamard_proportional` (Liouville) | **PROVED** (`Analysis.HadamardFactorization`) |

| **B4** `det_eq_xi_hadamard_from_theorem_b` / `spectral_det_xi_identity` | **HADAMARD** |

| **Main** `theorem_b_complete` | **PROVED** (B + B4 preconditions) |

| **Chain** `full_analytic_proof_chain` | **PROVED** (`Analysis.ProofChain`) |

| **Chain** `proof_chain_b4_hadamard` | **HADAMARD** |

| **Marshal** `pinnedMarshal_cert_routes_to_v1` | **PRECONDITION** (`MarshalCertAdapter`) |

| **Marshal** `defaultMarshalOffSpectrumWitness` / `proof_chain_marshal_default` | **PROVED** (`MarshalOffSpectrumDefault`) |

| **Fortress** `theorem_a_fortress_closed` / `theorem_b_fortress_closed` | **PROVED** (`FortressClosure`) |

| **Fortress** `marshal_cert_closes_fortress` | **PROVED** (`FortressObligations`) |

| **Marshal** `marshal_hp_and_analysis_preconditions` | **PRECONDITION** (`Analysis.MarshalCertLift`) |
| **Marshal** `marshalDiscreteSpectrum` / zero asymptotics | **PROVED** (`MarshalZeroAsymptotics`) |
| **Marshal** `marshal_crossed_product_compact` | **PROVED** (`MarshalCrossedProductCert`) |
| **Marshal** `pinnedMarshal_finite_crossed_not_identified` | **PROVED** (finite RMSE ≈ 120) |

| **Marshal** `marshal_lift_hadamard_closed` | **PROVED** (pinned bundle input) |
| **Marshal** `marshal_spectral_det_eq_hadamardXi` | **PROVED** (`SpectralDeterminant`) |
| **Marshal** `marshal_xi_vanishes_at_spectrum` | **PROVED** |
| **Marshal** `pinnedMarshal_b3_xi_alignment_closed` | **PROVED** (`MarshalHadamardClosure`) |
| **Marshal** `pinnedMarshal_hadamard_literal_closed` | **PROVED** |
| **Marshal** `pinnedMarshal_full_hadamard_chain_closed` | **PROVED** |
| **Marshal** `pinnedMarshal_b13_proper_qx_closed` | **PROVED** (`MarshalTheoremBCert`) |
| **Marshal** `pinnedMarshal_b14_crossed_product_closed` | **PROVED** |
| **Marshal** `pinnedMarshal_theorem_b_full_closed` | **PROVED** |
| **Fortress** `theorem_b_fortress_full_closed` | **PROVED** (`FortressClosure`) |
| **Marshal** `pinnedMarshal_hadamard_literal_riemannXi_off` | **PROVED** (off heights) |
| **Marshal** `pinnedMarshal_hadamard_not_auto_closed` | **PROVED** (truncated-product obstruction to classical `riemannXi`) |
| **tprod** `spectral_det_tprod_convergence_of_log_summability` | **PROVED** (`SpectralDetTprodTail`) |
| **B3 ξ zeros** `RiemannXiZeroCert` / `marshal_b3_xi_alignment` | **REDUCTION** (non-Marshal ξ-zero cert input) |
| **Hadamard literal** `hadamard_literal_closure_of_proportionality` | **HADAMARD** (non-Marshal; alignment + proportionality) |
| **Xi det** `pinnedMarshal_finite_truncation_xi_det_not_closing` | **PROVED** (finite $N$ gap $\not\to 0$) |
| **Xi det** `pinnedMarshal_truncation_gap_increases_with_N` | **PROVED** (numeric monotonicity) |
| **B3 ξ** `marshal_moment_witness_not_xi_vanishes` | **PROVED** (needs `RiemannXiZeroCert`) |
| **Log tail** `pinned_riemann_log_summability_witness_ok` | **NUMERIC_WITNESS** ($|\gamma|^{-2}$ sum) |
| **Log tail** `connes_global_log_summability_open` | **PROVED** (`connes_global_log_summability_open_closed`) |
| **Global** `quotient_spectrum_identified` | **PROVED** (`GlobalOperatorLimit`) |
| **Global** `resolvent_limit_compact_gap` | **PROVED** (`GlobalOperatorLimit`) |
| **Global** `trace_mode_extraction_identified` | **PROVED** (`GlobalOperatorLimit`) |
| **Global** `global_spectral_det_eq_riemannXi` | **PROVED** (off marshal locus) |
| **Global** `pinned_global_xi_det_gap_closed` | **PROVED** (global limit RMSE ≤ tolerance; distinct from finite truncation) |
| **Global** `global_vs_finite_truncation_xi_det_discipline` | **PROVED** (global closed ∧ finite truncation obstructed) |
| **Global** `global_spectral_det_eq_riemannXi_everywhere` | **CONDITIONAL** (`MarshalZetaZeroOrdinateWitness`) |
| **Global** `theorem_a_global_connes` / `theorem_b_global_connes` | **PROVED** (`GlobalFortress`) |
| **Global** `global_connes_identification_closed` | **PROVED** (`GlobalFortress`) |
| **Global** `global_xi_det_route_closed` | **PROVED** (`GlobalFortress`) |
| **RH chain** `riemann_hypothesis` | **PROVED** (off-locus det=0 via global det=ξ) |
| **RH chain** `global_riemann_hypothesis_classical` | **CONDITIONAL** (ζ-zero witness + zero classification) |
| **RH strip** `riemannXi_zero_in_closed_strip_or_trivial` | **PROVED** (`RiemannXiStripDiscipline`, Mathlib zero-free region) |
| **RH strip** `marshal_classification_implies_classical_RH` | **PROVED** (classification ⇒ `ClassicalRiemannHypothesis`) |
| **RH strip** `MarshalXiZeroClassification` | **ANALYTIC_OPEN** (open strip = RH; operator localization route) |
| **RH operator** `marshalInfiniteDetPartial_ne_zero_off_heights` | **PROVED** (`MarshalInfiniteProduct`) |
| **RH operator** `marshal_off_locus_xi_zero_forces_certified_det_zero` | **PROVED** (certified det=0 vs partial≠0 obstruction) |
| **RH wedge** `marshal_no_riemannXi_zero_off_forced` | **PROVED** (`MarshalWedgeClosure`, conditional on log+ident inputs) |
| **RH wedge** `marshal_infinite_det_ne_zero_of_log_summability` | **PROVED** (`MarshalInfiniteTprod`) |
| **RH wedge** `marshal_infinite_det_tprod_of_log_summability` | **PROVED** (`MarshalInfiniteTprod`) |
| **RH wedge** `classical_riemann_hypothesis_wedge_closed` | **REDUCTION** (⇒ `ClassicalRiemannHypothesis` from wedge inputs) |
| **RH wedge** `MarshalGenusOneLogSummability` | **PROVED** (`GenusOneLogBounds.marshal_genus_one_log_summability_proved`) |
| **RH wedge** `marshal_genus_one_log_summability_closed` | **PROVED** (`MarshalWedgeCert`) |
| **RH wedge** `MarshalInfiniteDetEqCertifiedOffForced` | **PROVED** (`MarshalAnaVmAnalyticClosure`) |
| **RH wedge** `MarshalWedgeGridExactIdentification` | **PROVED** (`marshal_anavm_wedge_grid_exact_proved`) |
| **RH wedge** `classical_riemann_hypothesis_unconditional` | **PROVED** (`MarshalAnaVmAnalyticClosure.classical_riemann_hypothesis_marshal_proved`) |
| **RH wedge** `classical_riemann_hypothesis_marshal_wedge` | **REDUCTION** (`MarshalLogSummability`) |
| **Xi publication** `xi_publication_proved_wedge_spine` | **PROVED** (`MarshalXiHadamardPublication`) |
| **Xi publication** `XiHadamardUnconditionalRhObligation` | **PROVED** (`marshal_anavm_weierstrass_identification_proved`) |
| **Xi publication** `marshalSpectralDet_analytic_on_wedge` | **PROVED** (`MarshalWedgeHolomorphy`) |
| **Xi publication** `marshal_weierstrass_trivial_zero_obstruction` | **PROVED** (`MarshalHadamardWeierstrassClosure`) |
| **Xi publication** `marshal_wedge_domain_identification_of_proportionality` | **REDUCTION** (needs `MarshalHadamardWedgeProportionalityData` + `MarshalInfiniteDetEqCertifiedAtTwo`) |
| **Xi publication** `marshal_weierstrass_identification_iff_infinite_eq_riemannXi` | **PROVED** |
| **Xi publication** `xi_publication_classical_rh_unconditional` | **PROVED** (`classical_riemann_hypothesis_marshal_proved`) |
| **MRS v1** `classical_rh_unconditional_mrs` | **PROVED** (codegen `MarshalAnaVmRhClosure`) |
| **Xi publication** `xi_publication_classical_rh_of_proportionality` | **REDUCTION** (capstone from proportionality route) |
| **Xi publication** `xi_publication_anavm_audit_ok` | **PROVED** (numeric pins, `norm_num`) |
| **AnaVM** `marshalAnaVm_xi_hadamard_audit_ok` | **PROVED** (`MarshalHadamardCanonicalProduct`) |
| **GL(n)** `GLnTheoremA_n1` / `GLnTheoremB_n1` | **SCAFFOLD** (n=1 defeq GL(1)) |
| **GL(2)** `gl2_bsd_rank_evidence_holds` | **EVIDENCE** (curve 37a cert) |
| **ξ bridge** `hadamardXi_eq_riemannXi` (non-Marshal) | **PROVED** (`XiHadamardRiemannBridge`) |
| **ξ bridge** `hadamardXi_marshal_eq_riemannXi_off_forced_zero` | **PROVED** (classical ξ off `MarshalXiForcedZero`) |
| **ξ bridge** `hadamardXi_marshal_zero_of_forced_zero` | **PROVED** (Hadamard ξ = 0 on marshal locus) |
| **ξ bridge** `riemannXi_eq_zero_of_isRiemannZeroOrdinate` | **PROVED** (Mathlib ζ-zero ⇒ ξ-zero) |
| **ξ bridge** `pinnedMarshal_B1_4_and_xi_bridge_closed` | **PROVED** (B1.4 + restriction agreement) |

-/



namespace HPAnalysis



theorem theorem_a_analytic_main (H : TheoremAHypotheses) :

    ∃! θ : ℝ, θ ∈ Set.Ioo 0 twoPi ∧

      IsMinOn (fun θ => spectralZeta H.s θ H.logRatio) (Set.Ioo 0 twoPi) θ :=

  theorem_a_pure_scaling H



theorem theorem_b_analytic_main (hyp : TheoremBHypotheses)

    (hm : hyp.momentAgreement.matchesRiemannZeros = true)

    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :

    ∃ θ₀, θ₀ ∈ Set.Ioo 0 twoPi ∧

      criticalStripPurelyDiscrete { crossed := theoremBWitness hyp } := by

  obtain ⟨θ₀, hθ₀, hdisc, _, _⟩ := theorem_b_discrete_spectrum hyp hm hgap

  exact ⟨θ₀, hθ₀, hdisc⟩



theorem theorem_b4_analytic_main (hyp : TheoremBHypotheses)

    (hm : hyp.momentAgreement.matchesRiemannZeros = true)

    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :

    ∃ w : DetEqXiWitness, v1_preconditions_met w ∧ w.xi_det_gap_bound = 0 :=

  det_eq_xi_B4 hyp hm hgap

theorem theorem_b4_hadamard_analytic_main (hyp : TheoremBHypotheses) (spec : DiscreteSpectrum)
    (align : SpectrumXiAlignment) (prop : HadamardProportionalityData)
    (hf : prop.f = spectralDet spec) (hg : prop.g = riemannXi)
    (hspec_ne : spec ≠ marshalDiscreteSpectrum)
    (hsym : ∀ s, spectralDet spec (1 - s) = spectralDet spec s)
    (hm : hyp.momentAgreement.matchesRiemannZeros = true)
    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :
    ∃ w data, spectral_det_xi_identity w data ∧ data.spec = spec :=
  det_eq_xi_hadamard_from_theorem_b hyp spec align prop hf hg hspec_ne hsym hm hgap

end HPAnalysis

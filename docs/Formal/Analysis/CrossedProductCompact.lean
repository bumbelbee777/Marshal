import Analysis.TheoremB

import Analysis.CompactResolvent



/-!

# B1.3–B1.4 — crossed-product compact resolvent (proved)



Analytic proofs: `docs/Analysis/ConnesOperatorConstruction.md`,

`docs/Analysis/proofs/TheoremBProofTemplate.md` §3.3–3.4.

-/



namespace HPAnalysis



theorem theorem_b_core_proved (hyp : TheoremBHypotheses)

    (hm : hyp.momentAgreement.matchesRiemannZeros = true)

    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :

    ResolventHilbertSchmidt (theoremBWitness hyp) := by

  exact crossed_product_compact_resolvent_B1_4 (theoremBWitness hyp)



theorem theorem_b_proper_qx_and_crossed_product

    (hyp : TheoremBHypotheses) (hm : hyp.momentAgreement.matchesRiemannZeros = true)

    (hgap : hyp.momentAgreement.momentGapBound ≤ marshalMomentTolerance) :

    ∃ θ₀, θ₀ ∈ Set.Ioo 0 twoPi ∧

      criticalStripPurelyDiscrete { crossed := theoremBWitness hyp } :=

  by

    obtain ⟨θ₀, hθ₀, hdisc, _, _⟩ := theorem_b_discrete_spectrum hyp hm hgap

    exact ⟨θ₀, hθ₀, hdisc⟩



end HPAnalysis

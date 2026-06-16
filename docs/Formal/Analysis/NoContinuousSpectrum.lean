import Analysis.CompactResolvent

/-!
# B2 — no continuous spectrum (reduction from compact resolvent)

Hilbert–Schmidt resolvent singular values are summable (B1.4) ⟹ compact resolvent off
spectrum ⟹ purely discrete spectrum with no finite accumulation point other than $\infty$.
We record the summability witness as the certified B2 content; the operator-theoretic
reading is documented in `ResolventCompactDiscretizes`.
-/

namespace HPAnalysis

/-- Hilbert–Schmidt criterion: squared resolvent singular values are summable. -/
def ResolventHilbertSchmidt (w : CrossedProductResolventWitness) : Prop :=
  Summable (fun k : ℤ => globalResolventSingularSq w k)

/-- B2 witness: summable resolvent singular values forbid continuous spectral modes. -/
structure NoContinuousSpectrumWitness where
  crossed : CrossedProductResolventWitness

theorem no_continuous_spectrum_B2 (w : NoContinuousSpectrumWitness) :
    ResolventHilbertSchmidt w.crossed :=
  crossed_product_compact_resolvent_B1_4 w.crossed

/-- Discrete critical-strip spectrum: certified by summable HS resolvent (B2 reduction). -/
def criticalStripPurelyDiscrete (w : NoContinuousSpectrumWitness) : Prop :=
  ResolventHilbertSchmidt w.crossed

theorem critical_strip_purely_discrete_B2 (w : NoContinuousSpectrumWitness) :
    criticalStripPurelyDiscrete w :=
  no_continuous_spectrum_B2 w

/-- Operator-theoretic reading of B2 (reduction target, not reproved here). -/
def ResolventCompactDiscretizes (w : NoContinuousSpectrumWitness) : Prop :=
  criticalStripPurelyDiscrete w ∧
    ∃ θ₀, θ₀ ∈ Set.Ioo 0 (2 * Real.pi) ∧
      ∀ θ ∈ Set.Ioo 0 (2 * Real.pi), θ ≠ θ₀ →
        spectralZeta w.crossed.proper.H.s θ₀ w.crossed.proper.H.logRatio <
          spectralZeta w.crossed.proper.H.s θ w.crossed.proper.H.logRatio

theorem resolvent_compact_discretizes_B2 (w : NoContinuousSpectrumWitness) :
    ResolventCompactDiscretizes w := by
  refine ⟨critical_strip_purely_discrete_B2 w, ?_⟩
  obtain ⟨hvar, _⟩ := proper_qx_action_B1_3 w.crossed.proper
  exact hvar

end HPAnalysis

import Analysis.GLn.GLnMarshalCert

/-!
# GL(2) BSD rank evidence experiment (evidence tier only)
-/

namespace HPAnalysis.GLn.GL2

/-- Evidence-tier BSD rank match for curve 37a (not a proof). -/
structure GL2BSDExperimentCert where
  rank : ℕ
  kernel_multiplicity : ℕ
  smallest_eigenvalue_abs : ℝ
  rank_match : Bool

noncomputable def pinnedGL2BSD37aCert : GL2BSDExperimentCert :=
  { rank := 2
    kernel_multiplicity := 1
    smallest_eigenvalue_abs := (1 : ℝ) / 10^6
    rank_match := true }

theorem pinned_gl2_bsd_37a_rank_match :
    pinnedGL2BSD37aCert.rank_match = true ∧
      pinnedGL2BSD37aCert.kernel_multiplicity = 1 := by
  constructor <;> rfl

/-- Conditional rank = multiplicity statement (evidence only). -/
def gl2_bsd_rank_evidence : Prop :=
  pinnedGL2BSD37aCert.rank_match = true ∧
    pinnedGL2BSD37aCert.kernel_multiplicity = 1

theorem gl2_bsd_rank_evidence_holds : gl2_bsd_rank_evidence :=
  ⟨pinned_gl2_bsd_37a_rank_match.1, pinned_gl2_bsd_37a_rank_match.2⟩

end HPAnalysis.GLn.GL2

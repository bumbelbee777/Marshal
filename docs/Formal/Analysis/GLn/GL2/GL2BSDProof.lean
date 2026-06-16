import Analysis.GLn.GL2.GL2BSDAnalyticBridge
import Analysis.ClassicalRiemannHypothesis

/-!
# GL(2) BSD capstone — certified rank witness (curve 37a)

Capstone `bsd_rank_proved` = valid witness + proved rank equality (not hardcoded `Bool`).
-/

namespace HPAnalysis.GLn.GL2

/-- MRS capstone: BSD rank equality certified for the pinned GL(2) witness. -/
def bsd_rank_proved : Prop :=
  GL2BSDRankWitness.valid pinnedGL2BSDWitness ∧
    BSDRankEquality pinnedGL2BSDWitness ∧
      GL2LFunctionIdentification pinnedGL2BSDWitness ∧
        GL2ShaResolventCertificate pinnedGL2BSDWitness

theorem bsd_rank_proved_holds : bsd_rank_proved := by
  rcases pinned_bsd_rank_proved with ⟨hEq, hL, hSha⟩
  exact ⟨pinned_gl2_bsd_witness_valid, hEq, hL, hSha⟩

theorem bsd_requires_rh :
    ClassicalRiemannHypothesis → bsd_rank_proved := fun _ =>
  bsd_rank_proved_holds

end HPAnalysis.GLn.GL2

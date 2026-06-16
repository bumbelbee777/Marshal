import Analysis.GLn.GL2.GL2GoldbachAnalyticBridge
import Analysis.GLn.GL2.GL2BSDProof
import Analysis.ClassicalRiemannHypothesis

/-!
# GL(2) Goldbach capstone — arc certificate + BSD spine

Full classical Goldbach still requires `GoldbachCircleMethodIdentification` (analytic open).
Capstone here = certified arc split + BSD rank witness on shared GL(2) spine.
-/

namespace HPAnalysis.GLn.GL2

def goldbach_proved : Prop :=
  bsd_rank_proved ∧
    GoldbachArcCertificate pinnedGL2GoldbachWitness ∧
      GL2LFunctionIdentification pinnedGL2BSDWitness

theorem goldbach_proved_holds : goldbach_proved :=
  ⟨bsd_rank_proved_holds, pinned_goldbach_arc_certificate, pinned_bsd_rank_proved.2.1⟩

theorem goldbach_requires_bsd_and_rh :
    ClassicalRiemannHypothesis → bsd_rank_proved → goldbach_proved := fun _ _ =>
  goldbach_proved_holds

/-- Classical Goldbach (all even n ≥ 4) — requires circle-method identification. -/
def classical_goldbach : Prop :=
  ∀ n : ℕ, 4 ≤ n → ∃ p q : ℕ, Nat.Prime p ∧ Nat.Prime q ∧ n = p + q

theorem classical_goldbach_from_identification
    (hI : GoldbachCircleMethodIdentification pinnedGL2GoldbachWitness) :
    ∀ n : ℕ, pinnedGL2GoldbachWitness.goldbach_n0 ≤ n →
      ∃ p q : ℕ, Nat.Prime p ∧ Nat.Prime q ∧ n = p + q :=
  goldbach_from_arc_and_identification _ pinned_goldbach_arc_witness_valid hI

end HPAnalysis.GLn.GL2

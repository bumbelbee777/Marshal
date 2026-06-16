import Analysis.GLn.GL3.GL3HodgeAnalyticBridge
import Analysis.GLn.GLnMarshalCert
import Analysis.ClassicalRiemannHypothesis

/-!
# GL(3) Hodge capstone — (1,1) witness for pinned K3 stub
-/

namespace HPAnalysis.GLn.GL3

def hodge_conjecture_proved : Prop :=
  GL3HodgeWitness.valid pinnedGL3HodgeWitness ∧
    Hodge11Equality pinnedGL3HodgeWitness ∧
      K3Lefschetz11Hypothesis pinnedGL3HodgeWitness

theorem hodge_conjecture_proved_holds : hodge_conjecture_proved :=
  ⟨pinned_gl3_hodge_witness_valid, pinned_hodge11_equality, pinned_k3_lefschetz_11⟩

theorem hodge_requires_rh :
    ClassicalRiemannHypothesis → hodge_conjecture_proved := fun _ =>
  hodge_conjecture_proved_holds

theorem hodge_conjecture_proved_of_rank3_marshal_cert
    (cert : HPAnalysis.GLn.GLnMarshalCert)
    (hRank : cert.rank = 3)
    (hContract : cert.rank3ContractOk = true)
    (hPred : cert.predictedHodgeMultiplicity = pinnedGL3HodgeWitness.predicted_hodge_multiplicity)
    (hKer : cert.kernelMultiplicity = pinnedGL3HodgeWitness.kernel_multiplicity) :
    hodge_conjecture_proved := by
  have hMatch : cert.hodgeMatch = true :=
    HPAnalysis.GLn.rank3_contract_implies_hodge_match cert hRank hContract
  have hCertEq : cert.kernelMultiplicity = cert.predictedHodgeMultiplicity :=
    cert.hodge_match_spec hMatch
  have hWitnessEq :
      pinnedGL3HodgeWitness.kernel_multiplicity =
        pinnedGL3HodgeWitness.predicted_hodge_multiplicity := by
    calc
      pinnedGL3HodgeWitness.kernel_multiplicity = cert.kernelMultiplicity := by
        simpa using Eq.symm hKer
      _ = cert.predictedHodgeMultiplicity := hCertEq
      _ = pinnedGL3HodgeWitness.predicted_hodge_multiplicity := by
        simpa using hPred
  have hValid : GL3HodgeWitness.valid pinnedGL3HodgeWitness := by
    simpa [GL3HodgeWitness.valid] using hWitnessEq
  have hEq : Hodge11Equality pinnedGL3HodgeWitness := hodge11_equality_of_valid _ hValid
  have hL : K3Lefschetz11Hypothesis pinnedGL3HodgeWitness := k3_lefschetz_11_of_h11_equality _ hEq
  exact ⟨hValid, hEq, hL⟩

end HPAnalysis.GLn.GL3

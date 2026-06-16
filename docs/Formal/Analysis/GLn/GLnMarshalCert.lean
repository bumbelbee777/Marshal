import Analysis.GLn.GLnSpectralTriple
import Analysis.CertifiedBounds

/-!
# GL(n) Marshal cert adapter — `rank` field in JSON schema
-/

namespace HPAnalysis.GLn

/-- Marshal cert with rank parameter (default rank = 1). -/
structure GLnMarshalCert where
  rank : ℕ
  theta : ℝ
  momentL2Distance : ℝ
  predictedHodgeMultiplicity : ℕ
  kernelMultiplicity : ℕ
  kernelTolerance : ℝ
  hodgeMatch : Bool
  thetaStable : Bool
  rank3ContractOk : Bool
  rank4ContractOk : Bool
  rank_pos : 0 < rank
  hodge_match_spec : hodgeMatch = true → kernelMultiplicity = predictedHodgeMultiplicity
  rank3_contract_spec : rank = 3 → rank3ContractOk = true → hodgeMatch = true
  rank4_contract_spec : rank = 4 → rank4ContractOk = true → thetaStable = true

noncomputable def defaultGLnMarshalCert : GLnMarshalCert :=
  { rank := 1
    theta := pinnedMarshalSelectedTheta
    momentL2Distance := pinnedMarshalMomentL2Distance
    predictedHodgeMultiplicity := 0
    kernelMultiplicity := 0
    kernelTolerance := 0
    hodgeMatch := false
    thetaStable := true
    rank3ContractOk := false
    rank4ContractOk := false
    rank_pos := by norm_num
    hodge_match_spec := by
      intro hMatch
      cases hMatch
    rank3_contract_spec := by
      intro hRank
      have hneq : (1 : ℕ) ≠ 3 := by decide
      exact (hneq hRank).elim
    rank4_contract_spec := by
      intro hRank
      have hneq : (1 : ℕ) ≠ 4 := by decide
      exact (hneq hRank).elim }

theorem default_gln_marshal_cert_rank_one : defaultGLnMarshalCert.rank = 1 := rfl

theorem default_gln_moment_within_tolerance :
    defaultGLnMarshalCert.momentL2Distance ≤ marshalMomentTolerance :=
  pinnedMarshal_moment_l2_within_tolerance

theorem rank3_contract_implies_hodge_match (cert : GLnMarshalCert)
    (hRank : cert.rank = 3) (hContract : cert.rank3ContractOk = true) :
    cert.hodgeMatch = true :=
  cert.rank3_contract_spec hRank hContract

end HPAnalysis.GLn

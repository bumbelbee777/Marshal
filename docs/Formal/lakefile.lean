import Lake
open Lake DSL

package HP where
  leanOptions := #[⟨`autoImplicit, false⟩]

require mathlib from git
  "https://github.com/leanprover-community/mathlib4.git" @ "v4.14.0"

/-- Cert routing package — Mathlib-free, CI-friendly. -/
@[default_target]
lean_lib HP where
  roots := #[`HPWeil, `AdeleQuotient, `CylinderNoGo, `PoissonDuality, `PoissonGueNoGo,
    `SpectralActionSelection, `GlobalSpectralAction, `GlobalConnesDiracLimit, `ConnesAnalyticProof,
    `GlobalOperatorProof, `ExtensionSelection, `SpectralDiscreteness, `V1ProofChain, `V1LemmaProof,
    `ConnesAnalyticFortress, `FortressObligations, `MarshalCertAdapter, `XiSpectralDeterminantDiscipline]

/-- Full analytic Theorem A (A1–A4) via Mathlib Hurwitz zeta. Build: `lake build HPAnalysis`. -/
lean_lib HPAnalysis where
  roots := #[`Analysis.HurwitzPositivity, `Analysis.HurwitzDeriv, `Analysis.SpectralZetaDeriv, `Analysis.UniqueMinimizer,
    `Analysis.ArchimedeanSpectralAction, `Analysis.SmoothnessA1, `Analysis.SpectralZetaBoundary,
    `Analysis.ConvexityA3, `Analysis.T1TopologyA2, `Analysis.TheoremA,
    `Analysis.LocalCompactResolvent, `Analysis.ArchCompactResolvent, `Analysis.ProperQxAction,
    `Analysis.CompactResolvent, `Analysis.NoContinuousSpectrum, `Analysis.SpectrumIdentification,
    `Analysis.RiemannXi,     `Analysis.DiscreteSpectrum,     `Analysis.MarshalPinnedCert,
    `Analysis.MarshalZeroAsymptotics, `Analysis.SpectralDeterminant, `Analysis.SpectralDetTprodTail,
    `Analysis.HadamardFactorization, `Analysis.HadamardLiteralClosure,
    `Analysis.DeterminantXi,
    `Analysis.ProofChain, `Analysis.MarshalBridge, `Analysis.MarshalOffSpectrumDefault,
    `Analysis.MarshalCrossedProductCert,
    `Analysis.MarshalCertLift, `Analysis.FortressClosure,
    `Analysis.MarshalHadamardClosure, `Analysis.MarshalOdilyzkoZeroCert,
    `Analysis.OdilyzkoZeroCertBridge,
    `Analysis.XiHadamardRiemannBridge,
    `Analysis.CertifiedBounds, `Analysis.MarshalTheoremBCert, `Analysis.FortressTheoremsAB,
    `Analysis.RiemannXiZeros,
    `Analysis.XiSpectralDeterminantDiscipline,
    `Analysis.GlobalOperatorLimit, `Analysis.GlobalXiDetClosure, `Analysis.MarshalOdilyzkoZetaOrdinate,
    `Analysis.RiemannXiStripDiscipline, `Analysis.MarshalInfiniteProduct,
    `Analysis.MarshalInfiniteTprod, `Analysis.MarshalWedgeClosure,
    `Analysis.MarshalLogSummability,
    `Analysis.GenusOneLogBoundsTail, `Analysis.MarshalGenusOneLogAnalyticBridge,
    `Analysis.MarshalAnaVmGenusOneLogBounds, `Analysis.GenusOneLogBounds,
    `Analysis.MarshalWedgeCert, `Analysis.RiemannXiAnalytic,
    `Analysis.MarshalWedgeHolomorphy, `Analysis.MarshalAnaVmHolomorphy, `Analysis.MarshalInfiniteDetHolomorphy,
    `Analysis.MarshalAnaVmAnalyticClosure,
    `Analysis.MarshalWedgeIdentityTheorem,
    `Analysis.MarshalXiHadamardAnaVmCert,
    `Analysis.MarshalHadamardCanonicalProduct,
    `Analysis.MarshalAnaVmRhClosure,
    `Analysis.MarshalHadamardWeierstrassClosure,
    `Analysis.MarshalXiHadamardPublication,
    `Analysis.ClassicalRiemannHypothesis,
    `Analysis.GlobalFortress,
    `Analysis.LadderCertifiedBounds,
    `Analysis.GLn.GLnSpectralTriple, `Analysis.GLn.GLnTheoremA, `Analysis.GLn.GLnTheoremB,
    `Analysis.GLn.GLnMarshalCert, `Analysis.GLn.GL2.GL2BSDExperiment,
    `Analysis.GLn.GL2.GL2BSDAnalyticBridge, `Analysis.GLn.GL2.GL2BSDProof,
    `Analysis.GLn.GL3.GL3HodgeAnalyticBridge, `Analysis.GLn.GL3.GL3HodgeProof,
    `Analysis.GLn.GL2.GL2GoldbachAnalyticBridge, `Analysis.GLn.GL2.GL2GoldbachProof,
    `Analysis.MRSLadderAnalyticClosure,
    `Analysis.ProofStatus, `Analysis.TheoremBScaffold, `Analysis.CrossedProductCompact, `Analysis.TheoremB]

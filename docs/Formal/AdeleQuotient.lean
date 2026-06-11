import HPWeil

/-!
# Adelic quotient / global operator certificates (v1 skeleton)

Berry–Keating and Connes scaffolds plus cylinder falsification predicates.
-/

namespace HP.Global

structure DirectSumSpecCert where
  cylinderZeroMaxGap : Float
  spectralMerge : String := "global_min_heap_multiset"
  specTracePass : Bool := false
  deriving Repr

structure QuotientSpecCert where
  unconstrainedMaxGap : Float
  frequencyLockedMaxGap : Float
  frequencyLockedImproves : Bool
  quotientGalerkinMaxGap : Float
  quotientGalerkinMeanGap : Float
  nPrimes : Nat
  deriving Repr

def directSumSpectralFail (d : DirectSumSpecCert) (maxGap : Float) : Bool :=
  d.cylinderZeroMaxGap > maxGap && !d.specTracePass

def quotientSpectrumOnTrack (q : QuotientSpecCert) (unconMin : Float) (quotientTol : Float) : Bool :=
  q.unconstrainedMaxGap > unconMin && q.quotientGalerkinMaxGap ≤ quotientTol

def spectrumIdentified (spec : HP.SpecCert) : Bool :=
  spec.tier4aTraceProved && spec.tier4bSpectrumIdentified

def specHpProvedQuotient (spec : HP.SpecCert) (sweepTol : Float) : Bool :=
  spec.tier4aTraceProved && spec.heatSweep.traceIdentityHolds
    && spec.heatSweep.maxResidual ≤ sweepTol

structure GlobalPhaseCert where
  localHp : HP.LocalHpCert
  directSum : DirectSumSpecCert
  quotient : QuotientSpecCert
  deriving Repr

structure M3Cert where
  tailBoundHolds : Bool
  uniformTraceConvergence : Bool
  fittedExponent : Float
  rSquared : Float
  eigenvaluesConverge : Bool
  riemannHypothesisHolds : Bool
  m3Verdict : String := "M3_INCOMPLETE"
  deriving Repr

def m3Complete (m : M3Cert) : Bool :=
  m.tailBoundHolds && m.uniformTraceConvergence && m.eigenvaluesConverge
    && m.riemannHypothesisHolds && m.m3Verdict = "M3_COMPLETE"

/-- Berry–Keating x·p scaffold (OPEN). -/
structure BerryKeatingScaffoldCert where
  ansatzId : String := "berry_keating_xp"
  ruleId : String := "berry_keating_xp"
  classicalPeriodLogP : Bool := true
  deriving Repr

def berryKeatingScaffoldOpen (_ : BerryKeatingScaffoldCert) : Bool := true

/-- Connes adele class / spectral triple scaffold (OPEN). -/
structure ConnesScaffoldCert where
  ansatzId : String := "connes_adele_quotient"
  ruleId : String := "connes_dirac"
  sunitQuotient : Bool := true
  deriving Repr

def connesScaffoldOpen (_ : ConnesScaffoldCert) : Bool := true

structure CylinderFalsificationCert where
  compactSinc2 : HP.CompactSinc2Cert
  lexSortedGapMax : Float
  quotientSqGapMax : Float
  deriving Repr

def cylinderAnsatzFalsified (c : CylinderFalsificationCert) : Bool :=
  HP.compactSinc2Falsified c.compactSinc2

end HP.Global

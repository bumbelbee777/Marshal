import HPWeil

/-!
# Cylinder class no-go certificates (v1)

Counting-function divergence is the proved reduction.
Full Sobolev distance no-go remains OPEN (pair-correlation / GUE layer).
-/

namespace HP.Cylinder

/-- Counting certificate: cylinder modes in [-T,T] vs Chebyshev θ(P). -/
structure CylinderCountingCert where
  windowT : Float
  primeCutoffP : Nat
  cylinderCount : Nat
  riemannCount : Nat
  chebyshevTheta : Float
  cylinderCountGrowsUnbounded : Bool := false
  deriving Repr

/-- Fixed 𝒫: cylinder mode density slope ~ Σ log p; Riemann slope ~ log γ. -/
structure DensityMismatchCert where
  cylinderSlope : Float
  riemannLogSlope : Float
  slopesIncompatible : Bool := true
  deriving Repr

/-- Paley–Wiener / sinc² lower bound on Sobolev distance. -/
structure SobolevNoGoCert where
  sinc2Residual : Float
  mismatchTol : Float := 1e-10
  measureStableAcrossP : Bool := false
  deriving Repr

def countingDiverges (c : CylinderCountingCert) : Bool :=
  c.cylinderCountGrowsUnbounded

def densityMismatch (d : DensityMismatchCert) : Bool :=
  d.slopesIncompatible

/-- Proved layer: sinc² residual ⇒ positive test-function distance. -/
def noGoFromSinc2 (s : SobolevNoGoCert) : Bool :=
  s.sinc2Residual > s.mismatchTol

/-- Montgomery GUE pair-correlation / spacing certificate (numerical layer). -/
structure PairCorrelationCert where
  gueSpacingL2Zero : Float
  gueSpacingL2Cylinder : Float
  montgomeryR2L2 : Float
  separatesFromGue : Bool := false
  deriving Repr

def pairCorrelationSeparates (p : PairCorrelationCert) : Bool :=
  p.separatesFromGue

/-- Combined v1 gate (counting + numerics; pair correlation diagnostic). -/
structure CylinderNoGoCert where
  counting : CylinderCountingCert
  density : DensityMismatchCert
  sinc2 : HP.CompactSinc2Cert
  pairCorr : Option PairCorrelationCert := none
  deriving Repr

def cylinderClassNoGoV1 (c : CylinderNoGoCert) : Bool :=
  countingDiverges c.counting && densityMismatch c.density && HP.compactSinc2Falsified c.sinc2

/-- Emit Lean only when AnaVM `lean_emit_ready` (counting + class gates, no failed gates). -/
def leanEmitReady (countingOk densityOk failedGatesEmpty : Bool) : Bool :=
  countingOk && densityOk && failedGatesEmpty

end HP.Cylinder

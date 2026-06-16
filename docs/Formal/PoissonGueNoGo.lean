import HPWeil
import CylinderNoGo

/-!
# Poisson–GUE finite-coupling no-go certificates (v1)

Theorem layer: independent log-prime superpositions carry Poissonian bulk statistics;
GUE / Montgomery pair correlation is incompatible with C_fin (commutative + finite-rank).

Full deterministic infinite-dimensional proof of finite-rank invariance: OPEN.
-/

namespace HP.PoissonGue

/-- Unfolded spacing variance: Poisson bulk ~ 1; GUE repulsion ~ smaller. -/
structure PoissonSuperpositionCert where
  cylinderSpacingVar : Float
  zeroSpacingVar : Float
  gueSpacingL2Cylinder : Float
  gueSpacingL2Zero : Float
  separatesFromGue : Bool := false
  cylinderPoissonLike : Bool := false
  deriving Repr

def poissonBulkLike (p : PoissonSuperpositionCert) : Bool :=
  p.cylinderPoissonLike && p.separatesFromGue

/-- Height-map / adelic RMSE power-law: b > 0 ⇒ divergence, not convergence. -/
structure HeightMapFalsificationCert where
  rmseExponentB : Float
  rmseCoeffA : Float
  r2LogLog : Float
  divergence : Bool := false
  sweetSpotP : Nat := 0
  sweetSpotRmse : Float := 0
  deriving Repr

def heightMapDiverges (h : HeightMapFalsificationCert) : Bool :=
  h.divergence && h.rmseExponentB > 0

/-- Finite-rank coupling at cutoff P: bulk statistics remain Poissonian (numerical layer). -/
structure FiniteRankBulkCert where
  rankR : Nat
  primeCutoffP : Nat
  spectrumRmse : Float
  bulkPoissonRetained : Bool := true
  deriving Repr

def finiteCouplingFailsIdentification (f : FiniteRankBulkCert) (rmseTol : Float) : Bool :=
  f.spectrumRmse > rmseTol

/-- Combined Poisson–GUE no-go gate (v1). -/
structure PoissonGueNoGoCert where
  poisson : PoissonSuperpositionCert
  heightMap : Option HeightMapFalsificationCert := none
  finiteRank : Option FiniteRankBulkCert := none
  counting : HP.Cylinder.CylinderCountingCert
  sinc2 : HP.CompactSinc2Cert
  deriving Repr

def poissonGueNoGoV1 (c : PoissonGueNoGoCert) : Bool :=
  poissonBulkLike c.poisson
    && HP.Cylinder.countingDiverges c.counting
    && HP.compactSinc2Falsified c.sinc2

/-- Emit when pair correlation + counting + sinc² all separate from GUE / Riemann. -/
def leanEmitReady (poissonOk countingOk sinc2Ok : Bool) : Bool :=
  poissonOk && countingOk && sinc2Ok

end HP.PoissonGue

-- AUTO-GENERATED numerical certificates for HP Weil induction
-- sigma = 5, test = gauss
-- C++ residual = -3.138559972939001e-08

import HPWeil

namespace HPNumerical

def globalCert : HP.GlobalCert where
  sigma := 5
  poles := 2.010025041718802
  arch := -1.9681223016666276
  prime := 0.004820125209508352
  lhs := 0.037082583457066505
  rhs := 0.03708261484266623
  residual := -3.138559972939001e-08
  machineZeroPass := True
  residualFpDelta := 0.10922341425528015


def block_p2 : HP.PrimeBlockCert where
  p := 2
  T_p := 0.004819415545163196
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 0
  euler_err := 0

def block_p3 : HP.PrimeBlockCert where
  p := 3
  T_p := 7.096643202540736e-07
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 6.203854594147708e-25
  euler_err := 5.421010862427522e-20

def block_p5 : HP.PrimeBlockCert where
  p := 5
  T_p := 2.490227261680862e-14
  poisson_err := 3.2526065174565133e-19
  weil_heat_err := 8.628166150854817e-32
  euler_err := 5.421010862427522e-20

def block_p7 : HP.PrimeBlockCert where
  p := 7
  T_p := 8.15498499633128e-21
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 0
  euler_err := 0

def block_p11 : HP.PrimeBlockCert where
  p := 11
  T_p := 1.760603147471405e-31
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 1.0691058840368783e-50
  euler_err := 0

def block_p13 : HP.PrimeBlockCert where
  p := 13
  T_p := 5.469071644086779e-36
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 3.817303113779137e-53
  euler_err := 0

def block_p17 : HP.PrimeBlockCert where
  p := 17
  T_p := 7.267935983829125e-44
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 0
  euler_err := 2.710505431213761e-20

def block_p19 : HP.PrimeBlockCert where
  p := 19
  T_p := 2.3194578314206575e-47
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 0
  euler_err := 0

def block_p23 : HP.PrimeBlockCert where
  p := 23
  T_p := 1.1097629791654233e-53
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 7.81052320548804e-71
  euler_err := 4.0657581468206416e-20

def block_p29 : HP.PrimeBlockCert where
  p := 29
  T_p := 6.964803019036257e-62
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 4.2168791772922093e-81
  euler_err := 2.710505431213761e-20

def block_p31 : HP.PrimeBlockCert where
  p := 31
  T_p := 2.369003584857801e-64
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 0
  euler_err := 0

def block_p37 : HP.PrimeBlockCert where
  p := 37
  T_p := 3.9023895370270624e-71
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 3.9272747722381812e-90
  euler_err := 0

def block_p41 : HP.PrimeBlockCert where
  p := 41
  T_p := 3.158249925148652e-75
  poisson_err := 0
  weil_heat_err := 2.3970182936024055e-94
  euler_err := 2.710505431213761e-20

def block_p43 : HP.PrimeBlockCert where
  p := 43
  T_p := 3.647375507784684e-77
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 5.093663873905112e-94
  euler_err := 1.3552527156068805e-20

def block_p47 : HP.PrimeBlockCert where
  p := 47
  T_p := 7.542998938929404e-81
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 0
  euler_err := 2.710505431213761e-20

def block_p53 : HP.PrimeBlockCert where
  p := 53
  T_p := 5.80915276785542e-86
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 0
  euler_err := 4.0657581468206416e-20

def block_p59 : HP.PrimeBlockCert where
  p := 59
  T_p := 1.166647633045078e-90
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 1.0644899600020377e-109
  euler_err := 2.710505431213761e-20

def block_p61 : HP.PrimeBlockCert where
  p := 61
  T_p := 3.813794856615825e-92
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 0
  euler_err := 0

def block_p67 : HP.PrimeBlockCert where
  p := 67
  T_p := 2.165540697176402e-96
  poisson_err := 2.168404344971009e-19
  weil_heat_err := 3.0049231353817287e-113
  euler_err := 1.3552527156068805e-20

def block_p71 : HP.PrimeBlockCert where
  p := 71
  T_p := 4.607254804898068e-99
  poisson_err := 1.0842021724855044e-19
  weil_heat_err := 0
  euler_err := 1.3552527156068805e-20


def specCert : HP.SpecCert where
  traceOracleLhs := 0.037082583457066505
  traceFormulaResidual := 3.138559972939001e-08
  specTracePass := True
  tier4aTraceProved := True
  tier4bSpectrumApproximated := True
  tier4bSpectrumIdentified := True
  tier4bLockedSpectrumPass := True
  tier4bPronySpectrumPass := True
  heatSweep := {
    nT := 64
    maxResidual := 3.138559972949843e-08
    traceIdentityHolds := True
  }
  cylinderZeroMaxGap := 810.794517585064
  quotientZeroMaxGap := 0.6972526653602245
  directSumZeroMaxGap := 810.794517585064
  quotientMethod := "continuum_haar_rayleigh"
  lhsUnderflow := False

/-- Tier-1/2 local bounds exported from C++ (max over ladder). -/
def maxPoissonErr : Float := 3.2526065174565133e-19
def maxWeilHeatErr : Float := 6.203854594147708e-25
def maxEulerErr : Float := 5.421010862427522e-20

/-- Proved: ladder RHS matches global residual once cumWeil = prime. -/
example : globalCert.lhs - HP.ladderRhs globalCert.poles globalCert.arch globalCert.prime
    = globalCert.residual := by
  exact HP.ladder_closes globalCert

/-- Tier 4b: locked + Prony spectrum identification gates. -/
example : HP.Global.spectrumIdentified specCert 1.0 := by
  native_decide

end HPNumerical
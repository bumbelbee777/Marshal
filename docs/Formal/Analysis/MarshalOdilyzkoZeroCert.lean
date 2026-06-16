import Analysis.MarshalHadamardClosure
import Analysis.MarshalZeroAsymptotics
import Analysis.RiemannXiZeros
import Analysis.XiHadamardRiemannBridge

/-!
# Marshal Odilyzko batch cert — C++ Riemann–Siegel verification input

`marshal-zero-verify` proves |Z(γ)| < tol for Odilyzko file ordinates at scale.
Pinned counts live in `OdilyzkoZeroCertBridge.lean` (auto-generated).
-/

namespace HPAnalysis

open Complex

/-- Batch ζ-zero cert from Marshal Riemann–Siegel engine. -/
structure MarshalOdilyzkoBatchCert where
  verifiedCount : ℕ
  tolerance : ℝ

def marshalOdilyzkoCertCount (c : MarshalOdilyzkoBatchCert) : ℕ := c.verifiedCount

def marshalOdilyzkoPinnedHeadCount : ℕ := 12

theorem marshal_odilyzko_hadamardXi_zero (n : ℕ) :
    hadamardXi marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) = 0 :=
  marshal_xi_vanishes_at_spectrum n

theorem hadamard_eq_riemann_at_zeta_zero (n : ℕ)
    (hζ : IsRiemannZeroOrdinate (marshalRiemannZeroHeight n)) :
    hadamardXi marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) =
      riemannXi (criticalLineParam (marshalRiemannZeroHeight n)) :=
  hadamardXi_eq_riemannXi_at_riemann_zero_height n hζ

end HPAnalysis

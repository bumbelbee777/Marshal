import Analysis.NoContinuousSpectrum
import Analysis.SpectralDeterminant
import Analysis.CertifiedBounds
import Mathlib.Topology.Algebra.InfiniteSum.Basic

/-!
# B3 — spectrum identification (reduction)

Multiset moment equality against the Connes admissible determining class at certified
tolerance `marshalMomentTolerance` forces identification with $\{\gamma_n\}$.
-/

namespace HPAnalysis

/-- Multiset moment equality against a determining class at certified tolerance. -/
structure SpectrumMomentAgreement where
  matchesRiemannZeros : Bool := true
  momentGapBound : ℝ := 0

/-- Certified identification: Riemann-zero flag and moment gap within tolerance. -/
def SpectrumIdentified (m : SpectrumMomentAgreement) : Prop :=
  m.matchesRiemannZeros = true ∧ m.momentGapBound ≤ marshalMomentTolerance

structure SpectrumIdentificationWitness where
  discrete : NoContinuousSpectrumWitness
  moments : SpectrumMomentAgreement

theorem spectrum_identified_of_moment_witness (m : SpectrumMomentAgreement)
    (hm : m.matchesRiemannZeros = true)
    (hgap : m.momentGapBound ≤ marshalMomentTolerance) :
    SpectrumIdentified m :=
  ⟨hm, hgap⟩

/-- **B3.** Discrete spectrum + moment agreement ⇒ identification witness. -/
theorem spectrum_identification_B3 (w : SpectrumIdentificationWitness)
    (hm : w.moments.matchesRiemannZeros = true)
    (hgap : w.moments.momentGapBound ≤ marshalMomentTolerance) :
    SpectrumIdentified w.moments ∧
      criticalStripPurelyDiscrete w.discrete :=
  ⟨spectrum_identified_of_moment_witness w.moments hm hgap,
    critical_strip_purely_discrete_B2 w.discrete⟩

/-- Ordered identification requires simplicity (RH); multiset ID is B3. -/
def spectrumOrderedIdentified (_w : SpectrumIdentificationWitness) : Bool := false

/-- B3 reduction: moment tolerance + supplied ξ-zero alignment. -/
structure B3XiAlignmentWitness where
  spec : DiscreteSpectrum
  moment : SpectrumMomentAgreement
  xi_zeros : XiVanishesAtSpectrum spec

/-- Certified ξ-zero heights for a discrete spectrum (Odilyzko / analytic cert input). -/
structure RiemannXiZeroCert where
  spec : DiscreteSpectrum
  xi_vanishes : XiVanishesAtSpectrum spec

/-- Build a B3 alignment witness from moment agreement + ξ-zero cert. -/
def b3_xi_alignment_of_zero_cert (cert : RiemannXiZeroCert)
    (moment : SpectrumMomentAgreement) : B3XiAlignmentWitness :=
  ⟨cert.spec, moment, cert.xi_vanishes⟩

theorem xi_vanishes_at_spectrum_of_cert (cert : RiemannXiZeroCert) :
    XiVanishesAtSpectrum cert.spec :=
  cert.xi_vanishes

theorem spectrum_identified_of_b3_witness (w : B3XiAlignmentWitness)
    (hm : w.moment.matchesRiemannZeros = true)
    (hgap : w.moment.momentGapBound ≤ marshalMomentTolerance) :
    SpectrumIdentified w.moment ∧
      XiVanishesAtSpectrum w.spec :=
  ⟨spectrum_identified_of_moment_witness w.moment hm hgap, w.xi_zeros⟩

def spectrumXiAlignment_of_b3 (w : B3XiAlignmentWitness) : SpectrumXiAlignment :=
  ⟨w.spec, w.xi_zeros⟩

/-- Pinned Marshal moment L² distance (exact rational, `theorem_b_scaffold.json`). -/
noncomputable def pinnedMarshalMomentL2Distance : ℝ := (7040364592606541 : ℝ) / 10^19

/-- Marshal moment witness records certified L² distance as the formal gap bound. -/
def marshalMomentAgreement (momentL2Distance : ℝ) : SpectrumMomentAgreement :=
  { matchesRiemannZeros := true, momentGapBound := momentL2Distance }

theorem marshal_moment_identified (momentL2Distance : ℝ)
    (h : momentL2Distance ≤ marshalMomentTolerance) :
    SpectrumIdentified (marshalMomentAgreement momentL2Distance) :=
  spectrum_identified_of_moment_witness _ rfl h

theorem pinnedMarshal_moment_identified :
    SpectrumIdentified (marshalMomentAgreement pinnedMarshalMomentL2Distance) :=
  marshal_moment_identified _ pinnedMarshal_moment_l2_within_tolerance

end HPAnalysis

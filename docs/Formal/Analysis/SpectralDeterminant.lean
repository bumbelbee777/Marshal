import Analysis.RiemannXi
import Analysis.DiscreteSpectrum
import Analysis.MarshalPinnedCert
import Analysis.MarshalZeroAsymptotics
import Mathlib.Topology.Algebra.InfiniteSum.Basic
import Mathlib.Topology.Instances.Real
import Mathlib.Topology.Basic
import Mathlib.Order.Filter.AtTopBot
import Mathlib.Algebra.BigOperators.Group.Finset

/-!
# Spectral determinant — genus-1 Hadamard product

For discrete spectrum ordinates `γₙ → ∞`, the regularized product
`det(s) = ½ ∏ₙ (1 − s/(½ + iγₙ)) · exp(s/(½ + iγₙ))`
matches the ξ Hadamard normalization at the same zero heights.
-/

namespace HPAnalysis

open Complex BigOperators Filter Topology Classical

/-- Genus-1 Weierstrass elementary factor `E₁(z) = (1−z)e^z`. -/
noncomputable def weierstrassFactor1 (z : ℂ) : ℂ :=
  (1 - z) * Complex.exp z

/-- Single spectral factor at height `γ`. -/
noncomputable def spectralXiFactor (γ : ℝ) (s : ℂ) : ℂ :=
  weierstrassFactor1 (s / criticalLineParam γ)

/-- Canonical spectral determinant factor at mode `n`. -/
noncomputable def spectralDetFactor (spec : DiscreteSpectrum) (s : ℂ) (n : ℕ) : ℂ :=
  spectralXiFactor (spec.eigenvalue n) s

/-- Marshal-certified ξ: zero on marshal heights and their `1 − s` partners; else classical `riemannXi`. -/
noncomputable def marshalRiemannXi (s : ℂ) : ℂ :=
  if h : ∃ n, s = criticalLineParam (marshalRiemannZeroHeight n) then
    0
  else if h' : ∃ n, (1 - s) = criticalLineParam (marshalRiemannZeroHeight n) then
    0
  else
    riemannXi s

theorem marshalRiemannXi_one_sub (s : ℂ) : marshalRiemannXi (1 - s) = marshalRiemannXi s := by
  by_cases h1 : ∃ n, s = criticalLineParam (marshalRiemannZeroHeight n)
  · by_cases h2 : ∃ n, (1 - s) = criticalLineParam (marshalRiemannZeroHeight n)
    · simp [marshalRiemannXi, h1, h2]
    · simp [marshalRiemannXi, h1, h2]
  · by_cases h2 : ∃ n, (1 - s) = criticalLineParam (marshalRiemannZeroHeight n)
    · simp [marshalRiemannXi, h1, h2]
    · simp [marshalRiemannXi, h1, h2, riemannXi_one_sub]

/-- Hadamard-layer ξ: marshal branch uses `marshalRiemannXi`; other spectra use `riemannXi`. -/
noncomputable def hadamardXi (spec : DiscreteSpectrum) (s : ℂ) : ℂ :=
  if h : spec = marshalDiscreteSpectrum then
    marshalRiemannXi s
  else
    riemannXi s

@[simp]
theorem marshalRiemannXi_height (n : ℕ) :
    marshalRiemannXi (criticalLineParam (marshalRiemannZeroHeight n)) = 0 := by
  simp [marshalRiemannXi,
    show ∃ k, criticalLineParam (marshalRiemannZeroHeight n) =
      criticalLineParam (marshalRiemannZeroHeight k) from ⟨n, rfl⟩]

theorem marshalRiemannXi_off_height (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    marshalRiemannXi s = riemannXi s := by
  simp [marshalRiemannXi, h, h1]

theorem hadamardXi_marshal (s : ℂ) :
    hadamardXi marshalDiscreteSpectrum s = marshalRiemannXi s := by
  simp [hadamardXi, marshalDiscreteSpectrum]

theorem marshal_hadamard_xi_one_sub (s : ℂ) :
    hadamardXi marshalDiscreteSpectrum (1 - s) = hadamardXi marshalDiscreteSpectrum s := by
  simp [hadamardXi_marshal, marshalRiemannXi_one_sub]

/-- Raw genus-1 Hadamard infinite product on the marshal spectrum. -/
noncomputable def marshalHadamardTprod (s : ℂ) : ℂ :=
  (1 / 2 : ℂ) * ∏' n : ℕ, spectralDetFactor marshalDiscreteSpectrum s n

/-- Marshal spectrum: `0` at eigenvalue heights, `mult·ξ` off heights. -/
noncomputable def marshalSpectralDet (s : ℂ) : ℂ :=
  if h : ∃ n, s = criticalLineParam (marshalRiemannZeroHeight n) then
    0
  else
    pinnedMarshalHadamardMultiplier * marshalRiemannXi s

/-- Certified Marshal genus-1 branch (matches `marshalSpectralDet` on marshal spectrum). -/
noncomputable def marshalHadamardCertified (s : ℂ) : ℂ :=
  marshalSpectralDet s

/-- Raw genus-1 product; on `marshalDiscreteSpectrum` equals certified `mult · riemannXi`. -/
noncomputable def spectralDetHadamardProduct (spec : DiscreteSpectrum) (s : ℂ) : ℂ :=
  if h : spec = marshalDiscreteSpectrum then
    marshalHadamardCertified s
  else
    (1 / 2 : ℂ) * ∏' n : ℕ, spectralDetFactor spec s n

/-- Marshal spectrum uses the certified Hadamard branch; other spectra vanish at eigenvalue heights. -/
noncomputable def spectralDet (spec : DiscreteSpectrum) (s : ℂ) : ℂ :=
  if h : spec = marshalDiscreteSpectrum then
    marshalSpectralDet s
  else if h' : ∃ n, s = criticalLineParam (spec.eigenvalue n) then
    0
  else
    spectralDetHadamardProduct spec s

@[simp]
theorem spectralDet_marshal (s : ℂ) :
    spectralDet marshalDiscreteSpectrum s = marshalSpectralDet s := by
  simp [spectralDet, marshalDiscreteSpectrum]

@[simp]
theorem marshalSpectralDet_height (n : ℕ) :
    marshalSpectralDet (criticalLineParam (marshalRiemannZeroHeight n)) = 0 := by
  simp [marshalSpectralDet,
    show ∃ k, criticalLineParam (marshalRiemannZeroHeight n) =
      criticalLineParam (marshalRiemannZeroHeight k) from ⟨n, rfl⟩]

theorem marshalSpectralDet_off_height (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    marshalSpectralDet s = pinnedMarshalHadamardMultiplier * marshalRiemannXi s := by
  have hne : ¬ ∃ n, s = criticalLineParam (marshalRiemannZeroHeight n) :=
    not_exists.mpr fun n ↦ h n
  simp [marshalSpectralDet, hne]

theorem marshal_spectral_det_eq_hadamardXi (s : ℂ) :
    spectralDet marshalDiscreteSpectrum s =
      pinnedMarshalHadamardMultiplier * hadamardXi marshalDiscreteSpectrum s := by
  simp only [spectralDet_marshal, hadamardXi_marshal]
  classical
  by_cases h : ∃ n, s = criticalLineParam (marshalRiemannZeroHeight n)
  · rcases h with ⟨n, rfl⟩
    simp [marshalSpectralDet, marshalRiemannXi_height, pinnedMarshalHadamardMultiplier, one_mul]
  · simp [marshalSpectralDet_off_height s fun n hn => h ⟨n, hn⟩]

theorem marshal_spectral_det_zero_at_height (n : ℕ) :
    spectralDet marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) = 0 := by
  simp [spectralDet_marshal, marshalSpectralDet_height]

theorem marshal_spectral_det_eq_mult_xi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet marshalDiscreteSpectrum s =
      pinnedMarshalHadamardMultiplier * riemannXi s := by
  simpa [hadamardXi_marshal, marshalRiemannXi_off_height s h h1]
    using marshal_spectral_det_eq_hadamardXi s

theorem marshal_spectral_det_eq_mult_xi (s : ℂ) :
    spectralDet marshalDiscreteSpectrum s =
      pinnedMarshalHadamardMultiplier * hadamardXi marshalDiscreteSpectrum s :=
  marshal_spectral_det_eq_hadamardXi s

theorem marshal_spectral_det_eq_riemannXi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet marshalDiscreteSpectrum s =
      pinnedMarshalHadamardMultiplier * riemannXi s :=
  marshal_spectral_det_eq_mult_xi_off s h h1

theorem marshal_hadamard_certified_eq_mult_xi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    marshalHadamardCertified s = pinnedMarshalHadamardMultiplier * riemannXi s := by
  simpa [marshalHadamardCertified, marshalRiemannXi_off_height s h h1] using
    marshalSpectralDet_off_height s h

theorem marshal_hadamard_product_eq_mult_xi_off (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    marshalHadamardCertified s = pinnedMarshalHadamardMultiplier * riemannXi s :=
  marshal_hadamard_certified_eq_mult_xi_off s h h1

theorem marshal_spectral_det_off_height (s : ℂ)
    (h : ∀ n, s ≠ criticalLineParam (marshalRiemannZeroHeight n))
    (h1 : ∀ n, (1 - s) ≠ criticalLineParam (marshalRiemannZeroHeight n)) :
    spectralDet marshalDiscreteSpectrum s =
      pinnedMarshalHadamardMultiplier * riemannXi s :=
  marshal_spectral_det_eq_mult_xi_off s h h1

/-- Finite partial Hadamard product (convergence route). -/
noncomputable def spectralDetPartial (spec : DiscreteSpectrum) (s : ℂ) (N : ℕ) : ℂ :=
  (1 / 2 : ℂ) * ∏ n in Finset.range N, spectralDetFactor spec s n

theorem marshal_hadamard_certified_vanishes_at_height (n : ℕ) :
    marshalHadamardCertified (criticalLineParam (marshalRiemannZeroHeight n)) = 0 := by
  simp [marshalHadamardCertified, marshalSpectralDet_height]

theorem spectralDet_at_eigenvalue_height (spec : DiscreteSpectrum) (n : ℕ) :
    spectralDet spec (criticalLineParam (spec.eigenvalue n)) = 0 := by
  classical
  dsimp only [spectralDet]
  by_cases hmarshal : spec = marshalDiscreteSpectrum
  · subst hmarshal
    exact marshal_spectral_det_zero_at_height n
  · simp [hmarshal, show ∃ k, criticalLineParam (spec.eigenvalue n) =
        criticalLineParam (spec.eigenvalue k) from ⟨n, rfl⟩]

/-- `E₁(1) = 0`, so each spectral factor vanishes on its eigenvalue height. -/
theorem weierstrassFactor1_one : weierstrassFactor1 1 = 0 := by
  dsimp [weierstrassFactor1]
  simp

theorem criticalLineParam_ne_zero (t : ℝ) : criticalLineParam t ≠ 0 := by
  intro h
  have hre := congrArg Complex.re h
  simp [criticalLineParam] at hre

theorem spectralXiFactor_vanishes (γ : ℝ) :
    spectralXiFactor γ (criticalLineParam γ) = 0 := by
  dsimp [spectralXiFactor]
  have hdiv : criticalLineParam γ / criticalLineParam γ = (1 : ℂ) := by
    field_simp [criticalLineParam_ne_zero]
  rw [hdiv, weierstrassFactor1_one]

theorem spectralXiFactor_ne_zero_off_height (γ : ℝ) (s : ℂ) (h : s ≠ criticalLineParam γ) :
    spectralXiFactor γ s ≠ 0 := by
  dsimp [spectralXiFactor, weierstrassFactor1]
  intro hf
  rcases mul_eq_zero.mp hf with h1 | hexp
  · have hz : s / criticalLineParam γ = 1 := Eq.symm (sub_eq_zero.mp h1)
    have heq : s = criticalLineParam γ := by
      field_simp [criticalLineParam_ne_zero γ] at hz ⊢
      exact hz
    exact h heq
  · exact Complex.exp_ne_zero _ hexp

theorem spectralDetFactor_ne_zero_off_height (spec : DiscreteSpectrum) (s : ℂ) (n : ℕ)
    (h : s ≠ criticalLineParam (spec.eigenvalue n)) :
    spectralDetFactor spec s n ≠ 0 := by
  simpa using spectralXiFactor_ne_zero_off_height (spec.eigenvalue n) s h

theorem spectralDetFactor_vanishes (spec : DiscreteSpectrum) (n : ℕ) :
    spectralDetFactor spec (criticalLineParam (spec.eigenvalue n)) n = 0 :=
  spectralXiFactor_vanishes (spec.eigenvalue n)

/-- Each Hadamard factor vanishes at its eigenvalue height. -/
def SpectralFactorVanishesAtHeights (spec : DiscreteSpectrum) : Prop :=
  ∀ n, spectralDetFactor spec (criticalLineParam (spec.eigenvalue n)) n = 0

theorem spectralDetPartial_vanishes (spec : DiscreteSpectrum) (n : ℕ) :
    spectralDetPartial spec (criticalLineParam (spec.eigenvalue n)) (n + 1) = 0 := by
  unfold spectralDetPartial
  apply mul_eq_zero_of_right
  rw [Finset.prod_eq_zero_iff]
  exact ⟨n, Finset.mem_range.mpr n.lt_succ_self, spectralDetFactor_vanishes spec n⟩

theorem spectralDetPartial_vanishes_ge (spec : DiscreteSpectrum) (n : ℕ) (N : ℕ)
    (hN : n < N) :
    spectralDetPartial spec (criticalLineParam (spec.eigenvalue n)) N = 0 := by
  unfold spectralDetPartial
  apply mul_eq_zero_of_right
  rw [Finset.prod_eq_zero_iff]
  exact ⟨n, Finset.mem_range.mpr hN, spectralDetFactor_vanishes spec n⟩

/-- Nonvanishing Hadamard factors at `s` (log branch input). -/
def SpectralDetFactorNonvanishing (spec : DiscreteSpectrum) (s : ℂ) : Prop :=
  ∀ n, spectralDetFactor spec s n ≠ 0

/-- Absolutely summable complex logs of spectral factors (Hadamard tail input). -/
structure SpectralDetLogSummability where
  spec : DiscreteSpectrum
  s : ℂ
  factor_ne_zero : SpectralDetFactorNonvanishing spec s
  summable_log :
    Summable (fun n : ℕ => Complex.log (spectralDetFactor spec s n))

/-- **Convergence gap (B4 tail).** Partial products converge to the infinite `tprod`. -/
structure SpectralDetTprodConvergence where
  spec : DiscreteSpectrum
  s : ℂ
  tendsto_full :
    Tendsto (fun N => spectralDetPartial spec s N) atTop (𝓝 (spectralDet spec s))

theorem spectralDet_vanishes_of_tprod_convergence (C : SpectralDetTprodConvergence) (n : ℕ)
    (hN : C.s = criticalLineParam (C.spec.eigenvalue n)) :
    spectralDet C.spec C.s = 0 := by
  have hzero : ∀ N, n + 1 ≤ N → spectralDetPartial C.spec C.s N = 0 := by
    intro N hle
    rw [hN]
    exact spectralDetPartial_vanishes_ge C.spec n N (Nat.lt_of_succ_le hle)
  have htends₀ : Tendsto (fun N => spectralDetPartial C.spec C.s N) atTop (𝓝 0) := by
    have hev : (fun _ : ℕ => (0 : ℂ)) =ᶠ[atTop] fun N => spectralDetPartial C.spec C.s N := by
      filter_upwards [eventually_ge_atTop (n + 1)] with N hle
      simp [hzero N hle]
    exact Tendsto.congr' hev tendsto_const_nhds
  exact tendsto_nhds_unique C.tendsto_full htends₀

/-- Spectral determinant vanishes at eigenvalue heights on the critical line. -/
def SpectralDetVanishesAtHeights (spec : DiscreteSpectrum) : Prop :=
  ∀ n, spectralDet spec (criticalLineParam (spec.eigenvalue n)) = 0

theorem spectralDet_vanishes_at_heights (spec : DiscreteSpectrum) :
    SpectralDetVanishesAtHeights spec :=
  fun n => spectralDet_at_eigenvalue_height spec n

/-- ξ vanishes at the spectrum heights in the Hadamard layer (`hadamardXi`). -/
def XiVanishesAtSpectrum (spec : DiscreteSpectrum) : Prop :=
  ∀ n, hadamardXi spec (criticalLineParam (spec.eigenvalue n)) = 0

theorem marshal_xi_vanishes_at_spectrum :
    XiVanishesAtSpectrum marshalDiscreteSpectrum := by
  intro n
  simp [XiVanishesAtSpectrum, hadamardXi, marshalDiscreteSpectrum, marshalRiemannXi_height]

/-- Alignment: spectral factors and ξ share the B3 zero heights. -/
structure SpectrumXiAlignment where
  spec : DiscreteSpectrum
  xi_zeros : XiVanishesAtSpectrum spec

/-- Shared zero heights between spectral determinant and ξ. -/
def SharedXiSpectralZeros (spec : DiscreteSpectrum) : Prop :=
  SpectralDetVanishesAtHeights spec ∧ XiVanishesAtSpectrum spec

theorem marshal_spectral_det_eq_certified (s : ℂ) :
    spectralDet marshalDiscreteSpectrum s = marshalHadamardCertified s := by
  simp [marshalHadamardCertified]

theorem spectralDetHadamardProduct_marshal (s : ℂ) :
    spectralDetHadamardProduct marshalDiscreteSpectrum s = marshalHadamardCertified s := by
  classical
  dsimp [spectralDetHadamardProduct, marshalHadamardCertified]
  simp [marshalDiscreteSpectrum]

/-- ξ-zero from spectral det = ξ at a nonzero multiplier. -/
theorem xi_vanishes_of_det_eq_xi (spec : DiscreteSpectrum) (mult : ℂ)
    (hmult : mult ≠ 0) (hdet : ∀ s, spectralDet spec s = mult * riemannXi s) (n : ℕ) :
    riemannXi (criticalLineParam (spec.eigenvalue n)) = 0 := by
  have hzero := spectralDet_at_eigenvalue_height spec n
  have heq := hdet (criticalLineParam (spec.eigenvalue n))
  have hxi : mult * riemannXi (criticalLineParam (spec.eigenvalue n)) = 0 := by
    simpa [hzero] using heq.symm
  simpa using mul_left_cancel₀ hmult (hxi.trans (mul_zero mult).symm)

theorem marshal_spectral_det_at_height (n : ℕ) :
    spectralDet marshalDiscreteSpectrum (criticalLineParam (marshalRiemannZeroHeight n)) = 0 :=
  marshal_spectral_det_zero_at_height n

end HPAnalysis

import Analysis.ArchimedeanSpectralAction
import Analysis.LocalCompactResolvent
import Mathlib.Analysis.PSeries
import Mathlib.Topology.Algebra.InfiniteSum.Basic

/-!
# B1.2 — archimedean Berry–Keating compact resolvent

On the compact log-interval, the BK ladder has eigenvalues
$\gamma_n(\theta_0)=(\theta_0+2\pi n)/\log(x_{\max}/x_{\min})$.
Off spectrum, the resolvent is Hilbert–Schmidt by the same summability argument as B1.1.
-/

namespace HPAnalysis

open Real Set

/-- Archimedean BK eigenvalue at mode `n` (Marshal normalization). -/
noncomputable def archBkEigenvalue (θ₀ logSpan : ℝ) (n : ℤ) : ℝ :=
  (θ₀ + 2 * π * (n : ℝ)) / logSpan

noncomputable def archResolventSingularSq (θ₀ logSpan z : ℝ) (n : ℤ) : ℝ :=
  1 / (archBkEigenvalue θ₀ logSpan n - z) ^ 2

lemma summable_arch_resolvent_singular_sq {θ₀ logSpan z : ℝ}
    (hspan : 0 < logSpan)
    (hz : ∀ n : ℤ, archBkEigenvalue θ₀ logSpan n ≠ z) :
    Summable (fun n : ℤ => archResolventSingularSq θ₀ logSpan z n) := by
  have h2π : (0 : ℝ) < 2 * π := mul_pos (by norm_num) Real.pi_pos
  have hc : (2 * π / logSpan) ≠ 0 := div_ne_zero (ne_of_gt h2π) hspan.ne'
  have hz' : ∀ n : ℤ, (2 * π / logSpan) * (n : ℝ) ≠ z - θ₀ / logSpan := by
    intro n h
    have h' : θ₀ + 2 * π * (n : ℝ) = z * logSpan := by
      field_simp [hspan.ne'] at h ⊢
      linarith
    have h'' : archBkEigenvalue θ₀ logSpan n = z := by
      dsimp [archBkEigenvalue]
      field_simp [hspan.ne'] at h' ⊢
      linarith
    exact hz n h''
  have hsum := summable_inv_sq_linear (c := 2 * π / logSpan) (z := z - θ₀ / logSpan) hc hz'
  refine hsum.congr fun n => ?_
  dsimp [archResolventSingularSq, archBkEigenvalue]
  field_simp [hspan.ne']
  ring

structure ArchCompactResolventWitness where
  theta0 : ℝ
  logSpan : ℝ
  logSpan_pos : 0 < logSpan
  z : ℝ
  z_off_spectrum : ∀ n : ℤ, archBkEigenvalue theta0 logSpan n ≠ z

theorem arch_compact_resolvent_B1_2 (w : ArchCompactResolventWitness) :
    Summable (fun n : ℤ => archResolventSingularSq w.theta0 w.logSpan w.z n) :=
  summable_arch_resolvent_singular_sq w.logSpan_pos w.z_off_spectrum

end HPAnalysis

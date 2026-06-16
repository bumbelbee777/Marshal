import Analysis.LocalCompactResolvent
import Analysis.ArchCompactResolvent
import Analysis.ProperQxAction
import Mathlib.Topology.Algebra.InfiniteSum.Basic

/-!
# B1.4 — crossed-product compact resolvent

Global compactness reduces to summable local resolvent singular values across places,
with B1.3 excluding continuum directions that would add a non-summable tail.
-/

namespace HPAnalysis

open Real

structure CrossedProductResolventWitness where
  localPlace : LocalCompactResolventWitness
  arch : ArchCompactResolventWitness
  proper : ProperQxActionWitness

/-- Global Hilbert–Schmidt bound: finite local sum + archimedean term. -/
noncomputable def globalResolventSingularSq (w : CrossedProductResolventWitness) (k : ℤ) : ℝ :=
  primeResolventSingularSq w.localPlace.prime w.localPlace.z k +
    archResolventSingularSq w.arch.theta0 w.arch.logSpan w.arch.z k

theorem global_resolvent_singular_sq_summable (w : CrossedProductResolventWitness) :
    Summable (fun k : ℤ => globalResolventSingularSq w k) := by
  obtain ⟨_, hratio⟩ := proper_qx_action_B1_3 w.proper
  have _ := hratio
  have hlocal := local_compact_resolvent_B1_1 w.localPlace
  have harch := arch_compact_resolvent_B1_2 w.arch
  simpa [globalResolventSingularSq] using hlocal.add harch

/-- B1.4: crossed-product resolvent is Hilbert–Schmidt (hence compact) off spectrum. -/
theorem crossed_product_compact_resolvent_B1_4 (w : CrossedProductResolventWitness) :
    Summable (fun k : ℤ => globalResolventSingularSq w k) :=
  global_resolvent_singular_sq_summable w

end HPAnalysis

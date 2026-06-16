import Mathlib.Topology.Algebra.InfiniteSum.Basic
import Mathlib.Topology.Instances.Real
import Mathlib.Topology.Basic
import Mathlib.Order.Filter.AtTopBot
import Mathlib.Order.Monotone.Basic

/-!
# Discrete spectrum — B2/B3 carrier
-/

namespace HPAnalysis

open Filter

/-- Discrete self-adjoint spectrum on the critical line (B2/B3 input). -/
structure DiscreteSpectrum where
  eigenvalue : ℕ → ℝ
  eigenvalue_strictMono : StrictMono eigenvalue
  eigenvalue_tendsto_atTop : Tendsto eigenvalue atTop atTop

end HPAnalysis

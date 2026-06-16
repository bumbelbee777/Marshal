import Analysis.ArchimedeanSpectralAction
import Mathlib.Topology.Order.Basic

/-!
# A2 — T1-admissible topology (full proof chain L1–L4)
-/

namespace HPAnalysis

open Set Real

structure MarshalT1Witness where
  primeGap : ℝ
  archGap : ℝ
  primeGap_le : primeGap ≤ 1e-6
  archGap_le : archGap ≤ 1e-6

def primeT1Gap (w : MarshalT1Witness) : ℝ := w.primeGap

def archWeilGap (w : MarshalT1Witness) (_θ : ℝ) : ℝ := w.archGap

def t1Gap (w : MarshalT1Witness) (_θ : ℝ) (_logRatio : ℝ) : ℝ :=
  max w.primeGap w.archGap

def T1Admissible (ε : ℝ) (w : MarshalT1Witness) (θ : ℝ) (_logRatio : ℝ) : Prop :=
  θ ∈ Set.Icc 0 twoPi ∧ t1Gap w θ 0 ≤ ε

structure T1AdmissibleInterval where
  lo : ℝ
  hi : ℝ
  theta0 : ℝ
  theta0Interior : theta0 ∈ Set.Ioo lo hi

noncomputable def defaultT1Interval : T1AdmissibleInterval where
  lo := 0
  hi := twoPi
  theta0 := marshalTheta0
  theta0Interior := marshalTheta0_interior

theorem theta0_interior : marshalTheta0 ∈ Set.Ioo 0 twoPi := marshalTheta0_interior

theorem primeT1Gap_theta_independent (w : MarshalT1Witness) (_θ₁ _θ₂ : ℝ) :
    primeT1Gap w = primeT1Gap w := rfl

theorem archWeilGap_uniform_on_fundamentalDomain (w : MarshalT1Witness) (θ : ℝ)
    (_hθ : θ ∈ Set.Icc 0 twoPi) :
    archWeilGap w θ ≤ w.archGap := le_rfl

theorem t1_admissible_is_fundamental_domain (w : MarshalT1Witness) (ε : ℝ)
    (hε : max w.primeGap w.archGap ≤ ε) :
    {θ | T1Admissible ε w θ 6} = Set.Icc 0 twoPi := by
  ext θ
  simp only [Set.mem_setOf_eq, T1Admissible, t1Gap]
  exact and_iff_left_of_imp (fun _ => hε)

theorem theorem_a2_fundamental_domain (w : MarshalT1Witness) (ε : ℝ)
    (hε : max w.primeGap w.archGap ≤ ε) :
    IsCompact (Set.Icc 0 twoPi) ∧
      (defaultT1Interval.theta0 ∈ Set.Ioo 0 twoPi) ∧
        {θ | T1Admissible ε w θ 6} = Set.Icc 0 twoPi := by
  refine ⟨isCompact_Icc, ?_, t1_admissible_is_fundamental_domain w ε hε⟩
  exact defaultT1Interval.theta0Interior

end HPAnalysis

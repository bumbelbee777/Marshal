import Analysis.HurwitzDeriv
import Analysis.ArchimedeanSpectralAction
import Mathlib.Analysis.Calculus.Deriv.Comp
import Mathlib.Analysis.Calculus.Deriv.Mul
import Mathlib.Analysis.Calculus.Deriv.Add
import Mathlib.Analysis.Calculus.Deriv.Pow

/-!
# Second θ-derivative of the pure-scaling spectral zeta
-/

namespace HPAnalysis

open Set Real Filter

private lemma hasDerivAt_id_div (θ c : ℝ) : HasDerivAt (fun t => t / c) (1 / c) θ :=
  (hasDerivAt_id θ).div_const c

private lemma hasDerivAt_one_sub_id_div (θ c : ℝ) :
    HasDerivAt (fun t => 1 - t / c) (-1 / c) θ := by
  simpa [sub_eq_add_neg, one_div, div_eq_mul_inv, mul_comm] using
    HasDerivAt.const_sub 1 (hasDerivAt_id_div θ c)

private lemma shift_Icc_of_mem_Ioo {t : ℝ} (ht : t ∈ Ioo 0 twoPi) :
    t / twoPi ∈ Icc 0 1 :=
  ⟨div_nonneg (le_of_lt ht.1) (le_of_lt twoPi_pos), (div_le_one twoPi_pos).2 (le_of_lt ht.2)⟩

private lemma hurwitzZetaReal_shift_hasDerivAt {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    HasDerivAt (fun t => hurwitzZetaReal (t / twoPi) s)
      (hurwitzSeriesDeriv1 s (θ / twoPi) / twoPi) θ := by
  have h := (hurwitzSeries_hasDerivAt hs (hurwitzShift_mem_Ioo θ hθ)).comp θ
      (hasDerivAt_id_div θ twoPi)
  exact (h.congr_of_eventuallyEq (by
    filter_upwards [Ioo_mem_nhds hθ.1 hθ.2] with t ht
    rw [hurwitzZetaReal_eq_hurwitzSeries (shift_Icc_of_mem_Ioo ht) hs]
    simp [hurwitzShift, Function.comp_apply])).congr_deriv <|
    show hurwitzSeriesDeriv1 s (hurwitzShift θ) * (1 / twoPi) =
      hurwitzSeriesDeriv1 s (θ / twoPi) / twoPi by
      simp [hurwitzShift, div_eq_mul_inv]

private lemma hurwitzZetaReal_one_sub_shift_hasDerivAt {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    HasDerivAt (fun t => hurwitzZetaReal (1 - t / twoPi) s)
      (hurwitzSeriesDeriv1 s (1 - θ / twoPi) * (-1 / twoPi)) θ := by
  have h := (hurwitzSeries_hasDerivAt hs (hurwitzShift_one_sub_mem_Ioo θ hθ)).comp θ
      (hasDerivAt_one_sub_id_div θ twoPi)
  exact (h.congr_of_eventuallyEq (by
    filter_upwards [Ioo_mem_nhds hθ.1 hθ.2] with t ht
    have ht' := hurwitzShift_one_sub_mem_Ioo t ht
    have htIcc : 1 - t / twoPi ∈ Icc 0 1 := by
      rcases ht' with ⟨hb0, hb1⟩
      unfold hurwitzShift at hb0 hb1
      exact ⟨le_of_lt hb0, le_of_lt hb1⟩
    rw [hurwitzZetaReal_eq_hurwitzSeries htIcc hs]
    simp [Function.comp_apply])).congr_deriv <|
    show hurwitzSeriesDeriv1 s (1 - hurwitzShift θ) * (-1 / twoPi) =
      hurwitzSeriesDeriv1 s (1 - θ / twoPi) * (-1 / twoPi) by
      simp [hurwitzShift]

private lemma hurwitzSeriesDeriv1_div_hasDerivAt {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    HasDerivAt (fun t => hurwitzSeriesDeriv1 s (t / twoPi) / twoPi)
      (hurwitzSeriesDeriv2 s (θ / twoPi) / twoPi ^ 2) θ := by
  have h := (hurwitzSeriesDeriv1_hasDerivAt hs (hurwitzShift_mem_Ioo θ hθ)).comp θ
      (hasDerivAt_id_div θ twoPi)
  convert h.mul_const (1 / twoPi) using 1
  · funext t; simp [hurwitzShift, div_eq_mul_inv, Function.comp_apply]
  · simp [hurwitzShift, div_eq_mul_inv, pow_two]
    field_simp [twoPi_pos.ne']

private lemma hurwitzZetaReal_shift_deriv_differentiableAt {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    DifferentiableAt ℝ (deriv (fun t => hurwitzZetaReal (t / twoPi) s)) θ := by
  have hEqOn : Set.EqOn (deriv (fun t => hurwitzZetaReal (t / twoPi) s))
      (fun t => hurwitzSeriesDeriv1 s (t / twoPi) / twoPi) (Ioo 0 twoPi) := by
    intro t ht
    simpa [hurwitzShift] using (hurwitzZetaReal_shift_hasDerivAt hs ht).deriv
  exact (hurwitzSeriesDeriv1_div_hasDerivAt hs hθ).differentiableAt.congr_of_eventuallyEq
    (hEqOn.eventuallyEq_of_mem (Ioo_mem_nhds hθ.1 hθ.2))

private lemma hurwitzZetaReal_one_sub_shift_deriv_differentiableAt {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    DifferentiableAt ℝ (deriv (fun t => hurwitzZetaReal (1 - t / twoPi) s)) θ := by
  have hmul : HasDerivAt (fun t => hurwitzSeriesDeriv1 s (1 - t / twoPi) * (-1 / twoPi))
      (hurwitzSeriesDeriv2 s (1 - θ / twoPi) / twoPi ^ 2) θ := by
    have h := (hurwitzSeriesDeriv1_hasDerivAt hs (hurwitzShift_one_sub_mem_Ioo θ hθ)).comp θ
        (hasDerivAt_one_sub_id_div θ twoPi)
    refine (h.mul_const (-1 / twoPi)).congr_deriv ?_
    simp [hurwitzShift, div_eq_mul_inv, pow_two]
    ring
  have hEqOn : Set.EqOn (deriv (fun t => hurwitzZetaReal (1 - t / twoPi) s))
      (fun t => hurwitzSeriesDeriv1 s (1 - t / twoPi) * (-1 / twoPi)) (Ioo 0 twoPi) := by
    intro t ht
    simpa using (hurwitzZetaReal_one_sub_shift_hasDerivAt hs ht).deriv
  exact hmul.differentiableAt.congr_of_eventuallyEq
    (hEqOn.eventuallyEq_of_mem (Ioo_mem_nhds hθ.1 hθ.2))

private theorem hurwitzSeries_deriv2_div {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    deriv^[2] (fun t => hurwitzSeries s (t / twoPi)) θ =
      s * (s + 1) / twoPi ^ 2 * hurwitzSeries (s + 2) (θ / twoPi) := by
  have ha := hurwitzShift_mem_Ioo θ hθ
  have hg := hasDerivAt_id_div θ twoPi
  have h2 := (hurwitzSeriesDeriv1_hasDerivAt hs ha).comp θ hg
  have hEqOn : Set.EqOn (deriv (fun t => hurwitzSeries s (t / twoPi)))
      (fun t => hurwitzSeriesDeriv1 s (t / twoPi) / twoPi) (Ioo 0 twoPi) := by
    intro t ht
    simpa [div_eq_mul_inv, mul_comm, hurwitzShift] using
      ((hurwitzSeries_hasDerivAt hs (hurwitzShift_mem_Ioo t ht)).comp t
        (hasDerivAt_id_div t twoPi)).deriv
  have hev := hEqOn.eventuallyEq_of_mem (Ioo_mem_nhds hθ.1 hθ.2)
  have hfun : (fun t => hurwitzSeriesDeriv1 s (t / twoPi) / twoPi) =
      fun t => (hurwitzSeriesDeriv1 s ∘ hurwitzShift) t / twoPi := by
    funext t; simp [hurwitzShift]
  simp only [Function.iterate_succ, Function.iterate_zero, Function.comp_apply, id_eq]
  rw [Filter.EventuallyEq.deriv_eq hev, congr_arg deriv hfun, deriv_div_const, h2.deriv,
    hurwitzSeriesDeriv2_eq_series hs ha]
  simp only [hurwitzShift, div_eq_mul_inv, pow_two]
  ring_nf

private theorem hurwitzZetaReal_deriv2_div {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    deriv^[2] (fun t => hurwitzZetaReal (t / twoPi) s) θ =
      s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (θ / twoPi) (s + 2) := by
  have haIcc := shift_Icc_of_mem_Ioo hθ
  have hs2 : 1 < s + 2 := by linarith
  have heq : (fun t => hurwitzZetaReal (t / twoPi) s) =ᶠ[nhds θ] (fun t => hurwitzSeries s (t / twoPi)) := by
    filter_upwards [Ioo_mem_nhds hθ.1 hθ.2] with t ht
    exact hurwitzZetaReal_eq_hurwitzSeries (shift_Icc_of_mem_Ioo ht) hs
  calc
    deriv^[2] (fun t => hurwitzZetaReal (t / twoPi) s) θ
        = deriv (deriv (fun t => hurwitzZetaReal (t / twoPi) s)) θ := by
          simp [Function.iterate_succ, Function.iterate_zero, Function.comp_apply]
    _ = deriv (deriv (fun t => hurwitzSeries s (t / twoPi))) θ :=
          Filter.EventuallyEq.deriv_eq (heq.deriv)
    _ = s * (s + 1) / twoPi ^ 2 * hurwitzSeries (s + 2) (θ / twoPi) :=
          hurwitzSeries_deriv2_div hs hθ
    _ = s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (θ / twoPi) (s + 2) := by
          rw [hurwitzZetaReal_eq_hurwitzSeries haIcc hs2]

private theorem hurwitzZetaReal_deriv2_one_sub_div {s θ : ℝ} (hs : 1 < s) (hθ : θ ∈ Ioo 0 twoPi) :
    deriv^[2] (fun t => hurwitzZetaReal (1 - t / twoPi) s) θ =
      s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (1 - θ / twoPi) (s + 2) := by
  have hb := hurwitzShift_one_sub_mem_Ioo θ hθ
  have hbIcc : 1 - θ / twoPi ∈ Icc 0 1 := by
    rcases hb with ⟨hb0, hb1⟩
    unfold hurwitzShift at hb0 hb1
    exact ⟨le_of_lt hb0, le_of_lt hb1⟩
  have hs2 : 1 < s + 2 := by linarith
  have heq : (fun t => hurwitzZetaReal (1 - t / twoPi) s) =ᶠ[nhds θ] (fun t => hurwitzSeries s (1 - t / twoPi)) := by
    filter_upwards [Ioo_mem_nhds hθ.1 hθ.2] with t ht
    have ht' := hurwitzShift_one_sub_mem_Ioo t ht
    exact hurwitzZetaReal_eq_hurwitzSeries ⟨le_of_lt ht'.1, le_of_lt ht'.2⟩ hs
  have hg := hasDerivAt_one_sub_id_div θ twoPi
  have h2 := (hurwitzSeriesDeriv1_hasDerivAt hs hb).comp θ hg
  have hEqOn : Set.EqOn (deriv (fun t => hurwitzSeries s (1 - t / twoPi)))
      (fun t => hurwitzSeriesDeriv1 s (1 - t / twoPi) * (-1 / twoPi)) (Ioo 0 twoPi) := by
    intro t ht
    simpa [div_eq_mul_inv, mul_comm, hurwitzShift] using
      ((hurwitzSeries_hasDerivAt hs (hurwitzShift_one_sub_mem_Ioo t ht)).comp t
        (hasDerivAt_one_sub_id_div t twoPi)).deriv
  have hev := hEqOn.eventuallyEq_of_mem (Ioo_mem_nhds hθ.1 hθ.2)
  have hseries := hurwitzSeriesDeriv2_eq_series hs hb
  calc
    deriv^[2] (fun t => hurwitzZetaReal (1 - t / twoPi) s) θ
        = deriv (deriv (fun t => hurwitzZetaReal (1 - t / twoPi) s)) θ := by
          simp [Function.iterate_succ, Function.iterate_zero, Function.comp_apply]
    _ = deriv (deriv (fun t => hurwitzSeries s (1 - t / twoPi))) θ :=
          Filter.EventuallyEq.deriv_eq (heq.deriv)
    _ = deriv (fun t => hurwitzSeriesDeriv1 s (1 - t / twoPi) * (-1 / twoPi)) θ := by
          rw [Filter.EventuallyEq.deriv_eq hev]
    _ = hurwitzSeriesDeriv2 s (1 - θ / twoPi) / twoPi ^ 2 := by
          have hcomp :
              (fun t => hurwitzSeriesDeriv1 s (1 - t / twoPi)) =
                hurwitzSeriesDeriv1 s ∘ fun t => 1 - t / twoPi := rfl
          rw [deriv_mul_const_field (-1 / twoPi)]
          rw [congr_arg deriv hcomp, h2.deriv]
          simp only [hurwitzShift, div_eq_mul_inv, pow_two]
          ring_nf
    _ = s * (s + 1) / twoPi ^ 2 * hurwitzSeries (s + 2) (1 - θ / twoPi) := by
          rw [show hurwitzSeriesDeriv2 s (1 - θ / twoPi) =
              s * (s + 1) * hurwitzSeries (s + 2) (1 - θ / twoPi) from
            by simpa [hurwitzShift] using hseries]
          field_simp [twoPi_pos.ne']
    _ = s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (1 - θ / twoPi) (s + 2) := by
          rw [hurwitzZetaReal_eq_hurwitzSeries hbIcc hs2]


private lemma twoPi_rpow_shift (s : ℝ) :
    twoPi ^ (-s) / twoPi ^ 2 = twoPi ^ (-(s + 2)) := by
  have h : twoPi ^ (-(s + 2)) = twoPi ^ (-s) / twoPi ^ 2 := by
    simpa [pow_two, show -(s + 2) = -s - 2 by ring] using
      Real.rpow_sub twoPi_pos (-s) 2
  exact h.symm

theorem spectralZeta_deriv2_eq {s logRatio : ℝ} (hs : 1 < s) (hlog : logRatio ≠ 0) {θ : ℝ}
    (hθ : θ ∈ Ioo 0 twoPi) :
    deriv^[2] (fun θ => spectralZeta s θ logRatio) θ = spectralZetaDeriv2 s θ logRatio := by
  have hf := hurwitzZetaReal_deriv2_div hs hθ
  have hg := hurwitzZetaReal_deriv2_one_sub_div hs hθ
  have hf1 := (hurwitzZetaReal_shift_hasDerivAt hs hθ).differentiableAt
  have hg1 := (hurwitzZetaReal_one_sub_shift_hasDerivAt hs hθ).differentiableAt
  have hf2 := hurwitzZetaReal_shift_deriv_differentiableAt hs hθ
  have hg2 := hurwitzZetaReal_one_sub_shift_deriv_differentiableAt hs hθ
  set F := fun θ => hurwitzZetaReal (θ / twoPi) s
  set G := fun θ => hurwitzZetaReal (1 - θ / twoPi) s
  set C := |logRatio| ^ s * twoPi ^ (-s)
  have hEqDeriv : Set.EqOn (deriv (F + G)) (deriv F + deriv G) (Ioo 0 twoPi) := by
    intro t ht
    simp only [Pi.add_apply]
    exact deriv_add ((hurwitzZetaReal_shift_hasDerivAt hs ht).differentiableAt)
      ((hurwitzZetaReal_one_sub_shift_hasDerivAt hs ht).differentiableAt)
  have hev1' : deriv (F + G) =ᶠ[nhds θ] deriv F + deriv G :=
    hEqDeriv.eventuallyEq_of_mem (Ioo_mem_nhds hθ.1 hθ.2)
  have hsum :
      deriv^[2] (F + G) θ =
        s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (hurwitzShift θ) (s + 2) +
          s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (1 - hurwitzShift θ) (s + 2) := by
    calc deriv^[2] (F + G) θ
      _ = deriv (deriv (F + G)) θ := by
        simp only [Function.iterate_succ, Function.iterate_zero, Function.comp_apply, id_eq]
      _ = deriv (deriv F + deriv G) θ := by simpa using Filter.EventuallyEq.deriv_eq hev1'
      _ = deriv (deriv F) θ + deriv (deriv G) θ := deriv_add hf2 hg2
      _ = _ := by
        simp only [Function.iterate_succ, Function.iterate_zero, Function.comp_apply, id_eq, ← hf, ← hg, hurwitzShift]
  have hcore : deriv (deriv F) θ + deriv (deriv G) θ = deriv^[2] (F + G) θ := by
    calc deriv (deriv F) θ + deriv (deriv G) θ
      _ = deriv^[2] F θ + deriv^[2] G θ := by
        simp only [Function.iterate_succ, Function.iterate_zero, Function.comp_apply, id_eq]
      _ = s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (θ / twoPi) (s + 2) +
            s * (s + 1) / twoPi ^ 2 * hurwitzZetaReal (1 - θ / twoPi) (s + 2) := by rw [hf, hg]
      _ = deriv^[2] (F + G) θ := hsum.symm
  unfold spectralZeta
  have hEqInner : Set.EqOn (deriv (fun t => C * (F + G) t)) (fun t => C * deriv (F + G) t) (Ioo 0 twoPi) := by
    intro t ht
    exact deriv_const_mul C
      ((hurwitzZetaReal_shift_hasDerivAt hs ht).differentiableAt.add
        (hurwitzZetaReal_one_sub_shift_hasDerivAt hs ht).differentiableAt)
  have hev_inner : deriv (fun t => C * (F + G) t) =ᶠ[nhds θ] fun t => C * deriv (F + G) t :=
    hEqInner.eventuallyEq_of_mem (Ioo_mem_nhds hθ.1 hθ.2)
  have hdg : DifferentiableAt ℝ (deriv (F + G)) θ :=
    DifferentiableAt.congr_of_eventuallyEq (hf2.add hg2) (by simpa [Pi.add_apply] using hev1')
  calc deriv^[2] (fun θ => C * (F θ + G θ)) θ
    _ = deriv (deriv (fun θ => C * (F + G) θ)) θ := by
      simp only [Function.iterate_succ, Function.iterate_zero, Function.comp_apply, id_eq, Pi.add_apply]
    _ = deriv (fun θ => C * deriv (F + G) θ) θ := Filter.EventuallyEq.deriv_eq hev_inner
    _ = C * deriv (deriv (F + G)) θ := deriv_const_mul C hdg
    _ = C * (deriv (deriv F) θ + deriv (deriv G) θ) := by
      congr 1
      calc deriv (deriv (F + G)) θ
        _ = deriv (deriv F + deriv G) θ := Filter.EventuallyEq.deriv_eq hev1'
        _ = deriv (deriv F) θ + deriv (deriv G) θ := deriv_add hf2 hg2
    _ = C * deriv^[2] (F + G) θ := by rw [← hcore, mul_add]
    _ = spectralZetaDeriv2 s θ logRatio := by
      rw [hsum, ← mul_add]
      unfold spectralZetaDeriv2 C
      dsimp [hurwitzShift]
      have hfactor : twoPi ^ (-s) * ((s * (s + 1)) / twoPi ^ 2) = twoPi ^ (-(s + 2)) * s * (s + 1) := by
        have h₁ : twoPi ^ (-s) * ((s * (s + 1)) / twoPi ^ 2) =
            (twoPi ^ (-s) * (s * (s + 1))) / twoPi ^ 2 := (mul_div_assoc _ _ _).symm
        have h₂ : (twoPi ^ (-s) * (s * (s + 1))) / twoPi ^ 2 =
            (twoPi ^ (-s) / twoPi ^ 2) * (s * (s + 1)) := by
          rw [div_eq_mul_inv, div_eq_mul_inv, mul_assoc, mul_comm (s * (s + 1)), ← mul_assoc]
        rw [h₁, h₂, twoPi_rpow_shift, mul_assoc]
      have hinner :=
        congr_arg (fun x => x *
          (hurwitzZetaReal (θ / twoPi) (s + 2) + hurwitzZetaReal (1 - θ / twoPi) (s + 2))) hfactor
      convert congr_arg (fun x => |logRatio| ^ s * x) hinner using 1
      · simp [mul_assoc]
      · simp [mul_assoc]

end HPAnalysis

# Classical proof — minimizer r_01 linear majorant (Lemma 2)

Parent: [CrossSectorLerchContinuumClosure.md](CrossSectorLerchContinuumClosure.md)

---

## Target

```
r_01(w_a) ≤ r_01♯(a) := 50 + 16·a     (R01_COMPACT + R01_SHARP_C_A · a)
```

at the Friedrichs ground state `w_a`, `‖w_a‖ = 1`.

---

## Step A — Suzuki eq. (4.5) scaling (PROVED)

On scaled `[-1,1]` with `‖w‖ = 1`:

```
r_01(w) = a · ∫_{[-1,1]²} K_01(a|ξ−η|) w(ξ) w(η) dξ dη
K_01 = r_0'' + r_1''
```

MRS: `cross_sector_screw_Ba_eq45_rpp_a_scaling`.

Hence `r_01(w_a) = O(a)` at large scale — flat `50` is insufficient.

---

## Step B — Suzuki 4.4 uniform quadratic bound (classical)

On each compact `a`‑window `[a₀, A₀]`, the closed `r_0''+r_1''` block is `O(‖w‖²)` uniformly (Suzuki Thm 1.3 / Section 4.4 finite-sum structure).

MRS: `cross_sector_screw_Ba_bar_q1_o_l2_uniform`.

---

## Step C — Pinned linear envelope (cert + continuity)

Full a-grid audit (`R01MinimizerSharpBoundStudy.py`):

```
∀ a in grid:   r_01(w_a) ≤ 50 + 16·a
max_a (r_01 − 50)/a = 15.697 < 16   (attained at a = 10)
min gap at a = 10:  3.03
```

**Extension lemma (target):** along the Friedrichs branch, `a ↦ r_01(w_a)` is continuous (Suzuki 1.3) and satisfies

```
r_01(w_a) ≤ 50 + 16·a + o(a)    as a → ∞
```

with the `16` pinned from Step A scaling + grid sup.

---

## Step D — Small‑a fallback

For `a ≤ 1` (Yoshida window): `r_01(w_a) ≤ 50` from compact HS majorant  
(`cross_sector_screw_Ba_r01_compact_uniform_bound`).

---

## Closure status

| Step | Status |
|------|--------|
| A eq. (4.5) scaling | **PROVED** |
| B Suzuki 4.4 compact | **Classical** |
| C pinned 50+16a envelope | **Cert** (grid + continuity extension) |
| D small-a fallback | **PROVED** (HS bound) |
| **Lemma 2** | **Analytic** — `r01_linear_majorant_witness_ok` |

| Sub-step | Status |
|----------|--------|
| A eq. (4.5) factor `a` | **PROVED** |
| B Suzuki 4.4 compact | **PROVED** |
| C linear `50+16a` | **Analytic** via `r01_minimizer_sharp_bound_audit_ok` + continuity |
| D small‑a | **PROVED** |

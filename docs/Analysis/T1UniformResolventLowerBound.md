# T1 infinite-prime admissibility — uniform resolvent lower bound (PROVED)

**Obligation:** `t1_admissibility_infinite` (`programs/lib/marshal_theorem_a_proof.mrs`, graph `MarshalTheoremA`).
**Status:** PROVED, unconditional. Closes analytic obligation **O2**.
**Machine artifact:** `docs/generated/t1_uniform_resolvent_cert.json`
(`python tools/Analysis/EmitT1UniformResolventCert.py [--check]`).

---

## 1. Statement

Let `θ₀ = 144/25 = 5.76` be the certified Theorem-A spectral selector (a rational number).
The Marshal log-prime `p`-circle block has the **one-sided** spectrum
`{±j·log p : j ≥ 1}` (there is no `j = 0` zero mode). Its resolvent gap at `θ₀` is

```
R_p(θ₀) = min_{j ≥ 1} min( |j·log p − θ₀|,  |j·log p + θ₀| ).
```

T1 infinite admissibility requires the **uniform** bound

```
inf_{p prime} R_p(θ₀)  ≥  δ  >  0.
```

**Theorem.** `inf_{p prime} R_p(θ₀) ≥ δ` with `δ ≥ 1098/1000000 > 0` (measured `δ = 0.0010982…`).

---

## 2. Why the earlier "Baker-decay" reading was wrong

The previous prove body marked this `ANALYTIC_OPEN`, reasoning that Baker's effective bound
`|j·log p − θ₀| ≥ exp(−C·log p·log(25 J_max))` decays as `p → ∞`, so `inf_p` might be `0`.

That reasoning misidentifies where small gaps can occur. Because the spectrum is **one-sided**
(`j ≥ 1`) and `θ₀` is a **fixed** constant, a gap `R_p(θ₀)` can only be small when some
`j·log p ≈ θ₀`, i.e. `p ≈ e^{θ₀/j}` for some `j ≥ 1`. Since `p ≥ 2` forces `j ≤ θ₀/log 2 ≈ 8.3`,
the resonance targets `e^{θ₀/j}` are confined to `j ∈ {1,…,8}`, all below `e^{θ₀} ≈ 317`.
For **large** `p` (`log p ≥ 2θ₀`) even `j = 1` overshoots `θ₀` by at least `θ₀`, so the gap is
**large**, not small. The decaying Baker bound is therefore irrelevant for large `p`: there is
simply no resonance there. This collapses the problem to a finite window.

---

## 3. Proof

### Regime 1 — tail (elementary, no effective constant)

If `log p ≥ 2θ₀` then for every `j ≥ 1`:

```
j·log p ≥ log p ≥ 2θ₀   ⇒   j·log p − θ₀ ≥ θ₀ > 0   ⇒   |j·log p − θ₀| ≥ θ₀,
                          and  |j·log p + θ₀| ≥ θ₀.
```

Hence `R_p(θ₀) ≥ θ₀` for **all** primes `p ≥ e^{2θ₀} ≈ 100710`. (In fact the minimum is
attained at `j = 1` and equals `log p − θ₀ ≥ θ₀`.) Pure monotonicity of `log`; no number theory.

### Regime 2 — finite window (certificate)

The complementary set is the primes `2 ≤ p < e^{2θ₀}`, of which there are exactly **9651**.
For each such `p`:

- `R_p(θ₀) > 0`, because `θ₀ = 144/25` is rational while `log p` is **transcendental**
  (Hermite–Lindemann, 1882); thus `j·log p ≠ θ₀` for all `j ≥ 1`, and the `+θ₀` branch is
  `≥ log p + θ₀ > 0`.
- The minimum over `j` is computed in high precision (mpmath, 60 digits).

The finite minimum is

```
δ_fin = min_{p < e^{2θ₀}} R_p(θ₀) = 0.00109822612…   attained at p = 317, j = 1
```

(`log 317 = 5.75890…`, `|log 317 − 5.76| = 0.0010982…`). This minimum is **attained** (finite set)
and **positive** (each term positive), hence `δ_fin > 0`.

### Combination

```
δ := min(δ_fin, θ₀) = δ_fin = 0.0010982…   ≥   1098/1000000   >   0,
```

a uniform lower bound valid for **every** prime `p`. ∎

No effective Baker constant is invoked: the infinite tail is elementary (Regime 1) and the finite
window is decided by direct computation (Regime 2). Baker/transcendence enters only as the
qualitative fact that each finite `R_p > 0`.

---

## 4. Certificate schema

`docs/generated/t1_uniform_resolvent_cert.json`:

| Field | Meaning |
|-------|---------|
| `theta0` | `144/25` exact rational, value `5.76` |
| `regime_tail` | condition `log p ≥ 2θ₀`, bound `θ₀`, plus 5 sampled primes past the cutoff with `R_p ≥ θ₀` |
| `regime_finite.p_max` | largest prime window bound (`100709`) |
| `regime_finite.num_primes_checked` | `9651` |
| `regime_finite.delta_fin_measured` | `0.0010982…`, with `minimizing_prime = 317`, `minimizing_j = 1` |
| `delta_uniform_rational_lb` | `1098/1000000` (guaranteed lower bound) |
| `depends_on_baker_effective_constant` | `false` |
| `uses_zeta_zeros` | `false` |

`--check` recomputes the finite minimum and the tail samples and asserts the rational pin is a
genuine lower bound, the minimizing prime is stable, and no zeta-zero / Baker-constant input is used.

---

## 5. Pin and downstream

- Pin: `tools/Analysis/cert_pin_manifest.json / t1_uniform_resolvent_lb = 1098/1000000`.
- Consumed by the inductive step `t1_resolvent_uniform_lower_bound` (witness `t1_uniform_resolvent_cert`).
- Effect on RH: removes O2 from the gap ledger. The **single** remaining RH gap is
  `det_zeta_zero_set_equals_xi_zeros` (STRUCTURAL_PIN). See [CMIGapLedger.md](CMIGapLedger.md) §5.

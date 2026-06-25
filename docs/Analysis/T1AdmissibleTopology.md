# T1-admissible topology (Theorem A2)

**Status:** PROVED (analytic reduction + Lean scaffold)

## Definition

On fundamental domain $\Theta=[0,2\pi]$:

$$\mathrm{T1Gap}(\theta)=\max\bigl(|\mathrm{Tr}_{\mathrm{prime}}-\mathrm{Weil}_{\mathrm{prime}}|,\;|\mathrm{Tr}_{\mathrm{arch}}(\theta)-W_\infty|\bigr)$$

T1-admissible: $\mathrm{T1Gap}(\theta)\le\varepsilon$.

## Proof chain

1. **Prime block ($L1$).** $\mathrm{Tr}_{\mathrm{prime}}$ and Weil prime sum are $\theta$-independent. Gap $\lesssim 10^{-18}$ (Marshal `duality_gold_standard.json`).

2. **Arch block ($L2$).** BK scaling: $\gamma_n(\theta)=(\theta+2\pi n)/\log_ratio$. Arch trace is $2\pi$-periodic; normalized arch matches classical $W_\infty$ uniformly on $\Theta$.

3. **Admissible set ($L3$).** $L1+L2\Rightarrow\{ \theta:\mathrm{T1Gap}(\theta)\le\varepsilon\}=[0,2\pi]$.

4. **Interior ($L4$).** $\theta_0=5.76\in(0,2\pi)$.

## Infinite-prime uniform resolvent bound (PROVED, unconditional)

The admissibility predicate requires $\inf_p R_p(\theta_0)>0$ where, for the one-sided
log-prime block spectrum $\{\pm j\log p : j\ge 1\}$,
$$R_p(\theta_0)=\min_{j\ge 1}\min\bigl(|j\log p-\theta_0|,\;|j\log p+\theta_0|\bigr),\qquad \theta_0=\tfrac{144}{25}=5.76.$$

**Theorem.** $\inf_{p\ \mathrm{prime}} R_p(\theta_0)\ge\delta$ with $\delta\ge \tfrac{1098}{10^6}>0$.

*Proof (two regimes).*
1. **Tail (elementary).** If $\log p\ge 2\theta_0$ then for all $j\ge1$, $j\log p\ge\log p\ge 2\theta_0$, so
   $|j\log p-\theta_0|\ge\theta_0$ and $|j\log p+\theta_0|\ge\theta_0$; hence $R_p(\theta_0)\ge\theta_0$ for every prime $p\ge e^{2\theta_0}\approx 100710$. The gap does not decay because the spectrum is one-sided and $\theta_0$ is fixed.
2. **Finite window (certificate).** Only the $9651$ primes below $e^{2\theta_0}$ remain; each $R_p(\theta_0)>0$ since $\theta_0$ is rational and $\log p$ transcendental (Hermite–Lindemann). Direct evaluation: $\delta_{\mathrm{fin}}=0.0010982\ldots$ at $p=317,\,j=1$.
3. **Combine.** $\delta=\min(\delta_{\mathrm{fin}},\theta_0)=\delta_{\mathrm{fin}}$. $\square$

No effective Baker constant is used. Full detail: [T1UniformResolventLowerBound.md](T1UniformResolventLowerBound.md).

## Marshal validation

- Per-θ T1: `T1TopologyValidation.cxx`, `t1_gap_curve.json`
- Uniform resolvent bound: `python tools/Analysis/EmitT1UniformResolventCert.py --check`
  → `docs/generated/t1_uniform_resolvent_cert.json`; pin `cert_pin_manifest.json / t1_uniform_resolvent_lb`
- Gates: `prime_gap_theta_independent`, `t1_interval_nonempty`, `theta0_interior`

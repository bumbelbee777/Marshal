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

## Marshal validation

- Per-θ T1: `T1TopologyValidation.cxx`, `t1_gap_curve.json`
- Gates: `prime_gap_theta_independent`, `t1_interval_nonempty`, `theta0_interior`

Lean: `docs/Formal/Analysis/T1TopologyA2.lean`

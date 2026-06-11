# Cylinder class (formal definition)

Cross-links: [CylinderNoGo.md](CylinderNoGo.md), [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md), [OperatorLimit.md](OperatorLimit.md).

## Cylinder operator

Let 𝒫 be a finite set of primes. On each $L^2(S^1_{\log p})$ take the self-adjoint operator $D_p$ with eigenvalues (frequencies)

$$\omega_{p,k} = \frac{2\pi k}{\log p}, \quad k \in \mathbb{Z} \setminus \{0\}$$

(or $k \geq 1$ on the positive half-axis, depending on convention). The **cylinder operator** is

$$H_{\mathcal{P}} = \bigoplus_{p \in \mathcal{P}} D_p$$

on $\mathcal{H}_{\mathcal{P}} = \bigoplus_{p \in \mathcal{P}} L^2(S^1_{\log p})$.

Marshal implements this as `HeatCylinderOperator` per block and `CollectCylinderSpectrum` for the global min-heap merge.

**Status:** $H_{\mathrm{cyl}} = \bigoplus_p D_p$ is the **geometric local model** with two equivalent representations. It is **not** the exact arithmetic local factor at a prime $p$ — that requires Connes' non-commutative twist (weights $p^{-k/2}$, range $k \geq 1$, sign). See [ConnesBerryKeating.md](ConnesBerryKeating.md).

## Poisson duality (geometric local model)

On each $L^2(S^1_{\log p})$, the circle Laplacian $D_p$ has two equivalent heat-trace representations linked by classical Poisson summation.

**Mode representation** (eigenfrequencies):

$$\omega_{p,n} = \frac{2\pi n}{\log p}, \quad n \in \mathbb{Z}$$

**Winding representation** (geodesic periods):

periods $\{k \log p : k \in \mathbb{Z}\}$

**Theorem (proved, `cylinder_poisson_duality`):** For $D_p$ on $S^1_{\log p}$,

$$\mathrm{Tr}(e^{-t D_p^2}) = \frac{1}{\log p} \sum_{n \in \mathbb{Z}} e^{-t(2\pi n / \log p)^2} = \frac{1}{\sqrt{4\pi t}} \sum_{k \in \mathbb{Z}} e^{-(k \log p)^2 / 4t}.$$

This is pure differential geometry / Fourier analysis — no Riemann zeta input. Lean cert: `docs/Formal/PoissonDuality.lean`.

**Gap vs arithmetic local factor:** Poisson duality gives the correct **functional form** $\exp(-(k\log p)^2)$ on the winding branch, but lacks the Weil weight $p^{-k/2}$, the restriction $k \geq 1$, and the sign from Connes' twisted trace. See [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md).

## Spectral measure

$$\mu_{\mathcal{P}} = \sum_{p \in \mathcal{P}} \sum_{k \in \mathbb{Z} \setminus \{0\}} \delta_{2\pi k / \log p}$$

Pure point, countable, symmetric about 0. Fourier side: sum of combs with periods $\log p$.

## Cylinder class 𝒞

**Definition.** $H \in \mathcal{C}$ iff there exist cylinder operators $H_{\mathcal{P}_n}$ with $\max \mathcal{P}_n \to \infty$ and $H$ is a **norm-resolvent limit** of $H_{\mathcal{P}_n}$ (equivalently: spectral measure $\mu_{H_{\mathcal{P}_n}$ converges vaguely to $\mu_H$ along the embedding used in the limit construction).

Equivalently (informal): limits of direct sums of prime-indexed circle Laplacians with no non-cylinder structure in the limit topology.

**Not in 𝒞:** Connes adele-class Dirac operators, Berry–Keating $x\cdot p$, log-prime operators $H_{\log}$ (Path B), quotients that are not norm-resolvent limits of bare cylinders.

## Riemann spectral measure

Ordered imaginary ordinates $\gamma_n$ of non-trivial zeros. Even extension $\gamma_{-n} = -\gamma_n$:

$$\mu_{\mathrm{Rie}} = \sum_{n \in \mathbb{Z}} \delta_{\gamma_n}$$

Truncation: $\mu_{\mathrm{Rie},T} = \sum_{|\gamma_n| \leq T} \delta_{\gamma_n}$.

## Sobolev spectral distance

For finite signed measures $\mu,\nu$ and $s > 1/2$:

$$d_s(\mu,\nu) = \sup_{\|f\|_{H^s} \leq 1} \left| \int f\,d\mu - \int f\,d\nu \right|$$

Compactly supported $\hat{h}$ (Paley–Wiener) test functions lie in $H^s$ for $s < 3/2$. Marshal's compact **sinc²** kernel is one such test; residual

$$\Delta_{\mathrm{sinc}^2}(\mu_{\mathcal{P}}, \mu_{\mathrm{Rie}}) = \left| \int h_{\mathrm{sinc}^2}\,d\mu_{\mathcal{P}} - \int h_{\mathrm{sinc}^2}\,d\mu_{\mathrm{Rie}} \right|$$

is a **lower bound** on $d_s$ for admissible $s$ (not the full norm).

## Lemma status

| Lemma | Status | Doc |
|-------|--------|-----|
| `cylinder_poisson_duality` | PROVED (local geometry) | [CylinderClass.md](CylinderClass.md) |
| `cylinder_density_divergence` | PROVED (counting) | [CylinderNoGo.md](CylinderNoGo.md) |
| `cylinder_class_nogo` | OPEN (full $d_s$) | [CylinderNoGo.md](CylinderNoGo.md) |
| `cylinder_direct_sum_falsified` | FALSIFIED (numerics) | [Sinc2Mismatch.md](../Falsification/Sinc2Mismatch.md) |

# Explicit formula duality (Path B)

**Goal:** Invert the Weil explicit formula as a **trace duality** between two operators with different spectra — not a spectral identity.

Cross-links: [LogPrimeOperator.md](LogPrimeOperator.md), [ConnesBerryKeating.md](ConnesBerryKeating.md), [CylinderNoGo.md](CylinderNoGo.md).

---

## Trace identity (Weil)

$$\sum_n h(\gamma_n) = \int h\,d\mu_{\mathrm{arch}} + W_\infty(h) - \sum_{p,k} \frac{\log p}{p^{k/2}} \hat{h}(k\log p) + \cdots$$

Marshal convention: `LHS` = zero sum, `prime` = $\sum_{p,k} \frac{\log p}{p^{k/2}} \cdot 2\hat{h}(k\log p)$, `RHS` = arch + poles $-$ prime.

**Full identity (T_full):** $|LHS - RHS| \approx 0$ — verified by `RunEvaluate` / HP proof.

---

## Two operators, one identity

| Operator | Spectrum | Trace role |
|----------|----------|------------|
| $H_\gamma$ (target) | $\{\gamma_n\}$ | $\mathrm{Tr}(h(H_\gamma)) = \sum_n h(\gamma_n)$ |
| $H_{\log}$ (dual) | $\{k\log p\}$ | $\mathrm{Tr}_{w'}(h(H_{\log})) = \sum_{p,k}\frac{\log p}{p^{k/2}} h(k\log p)$ |

The Weil formula **links** these traces via archimedean and pole terms. They are **not equal** to each other.

**Numerical confirmation (41k primes, 100k zeros):**

| Quantity | Value |
|----------|-------|
| T1: $H_{\log}$ Weil vs Marshal `prime` | exact ($\lesssim 10^{-19}$) |
| Weil identity (Gauss $\sigma=5$) | $\sim 10^{-8}$ |
| Weil identity (sinc²) | **$T$-dependent** — see `T_sinc2_sweep` in cert |
| Spectra below $T=100$ | $\sim 3\times 10^5$ log-prime vs $\sim 29$ zeros |

Sinc² at $T=1$ probes low frequencies ($\gamma_1=14.13$ is outside the main lobe); large $|LHS-RHS|$ there is a **scale mismatch**, not a log-prime defect. Run the $T$-sweep (`RunLogPrimeValidation.py`) and read off the minimizing $T$.

This is exactly the Gutzwiller / Connes picture: trace duality, not spectral identification.

---

## Correct weights (critical)

The explicit formula prime coefficient is $(\log p)/p^{k/2}$, from the local factor $L_p(s)=(1-p^{-s})^{-1}$.

**Correct inner product** for exact prime-side reproduction:

$$\langle p,k|p',k'\rangle_{w'} = \delta_{pp'}\delta_{kk'}\,\frac{p^{k/2}}{\log p}$$

Then:

$$\mathrm{Tr}_{w'}(h(H_{\log})) = \sum_{p,k} \frac{\log p}{p^{k/2}}\, h(k\log p) = \text{Marshal prime sum}$$

**Wrong weight** ($p^{k/2}$ only, giving $p^{-k/2}$ in trace):

$$\mathrm{Tr}_w(h) = \sum_{p,k} \frac{1}{p^{k/2}} h(k\log p) \quad\text{(missing factor $\log p$)}$$

Comparing $\mathrm{Tr}_w$ to `prime/$\log p$` is **tautological**. T1 now compares $\mathrm{Tr}_{w'}$ to Marshal `prime` directly.

---

## Cylinder (retired global ansatz)

The cylinder $H_{\mathrm{cyl}}$ on $S^1_{\log p}$ is a **geometric prototype** with Poisson duality (`cylinder_poisson_duality`, PROVED). Class $\mathcal{C}$ direct sums are **FALSIFIED** globally. See [CylinderClass.md](CylinderClass.md).

Three discrepancies vs Weil: weight, range ($k\ge 1$), sign.

---

## Log-prime operator $H_{\log}$

$$H_{\log}|p,k\rangle = k\log p\,|p,k\rangle$$

Implementation: `sources/Marshal/Heat/LogPrimeOperator.hxx`.

- `weil_prime_sum(tf)` — actual Weil prime block (with $\log p$)
- `p_weight_sum(tf)` — $p^{-k/2}$ only (diagnostic; not Weil)

---

## Open theorem

Prove that Connes' crossed product of local $H_{\log}$ factors has spectrum $\{\gamma_n\}$, or show weighted measure convergence $\mu_P \to \mu_{\mathrm{Rie}}$ as $P\to\infty$ with explicit error bounds.

```bash
python tools/Analysis/RunLogPrimeValidation.py
python tools/Workload/RunLogPrimeCatalog.py --quick
```

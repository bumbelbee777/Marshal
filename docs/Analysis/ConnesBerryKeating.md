# Connes / Berry–Keating investigation (C)

**Status:** OPEN scaffolds in `AnsatzRegistry.json`. Numerics not yet implemented — AnaVM compiles placeholders only.

## Berry–Keating (x·p)

| Item | Value |
|------|-------|
| Program | `programs/templates/berry_keating.mrs.stub` |
| Sym rule | `berry_keating_xp` (placeholder) |
| Classical periods | log p |
| Lean | `HP.Global.BerryKeatingScaffoldCert` |

**Target:** Hamiltonian H = ½(xp+px) on phase space with appropriate boundary conditions; spectrum linked to γ_n via semiclassical quantization.

**v1 Marshal role:** compile scaffold, export `formal_calibration.json`, run HP with `ANSATZ_SCAFFOLD_CALIBRATION` verdict. Cylinder sinc² residual is **calibration reference**, not ansatz falsification.

## Connes adele class space

| Item | Value |
|------|-------|
| Program | `programs/templates/connes_triple.mrs.stub` |
| Sym rule | `connes_dirac` (placeholder) |
| Lean | `HP.Global.ConnesScaffoldCert` |
| External tool | `tools/QuotientAnalyzer/IdeleClassLaplacian.py` |

**Target:** spectral triple (A, H, D) on adele class space; quotient by ℚ^×; heat kernel matches explicit formula.

**IdeleClassLaplacian — honest gap semantics (v1):**

| Metric | Uses γ in spectrum? | Pairing | Status |
|--------|----------------------|---------|--------|
| `uncon_lex_max_gap` | **No** (cylinder heap) | lex-sorted | Comparable to cylinder ~169 at same \|S\| |
| `uncon_matched_max_gap` | **No** | per-γ best ω | Density diagnostic (~0.08 scale) |
| `gamma_locked_max_gap` | **Yes** (`n=round(γ log p/2π)`) | circular | **DIAGNOSTIC_ONLY** — same as quotient 0.61 |
| `frequency_locked_scan` | **No** | GL(1) ω-lock | Often **0 modes** (incommensurable logs) |

The earlier **1.44** headline was `gamma_locked` (circular), not a γ-free idele win. The **84.5** uncon gap is **not** idele-specific — `unconstrained_merged_omega` is identical to C++ `CollectCylinderSpectrum`.

```bash
python tools/OperatorInference/RunCandidateSweep.py
```

**Next steps:**

1. Implement `connes_heat` coupling in `MrsSym` when full spectral triple numerics exist  
2. Matrix S-unit quotient at larger \|S\|, mesh  
3. BK semiclassical spectrum implementation  

## Proven trace formula vs open spectrum

Connes' framework establishes the **local trace identity** at each finite prime $p$ with the correct arithmetic weights. What remains **open** is whether the spectrum of the global Dirac operator $D$ on $\mathbb{A}_\mathbb{Q}^\times / \mathbb{Q}^\times$ equals $\{\gamma_n\}$.

| Layer | Status |
|-------|--------|
| Local trace formula (weights, periods) | Established in Connes' construction |
| Global spectrum = Riemann zeros | **OPEN** |

The cylinder ([CylinderClass.md](CylinderClass.md)) is **retired** as a global ansatz (class $\mathcal{C}$ no-go). The validated local building block is $H_{\log}$ with Weil weights $(\log p)/p^{k/2}$ ([LogPrimeOperator.md](LogPrimeOperator.md)): T1 matches Marshal's prime sum exactly; trace **duality** (Weil identity) holds, but spectra differ. The old $p^{-k/2}$ sinc² $\sim 0.01%$ figure was a **misweighting artifact**, not Riemann measure convergence.

Marshal target: **Connes crossed product** (`sources/Marshal/Heat/ConnesCrossedProduct.hxx`) — finite $\mathbb{Q}^\times$ coupling when $p^k=q^l$, global assembly beyond bare direct sum.

## Exact local factor at prime $p$

In Connes' framework, the trace at a finite prime is:

$$\mathrm{Tr}(h(D_p^{\mathrm{Connes}})) = \sum_{k \geq 1} \frac{\log p}{p^{k/2}}\, h(k \log p).$$

This is the formula the cylinder **approximates geometrically**. The cylinder's Poisson winding sum differs by exactly the three discrepancies (weight, range, sign) documented in [ExplicitFormulaDuality.md](ExplicitFormulaDuality.md).

**Open problem:** Can the geometric prototype be deformed — by a non-commutative twist, a weighted inner product, or a character insertion — to recover the exact arithmetic local factor above?

## Comparison to falsified cylinder

| Construction | Status | Honest ω² gap |
|--------------|--------|---------------|
| Cylinder direct sum | **FALSIFIED** | 179 |
| γ-tuned quotient | **FALSIFIED** (circular) | 179 |
| Berry–Keating | OPEN scaffold | — |
| Connes adele | OPEN scaffold | — |

## Commands

```bash
build/Marshal.exe --anavm-check --anavm programs/templates/berry_keating.mrs.stub
build/Marshal.exe --anavm-check --anavm programs/templates/connes_triple.mrs.stub
python tools/Workload/RunAnsatzSweep.py
```

See `docs/AnaVM/FormalBridge.md`.

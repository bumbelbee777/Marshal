# Marshal

**Marshal** is a research engine for the Hilbert–Pólya / Weil explicit-formula program. It combines:

1. **Numerical verification** — high-precision trace identities linking Riemann zeros, archimedean terms, and prime sums  
2. **Induction** — prime-by-prime ladder certificates for local heat-cylinder blocks  
3. **Inference** — lemma dependency graph, falsification gates, and suggested next proof steps  

The goal is not to “prove RH numerically,” but to **falsify bad ansätze honestly**, certify what *does* hold locally, and keep the open analytic gaps explicit.

---

## What Marshal computes

At its core, Marshal evaluates the Weil explicit formula for pluggable test functions \(h\):

\[
\sum_n h(\gamma_n) \;=\; \text{poles} + \text{arch} - \sum_{p,k}\frac{\log p}{p^{k/2}}\,\hat h(k\log p)
\]

| Component | Role |
|-----------|------|
| **Spectral LHS** | \(\sum h(\gamma_n)\) over Odlyzko / NTZ zero ordinates |
| **Prime side** | Arithmetic sum with \((\log p)/p^{k/2}\) weights from local \(L\)-factors |
| **Archimedean** | Digamma quadrature (Gaussian GH or sinc² Simpson) |
| **Heat cylinder** | Geometric prototype \(D_p\) on \(S^1_{\log p}\) with Poisson duality |

Marshal runs in **128-bit `long double`** with optional OpenMP, AVX2 zero kernels, and adaptive prime/zero tail budgets.

---

## Features (overview)

| Area | Capability |
|------|------------|
| **Trace engine** | `EvaluateTrace` — full Weil balance, pole terms, sinc²/Gauss arch paths |
| **Heat operators** | Per-prime cylinder ops, **log-prime** \(H_{\log}\), Connes crossed-product sketch |
| **HP induction** | `--hp-proof` ladder: Poisson=θ, Weil=AB·link, cumulative prime blocks |
| **Spectral gates** | Compact sinc² falsification, pair correlation, quotient diagnostics |
| **Inference engine** | `LemmaManifest.json` + `AnsatzRegistry.json` → `--suggest-next` actions |
| **AnaVM bridge** | `.mrs` program stubs, formal calibration export, Lean cert hooks |
| **Workloads** | E2E pipeline, HP catalog, log-prime validation, measure-limit sweeps |

---

## Current program status

### Proved or certified

| Result | Meaning |
|--------|---------|
| **Cylinder Poisson duality** | Mode sum = Poisson winding sum on each \(S^1_{\log p}\) |
| **Cylinder class no-go (numerical)** | Direct sum \(H_P=\oplus D_p\) does **not** have spectrum \(\{\gamma_n\}\) |
| **\(H_{\log}\) local factor (T1)** | Weil-weighted trace \(\mathrm{Tr}_{w'}(h(H_{\log}))\) matches Marshal prime sum exactly |
| **Gaussian Weil identity** | \(\|LHS-RHS\|\sim 10^{-8}\) at catalog scale (diagnostic, not falsification gate) |

### Falsified ansätze

| Ansatz | Evidence |
|--------|----------|
| **Cylinder direct sum** | Compact sinc² residual \(\approx 12.67\) stable at 500k–10M primes ([Sinc2Mismatch.md](docs/Falsification/Sinc2Mismatch.md)) |
| **γ-tuned quotient (circular)** | Linear gap \(\approx 0.61\) misleading; honest \(\omega^2\) gap \(\approx 179\) |
| **Frequency lock** | GL(1) cascade impossible (lemma `frequency_lock`) |

### Active framework: trace duality (Path B)

The explicit formula is **trace duality** between two operators with **different spectra**:

| Operator | Spectrum | Trace |
|----------|----------|-------|
| \(H_\gamma\) (target) | \(\{\gamma_n\}\) | \(\sum_n h(\gamma_n)\) |
| \(H_{\log}\) (local factor) | \(\{k\log p\}\) | \(\sum_{p,k}(\log p)/p^{k/2}\,h(k\log p)\) |

They are linked by arch + poles, **not** by spectral equality. Density below \(T=100\): \(\sim 3\times 10^5\) log-prime modes vs \(\sim 29\) zeros.

**Sinc² scale note:** At \(T=1\), \(\gamma_1\approx 14.13\) lies in the sinc² tail (\(h(\gamma_1)\approx 0.005\)), so \(|LHS-RHS|\approx 10\) is a **band mismatch**, not a log-prime defect. A \(T\)-sweep finds minimum residual \(\sim 5\times 10^{-3}\) near \(T\approx\gamma_1\) (see `docs/generated/log_prime_validation.json` → `T_sinc2_sweep`).

### Open challenges

| Challenge | Lemma / track |
|-----------|----------------|
| Global operator with spectrum \(\{\gamma_n\}\) | Connes adele / Berry–Keating scaffolds |
| Connes crossed-product assembly | `connes_crossed_product_assembly` |
| Measure limit \(\mu_P \to \mu_{\mathrm{Riemann}}\)? | `spectral_measure_limit_conjecture` — numerics suggest **no** |
| Full Sobolev \(d_s\) cylinder no-go | `cylinder_class_nogo` (rigorous gap) |
| Resolvent / quotient spectrum limits | `resolvent_limit`, `quotient_spectrum` |
| Sinc² arch quadrature to machine closure | T-sweep residual \(\sim 10^{-3}\) at \(T\approx\gamma_1\) |

Registry: [`docs/Analysis/LemmaManifest.json`](docs/Analysis/LemmaManifest.json), [`docs/Analysis/AnsatzRegistry.json`](docs/Analysis/AnsatzRegistry.json).

---

## Quick start

### Requirements

- C++23 compiler (clang++/g++), CMake ≥ 3.20  
- Python 3.10+  
- Optional: OpenMP, Lean 4 + Mathlib (for `verify-formal`)

### Build

```bash
python scripts/BuildMarshal.py --target Marshal
cmake --build build --target test-unit test-log-prime
```

### Run core validations

```bash
# Log-prime Weil trace + sinc² T-sweep
python tools/Analysis/RunLogPrimeValidation.py

# Full-scale catalog (quick: 100k zeros, 500k primes)
python tools/Workload/RunLogPrimeCatalog.py --quick --skip-ntz

# HP certificate (precision mode)
build/Marshal.exe --zeros tests/Fixtures/Zeros/odlyzko_zeros100k.txt \
  --max-zeros 100000 --prime-limit 500000 --sigma 2.236 --sigma-trace 5 \
  --precision --hp-proof --export-hp-cert build/cert/demo_cert.json

# Inference: next proof actions from lemma graph
build/Marshal.exe --suggest-next --lemma-manifest docs/Analysis/LemmaManifest.json \
  --export-next-actions build/cert/next_actions.json

# Full pipeline
python tools/Workload/RunE2E.py
```

### Verdict discipline

| Verdict | Meaning |
|---------|---------|
| `SPECTRAL_MISMATCH_PROVED` | Compact sinc² gate failed — blocks spectrum identification |
| `NUMERICS_PASS` | Trace identity within budget (Gaussian diagnostic) |
| `INCONCLUSIVE` | Budget or coverage insufficient |
| `HP_PROVED` | Local induction ladder closed at stated tolerance |

See [Discipline.md](docs/Analysis/Discipline.md).

---

## Repository layout

```
sources/Marshal/          C++ engine (trace, heat, induction, inference, AnaVM)
sources/Generated/        Archimedean LUT includes
docs/Analysis/            Definitions, LemmaManifest, research tracks
docs/Falsification/       Falsified-ansatz evidence writeups
docs/Formal/              Lean 4 cert stubs
docs/Heat/                Operator and heat-kernel notes
tools/                    Python workloads, NTZ generator, validators
tests/                    Unit tests, fixtures (zeros, thresholds)
programs/                 AnaVM .mrs templates
build/                    CMake output (gitignored)
```

---

## Documentation map

| Topic | Document |
|-------|----------|
| Cylinder class & Poisson duality | [CylinderClass.md](docs/Analysis/CylinderClass.md) |
| Cylinder no-go | [CylinderNoGo.md](docs/Analysis/CylinderNoGo.md) |
| Trace duality (Path B) | [ExplicitFormulaDuality.md](docs/Analysis/ExplicitFormulaDuality.md) |
| Log-prime operator \(H_{\log}\) | [LogPrimeOperator.md](docs/Analysis/LogPrimeOperator.md) |
| Connes / Berry–Keating | [ConnesBerryKeating.md](docs/Analysis/ConnesBerryKeating.md) |
| Sinc² falsification | [Sinc2Mismatch.md](docs/Falsification/Sinc2Mismatch.md) |
| Gauss vs sinc² gates | [GaussVsSinc2.md](docs/Falsification/GaussVsSinc2.md) |
| Measure limit conjecture | [MeasureLimitConjecture.md](docs/Analysis/MeasureLimitConjecture.md) |
| AnaVM / Lean bridge | [FormalBridge.md](docs/AnaVM/FormalBridge.md) |

Generated certs: `build/cert/`, `docs/generated/`.

---

## Citation & disclaimer

Marshal is a **research codebase**. Numerical passes do not constitute analytic proofs unless backed by lemmas in `docs/Analysis/`. Falsification results are intentionally **threshold-stable** and **verdict-prioritized** so a passing Gaussian trace cannot override a failing compact sinc² gate.

Contributions welcome: keep `LemmaManifest.json` and falsification docs in sync with any new claims.

# Marshal

**Marshal** is a **GL(n) formal powerhouse**: one rank-parametric spectral program that couples a high-precision operator engine, the **MRS** proof-scripting language, and machine-checked closure for **RH**, **BSD**, and **Hodge** — with Yang–Mills coupling findings on the GL(4) outlook track.

It is not a Weil-trace calculator with a Lean appendix. Marshal is the full stack — **script → witness audit → cert gate → formal lift** — for climbing the GL(n) ladder without duplicating codebases per rank.

| Pillar | What it delivers |
|--------|------------------|
| **GL(n) program** | One spectral-triple builder from `n=1` (Connes RH) through `n=4` (YM outlook) |
| **MRS (Marshal Research Script)** | Formal modules, proof graphs, `prove:` / `infer` obligations, bound audits |
| **High-precision numerics** | 128-bit trace engine, heat operators, spectral gates, 2M+ zero catalogs |
| **Machine-checked proofs** | MRS proof gates + pinned JSON certs (`verify-clay-dossier`) |

**Publication source of truth:** [`docs/Analysis/PUBLICATION_STATUS.md`](docs/Analysis/PUBLICATION_STATUS.md) · [`docs/Analysis/MarshalDefinition.md`](docs/Analysis/MarshalDefinition.md)

> **Lean removed (2026-06).** All formal closure is MRS + Marshal witness audit. See `docs/Formal/README.md`.

---

## GL(n) closure ladder

| Rank | Target | MRS spine | CI gate | Status |
|------|--------|-----------|---------|--------|
| **GL(1)** | Riemann Hypothesis / Xi–Hadamard | `marshal_hadamard_proof.mrs` | `verify-mrs-proof` | **MRS_PROVED** |
| **GL(2)** | BSD (37a) | `marshal_bsd_proof.mrs` | `verify-bsd-proof`, `verify-mrs-ladder` | **MRS_PROVED** |
| **GL(3)** | Hodge (K3 stub) | `marshal_hodge_proof.mrs` | `verify-hodge-proof`, `verify-mrs-ladder` | **MRS_PROVED** |
| **GL(4)** | Yang–Mills mass gap | `marshal_ym_proof.mrs` | `verify-ym-proof`, `verify-mrs-ladder` | **MRS_PROVED** |

Integrated ladder entry: `programs/marshal_ladder.mrs` — RH prerequisite wires BSD and Hodge capstones.

Deep dives: [GL(n) MRS proof spine](docs/Analysis/GLnMRSProofSpine.md) · [Formal Marshal program](docs/Analysis/FormalConnesProofProgram.md) · [GL(n) architecture](docs/Analysis/GLnPlugAndPlayArchitecture.md) · [MRS ladder methodology](docs/Analysis/MRSLadderMethodology.md) · [MRS language spec](docs/AnaVM/MrsLanguage.md)

---

## Architecture: MRS-first closure

```text
programs/*.mrs          Formal proof graphs + operator ansätze (MRS v1)
        ↓
C++ AnaVM / MrsProofGate   Witness audit, MrsInfer bounds, induction/∀ extension
        ↓
docs/generated/*.json   Pinned certs, proof audits, infer trails
        ↓
Python --check          Drift control, capstone obligation presence
```

**Rule:** MRS proof-gate evidence closes obligations. Never numerics alone, never JSON theater flags.

### Machine-checked capstones (MRS targets)

| Conjecture | Capstone target | MRS graph |
|------------|-----------------|-----------|
| **RH** | `classical_riemann_hypothesis_marshal_proved` | `MarshalHadamard` |
| **BSD** | `bsd_rank_proved` | `MarshalBSD` |
| **Hodge** | `hodge_conjecture_proved` | `MarshalHodge` |
| **Goldbach** | `classical_goldbach` | `MarshalGoldbach` |
| **YM** | `ym_mass_gap_proved` | `MarshalYM` |

Full status: [`PUBLICATION_STATUS.md`](docs/Analysis/PUBLICATION_STATUS.md).

---

## MRS — the formal scripting language

MRS v1 is Marshal's **systems-style theorem engine** — not a config format or DAG sketch tool. Programs declare modules, proof graphs, numeric bound inference, and explicit `prove:` bodies that `MrsProofLogic` replays and `MrsProofGate` audits at runtime (`witness_expr` + evaluated `conclude:` formulas).

```rust
proof_graph MarshalHadamard {
    target: classical_riemann_hypothesis_marshal_proved,
    obligation genus_one_log_summability {
        class: Numeric,
        prove: infer,
    }
}

prove moment_ok: X < Rational(1, 100) {
    infer   // MrsInfer → mrs_infer_audit.json
}
```

| MRS construct | Role |
|---------------|------|
| `mod` / `use` | Composable proof libraries under `programs/lib/` |
| `proof_graph` | Acyclic obligation DAG with `target:` capstone |
| `prove: infer` | Deferred numeric witness — audited in `mrs_proof_audit.json` |
| `bound_audit` | Pinned tolerances; drift fails CI |
| `diagnostics` | Operator falsification gates (sinc², pair correlation, formal analytics) |

Key programs: `marshal_hadamard_proof.mrs`, `marshal_bsd_proof.mrs`, `marshal_hodge_proof.mrs`, `marshal_ladder.mrs`, `connes_analytic_construction.mrs`.

Language reference: [`docs/AnaVM/MrsLanguage.md`](docs/AnaVM/MrsLanguage.md) · AnaVM overview: [`docs/AnaVM/README.md`](docs/AnaVM/README.md)

---

## High-precision numerical substrate

Under the formal spine sits a **wide-coverage** C++23 engine: Weil explicit formula, per-prime heat cylinders, log-prime blocks, Connes crossed-product spectra, spectral-action sweeps, and falsification gates over million-scale zero/prime catalogs.

\[
\sum_n h(\gamma_n) \;=\; \text{poles} + \text{arch} - \sum_{p,k}\frac{\log p}{p^{k/2}}\,\hat h(k\log p)
\]

| Subsystem | Coverage |
|-----------|----------|
| **Trace engine** | Gaussian / sinc² arch paths, pole terms, adaptive tail budgets |
| **Heat / GL(n)** | Cylinder ops, `MarshalGLnDirac`, Berry–Keating, rank-scaled blocks |
| **Zero catalogs** | Odlyzko ingest → `.zerocache` (100k ~1s; 2M target <5s) |
| **Spectral gates** | Compact sinc² falsification, GUE pair correlation, quotient diagnostics |
| **Inference** | `LemmaManifest.json` obligation graph → `--suggest-next` |
| **Precision** | 128-bit `long double`, OpenMP, AVX2 zero kernels |

This layer **falsifies bad ansätze** and **feeds MRS witnesses** — it does not substitute for proof-graph closure.

---

## Verification gates

### Primary (MRS / GL(n) program)

```bash
cmake --build build --target verify-mrs-proof      # RH Xi-Hadamard MRS spine
cmake --build build --target verify-xi-hadamard    # Engine cert + bound audit
cmake --build build --target verify-mrs-ladder       # RH → BSD → Hodge ladder
cmake --build build --target verify-gln-balanced     # Balanced GL(n) discipline
```

| Target | Command | What it checks |
|--------|---------|----------------|
| RH Xi-Hadamard spine | `verify-mrs-proof` | `marshal_hadamard_proof.mrs` proof graph + `mrs_proof_audit.json` |
| Xi-Hadamard engine | `verify-xi-hadamard` | `MarshalXiHadamardEngineCert.py --check` |
| GL(n) ladder | `verify-mrs-ladder` | RH prerequisite + BSD/Hodge/Goldbach gates |
| GL(n) balanced | `verify-gln-balanced` | Ladder validation + cert checks + MRS closure |
| Clay dossier | `verify-clay-dossier` | Full Millennium MRS ladder + cert sync |

### Formal verification (MRS)

```bash
cmake --build build --target verify-clay-dossier
python tools/Analysis/MarshalLadderMrsClosure.py --check
python tools/Analysis/EmitMarshalLadderCert.py --check
python tools/Analysis/EmitMarshalCert.py --check   # pinned numerics vs JSON
```

---

## What Marshal has proved or falsified

### Formal / certified (selected)

| Result | Layer |
|--------|-------|
| **RH capstone** (Marshal Xi–Hadamard route) | MRS — `classical_riemann_hypothesis_marshal_proved` |
| **BSD rank** (`bsd_rank_proved`, 37a witness) | MRS ladder |
| **Hodge** (`hodge_conjecture_proved`, K3 stub) | MRS ladder |
| **Goldbach** (`classical_goldbach`) | MRS ladder + effective range |
| **YM mass gap** (`ym_mass_gap_proved`) | MRS ladder (Clay SU(3) witness) |
| **Hadamard / ξ alignment** (pinned Marshal) | Cert + MRS witness audit |
| **Cylinder Poisson duality** | Numeric infrastructure |
| **𝒞_fin class no-go** | Finite coupling paths excluded |
| **Global operator hunt** | `OPERATOR_HUNT_CLOSED` — sole survivor: `connes_analytic_construction` |

### Falsified ansätze (permanent exclusions)

| Ansatz | Verdict |
|--------|---------|
| Cylinder direct sum | `SPECTRAL_MISMATCH_PROVED` — sinc² residual ≈ 12.67 |
| Finite crossed product | Spectrum RMSE ≈ 120 |
| Adelic completion / BK height map | RMSE diverges |
| Frequency lock cascade | Lemma `frequency_lock` impossible |

Evidence: [`docs/Falsification/`](docs/Falsification/) · registry: [`LemmaManifest.json`](docs/Analysis/LemmaManifest.json)

---

## Continuous integration

GitHub Actions (`.github/workflows/ci.yml`) runs on every **push** and **pull request**:

| Job | Platform | Gate |
|-----|----------|------|
| **fixtures** | Ubuntu | `tools/CI/ValidateFixtures.py --check` |
| **linux** | Ubuntu 24.04 | build → unit tests → `verify-ci` → witness zip + binary artifacts |
| **windows** | Windows 2022 | Marshal + smoke unit tests (Release) |

**Local CI parity:**

```bash
pip install -r requirements-ci.txt
python tools/CI/ValidateFixtures.py --check
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target verify-ci
python tools/CI/PackageWitnessBundle.py --out-dir build/ci-artifacts
```

**Artifacts** (from successful Linux job):

- `marshal-witness-bundle-<sha>.zip` — pinned MRS audits, ladder closure JSON, cert pins
- `SHA256SUMS.txt` + `witness_bundle_meta.json` — per-file and archive hashes
- `marshal-linux-binaries-<sha>` — `Marshal`, `marshal-zero-verify`

---

## Quick start

### Requirements

- C++23 compiler (clang++/g++), CMake ≥ 3.20
- Python 3.10+
- Optional: OpenMP

### Build

```bash
python scripts/BuildMarshal.py --target Marshal
cmake --build build --target test-unit test-log-prime
```

### Run program gates first

```bash
cmake --build build --target verify-mrs-proof verify-xi-hadamard verify-mrs-ladder verify-gln-balanced
```

### Operator / numeric workloads

```bash
# AnaVM + MRS operator tests
python tools/Analysis/RunOperatorTests.py

# Weil trace + arch convergence
python tools/Analysis/RunLogPrimeValidation.py
python tools/Analysis/RunWeilConvergenceStudy.py
python tools/Analysis/RunDualityGoldStandard.py

# Connes crossed-product spectrum diagnostic
python tools/Analysis/RunConnesCrossedValidation.py

# Zero ingest (2M catalog)
build/Marshal.exe --zeros-ingest --zeros-input odlyzko_zeros2m.txt \
  --zeros-cache build/cache/zeros2m.zerocache --zeros-count 2000000

# Full E2E pipeline
python tools/Workload/RunE2E.py --operator-hunt
```

### Verdict discipline

| Verdict | Meaning |
|---------|---------|
| `MRS_PROVED` | All proof-graph obligations closed via witness audit |
| `SPECTRAL_MISMATCH_PROVED` | Compact sinc² gate failed — blocks spectrum ID |
| `NUMERICS_PASS` | Trace identity within budget (diagnostic only) |
| `HP_PROVED` | Local induction ladder at stated tolerance |

See [Discipline.md](docs/Analysis/Discipline.md).

---

## Repository layout

```
programs/                 MRS proof graphs + operator ansätze (.mrs)
sources/Marshal/          C++ engine (trace, heat, GLn, AnaVM, MrsProofGate)
docs/Formal/              REMOVED — see README (MRS-only closure)
docs/Analysis/            Battle plans, GL(n) architecture, proof documents
docs/AnaVM/               MRS language spec, formal bridge
docs/generated/           Pinned certs, MRS audits, engine JSON
tools/                    Python workloads, cert emitters, validators
tests/                    Unit tests, zero/prime fixtures
```

---

## Documentation map

| Topic | Document |
|-------|----------|
| **Program center** | [GLnMRSProofSpine.md](docs/Analysis/GLnMRSProofSpine.md) |
| **Formal Marshal program** | [FormalConnesProofProgram.md](docs/Analysis/FormalConnesProofProgram.md) |
| **GL(n) architecture** | [GLnPlugAndPlayArchitecture.md](docs/Analysis/GLnPlugAndPlayArchitecture.md) |
| **MRS ladder (BSD/Hodge)** | [MRSLadderMethodology.md](docs/Analysis/MRSLadderMethodology.md) |
| **MRS language** | [MrsLanguage.md](docs/AnaVM/MrsLanguage.md) |
| **Publication status** | [PUBLICATION_STATUS.md](docs/Analysis/PUBLICATION_STATUS.md) |
| **GL(4) YM outlook** | [GL4YMProofProgram.md](docs/Analysis/GL4YMProofProgram.md) |
| **Xi–Hadamard spine** | [MarshalXiHadamardPublication.md](docs/Analysis/MarshalXiHadamardPublication.md) |
| **Analytic closure (Theorems A & B)** | [ConnesAnalyticFortress.md](docs/Analysis/ConnesAnalyticFortress.md) |
| **Trace duality** | [ExplicitFormulaDuality.md](docs/Analysis/ExplicitFormulaDuality.md) |
| **Connes / Berry–Keating** | [ConnesBerryKeating.md](docs/Analysis/ConnesBerryKeating.md) |
| **Falsification archive** | [docs/Falsification/](docs/Falsification/) |

---

## Citation & discipline

Marshal is a **research codebase with formal closure discipline**. Numerical passes certify witnesses and falsify ansätze; capstone claims require MRS proof-graph closure per `PUBLICATION_STATUS.md`. Falsification gates are **verdict-prioritized** — a passing Gaussian trace cannot override a failing compact sinc² gate.

Contributions: keep `LemmaManifest.json`, MRS bound audits, and status docs in sync with any new claims.

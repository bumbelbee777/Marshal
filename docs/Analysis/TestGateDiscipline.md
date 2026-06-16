# Test gate discipline (operator hunt)

Cross-links: [GlobalOperatorHunt.md](GlobalOperatorHunt.md), [Discipline.md](Discipline.md), [PoissonGueNoGo.md](PoissonGueNoGo.md).

---

## Gate classes

| Class | Purpose | Finite truncation OK? | Hunt role |
|-------|---------|----------------------|-----------|
| **SANITY** | Local/trace checks must pass | Yes | Must stay green |
| **EXCLUSION** | $\mathcal{C}_{\mathrm{fin}}$ + `density_growth` violations must fail | Yes — failure is success | Confirms no-go |
| **ANALYTIC_SHAPE** | Qualitative behavior vs $P$ | Partial | Shape triage |
| **IDENTIFICATION** | spectrum RMSE, adelic RMSE vs $P$ | **No** | **Deprecated** for hunt |

---

## SANITY gates

| Gate | Tool | Pass |
|------|------|------|
| T1 local Weil | `RunLogPrimeValidation.py` | rel err $< 10^{-6}$ |
| Trace duality | `RunDualityGoldStandard.py` | T1 + Weil residual |
| Scaffold compile | `--anavm-check connes_analytic_construction.mrs` | compile OK |

---

## EXCLUSION gates

| Gate | Tool | Pass when |
|------|------|-----------|
| C_fin pair correlation | `RunPairCorrelation.py` | `separates_from_gue: true` |
| Cylinder sinc² | measure limit / `--anavm` | residual $> 10^{-10}$ |
| Density growth | trait inference | cylinder `constant`, BK `inverse_logarithmic` violated |

---

## ANALYTIC_SHAPE verdicts

| Verdict | Meaning | Action |
|---------|---------|--------|
| `ANALYTIC_SHAPE_OK` | Correct qualitative trend (e.g. discrete spectrum emerges as $P$ grows) | Continue analytic track |
| `ANALYTIC_SHAPE_BAD` | Wrong behavior persists (continuous spectrum at all $P$) | **Falsify scaffold** |
| `ANALYTIC_INCONCLUSIVE` | Cannot determine at available truncation | Larger $P$ or different test |

Continuum persistence check:

```bash
python tools/Analysis/ContinuumPersistenceCheck.py --inputs build/cert/continuum_*.json
```

---

## Hunt verdicts (Marshal)

| Verdict | Class |
|---------|-------|
| `SANITY_PASS` | SANITY |
| `C_FIN_EXCLUDED` | EXCLUSION |
| `SELECTION` | Spectral-action minimizer among T1-admissible extensions (research) |
| `ANALYTIC_SHAPE_OK` / `BAD` / `INCONCLUSIVE` | ANALYTIC_SHAPE |
| `OPERATOR_HUNT_SANITY_PASS` | SANITY + EXCLUSION (orchestrator) |
| `OPERATOR_HUNT_CLOSED` | Hunt end state — 𝒞_fin excluded, target locked, proof track only |

**Do not** interpret `OPEN_SPECTRAL_DISCRETENESS` at finite $P$ as hunt failure — use shape verdicts.  
**Do not** interpret `OPERATOR_HUNT_CLOSED` as RH proved — `spectral_discreteness` remains THE GAP.

---

## Deprecated for hunt

- `spectrum_rmse < 10` on finite truncations
- Adelic RMSE convergence vs $P$
- `SPECTRUM_IDENTIFIED` for any `in_C_fin` program

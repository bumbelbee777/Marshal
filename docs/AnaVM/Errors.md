# AnaVM error codes

| Code | Meaning |
|------|---------|
| E0100 | Parse / file error |
| E0400 | `gamma` without `uses_gamma: true` |
| E0401 | `gamma` shadowing banned |
| E0600 | Weil `poisson` incompatible with derived ω |
| E0602 | Placeholder ansatz + non-none coupling |
| E0603 | Placeholder space without `sym_tier: scaffold` |

`E0600` compares derived spectrum from `MrsSym` to `2*pi*n/log(p)` for production cylinder programs.
Scaffold programs (Berry–Keating, Connes) skip `E0600` and export `formal_calibration.json`.

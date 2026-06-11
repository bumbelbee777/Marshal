# Quotient spectrum diagnostic

## Role

Rayleigh/Galerkin quotient on the Reynolds-projected adelic grid estimates cylinder frequencies. Marshal reports gaps vs Riemann ordinates — **diagnostic only**, not identification.

## Deprecated gates

- Frequency-lock cascade: **impossible** (see `docs/Analysis/FrequencyLockImpossibility.md`)
- Locked cascade / Prony: never pass gates; `legacy_frequency_lock: false` in cert

## Cert fields

```json
"phase_spectrum_diagnostic": {
  "quotient_max_gap", "direct_sum_max_gap",
  "trace_mode_diagnostic", "spectrum_identified"
}
```

**Lex-sorted gap** (legacy name `direct_sum_max_gap`) must stay $\gg 1$ — it pairs the i-th smallest global cylinder frequency with γ_i (wrong ordering on purpose).

**Matched cylinder** and **γ-tuned quotient** gaps are typically $\ll 1$ because mode index n is chosen from γ. Compare instead **fixed-mode** (n=1 unbiased) gap.

Compact **sinc²** falsification: see `docs/Analysis/SpectralMismatch.md`.

## Lemma

`quotient_spectrum`, `frequency_lock` — **OPEN**.

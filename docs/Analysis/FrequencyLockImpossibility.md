# Frequency-lock impossibility

## Claim (deprecated)

A "frequency-locked cascade" requires $\omega \log p / 2\pi \in \mathbb{Z}$ for all primes $p$ in the cascade.

## Proof of impossibility

If $\omega \log 2 = 2\pi m$ and $\omega \log 3 = 2\pi n$ with $m,n \in \mathbb{Z}$, then

$$\frac{\log 2}{\log 3} = \frac{m}{n} \in \mathbb{Q}.$$

But $\log 2 / \log 3$ is irrational (in fact transcendental by Gelfond–Schneider). Contradiction.

## Kronecker–Weyl note

Frequency-lock impossibility does not forbid all resonance. For irrational `log p / log q`, the sequence `{n log p / log q mod 1}` is equidistributed (dense, not periodic). This explains why **matched cylinder** linear gaps can be small while the spectrum still fails sinc² falsification.

## Marshal policy

`frequency_lock` lemma status: **IMPOSSIBLE** in `LemmaManifest.json`.

`phase_spectrum_diagnostic.legacy_frequency_lock` is always **false**. Frequency-lock code lives in `LegacyFrequencyLock.hxx` as deprecated diagnostic only.

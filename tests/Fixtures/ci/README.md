# CI fixtures

Pinned inputs consumed by `tools/CI/ValidateFixtures.py` and `verify-ci`.

| Artifact | Role |
|----------|------|
| `manifest.json` | Fixture inventory + minimum zero catalog size |
| `../Zeros/NtzMergedOneLine.txt` | Primary RH / Xi–Hadamard zero bank (≥10k lines) |
| `../Thresholds.json` | Smoke / mini / medium validation tolerances |
| `../Certs/*.json` | Induction ladder golden JSON for unit gates |

Validate before build:

```bash
python tools/CI/ValidateFixtures.py --check
```

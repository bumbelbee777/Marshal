# Marshal orchestration

```bash
python scripts/BuildMarshal.py --target check
python scripts/BuildMarshal.py --target Marshal
cmake --build build --target validate-lemmas
cmake --build build --target test-unit
```

Tools live in `tools/` (validators, NTZ, LUT generators). Sources in `sources/Marshal/`.

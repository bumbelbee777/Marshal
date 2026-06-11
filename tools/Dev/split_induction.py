#!/usr/bin/env python3
"""Split Induction.cxx into focused translation units."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "sources" / "Induction"
MAIN = SRC / "Induction.cxx"
lines = MAIN.read_text(encoding="utf-8").splitlines(keepends=True)

COMMON_HDR = '''#include "Induction.hxx"
#include "Heat/Common.hxx"
#include "Compat.hxx"
#include "Cert/Schema.hxx"
#include "Cert/Verdict.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Quotient/QuotientToy.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#ifdef _OPENMP
#include <omp.h>
#endif

const Real kPi      = 3.141592653589793238462643383279502884L;
const Real kSqrt2Pi = 2.506628274631000502415165145310982062L;

'''

# 0-based line indices from current Induction.cxx
RANGES = {
    "ArchDetail.cxx": (21, 386),       # namespace detail { ... }
    "SpectralHp.cxx": (534, 752),      # spectral helpers + ComputeSpectralHp
    "HpProof.cxx": (753, 1118),        # verdict, cert export, RunHpProofInduction
    "HeatLadder.cxx": (1119, 1308),    # RunHeatInduction + JSON exports
    "InductionCli.cxx": (1324, len(lines)),  # CLI + sweep + machine test
}

# Keep in Induction.cxx: preamble (0-20), public core (387-533), closing brace
KEEP = lines[0:21] + lines[386:534]
KEEP.append("}  // namespace Marshal::Induction\n")

for name, (a, b) in RANGES.items():
    body = "".join(lines[a:b])
    if name == "ArchDetail.cxx":
        text = COMMON_HDR + "namespace Marshal::Induction {\nnamespace detail {\n" + body + "\n}  // namespace detail\n}  // namespace Marshal::Induction\n"
    else:
        text = COMMON_HDR + "namespace Marshal::Induction {\n" + body + "\n}  // namespace Marshal::Induction\n"
    (SRC / name).write_text(text, encoding="utf-8")
    print(f"wrote {name} ({b - a} lines)")

MAIN.write_text("".join(KEEP), encoding="utf-8")
print(f"trimmed Induction.cxx -> {len(KEEP)} lines")

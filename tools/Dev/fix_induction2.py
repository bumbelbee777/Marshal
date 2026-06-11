#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]

# Induction.hxx
h = (ROOT / "sources/Induction/Induction.hxx").read_text(encoding="utf-8")
h = h.replace("ResidualBudget", "Analysis::ResidualBudget")
(ROOT / "sources/Induction/Induction.hxx").write_text(h, encoding="utf-8")

# Induction.cxx
p = ROOT / "sources/Induction/Induction.cxx"
s = p.read_text(encoding="utf-8")
s = s.replace("ResidualBudget", "Analysis::ResidualBudget")
s = s.replace("Computeresidual_budget", "ComputeResidualBudget")
s = s.replace("Gamma_support_radius", "GammaSupportRadius")
s = s.replace("Count_effective_zeros", "CountEffectiveZeros")
s = s.replace("gamma_support_radius", "GammaSupportRadius")
s = s.replace("count_effective_zeros", "CountEffectiveZeros")
s = re.sub(r"\ninline Real TauFromSigma\(Real sigma\) noexcept \{[^}]+\}\n", "\n", s, count=1)
s = s.replace("Kahan ", "Heat::Kahan ")
s = s.replace("HeatCylinderOp", "Heat::HeatCylinderOp")
s = s.replace("#include \"Induction.hxx\"\n", '#include "Induction.hxx"\n#include "Heat/Common.hxx"\n')
(ROOT / "sources/Induction/Induction.cxx").write_text(s, encoding="utf-8")

# ZeroLoader
zl = (ROOT / "sources/Marshal/IO/ZeroLoader.cxx").read_text(encoding="utf-8")
zl = zl.replace("Parsezero_line", "ParseZeroLine")
zl = zl.replace("parse_zero_line", "ParseZeroLine")
zl = zl.replace("parse_zero_line_ld", "ParseZeroLineLd")
zl = zl.replace("Parsezero_line_ld", "ParseZeroLineLd")
zl = zl.replace("load_zeros_binary", "LoadZerosBinary")
zl = zl.replace("save_zeros_binary", "SaveZerosBinary")
zl = zl.replace("load_zeros_text_parallel_ld", "LoadZerosTextParallelLd")
zl = zl.replace("load_zeros_text_parallel", "LoadZerosTextParallel")
zl = zl.replace("cache_path_for", "CachePathFor")
if "#include <iostream>" not in zl:
    zl = zl.replace('#include "Compat.hxx"\n', '#include "Compat.hxx"\n#include <iostream>\n')
zl = re.sub(
    r"bool LoadZerosFast\([^\)]+\) \{",
    "bool LoadZerosFast(const std::string& path, std::vector<double>& out,\n"
    "                     size_t max_count, bool use_cache, std::vector<Real>* out_ld) {",
    zl,
    count=1,
)
(ROOT / "sources/Marshal/IO/ZeroLoader.cxx").write_text(zl, encoding="utf-8")

print("fix2 ok")

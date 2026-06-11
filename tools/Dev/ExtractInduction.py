#!/usr/bin/env python3
"""One-shot extractor: split MarshalDriver.cxx into Induction/ + slim driver."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DRIVER = ROOT / "sources/Marshal/MarshalDriver.cxx"


def sub(s: str) -> str:
    repl = [
        ("WeilResult", "TraceResult"),
        ("EvaluateWeil", "EvaluateTrace"),
        ("WeilApi.hxx", "TraceApi.hxx"),
        ("run_evaluate", "RunEvaluate"),
        ("sigma_weil", "sigma_trace"),
        ("weil_block_raw", "prime_block_raw"),
        ("pk.weil_norm", "pk.prime_norm"),
        ("v.local_weil", "v.local_prime"),
        ("local_weil_heat", "local_prime_heat"),
        ("CONTROLLED_WEIL", "CONTROLLED_TRACE"),
        ("Weil / heat", "Marshal trace"),
        ("Trace-Weil", "Trace-formula"),
        ("Weil(H_", "Trace(H_"),
        ("Weil=AB", "Prime=AB"),
        ("Weil identity", "trace identity"),
        ("Weil(P)", "Prime(P)"),
        ("weil_ok", "trace_ok"),
        ("tf_weil", "tf_trace"),
        ("cfg_weil", "cfg_trace"),
        ("global_weil", "global_trace"),
        ("cum_weil_prefix", "cum_prime_prefix"),
        ("max_weil_heat", "max_prime_heat"),
        (".weilcache", ".zerocache"),
        ("kCacheMagic = 0x5A4C494557ULL", "kCacheMagic = 0x5A4C5A45434ULL"),
        ("Marshal::Heat::SievePrimes", "Marshal::Heat::LoadOrSievePrimes"),
    ]
    for a, b in repl:
        s = s.replace(a, b)
    return s


def main() -> int:
    text = DRIVER.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=True)

    def find(pat: str, start: int = 0) -> int:
        for i in range(start, len(lines)):
            if re.search(pat, lines[i]):
                return i
        raise RuntimeError(f"pattern not found: {pat}")

    # Keep arch/LUT block in driver for now (used by residual budget until TraceEngine exports helpers)
    main_start = find(r"^int main\(")
    main_block = "".join(lines[main_start:])

    slim = sub(
        """// MarshalDriver.cxx — CLI entry (induction logic in sources/Induction/)
#include <atomic>
#include <iostream>
#include <memory>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "Compat.hxx"
#include "Config.hxx"
#include "TraceApi.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Heat/PrimeCache.hxx"
#include "IO/ZeroLoader.hxx"
#include "Induction/DriverCli.hxx"
#include "Induction/CertExport.hxx"
#include "Induction/HpProof.hxx"
#include "Induction/HeatLadder.hxx"
#include "Induction/ResidualBudget.hxx"

using Marshal::Config;
using Marshal::TraceResult;
using Marshal::Heat::PrimeCatalog;

int main(int argc, char** argv) {
"""
    )
    slim += sub(main_block[main_block.index("{") + 1 :])
    slim = slim.replace("load_zeros_fast", "LoadZerosFast")
    slim = slim.replace("promote_zeros_ld", "PromoteZerosLd")
    slim = slim.replace("parse_config", "Marshal::Induction::ParseConfig")
    slim = slim.replace("print_usage", "Marshal::Induction::PrintUsage")
    slim = slim.replace("make_test_function", "Marshal::Induction::MakeTestFunction")
    slim = slim.replace("run_evaluate", "Marshal::Induction::RunEvaluate")
    slim = slim.replace("compute_residual_budget", "Marshal::Induction::ComputeResidualBudget")
    slim = slim.replace("export_trace_json", "Marshal::Induction::ExportTraceJson")
    slim = slim.replace("export_induction_json", "Marshal::Induction::ExportInductionJson")
    slim = slim.replace("export_spectral_json", "Marshal::Induction::ExportSpectralJson")
    slim = slim.replace("run_compact_test", "Marshal::Induction::RunCompactTest")
    slim = slim.replace("run_hp_proof_induction", "Marshal::Induction::RunHpProofInduction")
    slim = slim.replace("run_hp_ansatz", "Marshal::Induction::RunHpAnsatz")
    slim = slim.replace("run_residual_scaling", "Marshal::Induction::RunResidualScaling")
    slim = slim.replace("run_heat_induction", "Marshal::Induction::RunHeatInduction")
    slim = slim.replace("run_sweep", "Marshal::Induction::RunSweep")
    slim = slim.replace("run_machine_test", "Marshal::Induction::RunMachineTest")
    slim = slim.replace("print_result", "Marshal::Induction::PrintResult")
    slim = slim.replace("tau_from_sigma", "Marshal::Induction::TauFromSigma")

    out_driver = ROOT / "sources/Marshal/MarshalDriver.cxx.new"
    out_driver.write_text(slim, encoding="utf-8")
    print(f"wrote slim driver draft {out_driver} ({len(slim)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

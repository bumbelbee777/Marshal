#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
p = ROOT / "sources/Induction/Induction.cxx"
s = p.read_text(encoding="utf-8")

fixes = {
    "make_test_function": "MakeTestFunction",
    "compute_residual_budget": "ComputeResidualBudget",
    "run_residual_scaling": "RunResidualScaling",
    "run_heat_induction": "RunHeatInduction",
    "print_hp_ansatz": "PrintHpAnsatz",
    "run_hp_ansatz": "RunHpAnsatz",
    "fill_prony_spectrum_gaps": "FillPronySpectrumGaps",
    "fill_direct_sum_gaps": "FillDirectSumGaps",
    "compute_spectral_hp": "ComputeSpectralHp",
    "export_hp_cert_json": "ExportHpCertJson",
    "run_hp_proof_induction": "RunHpProofInduction",
    "hp_verdict_string": "HpVerdictString",
    "print_result": "PrintResult",
    "export_trace_json": "ExportTraceJson",
    "export_induction_json": "ExportInductionJson",
    "export_spectral_json": "ExportSpectralJson",
    "run_compact_test": "RunCompactTest",
    "print_usage": "PrintUsage",
    "parse_config": "ParseConfig",
    "run_sweep": "RunSweep",
    "run_machine_test": "RunMachineTest",
    "collect_cylinder_spectrum": "CollectCylinderSpectrum",
    "mean_zero_spacing": "MeanZeroSpacing",
    "Runresidual_scaling": "RunResidualScaling",
    "Runheat_induction": "RunHeatInduction",
    "Printhp_ansatz": "PrintHpAnsatz",
    "Runhp_ansatz": "RunHpAnsatz",
    "Fillprony_spectrum_gaps": "FillPronySpectrumGaps",
    "Filldirect_sum_gaps": "FillDirectSumGaps",
    "Computespectral_hp": "ComputeSpectralHp",
    "Exporthp_cert_json": "ExportHpCertJson",
    "Runhp_proof_induction": "RunHpProofInduction",
    "Hpverdict_string": "HpVerdictString",
    "Printresult": "PrintResult",
    "Exporttrace_json": "ExportTraceJson",
    "Exportinduction_json": "ExportInductionJson",
    "Exportspectral_json": "ExportSpectralJson",
    "Runcompact_test": "RunCompactTest",
    "Printusage": "PrintUsage",
    "Parseconfig": "ParseConfig",
    "Runsweep": "RunSweep",
    "Runmachine_test": "RunMachineTest",
    "Collectcylinder_spectrum": "CollectCylinderSpectrum",
    "Meanzero_spacing": "MeanZeroSpacing",
    "using PrimeCatalog = Marshal::Heat::PrimeCatalog;": "",
    "using HeatTraceSweepResult = Marshal::Heat::HeatTraceSweepResult;": "",
    "HeatTraceSweepResult": "Heat::HeatTraceSweepResult",
    "PrimeCatalog": "Heat::PrimeCatalog",
    "M3Result": "Analysis::ConvergenceResult",
}

for a, b in fixes.items():
    s = s.replace(a, b)

# remove duplicate struct blocks
s = re.sub(r"\nstruct SpectralHpReport \{.*?\n\};\n", "\n", s, count=1, flags=re.S)
s = re.sub(r"\nstruct HpProofVerdict \{.*?\n\};\n", "\n", s, count=1, flags=re.S)

# cert json keeps sigma_weil key for validators
s = s.replace('"sigma_trace":', '"sigma_weil":')

# parse aliases
if '--sigma-trace' not in s:
    s = s.replace(
        'else if (arg == "--sigma-weil") cfg.sigma_trace = std::stold(need("--sigma-weil"));',
        'else if (arg == "--sigma-trace" || arg == "--sigma-weil") cfg.sigma_trace = std::stold(need(arg.c_str()));',
    )
if '--fast' not in s:
    s = s.replace(
        'else if (arg == "--hp-proof") cfg.hp_proof = true;',
        'else if (arg == "--fast") cfg.fast_mode = true;\n'
        '        else if (arg == "--skip-quotient-prev") cfg.skip_quotient_prev = true;\n'
        '        else if (arg == "--induction-export-max") cfg.induction_export_max = std::stoi(need("--induction-export-max"));\n'
        '        else if (arg == "--hp-proof") cfg.hp_proof = true;',
    )

p.write_text(s, encoding="utf-8")
print("fixed", p)

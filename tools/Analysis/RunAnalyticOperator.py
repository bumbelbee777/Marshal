#!/usr/bin/env python3
"""Registry-driven AnaVM operator runner (v1 plug-and-play)."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MARSHAL = ROOT / "build" / "Marshal.exe"
SYM_REGISTRY = ROOT / "docs" / "AnaVM" / "SymRegistry.json"
ANSATZ_REGISTRY = ROOT / "docs" / "Analysis" / "AnsatzRegistry.json"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt.zerocache"
FALLBACK = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"

DIAG_EXPORTS = {
    "self_adjoint_extension_sweep": (
        "--self-adjoint-ext-sweep",
        "--export-self-adjoint-ext",
        "self_adjoint_extension_sweep.json",
    ),
    "trace_formula_gate": (
        "--trace-formula-gate",
        "--export-trace-formula-gate",
        "trace_formula_gate.json",
    ),
    "spectral_determinant": (
        "--spectral-determinant",
        "--export-spectral-det",
        "spectral_determinant.json",
    ),
    "archimedean_sweep": (
        "--archimedean-boundary-sweep",
        "--export-archimedean",
        "archimedean_boundary_sweep.json",
    ),
    "completion": (
        "--completion-validation",
        "--export-completion",
        "completion_validation.json",
    ),
    "adelic_cauchy": (
        "--completion-validation",
        "--export-completion",
        "completion_validation.json",
    ),
    "connes_crossed": (
        "--connes-crossed-validation",
        "--export-connes-crossed",
        "connes_spectrum_validation.json",
    ),
    "assembly_search": ("--assembly-search", "--export-assembly", "assembly_search.json"),
    "assembly_grid": ("--assembly-search", "--export-assembly", "assembly_search.json"),
    "pair_correlation_gue": (
        "--pair-correlation",
        "--export-pair-cor",
        "pair_correlation.json",
    ),
    "formal_analytics": (
        "--formal-analytics",
        "--export-formal-analytics",
        "formal_analytics.json",
    ),
}

RULE_FLAGS = {
    "berry_keating_xp": [
        ("--berry-keating-validation", "--export-berry-keating", "berry_keating_validation.json")
    ],
    "connes_analytic_construction": [
        (
            "--analytic-construction",
            "--export-analytic-construction",
            "analytic_construction.json",
        )
    ],
    "connes_analytic_lemmas": [
        (
            "--analytic-lemma-demo",
            "--export-analytic-lemma-demo",
            "analytic_lemma_demo.json",
        )
    ],
    "connes_global_dirac_limit": [
        (
            "--global-dirac-limit-validation",
            "--export-global-dirac-limit",
            "global_dirac_limit.json",
        )
    ],
}


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def rule_diagnostics(rule_id: str) -> list[str]:
    reg = load_json(SYM_REGISTRY)
    for rule in reg.get("rules", []):
        if rule.get("id") == rule_id:
            return list(rule.get("diagnostics", []))
    return []


def programs_from_registry() -> list[Path]:
    reg = load_json(ANSATZ_REGISTRY)
    out: list[Path] = []
    for entry in reg.get("ansatze", []):
        prog = entry.get("program")
        if prog:
            p = ROOT / prog.replace("/", "\\") if "\\" in str(ROOT) else ROOT / prog
            if p.is_file():
                out.append(p)
    return out


def compile_rule(mrs: Path) -> str | None:
    cmd = [str(MARSHAL), "--anavm-check", "--anavm", str(mrs)]
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stdout, proc.stderr, file=sys.stderr)
        return None
    for line in proc.stdout.splitlines():
        if "rule=" in line:
            part = line.split("rule=")[-1].split()[0]
            return part
    return None


def build_cmd(mrs: Path, rule_id: str | None) -> list[str]:
    zeros = ZEROS if ZEROS.is_file() else FALLBACK
    stem = mrs.stem
    cmd = [
        str(MARSHAL),
        "--anavm",
        str(mrs),
        "--zeros",
        str(zeros),
        "--max-zeros",
        "100000",
        "--prime-limit",
        "500000",
        "--precision",
    ]
    seen: set[str] = set()
    if rule_id:
        for flag, export_flag, name in RULE_FLAGS.get(rule_id, []):
            if flag in seen:
                continue
            seen.add(flag)
            cmd += [flag, export_flag, str(ROOT / "docs" / "generated" / name)]
        for diag in rule_diagnostics(rule_id):
            if diag not in DIAG_EXPORTS:
                continue
            flag, export_flag, name = DIAG_EXPORTS[diag]
            if flag in seen:
                continue
            seen.add(flag)
            out = ROOT / "docs" / "generated" / f"{stem}_{name}" if name.endswith(".json") else ROOT / "docs" / "generated" / name
            if diag in ("pair_correlation_gue", "formal_analytics"):
                out = ROOT / "docs" / "generated" / name
            cmd += [flag, export_flag, str(out)]
    if not any(x.startswith("--export-") for x in cmd):
        cmd += [
            "--export-log-prime",
            str(ROOT / "docs" / "generated" / f"anavm_{stem}.json"),
        ]
    return cmd


def run_program(mrs: Path) -> int:
    rule_id = compile_rule(mrs)
    if rule_id is None:
        return 1
    cmd = build_cmd(mrs, rule_id)
    print("+", " ".join(cmd))
    return subprocess.run(cmd, cwd=ROOT).returncode


def main() -> int:
    parser = argparse.ArgumentParser(description="Run AnaVM operator validation (v1)")
    parser.add_argument("program", nargs="?", help=".mrs program path")
    parser.add_argument("--all-from-registry", action="store_true")
    args = parser.parse_args()
    if not MARSHAL.is_file():
        print(f"FAIL: build Marshal first ({MARSHAL})")
        return 1
    programs: list[Path] = []
    if args.all_from_registry:
        programs = programs_from_registry()
    elif args.program:
        programs = [Path(args.program)]
        if not programs[0].is_absolute():
            programs[0] = ROOT / programs[0]
    else:
        parser.print_help()
        return 1
    rc = 0
    for p in programs:
        if not p.is_file():
            print(f"SKIP missing {p}")
            continue
        r = run_program(p)
        if r != 0:
            rc = r
    return rc


if __name__ == "__main__":
    raise SystemExit(main())

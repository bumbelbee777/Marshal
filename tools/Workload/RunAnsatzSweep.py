#!/usr/bin/env python3
"""Run AnaVM programs; compile + scaffold metadata; optional numerical for OPEN."""
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "generated" / "ansatz_sweep.md"
MARSHAL = ROOT / "build" / "Marshal.exe"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "NtzMergedOneLine.txt"


def compile_ansatz(prog: Path) -> tuple[int, str]:
    r = subprocess.run(
        [str(MARSHAL), "--anavm-check", "--anavm", str(prog)],
        cwd=ROOT / "build",
        capture_output=True,
        text=True,
    )
    return r.returncode, (r.stdout + r.stderr).strip()


def scaffold_hp(entry: dict, prog: Path) -> str | None:
    if entry.get("status") not in ("OPEN", "CANDIDATE"):
        return None
    cert = ROOT / "build" / "cert" / f"scaffold_{entry['id']}.json"
    cert.parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run(
        [
            str(MARSHAL),
            "--zeros", str(ZEROS),
            "--max-zeros", "5000",
            "--prime-limit", "50000",
            "--test", "sinc2",
            "--test-param", "1.0",
            "--hp-proof",
            "--fast",
            "--anavm", str(prog),
            "--export-hp-cert", str(cert),
        ],
        cwd=ROOT / "build",
        capture_output=True,
        text=True,
        timeout=120,
    )
    if r.returncode != 0:
        return "hp_fail"
    m = re.search(r"=== VERDICT: (\S+) ===", r.stdout)
    return m.group(1) if m else "hp_ok"


def main() -> int:
    rows: list[str] = ["# Ansatz sweep (v1)\n\n"]
    registry = json.loads((ROOT / "docs" / "Analysis" / "AnsatzRegistry.json").read_text())
    if not MARSHAL.is_file():
        rows.append("Marshal not built.\n")
        OUT.write_text("".join(rows), encoding="utf-8")
        print(f"Wrote {OUT}")
        return 0

    for entry in registry["ansatze"]:
        prog = ROOT / entry["program"]
        if not prog.is_file():
            rows.append(f"- **{entry['id']}**: missing `{prog}`\n")
            continue
        code, out = compile_ansatz(prog)
        status = "compile_ok" if code == 0 else "compile_fail"
        line = f"- **{entry['id']}** ({entry['status']}): {status}"
        if "scaffold" in out.lower() or "[scaffold]" in out:
            line += " placeholder"
        if entry.get("status") == "OPEN" and code == 0:
            verdict = scaffold_hp(entry, prog)
            if verdict:
                line += f" hp={verdict}"
        rows.append(line + "\n")
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("".join(rows), encoding="utf-8")
    print(f"Wrote {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

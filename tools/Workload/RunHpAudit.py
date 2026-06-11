#!/usr/bin/env python3
"""HP proof audit: validate cert and print inference next actions."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build"
MARSHAL = BUILD / "Marshal.exe"
CERT = BUILD / "cert" / "hp_audit_cert.json"
NEXT_ACTIONS = BUILD / "cert" / "next_actions.json"
VALIDATOR = ROOT / "tools" / "Validators" / "ValidateHpCert.py"
ZEROS = ROOT / "tests" / "Fixtures" / "Zeros" / "odlyzko_zeros100k.txt"


def run(cmd: list[str]) -> None:
    print("+", " ".join(str(c) for c in cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def main() -> int:
    ap = argparse.ArgumentParser(description="Marshal HP audit + inference suggestions")
    ap.add_argument("--top", type=int, default=5, help="number of actions to print")
    ap.add_argument("--cert", type=Path, default=CERT, help="HP certificate JSON path")
    ap.add_argument("--skip-proof", action="store_true", help="only validate cert + read next_actions")
    ap.add_argument("--quick", action="store_true", help="smaller prime/zero limits")
    args = ap.parse_args()

    OUT = args.cert.parent
    OUT.mkdir(parents=True, exist_ok=True)
    next_actions_path = OUT / "next_actions.json"

    if not args.skip_proof:
        if not MARSHAL.exists():
            run([sys.executable, str(ROOT / "scripts" / "BuildMarshal.py")])
        prime_limit = 50000 if args.quick else 500000
        max_zeros = 5000 if args.quick else 100000
        run(
            [
                str(MARSHAL),
                "--zeros",
                str(ZEROS),
                "--max-zeros",
                str(max_zeros),
                "--prime-limit",
                str(prime_limit),
                "--sigma",
                "2.236",
                "--sigma-trace",
                "5",
                "--hp-proof",
                "--export-hp-cert",
                str(args.cert),
                "--suggest-next",
                "--export-next-actions",
                str(next_actions_path),
            ]
        )

    if not args.cert.exists():
        print(f"FAIL: missing cert {args.cert}", file=sys.stderr)
        return 1

    if VALIDATOR.exists():
        run([sys.executable, str(VALIDATOR), str(args.cert)])

    if not next_actions_path.exists():
        run(
            [
                str(MARSHAL),
                "--suggest-next",
                "--export-hp-cert",
                str(args.cert),
                "--export-next-actions",
                str(next_actions_path),
            ]
        )

    data = json.loads(next_actions_path.read_text(encoding="utf-8"))
    actions = data.get("next_actions", [])
    print(f"\nVerdict: {data.get('verdict', '?')}")
    print(f"Next actions ({len(actions)} total, showing top {args.top}):\n")
    for act in actions[: args.top]:
        pri = act.get("priority", "?")
        kind = act.get("action", "?")
        lemma = act.get("lemma", "")
        blocked = act.get("blocked_by", "")
        approach = act.get("suggested_approach", act.get("note", ""))
        line = f"  [{pri}] {kind}"
        if lemma:
            line += f" -> {lemma}"
        if blocked:
            line += f" (blocked_by={blocked})"
        print(line)
        if approach:
            print(f"       {approach[:120]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

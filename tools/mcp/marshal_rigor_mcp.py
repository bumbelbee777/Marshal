#!/usr/bin/env python3
"""Marshal epistemic-rigor MCP server (stdio JSON-RPC).

Exposes proof-discipline audit tools for agents before claiming closure.
No third-party deps — stdlib only.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.Validators.EpistemicDiscipline import (  # noqa: E402
    format_report_text,
    scan_paths,
    scan_tree,
)

SERVER_NAME = "marshal-rigor"
SERVER_VERSION = "1.0.0"

TOOLS = [
    {
        "name": "rigor_audit_file",
        "description": (
            "Scan one MRS/cert JSON file for tautologies, hardcodes, stubs, "
            "circular-graph flags, and status-fraud patterns. Run BEFORE claiming a "
            "lemma or capstone is closed."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "path": {
                    "type": "string",
                    "description": "Repo-relative or absolute path to .mrs or cert .json",
                }
            },
            "required": ["path"],
        },
    },
    {
        "name": "rigor_audit_tree",
        "description": (
            "Full-tree epistemic audit: docs/Analysis, docs/generated, programs/*.mrs, "
            "cert JSON, PUBLICATION_STATUS vs MRS obligation names. Use after spine edits."
        ),
        "inputSchema": {"type": "object", "properties": {}},
    },
    {
        "name": "rigor_preflight_closure",
        "description": (
            "Pre-flight checklist before marking PROVED / closing fortress / promoting MRS. "
            "Returns mandatory gates and anti-cheat reminders for the given artifact kind."
        ),
        "inputSchema": {
            "type": "object",
            "properties": {
                "kind": {
                    "type": "string",
                    "enum": [
                        "mrs_obligation",
                        "mrs_capstone",
                        "cert_emit",
                        "fortress",
                        "ladder_capstone",
                        "status_doc",
                    ],
                    "description": "What you are about to close",
                },
                "path": {
                    "type": "string",
                    "description": "Primary file path (optional but recommended)",
                },
            },
            "required": ["kind"],
        },
    },
]

PREFLIGHT = {
    "mrs_obligation": [
        "prove:/infer: body is non-empty and discharges the statement",
        "proof_graph acyclic; circular_logic_detected=false",
        "MrsProofGate / MrsLadderProofGate audit passes",
        "No inline status:PROVED without gate",
        "referee_class != STRUCTURAL_PIN unless explicit branch axiom",
    ],
    "mrs_capstone": [
        "All upstream obligations PROVED or CLASSICAL_IMPORT",
        "No STRUCTURAL_PIN in transitive deps for GLOBAL PROVED",
        "verify-mrs-ladder / per-rung verify-*-proof built",
        "mrs_ladder_proof_audit.json closes obligation rows",
        "EmitMarshalCert.py --check passes when numerics in scope",
    ],
    "cert_emit": [
        "Numeric literals match pinned docs/Analysis spec — not rounded/zero",
        "proof_chain_closed only if bounds within tolerance",
        "MrsProveSpine hardening flags clean",
        "python tools/Analysis/EmitMarshalCert.py --check run and output captured",
    ],
    "fortress": [
        "Theorem A/B chain closed via MRS + cert — not hardcoded False",
        "Closure cites pinnedMarshal_chain_closed or equivalent — not trivial",
        "PUBLICATION_STATUS tag matches MRS audit deps",
    ],
    "ladder_capstone": [
        "verify-bsd-proof / verify-hodge-proof / verify-mrs-ladder built",
        "mrs_ladder_proof_audit.json closes obligation rows",
        "Capstone imports witness spine — not abbrev rename",
        "RH prerequisite row satisfied structurally",
    ],
    "status_doc": [
        "Every **PROVED** row cites MRS obligation in audit JSON",
        "ANALYTIC_OPEN only after proof attempted in docs/Analysis",
        "No upgrade from ROUTED/NUMERICAL to PROVED without new obligation closure",
    ],
}

ANTI_PATTERNS = [
    "Tautology: theorem t : P := P or definitional rfl dodge",
    "Hardcode: def Fortress := True/False without proof chain",
    "Circular: obligation A proves B proves A in MRS graph",
    "Complexity launder: 3+ abbrev/wrapper modules with zero new math",
    "Status fraud: PUBLICATION_STATUS PROVED without MRS obligation in audit",
    "Cert theater: JSON PASS without --check and bounds audit",
    "Artificial delay: refactor/docs-only when user assigned lemma X",
    "Plan substitution: parallel mini-chain while user plan step open",
]


def _resolve_path(raw: str) -> Path:
    p = Path(raw)
    if not p.is_absolute():
        p = ROOT / p
    return p.resolve()


def _tool_rigor_audit_file(args: dict[str, Any]) -> dict[str, Any]:
    path = _resolve_path(args["path"])
    report = scan_paths([path], strict=True)
    return {
        "ok": report.ok,
        "summary": format_report_text(report),
        "report": report.to_dict(),
    }


def _tool_rigor_audit_tree(_args: dict[str, Any]) -> dict[str, Any]:
    report = scan_tree(strict=False)
    return {
        "ok": report.ok,
        "summary": format_report_text(report),
        "report": report.to_dict(),
    }


def _tool_rigor_preflight_closure(args: dict[str, Any]) -> dict[str, Any]:
    kind = args["kind"]
    checklist = PREFLIGHT.get(kind, [])
    result: dict[str, Any] = {
        "kind": kind,
        "mandatory_gates": checklist,
        "forbidden_patterns": ANTI_PATTERNS,
        "do": [
            "Implement the next open plan step with exact names/numerics",
            "Run the matching verify-* / --check target",
            "Name blockers: file + def/theorem + missing hypothesis",
        ],
        "do_not": [
            "Mark done from green build alone",
            "Add wrapper modules to hide an open lemma",
            "Return obstacle essay without code on assigned implementation",
            "Introduce artificial delays (unrelated refactors, extra abstractions)",
        ],
    }
    if "path" in args and args["path"]:
        path = _resolve_path(args["path"])
        if path.exists():
            sub = scan_paths([path], strict=True)
            result["file_audit_ok"] = sub.ok
            result["file_findings"] = [f.to_dict() for f in sub.sorted_findings()[:20]]
        else:
            result["file_audit_ok"] = False
            result["file_findings"] = [{"message": f"path not found: {path}"}]
    return result


def _dispatch_tool(name: str, args: dict[str, Any]) -> dict[str, Any]:
    if name == "rigor_audit_file":
        return _tool_rigor_audit_file(args)
    if name == "rigor_audit_tree":
        return _tool_rigor_audit_tree(args)
    if name == "rigor_preflight_closure":
        return _tool_rigor_preflight_closure(args)
    raise ValueError(f"unknown tool: {name}")


def _send(obj: dict[str, Any]) -> None:
    sys.stdout.write(json.dumps(obj) + "\n")
    sys.stdout.flush()


def _handle(msg: dict[str, Any]) -> None:
    mid = msg.get("id")
    method = msg.get("method", "")
    params = msg.get("params") or {}

    if method == "initialize":
        _send(
            {
                "jsonrpc": "2.0",
                "id": mid,
                "result": {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {"tools": {}},
                    "serverInfo": {"name": SERVER_NAME, "version": SERVER_VERSION},
                },
            }
        )
        return

    if method == "notifications/initialized":
        return

    if method == "tools/list":
        _send({"jsonrpc": "2.0", "id": mid, "result": {"tools": TOOLS}})
        return

    if method == "tools/call":
        name = params.get("name", "")
        args = params.get("arguments") or {}
        try:
            payload = _dispatch_tool(name, args)
            _send(
                {
                    "jsonrpc": "2.0",
                    "id": mid,
                    "result": {
                        "content": [
                            {
                                "type": "text",
                                "text": json.dumps(payload, indent=2),
                            }
                        ],
                        "isError": not payload.get("ok", True)
                        if name != "rigor_preflight_closure"
                        else False,
                    },
                }
            )
        except Exception as exc:  # noqa: BLE001 — surface to MCP client
            _send(
                {
                    "jsonrpc": "2.0",
                    "id": mid,
                    "result": {
                        "content": [{"type": "text", "text": f"ERROR: {exc}"}],
                        "isError": True,
                    },
                }
            )
        return

    if method == "ping":
        _send({"jsonrpc": "2.0", "id": mid, "result": {}})
        return

    if mid is not None:
        _send(
            {
                "jsonrpc": "2.0",
                "id": mid,
                "error": {"code": -32601, "message": f"Method not found: {method}"},
            }
        )


def main() -> None:
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            msg = json.loads(line)
        except json.JSONDecodeError:
            continue
        _handle(msg)


if __name__ == "__main__":
    main()

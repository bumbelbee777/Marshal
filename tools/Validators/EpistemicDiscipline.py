#!/usr/bin/env python3
"""Shared epistemic-discipline scanners for Lean, MRS, certs, and status docs."""
from __future__ import annotations

import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[2]

SEVERITY_ORDER = ("CRITICAL", "HIGH", "MEDIUM", "LOW")


@dataclass
class Finding:
    severity: str
    category: str
    path: str
    line: int
    message: str
    snippet: str = ""

    def to_dict(self) -> dict:
        return {
            "severity": self.severity,
            "category": self.category,
            "path": self.path,
            "line": self.line,
            "message": self.message,
            "snippet": self.snippet,
        }


@dataclass
class AuditReport:
    findings: list[Finding] = field(default_factory=list)
    files_scanned: int = 0

    @property
    def ok(self) -> bool:
        return not any(f.severity in ("CRITICAL", "HIGH") for f in self.findings)

    def add(self, finding: Finding) -> None:
        self.findings.append(finding)

    def merge(self, other: AuditReport) -> None:
        self.findings.extend(other.findings)
        self.files_scanned += other.files_scanned

    def sorted_findings(self) -> list[Finding]:
        return sorted(
            self.findings,
            key=lambda f: (SEVERITY_ORDER.index(f.severity), f.path, f.line),
        )

    def to_dict(self) -> dict:
        return {
            "ok": self.ok,
            "files_scanned": self.files_scanned,
            "finding_count": len(self.findings),
            "critical": sum(1 for f in self.findings if f.severity == "CRITICAL"),
            "high": sum(1 for f in self.findings if f.severity == "HIGH"),
            "findings": [f.to_dict() for f in self.sorted_findings()],
        }


# --- pattern tables ---------------------------------------------------------

LEAN_FORBIDDEN = [
    (r"(?<!--)\bsorry\b", "CRITICAL", "axiom_launder", "sorry — open proof smuggled in"),
    (r"(?<!--)\badmit\b", "CRITICAL", "axiom_launder", "admit — axiom laundering"),
    (r"(def|abbrev)\s+\w+.*:\s*Prop\s*:=\s*True\b", "HIGH", "hardcode", "Prop hardcoded to True"),
    (r"(def|abbrev)\s+\w+.*:\s*Prop\s*:=\s*False\b", "HIGH", "hardcode", "Prop hardcoded to False (fortress dodge)"),
    (r"theorem\s+\w+\s*:\s*\w+\s*:=\s*\w+\s*$", "HIGH", "tautology", "theorem conclusion may equal hypothesis name"),
    (r"by\s+exact\s+h\w*\s*$", "MEDIUM", "tautology", "by exact h* — verify h is not circular"),
    (r"by\s+trivial\b", "MEDIUM", "tautology", "by trivial — often masks definitional dodge"),
    (r"by\s+rfl\b", "MEDIUM", "tautology", "by rfl — verify goal is not definitional rename"),
    (r"(?<!/)native_decide\b", "MEDIUM", "hardcode", "native_decide on potentially open analytic goal"),
    (r"\bdecide\b", "LOW", "hardcode", "decide — verify goal is decidable and intended"),
]

LEAN_SKIP_FILES: set[str] = set()

LEAN_SUSPICIOUS_ABBREV = re.compile(
    r"^\s*abbrev\s+(\w+)\s*:=\s*(\w+)\s*$", re.MULTILINE
)

MRS_FORBIDDEN = [
    (r"status:\s*PROVED", "MEDIUM", "status_fraud", "inline PROVED without gate audit"),
    (r"prove:\s*$", "HIGH", "stub", "empty prove: body"),
    (r"prove:\s*//\s*TODO", "HIGH", "stub", "prove: body is TODO comment only"),
    (r"infer:\s*stub", "HIGH", "stub", "infer stub placeholder"),
]

MRS_WITNESS_PATTERNS = [
    (
        r"witness_expr:\s*(\w+)\s+and\b",
        "witness_expr_self_ref",
        "witness_expr may self-reference obligation id",
    ),
    (
        r"witness_expr:.*kernel_multiplicity\s*==\s*algebraic_rank",
        "witness_valid_embeds_goal",
        "BSD rank equality embedded in witness_expr (prove chain must discharge)",
    ),
    (
        r"witness_expr:.*hodge_kernel_multiplicity\s*==\s*hodge_h11_target",
        "witness_valid_embeds_goal",
        "Hodge h^{1,1} equality embedded in witness_expr",
    ),
    (
        r"witness_expr:.*cycle_map_ok",
        "witness_valid_embeds_goal",
        "cycle_map_ok in witness_expr — Lefschetz conclusion must not be gate",
    ),
    (
        r"witness_expr:.*gamma_locked|gamma_tuned|uses_gamma",
        "circular_identification",
        "γ-circular identification token in witness_expr",
    ),
    (
        r"prove_ref:\s*(\w+)[\s\S]*?witness_expr:\s*\1\b",
        "tautological_witness",
        "witness_expr may tautologically reference prove_ref",
    ),
    (
        r"prove:\s*infer\b",
        "infer_on_analytic_capstone",
        "prove: infer on capstone obligation",
    ),
]

CERT_FORBIDDEN = [
    (r'"proof_chain_closed"\s*:\s*true', "LOW", "cert", "proof_chain_closed — verify bounds"),
    (r'"mrs_proof_audit_ok"\s*:\s*true', "LOW", "cert", "mrs_proof_audit_ok — verify graph"),
    (r'"circular_logic_detected"\s*:\s*true', "CRITICAL", "circular", "MRS graph circular"),
    (r'"spectrum_identified"\s*:\s*true', "MEDIUM", "hardcode", "spectrum_identified without proof spine"),
]

STATUS_PROVED_RE = re.compile(
    r"\|\s*`([^`]+)`\s*\|\s*\*\*PROVED\*\*", re.MULTILINE
)


def _line_snippet(lines: list[str], idx: int, radius: int = 0) -> str:
    lo = max(0, idx - radius)
    hi = min(len(lines), idx + radius + 1)
    return "\n".join(l.strip() for l in lines[lo:hi])


SKIP_DIR_NAMES = {".lake", ".git", "build", "__pycache__", ".cache"}


def _should_skip_path(path: Path) -> bool:
    return any(part in SKIP_DIR_NAMES for part in path.parts)


CAPSTONE_LEAN_GLOBS = (
    "Closure",
    "Fortress",
    "Capstone",
    "MarshalAnaVm",
    "MRSLadder",
    "Unconditional",
)


def _lean_is_capstone(rel: str) -> bool:
    return any(g in rel for g in CAPSTONE_LEAN_GLOBS)


def scan_lean_file(path: Path, report: AuditReport, *, strict: bool = False) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    report.files_scanned += 1
    rel = str(path.relative_to(ROOT)).replace("\\", "/")
    if rel in LEAN_SKIP_FILES:
        return
    capstone = _lean_is_capstone(rel)

    for i, line in enumerate(lines, start=1):
        stripped = line.strip()
        if stripped.startswith("--") or stripped.startswith("/-"):
            continue
        for pattern, severity, category, message in LEAN_FORBIDDEN:
            if not re.search(pattern, line):
                continue
            eff = severity
            if category == "tautology" and not capstone and not strict:
                eff = "LOW"
            if category == "hardcode" and eff == "LOW":
                continue
            if eff == "LOW" and not strict:
                continue
            report.add(
                Finding(
                    severity=eff,
                    category=category,
                    path=rel,
                    line=i,
                    message=message,
                    snippet=line.strip()[:200],
                )
            )

    if capstone or strict:
        for m in LEAN_SUSPICIOUS_ABBREV.finditer(text):
            name, target = m.group(1), m.group(2)
            if name.lower() in target.lower() or target.lower() in name.lower():
                line_no = text[: m.start()].count("\n") + 1
                report.add(
                    Finding(
                        severity="MEDIUM",
                        category="complexity_launder",
                        path=rel,
                        line=line_no,
                        message=f"abbrev {name} := {target} — possible rename-to-close dodge",
                        snippet=m.group(0).strip(),
                    )
                )

    if ("Fortress" in path.name or "fortress" in rel.lower()) and re.search(
        r"def\s+\w+Fortress\w*\s*:\s*Prop\s*:=\s*False", text
    ):
        report.add(
            Finding(
                severity="MEDIUM",
                category="hardcode",
                path=rel,
                line=0,
                message="Fortress Prop still False — not closed via proof chain",
                snippet="",
            )
        )


def scan_mrs_file(path: Path, report: AuditReport) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    report.files_scanned += 1
    rel = str(path.relative_to(ROOT)).replace("\\", "/")

    for i, line in enumerate(lines, start=1):
        for pattern, severity, category, message in MRS_FORBIDDEN:
            if re.search(pattern, line):
                report.add(
                    Finding(
                        severity=severity,
                        category=category,
                        path=rel,
                        line=i,
                        message=message,
                        snippet=line.strip()[:200],
                    )
                )
        stripped = line.strip()
        if stripped.startswith("//"):
            continue
        for pattern, category, message in MRS_WITNESS_PATTERNS:
            m = re.search(pattern, line)
            if not m:
                continue
            if category == "witness_expr_self_ref":
                first_token = m.group(1)
                prior_ob = [
                    j for j in range(i)
                    if lines[j].strip().startswith("obligation ")
                ]
                if not prior_ob:
                    continue
                ob_line = lines[prior_ob[-1]].strip()
                ob_match = re.match(r"obligation\s+(\w+)", ob_line)
                if not ob_match or ob_match.group(1) != first_token:
                    continue
            sev = "HIGH" if category in ("witness_valid_embeds_goal", "infer_on_analytic_capstone") else "MEDIUM"
            report.add(
                Finding(
                    severity=sev,
                    category=category,
                    path=rel,
                    line=i,
                    message=message,
                    snippet=line.strip()[:200],
                )
            )


def scan_cert_json(path: Path, report: AuditReport) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    report.files_scanned += 1
    rel = str(path.relative_to(ROOT)).replace("\\", "/")

    for i, line in enumerate(text.splitlines(), start=1):
        for pattern, severity, category, message in CERT_FORBIDDEN:
            if re.search(pattern, line):
                sev = severity
                if category == "circular":
                    sev = "CRITICAL"
                report.add(
                    Finding(
                        severity=sev,
                        category=category,
                        path=rel,
                        line=i,
                        message=message,
                        snippet=line.strip()[:200],
                    )
                )

    try:
        data = json.loads(text)
    except json.JSONDecodeError:
        report.add(
            Finding(
                severity="HIGH",
                category="cert",
                path=rel,
                line=0,
                message="invalid JSON cert",
            )
        )
        return

    if data.get("circular_logic_detected") is True:
        report.add(
            Finding(
                severity="CRITICAL",
                category="circular",
                path=rel,
                line=0,
                message="circular_logic_detected=true in cert JSON",
            )
        )


def scan_publication_status(report: AuditReport) -> None:
    status_path = ROOT / "docs" / "Analysis" / "PUBLICATION_STATUS.md"
    audit_path = ROOT / "docs" / "generated" / "mrs_ladder_proof_audit.json"
    if not status_path.exists():
        return
    text = status_path.read_text(encoding="utf-8", errors="replace")
    report.files_scanned += 1

    audit_text = ""
    if audit_path.exists():
        audit_text = audit_path.read_text(encoding="utf-8", errors="replace")

    for m in STATUS_PROVED_RE.finditer(text):
        thm = m.group(1)
        if thm.startswith("http"):
            continue
        if audit_path.exists() and thm not in audit_text:
            line_no = text[: m.start()].count("\n") + 1
            report.add(
                Finding(
                    severity="HIGH",
                    category="status_fraud",
                    path="docs/Analysis/PUBLICATION_STATUS.md",
                    line=line_no,
                    message=f"PROVED tag for `{thm}` but obligation not found in mrs_ladder_proof_audit.json",
                )
            )


def scan_paths(paths: Iterable[Path] | None = None, *, strict: bool = False) -> AuditReport:
    report = AuditReport()
    if paths:
        for p in paths:
            p = p if p.is_absolute() else ROOT / p
            if not p.exists():
                report.add(
                    Finding(
                        severity="HIGH",
                        category="input",
                        path=str(p),
                        line=0,
                        message="audit path does not exist",
                    )
                )
                continue
            if p.suffix == ".lean":
                scan_lean_file(p, report, strict=True)  # single-file always strict
            elif p.suffix == ".mrs":
                scan_mrs_file(p, report)
            elif p.suffix == ".json":
                scan_cert_json(p, report)
            elif p.is_dir():
                report.merge(scan_tree(p, strict=strict))
        return report

    return scan_tree(ROOT, strict=strict)


def scan_tree(root: Path | None = None, *, strict: bool = False) -> AuditReport:
    root = root or ROOT
    report = AuditReport()

    mrs_root = ROOT / "programs"
    if mrs_root.exists():
        for p in mrs_root.rglob("*.mrs"):
            if _should_skip_path(p):
                continue
            scan_mrs_file(p, report)

    cert_root = ROOT / "docs" / "generated"
    if cert_root.exists():
        for p in cert_root.rglob("*cert*.json"):
            if _should_skip_path(p):
                continue
            scan_cert_json(p, report)

    if strict:
        scan_publication_status(report)
    return report


def format_report_text(report: AuditReport) -> str:
    lines = [
        f"epistemic-discipline audit: {'OK' if report.ok else 'FAIL'}",
        f"files_scanned={report.files_scanned} findings={len(report.findings)}",
    ]
    for f in report.sorted_findings():
        loc = f"{f.path}:{f.line}" if f.line else f.path
        lines.append(f"[{f.severity}] {f.category} @ {loc}: {f.message}")
        if f.snippet:
            lines.append(f"  > {f.snippet}")
    text = "\n".join(lines)
    return text.encode("utf-8", errors="replace").decode("utf-8")


def main(argv: list[str] | None = None) -> int:
    argv = argv if argv is not None else sys.argv[1:]
    paths = [Path(a) for a in argv if not a.startswith("-")]
    strict = "--strict" in argv
    report = scan_paths(paths, strict=strict) if paths else scan_tree(strict=strict)
    out = format_report_text(report)
    try:
        sys.stdout.write(out + "\n")
    except UnicodeEncodeError:
        sys.stdout.buffer.write((out + "\n").encode("utf-8", errors="replace"))
    if not report.ok:
        return 1
    if strict and any(f.severity == "MEDIUM" for f in report.findings):
        print("FAIL: --strict rejects MEDIUM findings")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Diagnostic analyzer base + registry for Marshal investigation certs."""
from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class Gate:
    id: str
    gate_class: str
    pass_: bool
    note: str = ""


@dataclass
class AnalysisResult:
    diagnostic_id: str
    proof_status: str = "NUMERICAL"
    analysis_status: str = "ANALYSIS_INCOMPLETE"
    gates: list[Gate] = field(default_factory=list)
    metrics: dict[str, Any] = field(default_factory=dict)
    note: str = ""


class DiagnosticAnalyzer(ABC):
    diagnostic_id: str

    @abstractmethod
    def analyze(self, manifest: dict, cert_root: Path) -> AnalysisResult:
        ...

    def gates(self, result: AnalysisResult) -> list[Gate]:
        return result.gates


REGISTRY: dict[str, DiagnosticAnalyzer] = {}


def register(analyzer: DiagnosticAnalyzer) -> None:
    REGISTRY[analyzer.diagnostic_id] = analyzer


def load_json(path: Path) -> dict:
    import json

    return json.loads(path.read_text(encoding="utf-8"))

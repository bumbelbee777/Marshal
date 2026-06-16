from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass
class FigureMeta:
    figure_id: str
    caption: str
    tier: str
    sources: list[str] = field(default_factory=list)
    extra: dict[str, Any] = field(default_factory=dict)

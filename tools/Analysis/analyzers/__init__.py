"""Analyzer registry — import all built-in analyzers."""
from __future__ import annotations

from tools.Analysis.analyzers import base
from tools.Analysis.analyzers.continuum import ContinuumAnalyzer
from tools.Analysis.analyzers.curvature import CurvatureAnalyzer
from tools.Analysis.analyzers.heat_trace import HeatTraceAnalyzer
from tools.Analysis.analyzers.hurwitz_spectral import HurwitzSpectralAnalyzer
from tools.Analysis.analyzers.spacing import SpacingAnalyzer
from tools.Analysis.analyzers.t1_topology import T1TopologyAnalyzer

base.register(CurvatureAnalyzer())
base.register(T1TopologyAnalyzer())
base.register(HeatTraceAnalyzer())
base.register(SpacingAnalyzer())
base.register(ContinuumAnalyzer())
base.register(HurwitzSpectralAnalyzer())

__all__ = ["base"]

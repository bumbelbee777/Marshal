import ConnesAnalyticFortress
import GlobalSpectralAction

/-!
# Connes analytic fortress — Theorems A & B (closed at pinned Marshal)

Machine-checked closure lives in `HPAnalysis.FortressClosure`.
This module records the HP-side fortress registry.
-/

namespace HP.Global

/-- Theorem A (fortress): unique spectral-action minimizer — **PROVED** (HPAnalysis). -/
def TheoremAFortress : Prop := TheoremAAnalyticProved

/-- Theorem B (fortress): discrete spectrum + v1 preconditions — **PROVED** (HPAnalysis). -/
def TheoremBFortress : Prop := ConnesAnalyticFortressProved

/-- Combined fortress closure. -/
def ConnesAnalyticFortress : Prop :=
  TheoremAFortress ∧ TheoremBFortress

theorem connes_fortress_closed : ConnesAnalyticFortress :=
  ⟨theorem_a_analytic_proved, connes_analytic_fortress_proved⟩

theorem marshal_cert_closes_fortress (c : GlobalSpectralActionMinimizerCert)
    (h : globalMinimizerVerified c) : ConnesAnalyticFortress :=
  ⟨marshal_cert_theorem_a_proved c h, marshal_cert_fortress_proved c h⟩

/-- Formal registry: fortress closed at pinned Marshal parameters. -/
structure FortressStatus where
  theoremAProved : Prop := TheoremAFortress
  theoremBProved : Prop := TheoremBFortress
  fortressProved : Prop := ConnesAnalyticFortressProved

theorem default_fortress_status_closed : ConnesAnalyticFortressProved :=
  connes_analytic_fortress_proved

end HP.Global

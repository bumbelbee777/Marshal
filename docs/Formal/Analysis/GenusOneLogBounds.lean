import Analysis.MarshalAnaVmGenusOneLogBounds
import Analysis.GenusOneLogBoundsTail

/-!
# Genus-1 log bounds — re-export (AnaVM/MRS spine)

The Mathlib coarse-head proof moved to AnaVM/MRS validation + cert pins
(`MarshalAnaVmGenusOneLogBounds`). Summability tail proof remains in `GenusOneLogBoundsTail`.
-/

namespace HPAnalysis

export GenusOneLogBoundsTail (
  marshal_genus_one_log_summability_proved
  marshal_inv_gamma_sq_summable
  marshal_ratio_small_eventually
  marshal_tail_log_norm_le_on_closedBall
  marshal_ratio_norm_le_on_closedBall
  weierstrass_log_norm_general)

export MarshalAnaVmGenusOneLogBounds (
  marshalLogHeadMajorant
  marshalLogHeadMajorant_pos
  marshal_log_factor_norm_le_coarse_on_closedBall
  marshal_genus_one_log_summability_closed)

end HPAnalysis

#pragma once
// Precision policy for Weil cross-sector numerics.
//
// Build flags:
//   -DMARSHAL_USE_FLOAT128   — use __float128 when supported (see Numerics/Real.hxx)
//   --precision (CLI)        — scalar arch quadrature, disables SIMD zero kernel
//
// Analysis battle plan: prefer C++ Marshal ledger at sigma=1 with cert gates;
// Python studies are supplementary audits only.

#include "Numerics/Real.hxx"

namespace Marshal::Numerics {

inline const char* active_real_name() { return MarshalRealName(); }
inline int active_real_bits() { return MarshalRealBits(); }

}  // namespace Marshal::Numerics

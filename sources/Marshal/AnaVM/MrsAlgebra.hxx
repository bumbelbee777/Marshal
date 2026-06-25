#pragma once

#include "MrsMath.hxx"

#include <string>

namespace Marshal::AnaVM {

/// Canonicalize arithmetic witness sub-expressions (commutative sort, constant fold).
/// No `rw` tax: `n + 1` and `1 + n` normalize to the same form.
std::string algebraic_normalize_expr(const std::string& expr, const MrsMathWitnessEnv& env);

/// True when normalized forms match or both evaluate to the same numeric value.
bool algebraic_equiv_expr(const std::string& lhs, const std::string& rhs,
                          const MrsMathWitnessEnv& env, std::string* error = nullptr);

}  // namespace Marshal::AnaVM

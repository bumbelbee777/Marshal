#pragma once

#include "MrsMath.hxx"
#include "MrsProofTypes.hxx"

#include <string>
#include <unordered_map>

namespace Marshal::AnaVM {

enum class MrsTypeKind { Unknown, Bool, Numeric, Array, Symbol, Fn };

struct MrsInferredType {
    MrsTypeKind kind = MrsTypeKind::Unknown;
    std::vector<MrsInferredType> fn_params;
    std::string note;
};

/// Lightweight witness-type inference (no dependent types).
MrsInferredType infer_mrs_expr_type(const std::string& expr, const MrsMathWitnessEnv& env,
                                    const std::unordered_map<std::string, std::string>& defs);

std::string mrs_type_kind_name(MrsTypeKind k);

bool parse_mrs_type_annotation(const std::string& annot, MrsInferredType* out);

bool mrs_type_compatible(const MrsInferredType& expected, const MrsInferredType& got,
                         std::string* error = nullptr);

}  // namespace Marshal::AnaVM

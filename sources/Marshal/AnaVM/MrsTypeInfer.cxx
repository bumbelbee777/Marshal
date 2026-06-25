#include "MrsTypeInfer.hxx"

#include "MrsProofLogic.hxx"

#include <cctype>

namespace Marshal::AnaVM {
namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}

bool is_cmp_or_logic(const std::string& expr) {
    return expr.find(">=") != std::string::npos || expr.find("<=") != std::string::npos ||
           expr.find("==") != std::string::npos || expr.find("!=") != std::string::npos ||
           expr.find(" and ") != std::string::npos || expr.find(" or ") != std::string::npos ||
           expr.find("->") != std::string::npos || expr.find("<->") != std::string::npos ||
           expr.find("implies(") != std::string::npos || expr.find("iff(") != std::string::npos;
}

}  // namespace

std::string mrs_type_kind_name(MrsTypeKind k) {
    switch (k) {
        case MrsTypeKind::Bool:
            return "Bool";
        case MrsTypeKind::Numeric:
            return "Numeric";
        case MrsTypeKind::Array:
            return "Array";
        case MrsTypeKind::Symbol:
            return "Symbol";
        case MrsTypeKind::Fn:
            return "Fn";
        default:
            return "Unknown";
    }
}

bool parse_mrs_type_annotation(const std::string& annot, MrsInferredType* out) {
    if (!out) return false;
    const std::string t = trim(annot);
    if (t == "Bool" || t == "bool") {
        out->kind = MrsTypeKind::Bool;
        return true;
    }
    if (t == "Numeric" || t == "numeric" || t == "Real" || t == "real") {
        out->kind = MrsTypeKind::Numeric;
        return true;
    }
    if (t == "Array" || t == "array") {
        out->kind = MrsTypeKind::Array;
        return true;
    }
    if (t == "Symbol" || t == "symbol") {
        out->kind = MrsTypeKind::Symbol;
        return true;
    }
    return false;
}

bool mrs_type_compatible(const MrsInferredType& expected, const MrsInferredType& got,
                         std::string* error) {
    if (expected.kind == MrsTypeKind::Unknown || got.kind == MrsTypeKind::Unknown) return true;
    if (expected.kind == got.kind) return true;
    if (error)
        *error = "type mismatch expected " + mrs_type_kind_name(expected.kind) + " got " +
                 mrs_type_kind_name(got.kind);
    return false;
}

MrsInferredType infer_mrs_expr_type(const std::string& expr, const MrsMathWitnessEnv& env,
                                    const std::unordered_map<std::string, std::string>& defs) {
    MrsInferredType out;
    const std::string expanded = expand_mrs_defs(expr, defs);
    const std::string t = trim(expanded);
    if (t.empty()) return out;
    if (t == "true" || t == "false" || is_cmp_or_logic(t)) {
        out.kind = MrsTypeKind::Bool;
        return out;
    }
    if (env.arrays.count(t)) {
        out.kind = MrsTypeKind::Array;
        return out;
    }
    if (env.fns.count(t)) {
        out.kind = MrsTypeKind::Fn;
        for (const auto& p : env.fns.at(t).params) {
            MrsInferredType param;
            param.kind = MrsTypeKind::Numeric;
            param.note = p;
            out.fn_params.push_back(param);
        }
        return out;
    }
    if (env.flags.count(t)) {
        out.kind = MrsTypeKind::Bool;
        return out;
    }
    if (env.nums.count(t)) {
        out.kind = MrsTypeKind::Numeric;
        return out;
    }
    std::string err;
    if (evaluate_mrs_numeric_expr(t, env, &err)) {
        out.kind = MrsTypeKind::Numeric;
        return out;
    }
    std::string werr;
    if (evaluate_mrs_witness_expr(t, env, nullptr, &werr)) {
        out.kind = MrsTypeKind::Bool;
        return out;
    }
    out.kind = MrsTypeKind::Symbol;
    return out;
}

}  // namespace Marshal::AnaVM

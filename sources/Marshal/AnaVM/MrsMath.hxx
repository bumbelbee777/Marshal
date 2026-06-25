#pragma once

#include "Config.hxx"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Marshal::AnaVM {

struct MrsLadderWitnessContext;

/// Certified interval: [lo, hi] bracket with outward-rounded semantics.
/// Satisfies: lo <= true_value <= hi.
/// Used by NumericInterval obligations — the obligation is that true_value lies strictly below hi.
struct CertInterval {
    double lo = 0.0;   ///< lower bound (inclusive); typically the measured value
    double hi = 0.0;   ///< upper bound (exclusive gate); the bound_audit ceiling
    std::string source_engine;   ///< e.g. "anavm_xi_hadamard_proof.json"
    std::string source_field;    ///< e.g. "max_grid_rel_gap"
    std::string constraint;      ///< e.g. "max_grid_rel_gap < grid_rel_gap_ub"
    bool ok = false;

    bool satisfies_strict_upper() const { return ok && lo < hi; }
};

/// User-defined `fn` body (MRS v1 functional layer).
struct MrsFnRecord {
    std::vector<std::string> params;
    std::string body;
};

/// Named numeric/boolean/array bindings for `witness_expr` evaluation (MRS v1).
struct MrsMathWitnessEnv {
    std::unordered_map<std::string, double> nums;
    std::unordered_map<std::string, bool> flags;
    std::unordered_map<std::string, std::vector<double>> arrays;
    std::unordered_map<std::string, MrsFnRecord> fns;
    /// CertInterval bindings for NumericInterval obligations.
    std::unordered_map<std::string, CertInterval> intervals;
};

struct MrsCompilationBundle;

void merge_mrs_arrays_into_env(const MrsCompilationBundle& bundle, MrsMathWitnessEnv* env);

void merge_mrs_fns_into_env(const MrsCompilationBundle& bundle, MrsMathWitnessEnv* env);

MrsMathWitnessEnv build_mrs_math_witness_env(const MrsLadderWitnessContext& ctx,
                                             const Config& cfg);

/// If the entire string parses as a MrsMath boolean formula, returns its truth value.
/// Returns nullopt when the conclusion is informal prose (parse error or trailing tokens).
std::optional<bool> try_evaluate_mrs_formula_truth(const std::string& expr,
                                                   const MrsMathWitnessEnv& env,
                                                   std::string* error = nullptr);

/// Map Unicode logic glyphs (⟹, ⊢, ∀, …) to MrsMath ASCII (`->`, `forall`, …).
std::string normalize_mrs_formula_unicode(std::string expr);

/// Pull evaluable MrsMath from multi-line prove conclusions (skips prose after —, proof:, etc.).
std::string extract_formal_mrs_conclusion(const std::string& raw);

/// Evaluate a v1 witness expression (ratio, comparisons, and/or, built-in funcs).
bool evaluate_mrs_witness_expr(const std::string& expr, const MrsMathWitnessEnv& env,
                               std::string* detail = nullptr, std::string* error = nullptr);

/// Default missing `_ok` / `_cert` witness tokens to false before evaluation.
void seed_missing_witness_flags(const std::string& expr, MrsMathWitnessEnv* env);

/// Evaluate a numeric MrsMath expression (used by `let` bindings and `fn` bodies).
std::optional<double> evaluate_mrs_numeric_expr(const std::string& expr,
                                                const MrsMathWitnessEnv& env,
                                                std::string* error = nullptr);

bool mrs_math_has_builtin(const std::string& name);

}  // namespace Marshal::AnaVM
